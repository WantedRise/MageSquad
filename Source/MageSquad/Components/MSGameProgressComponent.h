// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MSGameProgressComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameTimeReached,float /* ElapsedSeconds */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnProgressChanged, float /*Normalized*/);


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
    // 타이머, 위젯에 알림
    FOnProgressChanged OnProgressChanged;
protected:
    void TickProgress();

private:
    
    bool bRunning = false;

    // 다음 체크할 이벤트 시간
    TArray<float> TimeCheckpoints;
    int32 NextCheckpointIndex = 0;

    float TotalGameTime = 600.f;

    FTimerHandle ProgressTimerHandle;
    UPROPERTY()
    class AMSGameState* OwnerGameState = nullptr;
};
