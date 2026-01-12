// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_PlayerBlinkStart.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"

#include "Types/MageSquadTypes.h"

UMSGC_PlayerBlinkStart::UMSGC_PlayerBlinkStart()
{
	// GameplayCue Tag 바인딩
	//const UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	//GameplayCueTag = TagsManager.RequestGameplayTag(FName("GameplayCue.Player.Blink.Start"), false);
}

bool UMSGC_PlayerBlinkStart::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget || !StartNiagara) return false;

	// 스폰 위치/회전값 구하기
	const FVector SpawnLocation = ResolveSpawnLocation(MyTarget, Parameters);
	const FRotator SpawnRotation = ResolveSpawnRotation(MyTarget);
	const FLinearColor Color = ResolveLinearColor(Parameters);

	// 나이아가라 스폰
	UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		MyTarget->GetWorld(),
		StartNiagara,
		SpawnLocation,
		SpawnRotation
	);

	// 서운드 재생
	if (StartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, StartSound, SpawnLocation);
	}

	if (Niagara)
	{
		Niagara->SetColorParameter(TEXT("User.Blink.Color"), Color);
		Niagara->SetColorParameter(TEXT("Blink_Color"), Color);
	}

	return true;
}

FVector UMSGC_PlayerBlinkStart::ResolveSpawnLocation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// Beam Start는 EffectContext에 기록된 BlinkStart를 우선 사용
	if (const FMSGameplayEffectContext* Context = static_cast<const FMSGameplayEffectContext*>(Parameters.EffectContext.Get()))
	{
		if (Context->HasBlinkSegment())
		{
			return Context->BlinkStart;
		}
	}

	// 기존 방식(Parameters.Location)
	return Parameters.Location;
}

FRotator UMSGC_PlayerBlinkStart::ResolveSpawnRotation(AActor* MyTarget) const
{
	return MyTarget ? MyTarget->GetActorRotation() : FRotator::ZeroRotator;
}

FLinearColor UMSGC_PlayerBlinkStart::ResolveLinearColor(const FGameplayCueParameters& Parameters) const
{
	if (const FMSGameplayEffectContext* Context = static_cast<const FMSGameplayEffectContext*>(Parameters.EffectContext.Get()))
	{
		return Context->CueColor;
	}

	return FLinearColor::White;
}
