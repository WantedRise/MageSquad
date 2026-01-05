// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MSMissionComponent.h"
#include "DataStructs/MSGameMissionData.h"
#include "GameStates/MSGameState.h"
#include <System/MSMissionDataSubsystem.h>
#include "DataStructs/MSMissionProgressUIData.h"

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
    BindGameStateDelegates();
}

void UMSMissionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 미션 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MissionTimerHandle);
    }

    if (IsServer() && MissionScript)
    {
        MissionScript->Deinitialize();
        MissionScript = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

void UMSMissionComponent::RequestMissionScriptDestroy()
{
    if (IsServer() && MissionScript)
    {
        MissionScript->Deinitialize();
        MissionScript = nullptr;
    }
}

void UMSMissionComponent::FinishMission(bool bSuccess)
{
    // 미션 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MissionTimerHandle);
    }

    // 미션 스크립트 정리
    if (MissionScript)
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            this,
            &UMSMissionComponent::RequestMissionScriptDestroy
        );
    }

    // GameState에 종료 통보
    if (OwnerGameState)
    {
        OwnerGameState->NotifyMissionFinished(bSuccess);
    }
}

void UMSMissionComponent::StartMission(const FMSMissionRow& MissionRow)
{
    if (!IsServer())
        return;

    CurrentMissionData = MissionRow;

    if (MissionScript)
    {
        // 기존 미션은 실패처리 하거나 단순 정리만 수행
        FinishMission(false);
    }
    if (MissionRow.ScriptClass)
    {
        MissionScript = NewObject<UMSMissionScript>(this, MissionRow.ScriptClass);
        MissionScript->SetOwnerMissionComponent(this);
        MissionScript->Initialize(GetWorld());
    }

    const float ServerTime = OwnerGameState->GetServerTime();
    OwnerGameState->SetMissionEndTime(ServerTime + MissionRow.TimeLimit);

    GetWorld()->GetTimerManager().SetTimer(
        MissionTimerHandle,
        this,
        &UMSMissionComponent::OnMissionTimeExpired,
        MissionRow.TimeLimit,
        false
    );
}

void UMSMissionComponent::UpdateMission()
{
    if (!IsServer() || !MissionScript || !OwnerGameState)
        return;

    MissionScript->GetProgress(CurrentProgress);
    OwnerGameState->SetMissionProgress(CurrentProgress);

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

    if (IsServer())
    {
        OwnerGameState->OnMissionChanged.AddUObject(
            this,
            &UMSMissionComponent::HandleMissionChanged
        );
    }
    // Todo : HandleMissionProgressChanged OnMissionFinished 둘 다 안쓰이는 중
    OwnerGameState->OnMissionFinished.AddUObject(
        this,
        &UMSMissionComponent::OnMissionFinished
    );
}

void UMSMissionComponent::HandleMissionChanged(int32 MissionID)
{
    
    UMSMissionDataSubsystem* MissionDataSubsystem = OwnerGameState->GetGameInstance()->GetSubsystem<UMSMissionDataSubsystem>();

    if (!MissionDataSubsystem) return;

    const FMSMissionRow* MissionData = MissionDataSubsystem->Find(MissionID);

    if (!MissionData) return;

    StartMission(*MissionData);
}

void UMSMissionComponent::OnMissionFinished(int32 MissionID, bool bSuccess)
{
    // Client:
    // - 성공 / 실패 연출
    if (bSuccess)
    {

    }
}

void UMSMissionComponent::OnMissionTimeExpired()
{
    if (OwnerGameState)
    {
        UE_LOG(LogTemp, Log, TEXT("OnMissionTimeExpired"));
        FinishMission(false);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("OnMissionTimeExpired no OwnerGameState"));
    }
}

bool UMSMissionComponent::IsServer() const
{
    return GetOwner() && GetOwner()->HasAuthority();
}