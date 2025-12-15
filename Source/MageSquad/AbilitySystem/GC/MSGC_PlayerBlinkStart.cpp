// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_PlayerBlinkStart.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UMSGC_PlayerBlinkStart::UMSGC_PlayerBlinkStart()
{
}

bool UMSGC_PlayerBlinkStart::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget || !StartNiagara) return false;

	// 스폰 위치/회전값 구하기
	const FVector SpawnLocation = ResolveSpawnLocation(MyTarget, Parameters);
	const FRotator SpawnRotation = ResolveSpawnRotation(MyTarget);

	// 나이아가라 스폰
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		MyTarget->GetWorld(),
		StartNiagara,
		SpawnLocation,
		SpawnRotation
	);

	return true;
}

FVector UMSGC_PlayerBlinkStart::ResolveSpawnLocation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// Ability에서 Params.Location을 넣어줬다면 그 위치가 우선
	if (!Parameters.Location.IsNearlyZero())
	{
		return Parameters.Location;
	}
	return MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector;
}

FRotator UMSGC_PlayerBlinkStart::ResolveSpawnRotation(AActor* MyTarget) const
{
	return MyTarget ? MyTarget->GetActorRotation() : FRotator::ZeroRotator;
}
