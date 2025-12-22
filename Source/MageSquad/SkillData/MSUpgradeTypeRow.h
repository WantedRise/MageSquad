// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MSUpgradeTypeRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMSUpgradeTypeRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UpgradeID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString UpgradeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag UpgradeTag; // Upgrade.Damage 같은 키
};
	