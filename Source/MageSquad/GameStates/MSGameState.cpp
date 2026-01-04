// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/MSGameState.h"
#include "GameModes/MSGameMode.h"
#include "GameFlow/MSGameFlowBase.h"
#include "MageSquad.h"
#include "Net/UnrealNetwork.h"
#include "Components/MSGameProgressComponent.h"
#include "Components/MSMissionComponent.h"

#include "GameFramework/PlayerState.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"

#include "Engine/DataTable.h"
#include "DataStructs/MSSharedExperienceData.h"

#include "GameplayTagsManager.h"
#include "MSFunctionLibrary.h"
#include "Player/MSPlayerController.h"
#include "Player/MSPlayerState.h"
#include "DataStructs/MSMissionProgressUIData.h"

AMSGameState::AMSGameState()
{
	MissionComponent = CreateDefaultSubobject<UMSMissionComponent>(TEXT("MissionComponent"));
	//GameProgress = CreateDefaultSubobject<UMSGameProgressComponent>(TEXT("GameProgress"));

	// GameState는 기본적으로 항상 리플리케이트되지만, 명시적으로 초기화
	bReplicates = true;
}

void AMSGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// 현재 플레이 인원 계산
		ActivePlayerCount = FMath::Clamp(ComputeActivePlayerCount_Server(), 1, 4);

		// 현재 레벨의 요구 경험치 재계산
		RecalculateRequiredXP_Server(false);

		InitializeSharedLives_Server(SharedLives);
	}
}
void AMSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSGameState, ProgressNormalized);
	DOREPLIFETIME(AMSGameState, CurrentMissionID);
	DOREPLIFETIME(AMSGameState, MissionProgress);
	DOREPLIFETIME(AMSGameState, bMissionSuccess);
	DOREPLIFETIME(AMSGameState, MissionFinishedCounter);

	DOREPLIFETIME(AMSGameState, SharedLevel);
	DOREPLIFETIME(AMSGameState, SharedCurrentXP);
	DOREPLIFETIME(AMSGameState, SharedXPRequired);
	DOREPLIFETIME(AMSGameState, ActivePlayerCount);
}

void AMSGameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	if (HasAuthority())
	{
		if (nullptr == GameProgress)
		{
			GameProgress = NewObject<UMSGameProgressComponent>(this);
			GameProgress->RegisterComponent();
		}
	}
}

void AMSGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (HasAuthority())
	{
		// 현재 플레이 인원 계산
		const int32 NewCount = FMath::Clamp(ComputeActivePlayerCount_Server(), 1, 4);

		// 새로 계산한 현재 플레이 인원이 현재 플레이 인원과 다를 경우
		if (NewCount != ActivePlayerCount)
		{
			// 현재 플레이 인원 초기화
			ActivePlayerCount = NewCount;

			// 인원 변화에 맞춰 요구 경험치 재계산 (옵션에 따라 퍼센트 유지)
			RecalculateRequiredXP_Server(bPreserveProgressPctOnPlayerCountChange);

			// 클라 UI용 브로드캐스트(서버도 리슨 서버면 즉시 UI 갱신 필요)
			BroadcastPlayerCountChanged();
		}
	}
}

void AMSGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	if (HasAuthority())
	{
		// 현재 플레이 인원 계산
		const int32 NewCount = FMath::Clamp(ComputeActivePlayerCount_Server(), 1, 4);

		// 새로 계산한 현재 플레이 인원이 현재 플레이 인원과 다를 경우
		if (NewCount != ActivePlayerCount)
		{
			// 현재 플레이 인원 초기화
			ActivePlayerCount = NewCount;

			// 인원 변화에 맞춰 요구 경험치 재계산 (옵션에 따라 퍼센트 유지)
			RecalculateRequiredXP_Server(bPreserveProgressPctOnPlayerCountChange);

			// 클라 UI용 브로드캐스트(서버도 리슨 서버면 즉시 UI 갱신 필요)
			BroadcastPlayerCountChanged();
		}
	}
}

