// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/AutoActiveSkill/MSGA_IceSpear.h"
#include "MSGA_IceSpear.h"
#include "SkillData/MSIceSpearSkillDataRow.h"

UMSGA_IceSpear::UMSGA_IceSpear()
{
    static ConstructorHelpers::FObjectFinder<UDataTable> DT_IceSpear(
        TEXT("DataTable'/Game/Data/DT_IceSpear.DT_IceSpear'")
    );

    if (DT_IceSpear.Succeeded())
    {
        SkillLevelDataTable = DT_IceSpear.Object;
    }
}

void UMSGA_IceSpear::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // 서버에서만 작동하도록 설정
    if (!HasAuthority(&ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEnd*/true, /*bWasCancelled*/false);
        return;
    }
    
    // 현재 스킬 레벨 가져오기
    const int32 Level = GetAbilityLevel(Handle, ActorInfo);

    // 데이터테이블이 없으면 에러메시지와 함께 종료
    if (!SkillLevelDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("IceSpear: SkillLevelDataTable is NULL"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // RowName 생성
    const FName RowName(*FString::FromInt(Level));
    const FString ContextString(TEXT("IceSpear Level Lookup"));

    // Level에 맞는 Row 가져오기
    const FMSIceSpearSkillDataRow* Row =
        SkillLevelDataTable->FindRow<FMSIceSpearSkillDataRow>(RowName, ContextString);

    // 데이터테이블 오류 시 종료
    if (!Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("IceSpear: No row found for Level %d"), Level);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // Row 데이터 → Ability 멤버 변수로 적용
    CurrentProjectileNumber = Row->ProjectileNumber;
    CurrentPenetration = Row->Penetration;
    CurrentSkillDamage = Row->SkillDamage;

    // Todo : 아바타의 위치, 공격방향, 투사체 개수만큼 스폰, 데미지 이펙트 적용
}
