 // Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_Indicator.h"
#include "Actors/Indicator/MSIndicatorActor.h"

 AMSGC_Indicator::AMSGC_Indicator()
 {
 	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Boss.AttackIndicator"));
 }

 bool AMSGC_Indicator::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters)
 {
	 Super::OnExecute_Implementation(Target, Parameters);
 
 	if (!Target || !IndicatorActorClass)
 	{
 		return false;
 	}
 	
 	// EffectContext에서 파라미터 추출
 	FAttackIndicatorParams IndicatorParams;

 	// Parameters.RawMagnitude, Parameters.Location 등을 활용하거나
 	// Custom EffectContext를 만들어서 FAttackIndicatorParams를 전달

 	// 위치와 회전 설정
 	FVector SpawnLocation = Parameters.Location;
 	FRotator SpawnRotation = Parameters.Normal.Rotation();

 	// 스폰
 	AMSIndicatorActor* Indicator = GetWorld()->SpawnActor<AMSIndicatorActor>(
		 IndicatorActorClass,
		 SpawnLocation,
		 SpawnRotation
	 );

 	if (Indicator)
 	{
 		Indicator->Initialize(IndicatorParams);
 	}

 	return true;
 }

 bool AMSGC_Indicator::OnRemove_Implementation(AActor* Target, const FGameplayCueParameters& Parameters)
 {
 	// Indicator는 Duration 후 자동 파괴되므로 별도 처리 불필요
 	return true;
 }
