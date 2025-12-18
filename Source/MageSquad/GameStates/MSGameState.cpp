// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/MSGameState.h"
#include "GameModes/MSGameMode.h"
#include "GameFlow/MSGameFlowBase.h"
#include "MageSquad.h"
#include "Net/UnrealNetwork.h"
#include "Components/MSGameProgressComponent.h"

void AMSGameState::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        if (AMSGameMode* GM = GetWorld()->GetAuthGameMode<AMSGameMode>())
        {
            TSubclassOf<class UMSGameFlowBase> GameFlowToSet = GM->GetGameFlowClass();
            if (GameFlowToSet)
            {
                GameFlow = NewObject<UMSGameFlowBase>(this, GameFlowToSet);
                GameFlow->Initialize(this);
            }
        }
        else
        {
            UE_LOG(LogMSNetwork, Warning, TEXT("Server: GameFlowClass is not set in GameMode"));
        }

        GameProgress = NewObject<UMSGameProgressComponent>(this);
        GameProgress->RegisterComponent();
        GameProgress->Initialize(600.f);
        //GameProgress->StartProgress();
    }
}
void AMSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMSGameState, ElapsedGameTime);
}
void AMSGameState::AddElapsedGameTime(float Delta)
{
    ElapsedGameTime += Delta;
}
void AMSGameState::OnRep_MissionIDs()
{

}
void AMSGameState::RequestSpawnFinalBoss()
{
}

bool AMSGameState::IsFinalBossDefeated() const
{
    return false;
}

// 서버에서 게임 시작을 트리거한다.
// 실제 진행(시간/미션)은 GameFlow가 담당한다.
void AMSGameState::StartGame()
{
    if (!GameFlow || GameFlow->GetCurrentState()!=EGameFlowState::None) return;

    ElapsedGameTime = 0.f;
    GameFlow->Start();
}

