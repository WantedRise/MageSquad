// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/Player/DA_PlayerStartUpData.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"

#include "Net/UnrealNetwork.h"

void UDA_PlayerStartUpData::GiveToAbilitySystemComponent(UMSPlayerAbilitySystemComponent* InASCToGive)
{
	check(InASCToGive);

	// 시작 시 부여 및 활성화(OnGive) 어빌리티 부여
	GrantAbilities(ActivateOnGiveAbilities, InASCToGive);

	// 시작 시 부여(OnTriggered) 어빌리티 부여
	GrantAbilities(ReactiveAbilities, InASCToGive);

	// 시작 시 적용 게임플레이 이펙트 적용
	if (!StartUpGameplayEffects.IsEmpty())
	{
		for (const TSubclassOf<class UGameplayEffect>& GameplayEffect : StartUpGameplayEffects)
		{
			if (GameplayEffect)
			{
				UGameplayEffect* EffectCDO = GameplayEffect->GetDefaultObject<UGameplayEffect>();
				InASCToGive->ApplyGameplayEffectToSelf(EffectCDO, 1, InASCToGive->MakeEffectContext());
			}
		}
	}
}

void UDA_PlayerStartUpData::GrantAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& InAbilities, UMSPlayerAbilitySystemComponent* InASCToGive)
{
	for (const TSubclassOf<UGameplayAbility>& Ability : InAbilities)
	{
		if (Ability)
		{
			FGameplayAbilitySpec AbilitySpec(Ability);
			AbilitySpec.SourceObject = this;
			AbilitySpec.Level = 1;

			InASCToGive->GiveAbility(AbilitySpec);
		}
	}
}
