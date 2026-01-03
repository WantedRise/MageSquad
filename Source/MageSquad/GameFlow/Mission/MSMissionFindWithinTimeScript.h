// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFlow/Mission/MSMissionScript.h"
#include "MSMissionFindWithinTimeScript.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMissionFindWithinTimeScript : public UMSMissionScript
{
	GENERATED_BODY()
	//MSGameMissionData
public:
	/** UMSMissionScript 인터페이스 */
	virtual void Initialize(UWorld* World) override;
	virtual void Deinitialize() override;
	virtual float GetProgress() const override;
	virtual bool IsCompleted() const override;
	/** 타겟 수집 알림 */
	void NotifyTargetFound();

protected:
	/** 타겟 생성 */
	void SpawnTargets(UWorld* World);
	TArray<int32> GetRandomIndicesLarge(int32 MaxIndex, int32 Count);
	void SetTargetInfo(TSubclassOf<AActor> InTargetClass, int32 InCount);
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Mission|Find")
	TSubclassOf<AActor> TargetActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Mission|Find")
	int32 RequiredFindCount = 5;

	int32 CurrentFindCount = 0;

	UPROPERTY()
	TArray<TWeakObjectPtr<class AMSMissionFindTargetActor>> SpawnedTargets;
};