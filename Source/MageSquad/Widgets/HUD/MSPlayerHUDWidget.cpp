// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSPlayerHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "Components/CanvasPanel.h"

#include "Widgets/HUD/MSTeamMemberWidget.h"
#include "Widgets/HUD/MSSkillSlotWidget.h"

#include "Kismet/GameplayStatics.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Player/MSPlayerCharacter.h"
#include "Player/MSPlayerController.h"
#include "Player/MSPlayerState.h"

#include "GameStates/MSGameState.h"

#include "MSGameplayTags.h"
#include "MSFunctionLibrary.h"

void UMSPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// HUD 위젯 초기화
	InitializeHUD();
}

void UMSPlayerHUDWidget::NativeDestruct()
{
	// 바인딩된 델리게이트/타이머 정리
	ClearRebindTimer();
	UnbindLocalHealth();
	UnbindSharedData();
	ClearTeamPollTimer();

	Super::NativeDestruct();
}

void UMSPlayerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 스킬 쿨다운 진행률 계산 및 업데이트
	if (bBoundLocalSkills)
	{
		UpdateCooldowns(InDeltaTime);
	}
}

void UMSPlayerHUDWidget::InitializeHUD()
{
	// 이미 바인딩 되어 있으면 그대로 유지(재진입 안전)
	if (bBoundLocalASC && LocalASC.IsValid())
	{
		RefreshLocalHealthUI(CachedHealth, CachedMaxHealth, false);
		RefreshSharedExperienceUI();

		// 로컬 스킬 바인딩도 시도
		if (!bBoundLocalSkills)
		{
			TryBindLocalSkills();
		}

		return;
	}

	// 바인딩 시도
	const bool bLocalHealthOk = TryBindLocalHealth();
	const bool bSharedDataOk = TryBindSharedData();
	const bool bLocalSkillsOk = TryBindLocalSkills();

	// 보스 스폰 시 HUD를 감추기 위한 추가 바인딩, 델리게이트 구조로 인해 호출 구조를 맞추지 못함 - 임희섭
	TryBindGameState();

	if (bLocalHealthOk && bSharedDataOk && bLocalSkillsOk)
	{
		// 바인딩 재시도 타이머 종료
		ClearRebindTimer();

		// 팀 데이터 갱신 시작
		StartTeamPoll();

		return;;
	}

	// 실패하면 일정 주기로 재시도
	ScheduleRebind();
}

void UMSPlayerHUDWidget::RequestReinitialize()
{
	// 바인딩된 델리게이트/타이머 정리
	ClearRebindTimer();
	UnbindLocalHealth();

	// HUD 위젯 초기화 (재초기화)
	InitializeHUD();
}

void UMSPlayerHUDWidget::SetSpectateUI(bool bInSpectating, const FText& InTargetPlayerName)
{
	// 관전 상태 및 관전 대상 변경 여부 저장
	const bool bStateChanged = (bSpectateUIActive != bInSpectating);
	const bool bNameChanged = !SpectateTargetPlayerName.EqualTo(InTargetPlayerName);

	// 관전 상태 및 관전 대상 초기화
	bSpectateUIActive = bInSpectating;
	SpectateTargetPlayerName = InTargetPlayerName;

	// 관전 상태가 바뀌었으면 관전 상태 변경 이벤트 호출
	if (bStateChanged)
	{
		BP_OnSpectateStateChanged(bSpectateUIActive);
	}

	// 관전 대상이 변경되었으면 현재 관전 대상 플레이어 이름 텍스트 변경
	if (bNameChanged)
	{
		if (SpectateTargetNameTextWidget)
		{
			SpectateTargetNameTextWidget->SetText(SpectateTargetPlayerName);
		}
	}
}

bool UMSPlayerHUDWidget::TryBindLocalHealth()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return false;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return false;

	UAbilitySystemComponent* FoundASC = nullptr;

	// #1: Pawn이 ASC를 제공하는 경우
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
	{
		FoundASC = ASI->GetAbilitySystemComponent();
	}

	// #2: PlayerState에서 ASC 찾기
	if (!IsValid(FoundASC))
	{
		if (APlayerState* PS = PC->PlayerState)
		{
			if (AMSPlayerState* MSPS = Cast<AMSPlayerState>(PS))
			{
				FoundASC = MSPS->GetAbilitySystemComponent();
			}
		}
	}

	if (!IsValid(FoundASC)) return false;

	// ASC에서 아직 ActorInfo(Owner/Avatar) 초기화가 이뤄지지 않은 경우
	if (!FoundASC->GetOwnerActor() || !FoundASC->GetAvatarActor()) return false;

	// 기존 바인딩 정리 후 새 ASC로 바인딩
	UnbindLocalHealth();
	LocalASC = FoundASC;

	// 초기 값 반영
	CachedHealth = FoundASC->GetNumericAttribute(UMSPlayerAttributeSet::GetHealthAttribute());
	CachedMaxHealth = FoundASC->GetNumericAttribute(UMSPlayerAttributeSet::GetMaxHealthAttribute());
	RefreshLocalHealthUI(CachedHealth, CachedMaxHealth, false);

	// 델리게이트 바인딩(핸들을 보관해 정확히 해제)
	HealthChangedHandle = FoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UMSPlayerHUDWidget::OnLocalHealthChanged);

	MaxHealthChangedHandle = FoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UMSPlayerHUDWidget::OnLocalMaxHealthChanged);

	// 현재 바인딩 상태 설정 및 카운트 초기화
	bBoundLocalASC = true;
	RebindAttemptCount = 0;
	return true;
}

