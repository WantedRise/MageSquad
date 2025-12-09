// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "MSPlayerState.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어의 ASC, AttributeSet을 관리하는 PlayerState
 * 폰의 사망 및 리스폰 후에도 유지되며 모든 클라이언트에 복제된다.
 */
UCLASS()
class MAGESQUAD_API AMSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMSPlayerState();

	// PlayerState가 소유한 ASC를 반환하는 함수
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// PlayerState가 소유한 AttributeSet을 반환하는 함수
	class UMSPlayerAttributeSet* GetAttributeSet() const;

protected:
	// 플레이어의 ASC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMSPlayerAbilitySystemComponent> AbilitySystemComponent;

	// 플레이어의 능력치(스탯)
	UPROPERTY(Transient)
	TObjectPtr< class UMSPlayerAttributeSet> AttributeSet;
};
