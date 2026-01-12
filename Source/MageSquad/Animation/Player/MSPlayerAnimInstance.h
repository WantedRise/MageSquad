// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MSPlayerAnimInstance.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 애님 인스턴스 클래스
 * 플레이어 애니메이셔 블루프린트에 사용할 데이터
 */
UCLASS()
class MAGESQUAD_API UMSPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 소유자가 특정 태그를 소유중인지 확인하는 헬퍼 함수
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool DoesOwnerHaveTag(const FGameplayTag TagToCheck) const;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom | Reference")
	TObjectPtr<class AMSPlayerCharacter> OwningPlayer;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom | Reference")
	TObjectPtr<class UCharacterMovementComponent> MovementComponent;

	// 이동 속도 (2D)
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom | MovementData")
	FVector Velocity;

	// 이동 속도 (1D)
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom | MovementData")
	float GroundSpeed;

	// 이동 방향
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom | MovementData")
	float Direction;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom | MovementData")
	uint8 bShoudMove : 1;
};