bool UMSPlayerHUDWidget::TryBindSharedData()
{
	CachedGameState = nullptr;

	if (!GetWorld()) return false;

	// GameState 가져오기
	AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>();
	if (!GS) return false;

	// 기존 바인딩 정리 후 새 GameState로 바인딩
	UnbindSharedData();

	// 게임 스테이트 초기화
	CachedGameState = GS;

	// 델리게이트 바인딩(핸들을 보관해 정확히 해제)
	// 공유 경험치 변경 바인딩
	SharedExpChangedHandle = CachedGameState->OnSharedExperienceChanged.AddUObject(
		this, &UMSPlayerHUDWidget::OnSharedExperienceChanged);

	// 공유 레벨 변경 바인딩
	SharedLevelUpHandle = CachedGameState->OnSharedLevelUp.AddUObject(
		this, &UMSPlayerHUDWidget::OnSharedLevelUp);

	// 공유 목숨 변경 바인딩
	SharedLivesChangedHandle = CachedGameState->OnSharedLivesChanged.AddUObject(
		this, &UMSPlayerHUDWidget::OnSharedLivesChanged);

	// 한 번 갱신
	RefreshSharedExperienceUI();

	return true;
}

bool UMSPlayerHUDWidget::TryBindLocalSkills()
{
	if (bBoundLocalSkills) return true;

	// 기본 전제로 PC, Pawn, ASC가 준비되어 있어야 함
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return false;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return false;

	AMSPlayerCharacter* Character = Cast<AMSPlayerCharacter>(Pawn);
	if (!Character) return false;

	if (!Character->IsLocallyControlled()) return false;

	// HUD 데이터 컴포넌트 찾기
	UMSHUDDataComponent* HUDData = Character->FindComponentByClass<UMSHUDDataComponent>();
	if (!HUDData) return false;

	// ASC가 유효하지 않으면 종료
	if (!LocalASC.IsValid()) return false;

	// ASC에서 아직 ActorInfo가 준비되지 않았으면 바인딩하지 않음
	if (!LocalASC->GetOwnerActor() || !LocalASC->GetAvatarActor()) return false;

	// 중복 바인딩 방지를 위해 기존 바인딩을 해제
	UnbindLocalSkills();

	// 로컬 플레이어 캐릭터 캐시 업데이트
	CachedLocalCharacter = Character;

	// 로컬 스킬 슬롯 데이터 초기화
	SkillSlotLocalDatas.Init(FMSHUDSkillSlotLocalData(), TotalSlots);

	// 현재 쿨타임 감소 비율 캐시
	CurrentCDR = LocalASC->GetNumericAttribute(UMSPlayerAttributeSet::GetCooldownReductionAttribute());

	// 델리게이트 바인딩(핸들을 보관해 정확히 해제)
	// 스킬 슬롯 배열 변경 바인딩
	SkillSlotsUpdatedHandle = Character->OnSkillSlotsUpdated.AddUObject(
		this, &UMSPlayerHUDWidget::HandleSkillSlotsUpdated);

	// 스킬 쿨다운 시작 바인딩
	SkillCooldownStartedHandle = Character->OnSkillCooldownStarted.AddUObject(
		this, &UMSPlayerHUDWidget::StartCooldownForSlot);

	// HUD 데이터 컴포넌트의 HUD에 표시하는 스킬 슬롯 데이터 업데이트 바인딩
	SkillSlotDataUpdatedHandle = HUDData->OnSkillSlotDataUpdated.AddUObject(
		this, &UMSPlayerHUDWidget::HandleSkillSlotDataUpdated);

	// 능력치 쿨타임 감소 속성 변경 바인딩
	CooldownReductionChangedHandle = LocalASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetCooldownReductionAttribute())
		.AddUObject(this, &UMSPlayerHUDWidget::OnLocalCooldownReductionChanged);

	// Blink 슬롯 위젯을 캐싱
	BlinkSkillSlotWidget = SlotBlinkWidget;

	// 블링크 스킬 쿨다운 시작 이벤트 바인딩
	// Blink 스킬은 일반 스킬 슬롯과 별도로 관리하므로 전용 콜백을 바인딩한다.
	BlinkSkillCooldownStartedHandle = Character->OnBlinkSkillCooldownStarted.AddUObject(
		this, &UMSPlayerHUDWidget::HandleBlinkSkillCooldownStarted);

	// 내부 배열을 초기화하기 위해 현재 슬롯 정보를 읽어 업데이트
	HandleSkillSlotsUpdated();

	// HUD에 표시하는 스킬 슬롯 데이터를 즉시 갱신하여 아이콘/레벨 UI 설정
	HandleSkillSlotDataUpdated();

	bBoundLocalSkills = true;

	return true;
}

