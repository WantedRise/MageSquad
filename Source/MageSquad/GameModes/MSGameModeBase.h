// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MSGameModeBase.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 * 
 * 게임 데이터 관리 및 게임 플로우 관리
 */
UCLASS()
class MAGESQUAD_API AMSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Attack")
	TSubclassOf<class AMSBaseProjectile> ProjectileClass;
};
