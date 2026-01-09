// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "MSGC_PlayerRevive_Burst.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/30
 *
 * 플레이어 부활 시 발생하는 VFX
 */
UCLASS()
class MAGESQUAD_API UMSGC_PlayerRevive_Burst : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

public:
	UMSGC_PlayerRevive_Burst();

protected:
	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;

protected:
	// 시작 지점에 스폰할 나이아가라
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | VFX")
	TObjectPtr<class UNiagaraSystem> StartNiagara;

	// 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Custom | SFX")
	TObjectPtr<class USoundBase> StartSound;
};