void UMSPlayerHUDWidget::TryBindGameState()
{
	if (AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>())
	{
		// 성공적으로 찾았을 때 바인딩
		GS->OnBossSpawnCutsceneStateChanged.AddUObject(this, &ThisClass::SetHudVisibility);
	}
	else
	{
		// 아직 GameState가 NULL이면 다음 프레임에 다시 시도 (성공할 때까지)
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UMSPlayerHUDWidget::TryBindGameState);
	}
}

void UMSPlayerHUDWidget::StartTeamPoll()
{
	if (bTeamPolling) return;
	bTeamPolling = true;

	// 공유 데이터 갱신 시작
	GetWorld()->GetTimerManager().SetTimer(
		TeamPollTimer,
		this,
		&UMSPlayerHUDWidget::PollTeamMembers,
		TeamPollInterval,
		true
	);

	// 즉시 1회 갱신
	PollTeamMembers();
}

void UMSPlayerHUDWidget::UnbindLocalHealth()
{
	bBoundLocalASC = false;

	// 바인딩한 델리게이트 언바인드
	if (UAbilitySystemComponent* ASC = LocalASC.Get())
	{
		if (HealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
			HealthChangedHandle.Reset();
		}
		if (MaxHealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
			MaxHealthChangedHandle.Reset();
		}
	}

	LocalASC.Reset();
}

void UMSPlayerHUDWidget::UnbindSharedData()
{
	// 바인딩한 델리게이트 언바인드
	if (CachedGameState)
	{
		// 공유 경험치 변경 델리게이트 언바인딩
		if (SharedExpChangedHandle.IsValid())
		{
			CachedGameState->OnSharedExperienceChanged.Remove(SharedExpChangedHandle);
			SharedExpChangedHandle.Reset();
		}

		// 공유 레벨 변경 델리게이트 언바인딩
		if (SharedLevelUpHandle.IsValid())
		{
			CachedGameState->OnSharedLevelUp.Remove(SharedLevelUpHandle);
			SharedLevelUpHandle.Reset();
		}

		// 공유 목숨 변경 델리게이트 언바인딩
		if (SharedLivesChangedHandle.IsValid())
		{
			CachedGameState->OnSharedLivesChanged.Remove(SharedLivesChangedHandle);
			SharedLivesChangedHandle.Reset();
		}
	}

	CachedGameState = nullptr;
}

void UMSPlayerHUDWidget::UnbindLocalSkills()
{
	if (!bBoundLocalSkills) return;

	// 캐시된 캐릭터가 유효하면 델리게이트를 해제
	if (CachedLocalCharacter.IsValid())
	{
		AMSPlayerCharacter* Character = CachedLocalCharacter.Get();
		if (SkillSlotsUpdatedHandle.IsValid())
		{
			Character->OnSkillSlotsUpdated.Remove(SkillSlotsUpdatedHandle);
			SkillSlotsUpdatedHandle.Reset();
		}
		if (SkillCooldownStartedHandle.IsValid())
		{
			Character->OnSkillCooldownStarted.Remove(SkillCooldownStartedHandle);
			SkillCooldownStartedHandle.Reset();
		}
		if (BlinkSkillCooldownStartedHandle.IsValid())
		{
			Character->OnBlinkSkillCooldownStarted.Remove(BlinkSkillCooldownStartedHandle);
			BlinkSkillCooldownStartedHandle.Reset();
		}

		// HUD 데이터 컴포넌트에서 이벤트 해제
		UMSHUDDataComponent* HUDData = Character->FindComponentByClass<UMSHUDDataComponent>();
		if (HUDData)
		{
			if (SkillSlotDataUpdatedHandle.IsValid())
			{
				HUDData->OnSkillSlotDataUpdated.Remove(SkillSlotDataUpdatedHandle);
				SkillSlotDataUpdatedHandle.Reset();
			}
		}
	}

	// ASC에서 쿨타임 감소 델리게이트 바인딩 해제
	if (LocalASC.IsValid() && CooldownReductionChangedHandle.IsValid())
	{
		LocalASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetCooldownReductionAttribute())
			.Remove(CooldownReductionChangedHandle);
		CooldownReductionChangedHandle.Reset();
	}

	// 내부 스킬 슬롯 데이터 초기화
	SkillSlotLocalDatas.Empty();

	// 인덱스 목록 초기화 (다음 바인딩 시에 다시 채움)
	ActiveSlotIndices.Empty();
	PassiveSlotIndices.Empty();

	CachedLocalCharacter = nullptr;
	bBoundLocalSkills = false;
}

