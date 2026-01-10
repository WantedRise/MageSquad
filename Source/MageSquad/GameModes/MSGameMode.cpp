// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSGameMode.h"
#include "System/MSProjectilePoolSystem.h"
#include "Actors/Projectile/MSBaseProjectile.h"
#include "Enemy/MSNormalEnemy.h"
#include "Player/MSPlayerCharacter.h"
#include "System/MSEnemySpawnSubsystem.h"
#include "GameFlow/MSGameFlowBase.h"
#include "GameStates/MSGameState.h"
#include "Player/MSPlayerState.h"
#include "MageSquad.h"
#include "Utils/MSUtils.h"
#include "System/MSLevelManagerSubsystem.h"
#include "System/MSMissionDataSubsystem.h"
#include <Player/MSPlayerController.h>
#include <System/MSCharacterDataSubsystem.h>
#include "OnlineSubsystemTypes.h"
#include "DataAssets/Player/DA_CharacterData.h"
#include "Blueprint/UserWidget.h"

#include "EngineUtils.h"

AMSGameMode::AMSGameMode()
{
	bUseSeamlessTravel = true;

	// 로비 이동 타이머가 게임 멈춤에 영향을 받지 않도록, Tick 활성화
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	// Tick은 필요할 때만 활성화되도록 기본적으로 꺼둠
	SetActorTickEnabled(false);
}

void AMSGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 로비 이동 카운트다운 처리
	if (LobbyTravelCountdown > 0.f)
	{
		LobbyTravelCountdown -= DeltaTime;
		if (LobbyTravelCountdown <= 0.f)
		{
			// 카운트다운이 끝나면 로비로 이동
			TravelToLobby_Internal();

			// Tick을 다시 비활성화
			SetActorTickEnabled(false);
		}
	}
}

void AMSGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Tick 비활성화
	SetActorTickEnabled(false);

	// 공유 목숨이 0 + 팀 전멸 이벤트에 로비 이동 함수 바인딩
	GetGameState<AMSGameState>()->OnSharedLivesDepleted.AddLambda(
		[&]()
		{
			// 플레이어 전원 사망 플래그 설정
			bAllPlayersDead = true;

			// 로비로 트래블 실행
			ExecuteTravelToLobby();
		}
	);
}

void AMSGameMode::SetupGameFlow()
{
	check(GameFlowClass);
	check(MissionTimelineTable);

	AMSGameState* GS = GetGameState<AMSGameState>();
	check(GS);
	GS->OnMissionFinished.AddUObject(this, &AMSGameMode::OnMissionFinished);

	//GameFlow 생성 (UObject, 서버 전용)
	GameFlow = NewObject<UMSGameFlowBase>(this, GameFlowClass);
	check(GameFlow);
	//DataTable + GameState 주입
	GameFlow->Initialize(GS, MissionTimelineTable);
}

void AMSGameMode::OnMissionFinished(int32 MissionID, bool bSuccess)
{
	auto* Subsystem = GetGameInstance()->GetSubsystem<UMSMissionDataSubsystem>();
	if (const FMSMissionRow* MissionData = Subsystem ? Subsystem->Find(MissionID) : nullptr)
	{
		if (MissionData->MissionType != EMissionType::Boss)
		{
			return;
		}
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;

		TimerDelegate.BindUObject(this, &AMSGameMode::ExecuteTravelToLobby);
		GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 3.0f, false);
	}
}

void AMSGameMode::ExecuteTravelToLobby()
{
	if (!HasAuthority()) return;

	// 중복 호출 방지(이미 예약되었으면 무시)
	if (bTravelScheduled) return;
	bTravelScheduled = true;

	// 승/패 판단
	const bool bIsVictory = (bAllPlayersDead == false);

	// 모든 플레이어에게 승리/패배 위젯 표시
	ShowEndGameWidgetToAllPlayers(bIsVictory);

	// Tick에서 처리될 카운트다운 시작
	LobbyTravelCountdown = TravelDelaySeconds;

	// 카운트다운을 위해 Tick 활성화
	SetActorTickEnabled(true);
}

