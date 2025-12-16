// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGCN_Skill_Explosion.h"
#include "NiagaraFunctionLibrary.h"
#include "MSGameplayTags.h"

UMSGCN_Skill_Explosion::UMSGCN_Skill_Explosion()
{
	GameplayCueTag = MSGameplayTags::GameplayCue_Skill_Explosion;
}

bool UMSGCN_Skill_Explosion::OnExecute_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters
) const
{
	if (!ExplosionNiagara)
	{
		return false;
	}

	const UWorld* World = MyTarget ? MyTarget->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	// 1) 위치
	const FVector SpawnLocation = Parameters.Location;

	// 2) 회전
	const FRotator SpawnRotation =
		Parameters.Normal.IsNearlyZero()
		? FRotator::ZeroRotator
		: Parameters.Normal.ToOrientationRotator();

	// 스케일 (GA → RawMagnitude)
	const float ScaleValue =
		(Parameters.RawMagnitude > 0.f)
		? Parameters.RawMagnitude
		: DefaultScale;

	const FVector SpawnScale(ScaleValue);

	// 4) Niagara 스폰
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		ExplosionNiagara,
		SpawnLocation,
		SpawnRotation,
		SpawnScale,
		true,   // bAutoDestroy
		true    // bAutoActivate
	);

	return true;
}