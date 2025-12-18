// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerState.h"
#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"

AMSPlayerState::AMSPlayerState()
{
	// PlayerState의 기본 복제 지연이 낮으므로, 네트워크 업데이트 빈도를 늘림
	SetNetUpdateFrequency(60.f);

	AbilitySystemComponent = CreateDefaultSubobject<UMSPlayerAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Minimal 모드는 클라이언트에는 필요한 정보만 복제하여 네트워크 부하를 줄임
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UMSPlayerAttributeSet>(TEXT("AttributeSet"));

	bUIReady = false;
}
void AMSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSPlayerState, bUIReady);
}

void AMSPlayerState::SetUIReady(bool bReady)
{
	if (!HasAuthority())
	{
		return;
	}
	bUIReady = bReady;
}

UAbilitySystemComponent* AMSPlayerState::GetAbilitySystemComponent() const
{
	check(AbilitySystemComponent);
	return AbilitySystemComponent;
}

UMSPlayerAttributeSet* AMSPlayerState::GetAttributeSet() const
{
	check(AttributeSet);
	return AttributeSet;
}