// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "NiagaraSystem.h"
#include "MSGCN_Skill_Explosion.generated.h"

/**
*  작성자: 박세찬
 * 작성일: 25/12/16
 * 
 * Explosion 스킬 이펙트(나이아가라 시스템)를 재생하는 GameplayCue
 */
UCLASS()
class MAGESQUAD_API UMSGCN_Skill_Explosion : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	UMSGCN_Skill_Explosion();

	// ExecuteGameplayCue() 호출 시 실행되는 함수
	virtual bool OnExecute_Implementation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters
	) const override;

protected:
	// 폭발 나이아가라 시스템
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> ExplosionNiagara;

	// RawMagnitude가 없을 때 사용할 기본 스케일
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	float DefaultScale = 1.0f;
};