void AMSGameState::SetCurrentMissionID(int32 NewMissionID)
{
	if (!HasAuthority())
		return;

	CurrentMissionID = NewMissionID;
	OnMissionChanged.Broadcast(CurrentMissionID);
}

void AMSGameState::SetMissionProgress(const FMSMissionProgressUIData& NewData)
{
	if (!HasAuthority())
		return;
	MissionProgress = NewData;

	MissionProgress.Normalized = FMath::Clamp(
		MissionProgress.Normalized, 0.f, 1.f
	);

	OnMissionProgressChanged.Broadcast(MissionProgress);
}

void AMSGameState::NotifyMissionFinished(bool bSuccess)
{
	if (!HasAuthority())
		return;
	bMissionSuccess = bSuccess;
	MissionFinishedCounter++;
	UE_LOG(LogMSNetwork, Log, TEXT("NotifyMissionFinished"));
	OnMissionFinished.Broadcast(CurrentMissionID, bMissionSuccess);
}

void AMSGameState::AddSharedExperience_Server(AActor* SourceActor, float BaseXP)
{
	if (!HasAuthority()) return;
	if (BaseXP <= 0.f) return;

	// 경험치 획득자의 개인 보정치를 반영하여 최종 획득 경험치 계산
	const float FinalXP = ApplyPersonalExperienceModifiers_Server(SourceActor, BaseXP);

	// 공유 경험치에 누적
	SharedCurrentXP += FinalXP;

	// 레벨업 처리
	ProcessLevelUps_Server();

	// 현재 레벨 내 공유 경험치 변동 콜백
	BroadcastExperienceChanged();
}

float AMSGameState::GetSharedXPPct() const
{
	// 현재 경험지 / 목표 경험치의 비율 계산 반환
	return (SharedXPRequired > 0.f) ? FMath::Clamp(SharedCurrentXP / SharedXPRequired, 0.f, 1.f) : 0.f;
}

void AMSGameState::EndSkillLevelUpPhase(bool bByTimeout)
{
	if (!HasAuthority() || !bSkillLevelUpPhaseActive)
		return;

	bSkillLevelUpPhaseActive = false;

	// Ticker 종료
	if (SkillLevelUpTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(SkillLevelUpTickerHandle);
		SkillLevelUpTickerHandle.Reset();
	}

	const int32 SessionId = CurrentSkillLevelUpSessionId;

	// 타임아웃이면: 미완료자 랜덤 선택 강제 적용
	if (bByTimeout)
	{
		for (APlayerState* PSBase : PlayerArray)
		{
			AMSPlayerState* PS = Cast<AMSPlayerState>(PSBase);
			if (!PS) continue;

			if (CompletedPlayers.Contains(PS))
				continue;

			PS->ApplyRandomSkillLevelUpChoice_Server();

			CompletedPlayers.Add(PS);
		}
	}

	// 전원 UI 닫기 (클라 Pause 해제 포함)
	for (APlayerState* PSBase : PlayerArray)
	{
		AMSPlayerState* PS = Cast<AMSPlayerState>(PSBase);
		if (!PS) continue;

		if (AController* C = PS->GetOwner<AController>())
		{
			if (AMSPlayerController* PC = Cast<AMSPlayerController>(C))
			{
				PC->Client_CloseSkillLevelUpChoices(SessionId);
			}
		}
	}
}

bool AMSGameState::AreAllPlayersCompleted() const
{
	int32 Valid = 0;
	for (APlayerState* PS : PlayerArray)
	{
		if (Cast<AMSPlayerState>(PS))
			++Valid;
	}
	return (Valid > 0) && (CompletedPlayers.Num() >= Valid);
}

bool AMSGameState::TickSkillLevelUpPhase(float DeltaTime)
{
	if (!HasAuthority() || !bSkillLevelUpPhaseActive)
		return false;

	if (AreAllPlayersCompleted())
	{
		EndSkillLevelUpPhase(false);
		return false;
	}

	if (FPlatformTime::Seconds() >= SkillLevelUpExpireAtRealTime)
	{
		EndSkillLevelUpPhase(true);
		return false;
	}

	return true;
}

