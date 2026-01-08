// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_Dissolve.h"

#include "GameFramework/Character.h"

UMSGC_Dissolve::UMSGC_Dissolve()
{
}

bool UMSGC_Dissolve::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
    if (!Target || !Target->GetWorld())
    {
        return false;
    }

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
    
    TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> WeakDynamicMaterials;
    for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
    {
        UMaterialInstanceDynamic* DMI = Mesh->CreateDynamicMaterialInstance(i);
        if (DMI)
        {
            WeakDynamicMaterials.Add(DMI);
            DMI->SetScalarParameterValue(DissolveParameterName, 1.0f);
        }
    }
    
    FName LocalParamName = DissolveValueParameterName; 
    float Duration = DissolveDuration;
    float StartTime = Target->GetWorld()->GetTimeSeconds();

    FTimerHandle TimerHandle;
    FTimerDelegate TimerDel;

    // 람다 구현 (TimerHandle을 값으로 캡처하지 않고, TimerManager에서 직접 찾아 해제하거나 조건부 종료)
    TimerDel.BindLambda([Target, WeakDynamicMaterials, LocalParamName, StartTime, Duration]()
    {
        if (!Target) return;

        float CurrentTime = Target->GetWorld()->GetTimeSeconds();
        float ElapsedTime = CurrentTime - StartTime;
        float Alpha = FMath::Clamp(ElapsedTime / Duration, -0.5f, 1.0f);

        for (auto& WeakDMI : WeakDynamicMaterials)
        {
            if (WeakDMI.IsValid())
            {
                WeakDMI->SetScalarParameterValue(LocalParamName, Alpha);
            }
        }

        // 완료 시 타이머 종료
        if (Alpha >= 1.0f)
        {
            Target->GetWorldTimerManager().ClearAllTimersForObject(Target);
        }
    });

    Target->GetWorldTimerManager().SetTimer(TimerHandle, TimerDel, 0.01f, true);

    return true;
}
