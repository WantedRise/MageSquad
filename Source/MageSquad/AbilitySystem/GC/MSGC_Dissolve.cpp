// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_Dissolve.h"

#include "GameFramework/Character.h"

UMSGC_Dissolve::UMSGC_Dissolve()
{
}

bool UMSGC_Dissolve::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
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
			DMI->SetScalarParameterValue(DissolveParameterName, 1.0f);
		}
	}
	
	FName LocalParamName = DissolveValueParameterName; 
	float Duration = DissolveDuration;
    
	// 시작 시간 기록
	float StartTime = Target->GetWorld()->GetTimeSeconds();

	// 타이머 핸들 선언 및 루프 타이머 시작
	FTimerHandle* TimerHandle = new FTimerHandle(); // 람다 내부에서 자신을 해제하기 위해 포인터 사용 권장
    
	// 매우 짧은 간격(0.01초 등)으로 반복 호출하여 보간 구현
	Target->GetWorldTimerManager().SetTimer(
		*TimerHandle,
		[Target, WeakDynamicMaterials, LocalParamName, StartTime, Duration, TimerHandle]() mutable
		{
			float CurrentTime = Target->GetWorld()->GetTimeSeconds();
			float ElapsedTime = CurrentTime - StartTime;
            
			// 0에서 1 사이의 알파값 계산
			float Alpha = FMath::Clamp(ElapsedTime / Duration, -0.5f, 1.0f);

			// 모든 머티리얼에 값 적용
			for (auto& WeakDMI : WeakDynamicMaterials)
			{
				if (WeakDMI.IsValid())
				{
					WeakDMI->SetScalarParameterValue(LocalParamName, Alpha);
				}
			}

			// 완료 시 타이머 종료 및 핸들 메모리 해제
			if (Alpha >= 1.0f)
			{
				Target->GetWorldTimerManager().ClearTimer(*TimerHandle);
				delete TimerHandle;
			}
		},
		0.01f,
		true  
	);

	return true;
}