void AMSGameMode::ShowEndGameWidgetToAllPlayers(bool bIsVictory)
{
	// 표시할 위젯 클래스 선택
	const TSubclassOf<UUserWidget> WidgetClassToShow = bIsVictory ? GameVictoryWidgetClass : GameOverWidgetClass;
	if (!WidgetClassToShow) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 모든 플레이어 컨트롤러 순회
	for (TActorIterator<AMSPlayerController> It(World); It; ++It)
	{
		AMSPlayerController* PC = *It;
		if (!PC) continue;

		// 위젯 표시 RPC 호출
		const int32 EndGameWidgetZOrder = 500;
		PC->ClientRPCShowEndGameWidget(WidgetClassToShow, EndGameWidgetZOrder);
	}
}

void AMSGameMode::TravelToLobby_Internal()
{
	if (!HasAuthority()) return;

	if (auto* LevelManager = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
	{
		LevelManager->HostGameAndTravelToLobby();
	}
}


TSubclassOf<UMSGameFlowBase> AMSGameMode::GetGameFlowClass() const
{
	return GameFlowClass;
}


void AMSGameMode::TryStartGame()
{
	for (APlayerState* PS : GameState->PlayerArray)
	{
		AMSPlayerState* MSPS = Cast<AMSPlayerState>(PS);
		if (!MSPS)
		{
			MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSPlayerState == null"));
			return;
		}
		else if (!MSPS->IsUIReady())
		{
			MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("AMSPlayerState IsUIReady"));
			return;
		}
	}

	if (nullptr == GameFlow)
	{
		SetupGameFlow();

		if (GameFlow)
		{
			GameFlow->Start();

			if (UMSEnemySpawnSubsystem* SpawnSystem = UMSEnemySpawnSubsystem::Get(GetWorld()))
			{
				// 오브젝트 풀링 시키기
				SpawnSystem->InitializePool();

				// 설정
				SpawnSystem->SetSpawnInterval(5.0f);
				SpawnSystem->SetEliteSpawnInterval(60.0f);
				SpawnSystem->SetMaxActiveMonsters(25);
				SpawnSystem->SetSpawnCountPerTick(10);

				// 5초 뒤 스폰 시작
				SpawnSystem->StartSpawning();
			}
		}
		else
		{
			MS_LOG(LogMSNetwork, Log, TEXT("%s"), TEXT("GameFlow == null"));
		}
	}
}

void AMSGameMode::NotifyClientsShowLoadingWidget()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AMSPlayerController* PC = Cast<AMSPlayerController>(It->Get()))
		{
			// 각 클라이언트에게 로딩 위젯을 띄우라고 전송
			PC->ClientShowLoadingWidget();
		}
	}
}

void AMSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);


}

void AMSGameMode::RestartPlayer(AController* NewPlayer)
{
	AMSPlayerState* PS = NewPlayer->GetPlayerState<AMSPlayerState>();
	if (!PS)
	{
		Super::RestartPlayer(NewPlayer);
		return;
	}

	const FUniqueNetIdRepl NetId = PS->GetUniqueId();
	auto* CharacterDataManager = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
	if (!NetId.IsValid() || !CharacterDataManager || CharacterDataManager->GetAllCharacter().Num() <= 0)
	{
		Super::RestartPlayer(NewPlayer);
		return;
	}

	const FMSCharacterSelection* CharacterSelection = CharacterDataManager->FindSelectionByNetId(NetId);
	if (!CharacterSelection || !CharacterSelection->PlayerCharacterClass)
	{
		Super::RestartPlayer(NewPlayer);
		return;
	}
	FTransform SpawnTM = ChoosePlayerStart(NewPlayer)->GetActorTransform();

	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
		CharacterSelection->PlayerCharacterClass,
		SpawnTM
	);

	NewPlayer->Possess(NewPawn);
}