void UMSPlayerHUDWidget::ClearRebindTimer()
{
	// 바인딩 재시도 타이머 초기화
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RebindTimer);
	}
	RebindTimer.Invalidate();
	RebindAttemptCount = 0;
}

void UMSPlayerHUDWidget::ClearTeamPollTimer()
{
	// 공유 데이터 갱신 타이머 초기화
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TeamPollTimer);
	}
	TeamPollTimer.Invalidate();
}

void UMSPlayerHUDWidget::RefreshLocalHealthUI(const float Health, const float MaxHealth, bool InbTakeDamage)
{
	// 대미지 받았는지 여부
	const bool bTakeDamage = InbTakeDamage;

	// 현재 체력 비율 계산 (ex. 0.1 = 10%)
	const float Pct = (MaxHealth > 0.f) ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 0.f;

	// 체력 바 갱신
	if (LocalHealthBarWidget)
	{
		LocalHealthBarWidget->SetPercent(Pct);
	}

	// 체력 바 텍스트 갱신
	if (LocalHealthTextWidget)
	{
		// "현재 체력/최대 체력"으로 표시 (ex. "264/600") 
		const FText Cur = FText::AsNumber(FMath::RoundToInt(Health));
		const FText Max = FText::AsNumber(FMath::RoundToInt(MaxHealth));
		LocalHealthTextWidget->SetText(FText::Format(NSLOCTEXT("MSHUD", "LocalHealthFmt", "{0}/{1}"), Cur, Max));
	}

	// 대미지를 받은 경우, 대미지를 받았을 때 델리게이트 브로드캐스트
	if (bTakeDamage)
	{
		OnTakeDamaged.Broadcast();
	}
}

void UMSPlayerHUDWidget::OnLocalHealthChanged(const FOnAttributeChangeData& Data)
{
	// 피가 달았는지 회복되었는지 확인하는 플래그
	const bool bTakeDamage = CachedHealth > Data.NewValue;

	// 현재 체력 갱신 후 UI 업데이트
	CachedHealth = Data.NewValue;
	RefreshLocalHealthUI(CachedHealth, CachedMaxHealth, bTakeDamage);
}

void UMSPlayerHUDWidget::OnLocalMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	// 최대 체력 갱신 후 UI 업데이트
	CachedMaxHealth = Data.NewValue;
	RefreshLocalHealthUI(CachedHealth, CachedMaxHealth, false);
}

void UMSPlayerHUDWidget::PollTeamMembers()
{
	if (!TeamMemberWidgetClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 첫 번째 플레이어(자신) 컨트롤러와 그 소유자 가져오기
	APlayerController* PC = World->GetFirstPlayerController();
	APawn* LocalPawn = PC ? PC->GetPawn() : nullptr;

	// 월드에있는 모든 플레이어 캐릭터를 찾아 저장
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(World, AMSPlayerCharacter::StaticClass(), Found);

	// 이번 프레임에 존재하는 멤버 집합
	TSet<TWeakObjectPtr<AActor>> Alive;

	for (AActor* Actor : Found)
	{
		// 자신 제외
		if (!Actor || Actor == LocalPawn) continue;

		// 팀 멤버에 추가
		Alive.Add(Actor);

		// 플레이어 캐릭터 클래스로 캐스팅
		AMSPlayerCharacter* Member = Cast<AMSPlayerCharacter>(Actor);
		if (!Member) continue;

		// 해당 캐릭터의 HUD 공유 데이터를 가져옴
		UMSHUDDataComponent* HUDData = Member->FindComponentByClass<UMSHUDDataComponent>();
		if (!HUDData) continue;

		// 팀 멤버 위젯 생성
		EnsureTeamMemberWidget(Actor, HUDData);
	}

	// 사라진 멤버의 위젯 정리
	for (auto It = TeamMembers.CreateIterator(); It; ++It)
	{
		if (!Alive.Contains(It.Key()))
		{
			if (UMSTeamMemberWidget* Widget = It.Value())
			{
				Widget->RemoveFromParent();
			}
			It.RemoveCurrent();
		}
	}
}

void UMSPlayerHUDWidget::EnsureTeamMemberWidget(AActor* MemberActor, UMSHUDDataComponent* HUDData)
{
	if (!MemberActor || !HUDData) return;

	// 이전 프레임에 있던 멤버라면 위젯 재사용
	UMSTeamMemberWidget* Widget = nullptr;
	if (TObjectPtr<UMSTeamMemberWidget>* Found = TeamMembers.Find(MemberActor))
	{
		Widget = Found->Get();
	}

	// 새 멤버라면 위젯 새로 생성
	if (!Widget)
	{
		Widget = CreateWidget<UMSTeamMemberWidget>(GetWorld(), TeamMemberWidgetClass);
		if (!Widget) return;

		// 생성된 위젯을 VerticalBox에 추가
		TeamMembers.Add(MemberActor, Widget);
		TeamMembersBoxWidget->AddChild(Widget);
	}

	// 위젯 데이터 갱신
	Widget->UpdateFromHUDData(HUDData);
}

void UMSPlayerHUDWidget::ScheduleRebind()
{
	if (RebindTimer.IsValid()) return;

	// 바인딩 재시도 타이머 설정
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RebindTimer,
			this,
			&UMSPlayerHUDWidget::TickRebind,
			RebindRetryInterval,
			true
		);
	}
}

