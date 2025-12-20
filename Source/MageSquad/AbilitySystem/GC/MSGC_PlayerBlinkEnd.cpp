// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_PlayerBlinkEnd.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"

#include "Types/MageSquadTypes.h"

UMSGC_PlayerBlinkEnd::UMSGC_PlayerBlinkEnd()
{
	// GameplayCue Tag 바인딩
	//const UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	//GameplayCueTag = TagsManager.RequestGameplayTag(FName("GameplayCue.Player.Blink.Start"), false);
}

bool UMSGC_PlayerBlinkEnd::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	// 둘 중 하나라도 있으면 실행
	if (!EndNiagaraA && !EndNiagaraB) return false;

	// 스폰 위치/회전값 구하기
	const FVector SpawnLocation = ResolveSpawnLocation(MyTarget, Parameters);
	const FRotator SpawnRotation = ResolveSpawnRotation(MyTarget);
	const FLinearColor Color = ResolveLinearColor(Parameters);

	// 나이아가라 스폰
	if (EndNiagaraA)
	{
		UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyTarget->GetWorld(), EndNiagaraA, SpawnLocation, SpawnRotation);
		
		// 파라미터 설정
		Niagara->SetVectorParameter(TEXT("Blink_End"), SpawnLocation);
		//Niagara->SetColorParameter(TEXT("Blink_Color"), Color);
	}
	if (EndNiagaraB)
	{
		UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyTarget->GetWorld(), EndNiagaraB, SpawnLocation, SpawnRotation);
		
		// 파라미터 설정
		Niagara->SetVectorParameter(TEXT("Blink_End"), SpawnLocation);
		//Niagara->SetColorParameter(TEXT("Blink_Color"), Color);
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

FLinearColor UMSGC_PlayerBlinkEnd::ResolveLinearColor(const FGameplayCueParameters& Parameters) const
{
	if (const FMSGameplayEffectContext* Context = static_cast<const FMSGameplayEffectContext*>(Parameters.EffectContext.Get()))
	{
		return Context->CueColor;
	}

	return FLinearColor();
}