void AMSGameState::OnRep_SharedLevel()
{
	// 클라이언트에 레벨 변경 이벤트 브로드캐스트
	OnSharedLevelUp.Broadcast(SharedLevel);
	BroadcastExperienceChanged();
}

void AMSGameState::OnRep_SharedExperience()
{
	// 현재 레벨 내 공유 경험치 변동 콜백 (현재 EXP or 요구 EXP)
	BroadcastExperienceChanged();
}

void AMSGameState::OnRep_ActivePlayerCount()
{
	// 인원 변경은 요구XP에 영향을 주므로 UI 갱신도 함께 트리거
	BroadcastPlayerCountChanged();
	BroadcastExperienceChanged();
}

void AMSGameState::StartSkillLevelUpPhase()
{
	if (!HasAuthority()) return;

	// 레벨업 세션 ID 증가 (중복 클릭 방지용)
	static int32 LevelUpSessionId = 0;
	LevelUpSessionId++;

	// 세션 상태 시작
	bSkillLevelUpPhaseActive = true;
	CurrentSkillLevelUpSessionId = LevelUpSessionId;
	CompletedPlayers.Reset();

	// 30초 리얼타임 마감
	SkillLevelUpExpireAtRealTime = FPlatformTime::Seconds() + 30.0;

	UE_LOG(LogTemp, Log,
		TEXT("[GameState] Start Skill LevelUp Phase. SessionId=%d"),
		LevelUpSessionId
	);

	// 모든 PlayerState에 레벨업 시작 알림
	for (APlayerState* PS : PlayerArray)
	{
		AMSPlayerState* MSPS = Cast<AMSPlayerState>(PS);
		if (!MSPS)
		{
			continue;
		}

		MSPS->BeginSkillLevelUp(LevelUpSessionId);
	}

	// Ticker 시작 (Pause 영향 없음)
	if (SkillLevelUpTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(SkillLevelUpTickerHandle);
		SkillLevelUpTickerHandle.Reset();
	}

	SkillLevelUpTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &AMSGameState::TickSkillLevelUpPhase),
		0.1f
	);
}

void AMSGameState::NotifySkillLevelUpCompleted(class AMSPlayerState* PS)
{
	if (!HasAuthority() || !bSkillLevelUpPhaseActive || !PS)
		return;

	CompletedPlayers.Add(PS);

	if (AreAllPlayersCompleted())
	{
		EndSkillLevelUpPhase(false);
	}
}

float AMSGameState::GetSkillLevelUpRemainingSeconds_Server() const
{
	if (!bSkillLevelUpPhaseActive) return 0.f;
	return (float)FMath::Max(0.0, SkillLevelUpExpireAtRealTime - FPlatformTime::Seconds());
}

int32 AMSGameState::ComputeActivePlayerCount_Server() const
{
	int32 Count = 0;

	// 플레이어 배열을 순회하며 현재 플레이 인원 계산
	for (APlayerState* PS : PlayerArray)
	{
		if (!PS) continue;

		// 관전자 제외는 선택
		//if (PS->IsOnlyASpectator()) continue;

		++Count;
	}

	return Count;
}

void AMSGameState::RecalculateRequiredXP_Server(bool bPreservePct)
{
	// 이전 요구 경험치 가져오기
	const float OldRequired = SharedXPRequired;

	// 이전 경험치 퍼센트 가져오기
	const float OldPct = (OldRequired > 0.f) ? FMath::Clamp(SharedCurrentXP / OldRequired, 0.f, 1.f) : 0.f;

	// 새 레벨별 요구 경험치 조회
	SharedXPRequired = GetRequiredXPForLevel_Server(SharedLevel, ActivePlayerCount);
	SharedXPRequired = FMath::Max(1.f, SharedXPRequired);

	// 현재 퍼센트 유지 옵션 활성화 시, 퍼센트 계산
	if (bPreservePct)
	{
		SharedCurrentXP = FMath::Clamp(OldPct * SharedXPRequired, 0.f, SharedXPRequired);
	}

	// 인원 감소 등으로 요구 경험치가 낮아졌다면 레벨업이 발생할 수도 있음
	ProcessLevelUps_Server();

	// 현재 레벨 내 공유 경험치 변동 콜백
	BroadcastExperienceChanged();
}

