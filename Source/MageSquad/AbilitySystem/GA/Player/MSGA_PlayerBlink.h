// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MSGA_PlayerBlink.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/15
 *
 * 플레이어의 이동 스킬
 * min(점멸 최대 거리, 마우스 커서) 위치로 무조건 이동 시도
 * 이동이 불가능하면 근처 위치로 이동
 */
UCLASS()
class MAGESQUAD_API UMSGA_PlayerBlink : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMSGA_PlayerBlink();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 점멸 수행 함수
	bool PerformBlink(ACharacter* Character, UAbilitySystemComponent* ASC);

	// 점멸 도착 위치 계산 함수
	FVector ComputeDesiredLocation(const ACharacter* Character) const;

	// 점멸 시도 후 근처까지 탐색하는 함수
	// 1) DesiredLocation으로 무조건 시도
	// 2) 불가하면 근처로 대체 위치 탐색
	bool ResolveFinalLocation(ACharacter* Character, const FVector& StartLocation, const FVector& DesiredLocation, FVector& OutFinalLocation) const;

	// 목표 지점 근처 탐색 함수
	bool FindNearbyValidLocation(ACharacter* Character, const FVector& StartLocation, const FVector& DesiredLocation, FVector& OutLocation) const;

	// 실제로 해당 위치로 이동 가능한지 검사하는 함수
	bool CanTeleportTo(ACharacter* Character, const FVector& Location, const FRotator& Rot) const;

	// GameplayCue 실행 함수 (서버에서 실행하면 ASC가 네트워크로 동기화)
	void ExecuteCue(UAbilitySystemComponent* ASC, const FGameplayTag& CueTag, const FVector& Location) const;

protected:
	// 점멸 최대 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	float BlinkDistance = 500.0f;

	// 충돌 판정 반경(근처 탐색에 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	float TeleportProbeRadius = 34.0f;

	// 목표 지점 근처 탐색(링 탐색) 스텝
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	float FallbackRingStep = 50.0f;

	// 목표 지점 근처 탐색(링 탐색) 최대 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	float FallbackMaxRadius = 300.0f;

	// 링에서의 각도 샘플 수(클수록 촘촘하지만 비용 증가)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Ability")
	int32 FallbackAngleSteps = 16;

	// 시작 위치 VFX (Gameplay Cue)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | VFX")
	FGameplayTag Cue_BlinkStart;

	// 최종 위치 VFX (Gameplay Cue)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | VFX")
	FGameplayTag Cue_BlinkEnd;
};
