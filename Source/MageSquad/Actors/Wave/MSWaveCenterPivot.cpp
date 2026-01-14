// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Wave/MSWaveCenterPivot.h"

// Sets default values
AMSWaveCenterPivot::AMSWaveCenterPivot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);      // 회전 동기화하려면 필요
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AMSWaveCenterPivot::SetYawRotation(float InYaw)
{
	SetActorRotation(FRotator(0.f, InYaw, 0.f));
}
