// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_PlayerBlinkEnd.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UMSGC_PlayerBlinkEnd::UMSGC_PlayerBlinkEnd()
{
}

bool UMSGC_PlayerBlinkEnd::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	// 둘 중 하나라도 있으면 실행
	if (!EndNiagaraA && !EndNiagaraB) return false;

	// 스폰 위치/회전값 구하기
	const FVector SpawnLocation = ResolveSpawnLocation(MyTarget, Parameters);
	const FRotator SpawnRotation = ResolveSpawnRotation(MyTarget);

	// 나이아가라 스폰
	if (EndNiagaraA)
	{
		auto a = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyTarget->GetWorld(), EndNiagaraA, SpawnLocation, SpawnRotation);
	}
	if (EndNiagaraB)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyTarget->GetWorld(), EndNiagaraB, SpawnLocation, SpawnRotation);
	}

	return true;
}

FVector UMSGC_PlayerBlinkEnd::ResolveSpawnLocation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// Ability에서 Params.Location을 넣어줬다면 그 위치가 우선
	if (!Parameters.Location.IsNearlyZero())
	{
		return Parameters.Location;
	}
	return MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector;
}

FRotator UMSGC_PlayerBlinkEnd::ResolveSpawnRotation(AActor* MyTarget) const
{
	return MyTarget ? MyTarget->GetActorRotation() : FRotator::ZeroRotator;
}