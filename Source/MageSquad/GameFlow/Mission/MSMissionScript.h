// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MSMissionScript.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMissionScript : public UObject
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(class UWorld* World);
    virtual void OnMissionStart();
    virtual void Deinitialize();
    virtual void OnMissionEnd();
    virtual float GetProgress() const { return 0.f; }
    virtual bool IsCompleted() const { return GetProgress() >= 1.f; }
};
