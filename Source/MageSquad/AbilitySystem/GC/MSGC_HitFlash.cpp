// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_HitFlash.h"

#include "GameplayTagsManager.h"
#include "GameFramework/Character.h"

UMSGC_HitFlash::UMSGC_HitFlash()
{

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
	// 약참조(WeakObjectPtr) 배열을 생성
	TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> WeakDynamicMaterials;
	for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
	{
		UMaterialInstanceDynamic* DMI = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(i));
		if (!DMI)
		{
			DMI = Mesh->CreateDynamicMaterialInstance(i);
		}

		if (DMI)
		{
			WeakDynamicMaterials.Add(DMI);
			// HitFlash 적용
			DMI->SetScalarParameterValue(MaterialParameterName, 1.0f);
		}
	}

	// 이름 변수를 값으로 복사 (this를 캡처하지 않기 위함)
	FName LocalParamName = MaterialParameterName;
	
	// 타이머로 리셋
	FTimerHandle TimerHandle;
	Target->GetWorldTimerManager().SetTimer(
		TimerHandle,
		[WeakDynamicMaterials, LocalParamName]()
		{
			for (auto& WeakDMI : WeakDynamicMaterials)
			{
				// 타이머가 실행될 때 객체가 아직 유효한지 확인
				if (WeakDMI.IsValid())
				{
				   WeakDMI->SetScalarParameterValue(LocalParamName, 0.0f);
				}
			}
		},
		FlashDuration,
		false
	);

	return true;
}
