 // Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_Indicator.h"
#include "Actors/Indicator/MSIndicatorActor.h"
#include "Types/MageSquadTypes.h"

 AMSGC_Indicator::AMSGC_Indicator()
 {
 	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Enemy.Indicator"));
 }

 bool AMSGC_Indicator::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters)
 {
	 Super::OnExecute_Implementation(Target, Parameters);
 
 	if (!Target || !IndicatorActorClass)
 	{
 		return false;
 	}
 	
 	// UE_LOG(LogTemp, Error, TEXT("[AMSGC_Indicator %s] - HasAuthority: %s"),
	 //   *GetName(),
	 //   Owner->HasAuthority() ? TEXT("Server") : TEXT("Client"));
 	
 	if (!GetOwner()->HasAuthority())
 	{
 		// 클라이언트: Replicated Actor를 사용하므로 직접 스폰하지 않음
 		return false;
 	}
 	
 	// EffectContext에서 파라미터 추출
 	FAttackIndicatorParams IndicatorParams;
    
 	if (const FGameplayEffectContext* BaseCtx = Parameters.EffectContext.Get())
 	{
 		if (BaseCtx->GetScriptStruct() == FMSGameplayEffectContext::StaticStruct())
 		{
 			const FMSGameplayEffectContext* MSCtx = static_cast<const FMSGameplayEffectContext*>(BaseCtx);
 			if (MSCtx->HasIndicatorParams())
 			{
 				IndicatorParams = MSCtx->GetIndicatorParams();
 			}
 		}
 	}

 	FVector SpawnLocation = Parameters.Location;
 	FRotator SpawnRotation = Parameters.Normal.Rotation();

 	AMSIndicatorActor* Indicator = GetWorld()->SpawnActor<AMSIndicatorActor>(
		 IndicatorActorClass,
		 SpawnLocation,
		 SpawnRotation
	 );

 	if (Indicator)
 	{
 		Indicator->Initialize(IndicatorParams);
 		
 		// Owner 설정 (데미지 정보를 인터페이스로 조회하기 위함)
 		// Target은 GameplayCue를 실행한 주체 (보스 등)
 		Indicator->SetIndicatorOwner(Target);
 	}

 	return true;
 }

 bool AMSGC_Indicator::OnRemove_Implementation(AActor* Target, const FGameplayCueParameters& Parameters)
 {
 	// Indicator는 Duration 후 자동 파괴되므로 별도 처리 불필요
 	return true;
 }
