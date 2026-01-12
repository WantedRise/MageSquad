// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSWaveCenterPivot.generated.h"

UCLASS()
class MAGESQUAD_API AMSWaveCenterPivot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMSWaveCenterPivot();

	// Yaw 회전만 담당
	void SetYawRotation(float InYaw);
};
