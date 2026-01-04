// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlow/Mission/MSMissionScript.h"
#include "Components/MSMissionComponent.h"
void UMSMissionScript::Initialize(UWorld* World)
{

}

void UMSMissionScript::Deinitialize()
{
}

void UMSMissionScript::GetProgress(FMSMissionProgressUIData& OutData) const
{

}

void UMSMissionScript::SetOwnerMissionComponent(UMSMissionComponent* InOwner)
{
	OwnerMissionComponent = InOwner;

}
