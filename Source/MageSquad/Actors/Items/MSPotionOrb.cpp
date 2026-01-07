// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Items/MSPotionOrb.h"

#include "AbilitySystemComponent.h"
#include "MSFunctionLibrary.h"

AMSPotionOrb::AMSPotionOrb()
{
}

void AMSPotionOrb::Collect_Server(AActor* CollectorActor)
{
	// 서버 및 중복 획득 여부 체크를 위해 현재 상태를 저장
	const bool bWasCollected = bCollected;

	// 공통 파이프라인 수행
	Super::Collect_Server(CollectorActor);

	// 이미 획득한 경우 더 이상 처리하지 않음
	if (!HasAuthority() || bWasCollected) return;

	// GE가 지정되지 않았거나 획득자가 유효하지 않으면 종료
	if (!PotionGameplayEffect || !CollectorActor) return;

	// ASC를 가져오기
	UAbilitySystemComponent* ASC = UMSFunctionLibrary::NativeGetAbilitySystemComponentFromActor(CollectorActor);
	if (!ASC) return;

	// 포션이 제공하는 GE 적용
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	// 스택은 1로 고정. 필요 시 확장 가능
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PotionGameplayEffect, 1.f, EffectContext);
	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
