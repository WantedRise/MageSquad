// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_IndicatorComplete.h"

#include "MageSquad.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Types/MageSquadTypes.h"

UMSGC_IndicatorComplete::UMSGC_IndicatorComplete()
{
	//GameplayCueTag = FGameplayTag::RequestGameplayTag("GameplayCue.IndicatorComplete");
}

bool UMSGC_IndicatorComplete::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	Super::OnExecute_Implementation(Target, Parameters);
	
	// UE_LOG(LogMSNetwork, Log, TEXT("UMSGC_IndicatorComplete::OnExecute_Implementation"));

	const FMSGameplayEffectContext* Context = static_cast<const FMSGameplayEffectContext*>(Parameters.EffectContext.Get());
    
	UParticleSystem* ParticleToPlay = CompleteParticle; // 기본값 (에디터 설정값)
	USoundBase* SoundToPlay = CompleteSound;           // 기본값

	if (Context && Context->bHasEffectAssets)
	{
		if (Context->ParticleAsset)
		{
			ParticleToPlay = Context->ParticleAsset;
			UE_LOG(LogMSNetwork, Log, TEXT("ParticleToPlay = Context->ParticleAsset;"));
		}
		if (Context->SoundAsset)
		{
			SoundToPlay = Context->SoundAsset;
		}
	}

	FVector SpawnLocation = Parameters.Location;
	if (Target && SpawnLocation.IsZero())
	{
		SpawnLocation = Target->GetActorLocation();
	}
	
	SpawnLocation.Z = 0.f;

	if (ParticleToPlay)
	{
		if (UParticleSystemComponent* SpawnParticle = UGameplayStatics::SpawnEmitterAtLocation(Target->GetWorld(), ParticleToPlay, SpawnLocation))
		{
			SpawnParticle->CustomTimeDilation = 0.5f;
		}
	}
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(Target->GetWorld(), SoundToPlay, SpawnLocation);
	}

	return true;
}
