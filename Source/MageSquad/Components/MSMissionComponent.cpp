// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MSMissionComponent.h"
#include "DataStructs/MSGameMissionData.h"
#include "GameStates/MSGameState.h"
#include <System/MSMissionDataSubsystem.h>

// Sets default values for this component's properties
UMSMissionComponent::UMSMissionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UMSMissionComponent::BeginPlay()
{
	Super::BeginPlay();

    OwnerGameState = GetOwner<AMSGameState>();
	// ...
    if (!IsServer())
    {
        BindGameStateDelegates();
    }
}

void UMSMissionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsServer() && MissionScript)
    {
        MissionScript->Deinitialize();
        MissionScript = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

void UMSMissionComponent::FinishMission(bool bSuccess)
{
    // 타이머 해제
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MissionTimerHandle);
    }

    // 미션 스크립트 정리
    if (MissionScript)
    {
        MissionScript->Deinitialize();
        MissionScript = nullptr;
    }

    // GameState 알림
    if (OwnerGameState)
    {
        OwnerGameState->NotifyMissionFinished(bSuccess);
    }
}

void UMSMissionComponent::StartMission(const FMSMissionRow& MissionRow)
{
    if (!IsServer())
        return;

    check(MissionRow.ScriptClass);

    if (MissionScript)
    {
        // 기존 미션은 실패처리 하거나 단순 정리만 수행
        FinishMission(false);
    }

    MissionScript = NewObject<UMSMissionScript>(this,MissionRow.ScriptClass);
    MissionScript->Initialize(GetWorld());

    GetWorld()->GetTimerManager().SetTimer(MissionTimerHandle,this,&UMSMissionComponent::OnMissionTimeExpired,MissionRow.TimeLimit,false);
}

void UMSMissionComponent::UpdateMission()
{
    if (!IsServer() || !MissionScript || !OwnerGameState)
        return;

    const float Progress = MissionScript->GetProgress();

    OwnerGameState->SetMissionProgress(Progress);

    if (MissionScript->IsCompleted())
    {
        FinishMission(true);
    }
}

void UMSMissionComponent::AbortMission()
{
    if (!IsServer() || OwnerGameState)
        return;

    FinishMission(false);
}

void UMSMissionComponent::BindGameStateDelegates()
{
    if (!OwnerGameState)
        return;

    OwnerGameState->OnMissionChanged.AddUObject(
        this,
        &UMSMissionComponent::HandleMissionChanged
    );

    OwnerGameState->OnMissionProgressChanged.AddUObject(
        this,
        &UMSMissionComponent::HandleMissionProgressChanged
    );

    OwnerGameState->OnMissionFinished.AddUObject(
        this,
        &UMSMissionComponent::HandleMissionFinished
    );
}

void UMSMissionComponent::HandleMissionChanged(int32 MissionID)
{
    // Client:
    // - MissionID → DataSubsystem 조회
    // - UI 표시 / 연출
    UMSMissionDataSubsystem* MissionDataSubsystem = OwnerGameState->GetGameInstance()->GetSubsystem<UMSMissionDataSubsystem>();

    if (!MissionDataSubsystem) return;

    const FMSMissionRow* MissionData = MissionDataSubsystem->Find(MissionID);

    if (!MissionData) return;

    StartMission(*MissionData);
}

void UMSMissionComponent::HandleMissionProgressChanged(float Progress)
{
    // Client:
    // - ProgressBar 갱신

}

void UMSMissionComponent::HandleMissionFinished(bool bSuccess)
{
    // Client:
    // - 성공 / 실패 연출

}

void UMSMissionComponent::OnMissionTimeExpired()
{
    if (OwnerGameState)
    {
        OwnerGameState->NotifyMissionFinished(false);
    }
}

bool UMSMissionComponent::IsServer() const
{
    return GetOwner() && GetOwner()->HasAuthority();
}