void UMSPlayerHUDWidget::TickRebind()
{
	// 최대 시도 제한이 있으면 초과 시 중단
	if (MaxRebindAttempts > 0 && RebindAttemptCount++ >= MaxRebindAttempts)
	{
		ClearRebindTimer();
		return;
	}

	// 바인딩 시도
	const bool bLocalHealthOk = TryBindLocalHealth();
	const bool bSharedDataOk = TryBindSharedData();
	const bool bLocalSkillsOk = TryBindLocalSkills();

	if (bLocalHealthOk && bSharedDataOk && bLocalSkillsOk)
	{
		// 바인딩 재시도 타이머 종료
		ClearRebindTimer();

		// 팀 데이터 갱신 시작
		StartTeamPoll();
	}
}

void UMSPlayerHUDWidget::OnSharedExperienceChanged()
{
	RefreshSharedExperienceUI();
}

void UMSPlayerHUDWidget::OnSharedLevelUp(int32 NewLevel)
{
	// HUD는 값만 갱신
	RefreshSharedExperienceUI();
}

void UMSPlayerHUDWidget::RefreshSharedExperienceUI()
{
	if (!CachedGameState) return;

	// GameState로부터 공유 데이터 모두 가져오기
	const int32 Level = CachedGameState->GetSharedLevel();
	const float Cur = CachedGameState->GetSharedCurrentXP();
	const float Req = CachedGameState->GetSharedXPRequired();
	const float Pct = CachedGameState->GetSharedXPPct();
	const int32 Lives = CachedGameState->GetSharedLives();

	// 공유 경험치 바 갱신
	if (SharedExpBarWidget)
	{
		SharedExpBarWidget->SetPercent(Pct);
	}

	// 공유 레벨 텍스트 갱신
	if (SharedLevelTextWidget)
	{
		SharedLevelTextWidget->SetText(FText::AsNumber(Level));
	}

	// 공유 목숨 텍스트 갱신
	if (SharedLivesTextWidget)
	{
		SharedLivesTextWidget->SetText(FText::AsNumber(Lives));
	}
}

void UMSPlayerHUDWidget::OnSharedLivesChanged(int32 NewLives)
{
	// 공유 목숨 텍스트 갱신
	if (SharedLivesTextWidget)
	{
		SharedLivesTextWidget->SetText(FText::AsNumber(NewLives));
	}
}

void UMSPlayerHUDWidget::SetHudVisibility(bool Result)
{
	UE_LOG(LogTemp, Log, TEXT("Hud Set Visibility : %s"), Result ? TEXT("Ture") : TEXT("False"));

	ESlateVisibility NewVisibility = ESlateVisibility::SelfHitTestInvisible;
	if (Result)
	{
		// 컷씬 상태 태그 부여
		UAbilitySystemComponent* AbilitySystemComponent = LocalASC.Get();
		AbilitySystemComponent->AddLooseGameplayTag(MSGameplayTags::Shared_State_CutScene);

		// 입력 모드 변경
		GetOwningPlayer()->SetInputMode(FInputModeUIOnly());

		NewVisibility = ESlateVisibility::Collapsed;
	}
	else
	{
		// 컷씬 상태 태그 제거
		UAbilitySystemComponent* AbilitySystemComponent = LocalASC.Get();
		AbilitySystemComponent->RemoveLooseGameplayTag(MSGameplayTags::Shared_State_CutScene);

		// 입력 모드 변경
		APlayerController* OwningPlayer = GetOwningPlayer();
		OwningPlayer->SetInputMode(FInputModeGameAndUI());

		// 컷씬이 끝날 때, 남아있는 입력 키를 초기화
		if (OwningPlayer && OwningPlayer->PlayerInput)
		{
			OwningPlayer->PlayerInput->FlushPressedKeys();
		}

		NewVisibility = ESlateVisibility::SelfHitTestInvisible;
	}

	// PC에 보스 컷씬 시작/종료 상태 설정
	if (AMSPlayerController* MSPC = Cast<AMSPlayerController>(GetOwningPlayer()))
	{
		MSPC->SetBossCutsceneActive(Result);
	}

	MainCanvas->SetVisibility(NewVisibility);

	if (CachedLocalCharacter.IsValid())
	{
		AMSPlayerController* MSPC = Cast<AMSPlayerController>(CachedLocalCharacter->GetController());
		if (MSPC)
		{
			// 관전 UI 레이아웃 설정(설정/해제)
			SetSpectateUI(CachedLocalCharacter->GetSpectating(), UMSFunctionLibrary::GetTargetPlayerNameText(MSPC->GetSpectateTargetActor()));
			
			// 사망 상태에 따른 관전 입력 적용
			MSPC->ApplyLocalInputState(CachedLocalCharacter->GetIsDead());
		}
	}
}

