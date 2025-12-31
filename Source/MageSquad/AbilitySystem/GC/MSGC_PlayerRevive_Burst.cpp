// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_PlayerRevive_Burst.h"

#include "GameFramework/Character.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UMSGC_PlayerRevive_Burst::UMSGC_PlayerRevive_Burst()
{
}

bool UMSGC_PlayerRevive_Burst::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	if (!Target) return false;

	// 나이아가라 스폰
	if (StartNiagara)
	{
		AActor* InstigatorActor = Parameters.Instigator.Get();
		if (!InstigatorActor) return false;

		APawn* InstigatorPawn = Cast<APawn>(InstigatorActor);
		if (!InstigatorPawn)
		{
			// Pawn이 아닌 Actor가 들어왔을 가능성까지 방어적으로 처리
			InstigatorPawn = Cast<APawn>(InstigatorActor->GetOwner());
		}
		if (!InstigatorPawn) return false;

		// 나이아가라를 부착시킬 SkeletalMeshComponent 찾기
		USkeletalMeshComponent* SkelMeshComp = nullptr;

		if (ACharacter* Character = Cast<ACharacter>(Parameters.Instigator.Get()))
		{
			SkelMeshComp = Character->GetMesh();
		}
		else
		{
			SkelMeshComp = InstigatorPawn->FindComponentByClass<USkeletalMeshComponent>();
		}

		if (!SkelMeshComp) return false;

		// 레벨업 나이아가라 부착
		UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
			StartNiagara,
			SkelMeshComp,
			NAME_None,
			Parameters.Location + FVector(0.f, 0.f, -100.f),
			FRotator::ZeroRotator,
			EAttachLocation::KeepWorldPosition,
			true
		);

		// 나이아가라의 회전 절댓값 설정
		if (Niagara)
		{
			Niagara->SetAbsolute(false, true, false);
		}
	}

	return true;
}
