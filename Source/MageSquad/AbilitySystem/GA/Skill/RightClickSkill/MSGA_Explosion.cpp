// Fill out your copyright notice in the Description page of Project Settings.


#include "MSGA_Explosion.h"
#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "Player/MSPlayerController.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehavior_AreaPeriodic.h"
#include "MSFunctionLibrary.h"

UMSGA_Explosion::UMSGA_Explosion()
{
	// 어빌리티 태그 설정
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Skill_Ability_Explosion);
	SetAssetTags(TagContainer);

	// 트리거 이벤트 태그 설정 (Gameplay Event로 활성화)
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Skill_Event_Explosion);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_Explosion::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	SkillID = CurrentSkillID;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CoolTime = SkillListRow.CoolTime;
	Range = SkillListRow.Range;
	TArray<float> FinalDamageSequence = BaseDamageSequence;
	for (float& Damage : FinalDamageSequence)
	{
		Damage *= SkillListRow.SkillDamage;
	}


	AActor* Avatar = GetAvatarActorFromActorInfo();
	AMSPlayerController* PC = Cast<AMSPlayerController>(ActorInfo->PlayerController.Get());
	if (!Avatar || !PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 서버가 알고 있는 커서 위치
	const FVector ExplosionLocation = PC->GetServerCursor();

	// 서버에서만 실제 스폰/판정
	if (!Avatar->HasAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 투사체 데이터 없으면 종료
	if (!ProjectileDataClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ProjectileDataClass is null"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// RuntimeData 생성
	FProjectileRuntimeData RuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(ProjectileDataClass);

	// RuntimeData 설정
	const FTransform SpawnTM(FRotator::ZeroRotator, ExplosionLocation);
	RuntimeData.Radius = Range;
	RuntimeData.LifeTime = 0.f;
	RuntimeData.DamageEffect = DamageEffect;
	RuntimeData.BehaviorClass = UMSProjectileBehavior_AreaPeriodic::StaticClass();
	RuntimeData.DamageSequence = FinalDamageSequence;
	RuntimeData.DamageInterval = DamageInterval;

	// 공격 스폰
	UMSFunctionLibrary::LaunchProjectile(
		this,
		ProjectileDataClass,
		RuntimeData,
		SpawnTM,
		Avatar,
		Cast<APawn>(Avatar)
	);


	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
