// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyNormalAttack.h"

#include "AbilitySystemComponent.h"
#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "AbilitySystem/Tasks/MSAT_PlayMontageAndWaitForEvent.h"
#include "Actors/Projectile/MSEnemyProjectile.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehavior_Normal.h"
#include "Enemy/MSBaseEnemy.h"


UMSGA_EnemyNormalAttack::UMSGA_EnemyNormalAttack()
{	
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_NormalAttack);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Attack);
}

void UMSGA_EnemyNormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (UAnimMontage* AttackMontage = Owner->GetAttackMontage())
	{
		// UAbilityTask_PlayMontageAndWait* EnemyAttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("NormalAttack"), AttackMontage);
		// EnemyAttackTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyNormalAttack::OnCompleteCallback); // 몽타주가 끝나면 호출될 함수
		// EnemyAttackTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyNormalAttack::OnInterruptedCallback); // 몽타주가 중단되면 호출될 함수
		// EnemyAttackTask->ReadyForActivation();
		
		FGameplayTagContainer Tags;
		Tags.AddTag(FGameplayTag::RequestGameplayTag("Enemy.Event.NormalAttack"));

		UMSAT_PlayMontageAndWaitForEvent* AttackTask = 
			UMSAT_PlayMontageAndWaitForEvent::CreateTask(this, AttackMontage, Tags);

		AttackTask->OnCompleted.AddDynamic(this, &UMSGA_EnemyNormalAttack::OnCompleteCallback);
		AttackTask->OnInterrupted.AddDynamic(this, &UMSGA_EnemyNormalAttack::OnInterruptedCallback);
		AttackTask->OnEventReceived.AddDynamic(this, &UMSGA_EnemyNormalAttack::OnEventReceivedCallback);
		AttackTask->ReadyForActivation();
	}
}

void UMSGA_EnemyNormalAttack::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMSGA_EnemyNormalAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMSGA_EnemyNormalAttack::OnCompleteCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyNormalAttack::OnInterruptedCallback()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UMSGA_EnemyNormalAttack::OnEventReceivedCallback(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// 서버가 아니면 리턴
	if (GetCurrentActorInfo()->AvatarActor->GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	
	// 이벤트 처리 - 발사체 발사
	
	FProjectileRuntimeData RuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(Owner->GetProjectileDataClass());
	RuntimeData.BehaviorClass = UMSProjectileBehavior_Normal::StaticClass();
	const UMSEnemyAttributeSet* AttributeSet =  Cast<UMSEnemyAttributeSet>(Owner->GetAbilitySystemComponent()->GetAttributeSet(UMSEnemyAttributeSet::StaticClass()));
	RuntimeData.Damage =  AttributeSet->GetAttackDamage();
	
	AActor* CachedAvatar = GetAvatarActorFromActorInfo();
	
	UMSFunctionLibrary::LaunchProjectile(
	this,
	Owner->GetProjectileDataClass(),
	RuntimeData,
	GetAvatarActorFromActorInfo()->GetActorTransform(),
	CachedAvatar,
	Cast<APawn>(CachedAvatar),
	AMSEnemyProjectile::StaticClass());
}