void AMSGameState::ProcessLevelUps_Server()
{
	// 요구 경험치가 0이면 무한 루프 방지
	if (SharedXPRequired <= 0.f)
	{
		SharedXPRequired = 1.f;
	}

	// 여러 레벨을 한 번에 넘길 수도 있으므로 while 처리
	while (SharedCurrentXP >= SharedXPRequired)
	{
		// 레벨별 요구 경험치만큼 누적 경험치 감소
		SharedCurrentXP -= SharedXPRequired;
		SharedLevel = FMath::Max(1, SharedLevel + 1);

		// 새 레벨의 요구 경험치 재계산
		SharedXPRequired = GetRequiredXPForLevel_Server(SharedLevel, ActivePlayerCount);
		SharedXPRequired = FMath::Max(1.f, SharedXPRequired);



		/* ======== 서버 측 레벨업 알림 지점 ======== */
		OnSharedLevelUp.Broadcast(SharedLevel);

		// 모든 클라이언트에게 레벨업 효과를 처리하도록 알림
		BroadcastSharedLevelUp_ServerOnly();

		// 스킬 레벨업 선택지 시작 호출
		StartSkillLevelUpPhase();
	}

	// XP/요구XP/레벨이 바뀌었으니 브로드캐스트
	BroadcastExperienceChanged();
}

void AMSGameState::BroadcastSharedLevelUp_ServerOnly()
{
	if (!HasAuthority()) return;

	// 레벨업 플로터 GameplayCue 태그 지정
	FGameplayTag Cue_LevelUpFloater;
	const UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	Cue_LevelUpFloater = TagsManager.RequestGameplayTag(FName("GameplayCue.UI.LevelUpFloater"), false);

	// 현재 플레이어들을 순회
	for (APlayerState* PS : PlayerArray)
	{
		if (!PS) continue;

		// MSPlayerState로 캐스팅
		AMSPlayerState* MSPS = Cast<AMSPlayerState>(PS);
		if (!MSPS) continue;

		// ASC 가져오기
		UAbilitySystemComponent* ASC = MSPS->GetAbilitySystemComponent();
		if (!ASC) continue;

		// 머리 위 UI를 띄울 대상 폰 가져오기
		// 관전중인 경우 없을 수도 있음
		APawn* Pawn = PS->GetPawn();

		// Cue파라미터로 Pawn의 위치 전달
		FGameplayCueParameters Params;
		Params.Location = Pawn->GetActorLocation();
		Params.Instigator = Pawn;

		// 원샷(Execute)로 브로드캐스트
		ASC->ExecuteGameplayCue(Cue_LevelUpFloater, Params);
	}
}

float AMSGameState::GetRequiredXPForLevel_Server(int32 Level, int32 PlayerCount) const
{
	Level = FMath::Max(1, Level);
	PlayerCount = FMath::Clamp(PlayerCount, 1, 4);

	// #1: 경험치 데이터 테이블 계산
	if (LevelExpTable)
	{
		// 해당 레벨의 인원별 경험치 반환
		const FName RowName(*FString::FromInt(Level));
		const FMSSharedLevelExpRow* Row = LevelExpTable->FindRow<FMSSharedLevelExpRow>(RowName, TEXT("GetRequiredXPForLevel_Server"));
		if (Row)
		{
			return Row->GetRequiredEXPByPlayerCount(PlayerCount);
		}
	}

	// #2: 만약에, 테이블이 없거나 해당 레벨의 Row가 없는 경우 (간단한 지수 성장 + 인원 스케일)
	const float Base = 100.f;
	const float Growth = 1.25f; // 레벨당 성장률(간단)
	float Required = Base * FMath::Pow(Growth, float(Level - 1));

	// 1P는 상대적으로 낮게, 4P는 높게(간단 선형)
	const float T = float(PlayerCount - 1) / 3.f; // 0~1
	const float Scale = FMath::Lerp(0.70f, 1.00f, T);
	return Required * Scale;
}