void UMSPlayerHUDWidget::UpdateCooldowns(float DeltaTime)
{
	if (!bBoundLocalSkills) return;

	// 현재 시간 계산
	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	// ============================================================
	// #1: 패시브 스킬의 남은 쿨타임 갱신 (타이머 기반)
	// ============================================================
	for (int32 Index : PassiveSlotIndices)
	{
		// 남은 쿨타임 비율
		float RemainingPercent = 0.f;

		// 쿨타임이 돌고있는지 확인
		if (SkillSlotLocalDatas.IsValidIndex(Index) && SkillSlotLocalDatas[Index].bSlotOnCooldown)
		{
			// 쿨타임 시작 시간, 지속 시간을 조회하여 경과된 시간을 구함
			const float StartTime = SkillSlotLocalDatas[Index].CooldownStartTime;
			const float Duration = SkillSlotLocalDatas[Index].CooldownDuration;
			const float Elapsed = CurrentTime - StartTime;

			if (Duration > UE_KINDA_SMALL_NUMBER)
			{
				// 0~1 비율 계산
				const float Fraction = FMath::Clamp(Elapsed / Duration, 0.f, 1.f);
				RemainingPercent = 1.f - Fraction;

				if (Fraction >= 1.f)
				{
					// 쿨다운 종료
					SkillSlotLocalDatas[Index].bSlotOnCooldown = false;
					RemainingPercent = 0.f;
				}
			}
			else
			{
				// 지속시간이 0이면 즉시 쿨다운 종료
				SkillSlotLocalDatas[Index].bSlotOnCooldown = false;
				RemainingPercent = 0.f;
			}
		}

		// 해당 스킬 슬롯 위젯의 머티리얼 파라미터 업데이트
		if (SkillSlotWidgets.IsValidIndex(Index) && SkillSlotWidgets[Index])
		{
			SkillSlotWidgets[Index]->SetCooldownPercent(RemainingPercent);
		}
	}


	// ============================================================
	// #2: 액티브 스킬의 남은 쿨타임 갱신 (GAS 쿨다운 기반)
	// ============================================================
	for (int32 Index : ActiveSlotIndices)
	{
		// 남은 쿨타임 비율
		float RemainingPercent = 0.f;

		// 쿨타임이 돌고있는지 확인
		if (SkillSlotLocalDatas.IsValidIndex(Index) && SkillSlotLocalDatas[Index].bSlotOnCooldown)
		{
			// 해당 스킬의 쿨다운 태그 가져오기
			const FGameplayTag& CooldownTag = SkillSlotLocalDatas[Index].SkillCooldownTags;
			if (CooldownTag.IsValid() && LocalASC.IsValid())
			{
				// 쿨다운 태그를 포함하는 게임플레이 효과의 시간 정보를 질의
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(CooldownTag);
				FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(TagContainer);
				TArray<TPair<float, float>> TimeRemainingAndDuration = LocalASC->GetActiveEffectsTimeRemainingAndDuration(Query);

				float MaxPct = 0.f;
				for (const TPair<float, float>& Pair : TimeRemainingAndDuration)
				{
					// 첫 번째 값은 남은 시간, 두 번째 값은 전체 지속시간
					const float Remaining = Pair.Key;
					const float Duration = Pair.Value;

					if (Duration > UE_KINDA_SMALL_NUMBER)
					{
						// 남은 시간 / 전체 지속시간으로 0~1 비율 계산
						const float Pct = FMath::Clamp(Remaining / Duration, 0.f, 1.f);
						if (Pct > MaxPct)
						{
							MaxPct = Pct;
						}
					}
				}
				RemainingPercent = MaxPct;
			}
		}

		// 해당 스킬 슬롯 위젯의 머티리얼 파라미터 업데이트
		if (SkillSlotWidgets.IsValidIndex(Index) && SkillSlotWidgets[Index])
		{
			SkillSlotWidgets[Index]->SetCooldownPercent(RemainingPercent);
		}
	}


	// ============================================================
	// #3: 블링크 스킬의 남은 쿨타임 갱신 (GAS 쿨다운 기반)
	// ============================================================
	if (BlinkSkillSlotWidget)
	{
		// 남은 쿨타임 비율
		float RemainingPercent = 0.f;

		// 블링크 스킬의 쿨다운 태그 가져오기
		const FGameplayTag& CooldownTag = MSGameplayTags::Player_Cooldown_Blink;
		if (CooldownTag.IsValid() && LocalASC.IsValid())
		{
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(CooldownTag);
			FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(TagContainer);
			TArray<TPair<float, float>> TimeRemainingAndDuration = LocalASC->GetActiveEffectsTimeRemainingAndDuration(Query);

			float MaxPct = 0.f;
			for (const TPair<float, float>& Pair : TimeRemainingAndDuration)
			{
				// Pair.Key = 남은 시간, Pair.Value = 전체 지속시간
				const float Remaining = Pair.Key;
				const float Duration = Pair.Value;
				if (Duration > UE_KINDA_SMALL_NUMBER)
				{
					const float Pct = FMath::Clamp(Remaining / Duration, 0.f, 1.f);
					if (Pct > MaxPct)
					{
						MaxPct = Pct;
					}
				}
			}
			RemainingPercent = MaxPct;
		}

		// 블링크 스킬 슬롯 위젯의 머티리얼 파라미터 업데이트
		BlinkSkillSlotWidget->SetCooldownPercent(RemainingPercent);
	}
}

