// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_EnemyAbilityData.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UDA_EnemyAbilityData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TArray<TSubclassOf<class UGameplayAbility>> EnemyAbilities;
	
};