float AMSGameState::ApplyPersonalExperienceModifiers_Server(AActor* SourceActor, float BaseXP) const
{
	// 개인 보정치 서버 권한의 ASC/AttributeSet을 기반으로 계산함
	// 즉, OwnerOnly 복제여도 서버는 정확한 값을 알고 있으므로 안전

	// ASC 가져오기
	UMSPlayerAbilitySystemComponent* ASC = UMSFunctionLibrary::NativeGetPlayerAbilitySystemComponentFromActor(SourceActor);
	if (!ASC) return BaseXP;

	// AttributeSet 가져오기
	const UMSPlayerAttributeSet* AttrSet = ASC->GetSet<UMSPlayerAttributeSet>();
	if (!AttrSet) return BaseXP;

	// 기본 경험치 * 경험치 획득량 보정 비율로 최종 경험치 계산 (ex. 0.2 = 20% 추가 경험치)
	const float GainMod = AttrSet->GetExperienceGainMod();
	const float FinalXP = BaseXP * (1.f + GainMod);
	return FMath::Max(0.f, FinalXP);
}

void AMSGameState::BroadcastExperienceChanged()
{
	OnSharedExperienceChanged.Broadcast();
}

void AMSGameState::BroadcastPlayerCountChanged()
{
	OnActivePlayerCountChanged.Broadcast(ActivePlayerCount);
}

void AMSGameState::OnRep_ProgressNormalized()
{
	OnProgressUpdated.Broadcast(ProgressNormalized);
}

void AMSGameState::OnRep_CurrentMissionID()
{
	OnMissionChanged.Broadcast(CurrentMissionID);
}

void AMSGameState::OnRep_MissionProgress()
{
	OnMissionProgressChanged.Broadcast(MissionProgress);
}

void AMSGameState::OnRep_MissionFinished()
{
	OnMissionFinished.Broadcast(CurrentMissionID, bMissionSuccess);
}

void AMSGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 등록된 모든 함수 바인딩 해제
	OnSharedExperienceChanged.Clear();
	OnActivePlayerCountChanged.Clear();
	OnProgressUpdated.Clear();
	OnMissionChanged.Clear();
	OnMissionProgressChanged.Clear();
	OnMissionFinished.Clear();
	Super::EndPlay(EndPlayReason);
}

void AMSGameState::InitializeSharedLives_Server(int32 InLives)
{
	if (!HasAuthority()) return;

	// 공유 목숨 설정 + 공유 목숨 변경 이벤트 브로드캐스트
	SharedLives = FMath::Max(0, InLives);
	OnSharedLivesChanged.Broadcast(SharedLives);

	// 공유 목숨이 0보다 작게 설정되면 전멸 이벤트 브로드캐스트
	if (SharedLives < 0)
	{
		OnSharedLivesDepleted.Broadcast();
	}
}

void AMSGameState::ConsumeLife_Server()
{
	if (!HasAuthority()) return;

	// 공유 목숨이 남아있을 때만 감소
	if (SharedLives > 0)
	{
		// 공유 목숨을 감소시키고 공유 목숨 변경 이벤트 브로드캐스트
		SharedLives--;
		OnSharedLivesChanged.Broadcast(SharedLives);
	}
}

void AMSGameState::AddLives_Server(int32 Count)
{
	if (!HasAuthority()) return;

	// 공유 목숨 증가 + 공유 목숨 변경 이벤트 브로드캐스트
	SharedLives = FMath::Max(0, SharedLives + Count);
	OnSharedLivesChanged.Broadcast(SharedLives);
}

void AMSGameState::OnRep_SharedLives()
{
	// 공유 목숨 변경 이벤트 브로드캐스트
	OnSharedLivesChanged.Broadcast(SharedLives);
}