void UMSPlayerHUDWidget::InitializeSkillBar()
{
	// 이미 슬롯 배열이 초기화 된 경우 종료
	if (SkillSlotWidgets.Num() == TotalSlots) return;

	// 슬롯 위젯 인스턴스 배열을 초기화하고 멤버를 순서대로 채움
	SkillSlotWidgets.Empty();
	SkillSlotWidgets.Reserve(TotalSlots);
	SkillSlotWidgets.Add(SlotActiveLeftWidget);
	SkillSlotWidgets.Add(SlotActiveRightWidget);
	SkillSlotWidgets.Add(SlotPassive01Widget);
	SkillSlotWidgets.Add(SlotPassive02Widget);
	SkillSlotWidgets.Add(SlotPassive03Widget);
	SkillSlotWidgets.Add(SlotPassive04Widget);

	// Blink 슬롯 위젯 할당
	BlinkSkillSlotWidget = SlotBlinkWidget;
}

void UMSPlayerHUDWidget::HandleSkillSlotDataUpdated()
{
	if (!CachedLocalCharacter.IsValid()) return;

	// HUD 공유 데이터 가져오기
	UMSHUDDataComponent* HUDData = CachedLocalCharacter->FindComponentByClass<UMSHUDDataComponent>();
	if (!HUDData) return;

	// HUD에 표시하는 스킬 슬롯 데이터의 복제된 스킬 슬롯 데이터를 가져옴
	const TArray<FMSHUDSkillSlotData>& ViewData = HUDData->GetSkillSlotData();
	const int32 NumSlots = ViewData.Num();

	// 슬롯 바가 초기화되지 않았다면 초기화 시도
	InitializeSkillBar();

	for (int32 i = 0; i < NumSlots; ++i)
	{
		if (!SkillSlotWidgets.IsValidIndex(i)) continue;

		// 스킬 슬롯 위젯 가져오기
		UMSSkillSlotWidget* SlotWidget = SkillSlotWidgets[i];
		if (!SlotWidget) continue;

		const FMSHUDSkillSlotData& Data = ViewData[i];
		if (!Data.bIsValid)
		{
			// 빈 슬롯: 아이콘과 레벨 숨김
			SlotWidget->SetEmpty();
		}
		else
		{
			// 유효한 스킬: 아이콘/레벨 바인딩
			SlotWidget->BindSkill(Data);
		}
	}
}

void UMSPlayerHUDWidget::HandleSkillSlotsUpdated()
{
	if (!CachedLocalCharacter.IsValid()) return;

	// 로컬 캐릭터의 스킬 슬롯 데이터 가져오기
	AMSPlayerCharacter* Character = CachedLocalCharacter.Get();
	const TArray<FMSPlayerSkillSlotNet>& Slots = Character->GetSkillSlots();

	// 배열 크기 보정
	if (SkillSlotLocalDatas.Num() != TotalSlots)
	{
		SkillSlotLocalDatas.Init(FMSHUDSkillSlotLocalData(), TotalSlots);
	}

	// 패시브/액티브 스킬 슬롯 정보 설정
	for (int32 i = 0; i < 6; ++i)
	{
		if (Slots.IsValidIndex(i) && Slots[i].IsValid())
		{
			SkillSlotLocalDatas[i].SkillTypes = static_cast<uint8>(Slots[i].SkillType);
			SkillSlotLocalDatas[i].BaseCoolTime = Slots[i].BaseCoolTime;

			// 쿨타임 설정
			if (Slots[i].SkillType == 1)
			{
				// 패시브 스킬은 타이머 기반으로 동작하므로 별도의 쿨다운 태그를 사용하지 않음
				SkillSlotLocalDatas[i].SkillCooldownTags = FGameplayTag();
			}
			else
			{
				// 액티브 스킬은 GAS 쿨다운 태그를 사용
				if (Slots[i].SkillCooldownTag.IsValid())
				{
					SkillSlotLocalDatas[i].SkillCooldownTags = Slots[i].SkillCooldownTag;
				}
			}
		}
		else
		{
			SkillSlotLocalDatas[i].SkillTypes = 0;
			SkillSlotLocalDatas[i].BaseCoolTime = 0.f;
			SkillSlotLocalDatas[i].SkillCooldownTags = FGameplayTag();
		}
	}

	// 블링크 스킬 정보 설정
	{
		BlinkSkillSlotData.bIsValid = true;
		BlinkSkillSlotData.Icon = BlinkSkillIcon;
		SlotBlinkWidget->BindSkill(BlinkSkillSlotData);
	}

	// 슬롯 타입별 인덱스 목록 갱신
	BuildSlotTypeIndices();

	// 패시브 슬롯의 쿨다운 지속시간 재계산
	RecalculatePassiveDurations();

	// HUD에 표시하는 스킬 슬롯 데이터를 즉시 갱신하여 아이콘/레벨 UI 설정
	HandleSkillSlotDataUpdated();
}

