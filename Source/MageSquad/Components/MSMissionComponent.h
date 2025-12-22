// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/GameMissionTypes.h"
#include "MSMissionComponent.generated.h"

struct FMSMissionRow;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGESQUAD_API UMSMissionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMSMissionComponent();
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public: /* ===== Server Only ===== */
	void StartMission(const FMSMissionRow& MissionRow);
	//
	void UpdateMission();
	// 강제 종료 (실패 등)
	void AbortMission();
private: /* ===== Client ===== */
	void BindGameStateDelegates();
	void HandleMissionChanged(int32 MissionID);
	void HandleMissionProgressChanged(float Progress);
	void HandleMissionFinished(bool bSuccess);
	void OnMissionTimeExpired();
	void FinishMission(bool bSuccess);
private:
	bool IsServer() const;
private:
	UPROPERTY()
	class UMSMissionScript* MissionScript = nullptr;
	EMissionType CurrentMissionType = EMissionType::None;
	float CurrentProgress = 0.f;
	class AMSGameState* OwnerGameState;
	FTimerHandle MissionTimerHandle;
};
