// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MSGameProgressComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnGameTimeReached);

/**
 * 작성자: 이상준
 * 작성일: 25/12/19
 *
 * 게임 진행 시간을 관리한다.
 * 이벤트 발생을 알린다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGESQUAD_API UMSGameProgressComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMSGameProgressComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
    void Initialize(float InTotalGameTime);
    void StartProgress();
    void StopProgress();

    float GetNormalizedProgress() const;

    // 특정 시간 도달 이벤트
    FOnGameTimeReached OnGameTimeReached;

protected:
    void TickProgress();

private:
    
    bool bRunning = false;

    // 다음 체크할 이벤트 시간
    TArray<float> TimeCheckpoints;
    int32 NextCheckpointIndex = 0;

    
    float ElapsedGameTime;
    float TotalGameTime;
    FTimerHandle ProgressTimerHandle;
    UPROPERTY()
    class AMSGameState* OwnerGameState = nullptr;
};
