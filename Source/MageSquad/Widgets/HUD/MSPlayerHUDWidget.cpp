// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSPlayerHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"

#include "Components/Player/MSHUDDataComponent.h"

#include "Widgets/HUD/MSTeamMemberWidget.h"

#include "Kismet/GameplayStatics.h"

#include "Player/MSPlayerCharacter.h"
#include "Player/MSPlayerController.h"
#include "Player/MSPlayerState.h"

#include "GameStates/MSGameState.h"

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
	UnbindSharedExperience();
	ClearTeamPollTimer();

	Super::NativeDestruct();
}

void UMSPlayerHUDWidget::InitializeHUD()
{
	// 이미 바인딩 되어 있으면 그대로 유지(재진입 안전)
	if (bBoundLocalASC && LocalASC.IsValid())
	{
		RefreshLocalHealthUI(CachedHealth, CachedMaxHealth);
		RefreshSharedExperienceUI();
		return;
	}

	// 바인딩 시도
	const bool bLocalHealthOk = TryBindLocalHealth();
	const bool bSharedExperienceOk = TryBindSharedExperience();

	if (bLocalHealthOk && bSharedExperienceOk)
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
	RefreshLocalHealthUI(CachedHealth, CachedMaxHealth);

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

bool UMSPlayerHUDWidget::TryBindSharedExperience()
{
	CachedGameState = nullptr;

	if (!GetWorld()) return false;

	AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>();
	if (!GS) return false;

	// 기존 바인딩 정리 후 새 GameState로 바인딩
	UnbindSharedExperience();

	// 게임 스테이트 초기화
	CachedGameState = GS;

	// 델리게이트 바인딩(핸들을 보관해 정확히 해제)
	SharedExpChangedHandle = CachedGameState->OnSharedExperienceChanged.AddUObject(
		this, &UMSPlayerHUDWidget::OnSharedExperienceChanged);

	SharedLevelUpHandle = CachedGameState->OnSharedLevelUp.AddUObject(
		this, &UMSPlayerHUDWidget::OnSharedLevelUp);

	return true;
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

void UMSPlayerHUDWidget::UnbindSharedExperience()
{
	// 바인딩한 델리게이트 언바인드
	if (CachedGameState)
	{
		if (SharedExpChangedHandle.IsValid())
		{
			CachedGameState->OnSharedExperienceChanged.Remove(SharedExpChangedHandle);
			SharedExpChangedHandle.Reset();
		}

		if (SharedLevelUpHandle.IsValid())
		{
			CachedGameState->OnSharedLevelUp.Remove(SharedLevelUpHandle);
			SharedLevelUpHandle.Reset();
		}
	}

	CachedGameState = nullptr;
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

void UMSPlayerHUDWidget::RefreshLocalHealthUI(const float Health, const float MaxHealth)
{
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
}

void UMSPlayerHUDWidget::OnLocalHealthChanged(const FOnAttributeChangeData& Data)
{
	// 현재 체력 갱신 후 UI 업데이트
	CachedHealth = Data.NewValue;
	RefreshLocalHealthUI(CachedHealth, CachedMaxHealth);
}

void UMSPlayerHUDWidget::OnLocalMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	// 최대 체력 갱신 후 UI 업데이트
	CachedMaxHealth = Data.NewValue;
	RefreshLocalHealthUI(CachedHealth, CachedMaxHealth);
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
	const bool bSharedExperienceOk = TryBindSharedExperience();

	if (bLocalHealthOk && bSharedExperienceOk)
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

	// GameState로부터 공유 레벨 및 경험치 모두 가져오기
	const int32 Level = CachedGameState->GetSharedLevel();
	const float Cur = CachedGameState->GetSharedCurrentXP();
	const float Req = CachedGameState->GetSharedXPRequired();
	const float Pct = CachedGameState->GetSharedXPPct();

	// 공유 경험치 바 갱신
	if (SharedExpBarWidget)
	{
		SharedExpBarWidget->SetPercent(Pct);
	}

	// 공유 레벨 텍스트 바 갱신
	if (SharedLevelTextWidget)
	{
		SharedLevelTextWidget->SetText(FText::AsNumber(Level));
	}
}
