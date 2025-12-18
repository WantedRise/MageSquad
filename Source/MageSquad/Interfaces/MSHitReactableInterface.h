// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MSHitReactableInterface.generated.h"

/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/18
 * 피격을 받아 이벤트가 발생하는 오브젝트가 상속받을 인터페이스(플레이어, 몬스터 등)
 */

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMSHitReactableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MAGESQUAD_API IMSHitReactableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HitReact")
	void OnHitByAttack(const FHitResult& HitResult, AActor* InInstigator);
};
