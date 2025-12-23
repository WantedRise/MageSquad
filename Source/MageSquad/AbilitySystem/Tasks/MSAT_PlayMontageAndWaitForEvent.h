// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "MSAT_PlayMontageAndWaitForEvent.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/23
 * 몽타주 재생 및 이벤트를 받기 위한 Ability Task
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMSMontageEventDelegate, 
	FGameplayTag, EventTag, 
	FGameplayEventData, EventData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMSMontageInterrupDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMSMontageEndDelegate);

UCLASS()
class MAGESQUAD_API UMSAT_PlayMontageAndWaitForEvent : public UAbilityTask
{
	GENERATED_BODY()
public:
	UMSAT_PlayMontageAndWaitForEvent();
	
public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UMSAT_PlayMontageAndWaitForEvent* CreateTask(
		UGameplayAbility* OwningAbility,
		UAnimMontage* Montage,
		FGameplayTagContainer EventTags,
		float Rate = 1.0f);

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
private:
	void OnMontageEnded(UAnimMontage* EndedMontage, bool bInterrupted);
	void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) const;
	
public:
	UPROPERTY(BlueprintAssignable)
	FMSMontageEndDelegate OnCompleted;

	UPROPERTY(BlueprintAssignable)
	FMSMontageInterrupDelegate OnInterrupted;

	UPROPERTY(BlueprintAssignable)
	FMSMontageEventDelegate OnEventReceived;
	
private:
	UPROPERTY()
	TObjectPtr<UAnimMontage> Montage;

	FGameplayTagContainer EventTags;
	float Rate;
	FDelegateHandle EventHandle;
};
