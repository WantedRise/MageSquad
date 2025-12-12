// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/AutoActiveSkill/MSGA_IceSpear.h"
#include "MSGA_IceSpear.h"
#include "SkillData/MSSkillDataRow.h"

UMSGA_IceSpear::UMSGA_IceSpear()
{
    
}

void UMSGA_IceSpear::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // 서버에서만 작동하도록 설정
    if (!HasAuthority(&ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEnd*/true, /*bWasCancelled*/false);
        return;
    }
    
    
}
