// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_Dissolve.h"

#include "DataAssets/Enemy/DA_EnemyStaticSoundData.h"
#include "Enemy/MSBaseEnemy.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

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
    
    if (AMSBaseEnemy* Avatar = Cast<AMSBaseEnemy>(Character))
    {
        if (UDA_EnemyStaticSoundData* EnemySoundData = Avatar->GetEnemySoundData())
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), EnemySoundData->DeathSound, Avatar->GetActorLocation());
        }
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
    
    // Target을 WeakObjectPtr로 캡처
    TWeakObjectPtr<AActor> WeakTarget = Target;
    FName LocalParamName = DissolveValueParameterName; 
    float Duration = DissolveDuration;
    float StartTime = Target->GetWorld()->GetTimeSeconds();

    FTimerHandle TimerHandle;
    FTimerDelegate TimerDel;

    TimerDel.BindLambda([WeakTarget, WeakDynamicMaterials, LocalParamName, StartTime, Duration]()
    {
        // Target 유효성 체크
        AActor* TargetActor = WeakTarget.Get();
        if (!TargetActor || !TargetActor->GetWorld())
        {
            return; // 타이머는 Target 파괴 시 자동 정리됨
        }

        float CurrentTime = TargetActor->GetWorld()->GetTimeSeconds();
        float ElapsedTime = CurrentTime - StartTime;
        float Alpha = FMath::Clamp(ElapsedTime / Duration, -0.5f, 1.0f);

        for (const auto& WeakDMI : WeakDynamicMaterials)
        {
            if (WeakDMI.IsValid())
            {
                WeakDMI->SetScalarParameterValue(LocalParamName, Alpha);
            }
        }

        if (Alpha >= 1.0f)
        {
            TargetActor->GetWorldTimerManager().ClearAllTimersForObject(TargetActor);
        }
    });

    Target->GetWorldTimerManager().SetTimer(TimerHandle, TimerDel, 0.01f, true);

    return true;
}
