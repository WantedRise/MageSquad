// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "MSGC_PlayerBlinkEnd.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/15
 *
 * 플레이어 점멸 종료 VFX/SFX 재생 게임플레이 큐
 */
UCLASS()
class MAGESQUAD_API UMSGC_PlayerBlinkEnd : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UMSGC_PlayerBlinkEnd();

protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	// 스폰 위치 구하기 함수
	FVector ResolveSpawnLocation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const;

	// 스폰 회전값 구하기 함수
	FRotator ResolveSpawnRotation(AActor* MyTarget) const;

protected:
	// 도착 지점에 스폰할 나이아가라
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | VFX")
	TObjectPtr<class UNiagaraSystem> EndNiagaraA;

	// 도착 지점에 스폰할 나이아가라
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | VFX")
	TObjectPtr<class UNiagaraSystem> EndNiagaraB;
};
