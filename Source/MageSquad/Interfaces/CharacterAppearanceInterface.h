// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DataStructs/MSCharacterData.h"
#include "CharacterAppearanceInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCharacterAppearanceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MAGESQUAD_API ICharacterAppearanceInterface
{
	GENERATED_BODY()

public:
	virtual void ApplyCharacterAppearance(const FMSCharacterData& CharacterData) = 0;
};