void UMSPlayerHUDWidget::OnLocalCooldownReductionChanged(const FOnAttributeChangeData& Data)
{
	// 쿨감 비율 변경 시 패시브 쿨다운 재계산
	const float NewCDR = FMath::Clamp(Data.NewValue, 0.f, 0.95f);

	// 변경 전과 동일하면 무시
	if (FMath::IsNearlyEqual(NewCDR, CurrentCDR)) return;
	CurrentCDR = NewCDR;

	// 패시브 슬롯의 쿨다운 지속시간 재계산
	RecalculatePassiveDurations();
}

void UMSPlayerHUDWidget::StartCooldownForSlot(uint8 SlotIndex, float Duration)
{
	// 스킬 쿨다운 시작
	const int32 Index = static_cast<int32>(SlotIndex);

	// 인덱스 유효성 검사
	if (!SkillSlotLocalDatas.IsValidIndex(Index)) return;

	// 쿨다운 플래그 설정
	SkillSlotLocalDatas[Index].bSlotOnCooldown = true;

	/*
	 * 패시브 및 액티브 스킬 쿨다운 정보를 초기화
	 * Duration은 서버에서 계산된 최종 쿨다운 지속시간이며,
	 * 0 이하의 값이 전달되는 경우에는 GAS 기반 업데이트에 의존
	 */
	if (Duration > 0.f)
	{
		const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		SkillSlotLocalDatas[Index].CooldownStartTime = CurrentTime;
		SkillSlotLocalDatas[Index].CooldownDuration = Duration;
	}
}

void UMSPlayerHUDWidget::HandleBlinkSkillCooldownStarted()
{
	// Blink 슬롯 위젯이 유효한 경우 쿨다운 퍼센트를 1로 설정하여 즉시 시각화
	if (BlinkSkillSlotWidget)
	{
		BlinkSkillSlotWidget->SetCooldownPercent(1.f);
	}
}

void UMSPlayerHUDWidget::BuildSlotTypeIndices()
{
	// 기존 스킬 목록 초기화
	ActiveSlotIndices.Empty();
	PassiveSlotIndices.Empty();

	// 로컬 스킬 슬롯 데이터를 기반으로 액티브/패시브 슬롯의 인덱스 목록을 갱신
	for (int32 i = 0; i < SkillSlotLocalDatas.Num(); ++i)
	{
		// 패시브 슬롯은 SkillTypes == 1, 그 외에는 액티브로 분류
		const uint8 Type = SkillSlotLocalDatas[i].SkillTypes;
		if (Type == 1)
		{
			PassiveSlotIndices.Add(i);
		}
		else
		{
			ActiveSlotIndices.Add(i);
		}
	}
}

void UMSPlayerHUDWidget::RecalculatePassiveDurations()
{
	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	// 패시브 슬롯만 처리
	for (int32 Index : PassiveSlotIndices)
	{
		if (!SkillSlotLocalDatas.IsValidIndex(Index)) continue;

		// 해당 패시브 스킬의 스킬 슬롯 데이터 가져오기
		FMSHUDSkillSlotLocalData& SlotData = SkillSlotLocalDatas[Index];

		// 현재 캐시된 CDR 값을 적용하여 패시브 슬롯의 쿨다운 지속시간을 재계산
		const float NewDuration = FMath::Max(0.05f, SlotData.BaseCoolTime * (1.f - CurrentCDR));

		if (SlotData.bSlotOnCooldown)
		{
			const float Elapsed = CurrentTime - SlotData.CooldownStartTime;
			const float OldDuration = SlotData.CooldownDuration;
			float PercentElapsed = 0.f;

			if (OldDuration > UE_KINDA_SMALL_NUMBER)
			{
				PercentElapsed = FMath::Clamp(Elapsed / OldDuration, 0.f, 1.f);
			}

			// 진행중인 쿨다운은 진행률을 유지하도록 StartTime을 보정
			SlotData.CooldownDuration = NewDuration;
			const float NewElapsed = PercentElapsed * NewDuration;
			SlotData.CooldownStartTime = CurrentTime - NewElapsed;
		}
		else
		{
			// 쿨다운 지속시간이 0인 경우라도 기본 쿨타임을 반영한 값으로 초기화
			SlotData.CooldownDuration = NewDuration;
		}
	}
}
