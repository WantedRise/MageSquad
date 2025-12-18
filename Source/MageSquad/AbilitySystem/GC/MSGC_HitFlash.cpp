// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_HitFlash.h"

#include "GameFramework/Character.h"

UMSGC_HitFlash::UMSGC_HitFlash()
{
	GameplayCueTag = FGameplayTag::RequestGameplayTag(
		FName("GameplayCue.HitFlash")
	);
}

bool UMSGC_HitFlash::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	if (!Target)
	{
		return false;
	}

	// Character의 Mesh 가져오기
	ACharacter* Character = Cast<ACharacter>(Target);
	if (!Character)
	{
		return false;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return false;
	}

	// Dynamic Material Instance 생성 또는 재사용
	TArray<UMaterialInstanceDynamic*> DynamicMaterials;
	for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
	{
		UMaterialInstanceDynamic* DMI = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(i));
		if (!DMI)
		{
			DMI = Mesh->CreateDynamicMaterialInstance(i);
		}

		if (DMI)
		{
			DynamicMaterials.Add(DMI);
			// HitFlash 적용
			DMI->SetScalarParameterValue(MaterialParameterName, 1.0f);
		}
	}

	// 타이머로 리셋
	FTimerHandle TimerHandle;
	Target->GetWorldTimerManager().SetTimer(
		TimerHandle,
		[DynamicMaterials, this]()
		{
			for (UMaterialInstanceDynamic* DMI : DynamicMaterials)
			{
				if (DMI)
				{
					DMI->SetScalarParameterValue(MaterialParameterName, 0.0f);
				}
			}
		},
		FlashDuration,
		false
	);

	return true;
}
