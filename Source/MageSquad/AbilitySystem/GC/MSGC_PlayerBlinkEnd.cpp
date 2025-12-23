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

	// Beam의 시작/끝 위치
	const FVector BeamStart = ResolveSpawnLocation(MyTarget, Parameters);
	const FVector BeamEnd = ResolveBeamEnd(MyTarget, Parameters);

	// 시스템 스폰 위치는 Beam End로 고정
	const FVector SpawnLocation = BeamEnd;
	const FRotator SpawnRotation = ResolveSpawnRotation(MyTarget);
	const FLinearColor Color = ResolveLinearColor(Parameters);

	// 나이아가라 스폰
	if (EndNiagaraA)
	{
		UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyTarget->GetWorld(), EndNiagaraA, SpawnLocation, SpawnRotation);

		// 파라미터 설정
		// Niagara User Parameter는 엔진/버전에 따라 "User." 접두어 필요 여부가 달라질 수 있어 두 형태를 모두 세팅
		Niagara->SetVectorParameter(TEXT("User.Blink_Start"), BeamStart);
		Niagara->SetVectorParameter(TEXT("User.Blink_End"), BeamEnd);
		Niagara->SetColorParameter(TEXT("User.Blink.Color"), Color);

		Niagara->SetVectorParameter(TEXT("Blink_Start"), BeamStart);
		Niagara->SetVectorParameter(TEXT("Blink_End"), BeamEnd);
		Niagara->SetColorParameter(TEXT("Blink_Color"), Color);
	}
	if (EndNiagaraB)
	{
		UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyTarget->GetWorld(), EndNiagaraB, SpawnLocation, SpawnRotation);

		// 파라미터 설정
		Niagara->SetVectorParameter(TEXT("User.Blink_Start"), BeamStart);
		Niagara->SetVectorParameter(TEXT("User.Blink_End"), BeamEnd);
		Niagara->SetColorParameter(TEXT("User.Blink.Color"), Color);

		Niagara->SetVectorParameter(TEXT("Blink_Start"), BeamStart);
		Niagara->SetVectorParameter(TEXT("Blink_End"), BeamEnd);
		Niagara->SetColorParameter(TEXT("Blink_Color"), Color);
	}

	return true;
}

FVector UMSGC_PlayerBlinkEnd::ResolveSpawnLocation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
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

FVector UMSGC_PlayerBlinkEnd::ResolveBeamEnd(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// Beam End는 EffectContext에 기록된 BlinkEnd를 우선 사용
	if (const FMSGameplayEffectContext* Context = static_cast<const FMSGameplayEffectContext*>(Parameters.EffectContext.Get()))
	{
		if (Context->HasBlinkSegment())
		{
			return Context->BlinkEnd;
		}
	}

	// 기존 방식(Parameters.Location)
	return Parameters.Location;
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

	return FLinearColor::White;
}
