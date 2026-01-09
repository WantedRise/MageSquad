// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Enemy/MSGA_EnemyNormalAttack.h"

#include "AbilitySystemComponent.h"
#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/AttributeSets/MSEnemyAttributeSet.h"
#include "AbilitySystem/Tasks/MSAT_PlayMontageAndWaitForEvent.h"
#include "Actors/Projectile/MSEnemyProjectile.h"
#include "Actors/Projectile/Behaviors/MSPB_Normal.h"
#include "Enemy/MSBaseEnemy.h"


UMSGA_EnemyNormalAttack::UMSGA_EnemyNormalAttack()
{	
	// 어빌리티 식별 Tag
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Enemy_Ability_NormalAttack);
	SetAssetTags(TagContainer);

	// 활성화 시 Owner에게 부여되는 Tag
	ActivationOwnedTags.AddTag(MSGameplayTags::Enemy_State_Attack);
	
	BlockAbilitiesWithTag.AddTag(MSGameplayTags::Enemy_Ability_Move);
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
		FGameplayTagContainer Tags;
		Tags.AddTag(FGameplayTag::RequestGameplayTag("Enemy.Event.NormalAttack"));
		
		// Todo : 추후에 페이즈 전환 관련 델리게이트로 빼서 관리할 예정
		FName StartSectionName = NAME_None;
		if (Owner->GetAbilitySystemComponent()->HasMatchingGameplayTag(MSGameplayTags::Enemy_State_Phase2))
		{
			StartSectionName = TEXT("Phase2");
		}

		UMSAT_PlayMontageAndWaitForEvent* AttackTask = 
			UMSAT_PlayMontageAndWaitForEvent::CreateTask(this, AttackMontage, Tags, 1.f, StartSectionName);

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
	
	UAbilitySystemComponent* ASC = Owner->GetAbilitySystemComponent();	
	// GameplayEffectSpec 생성
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(Owner);
	
	FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(Owner->GetCooldownEffectClass(), 1.f, Context);

	if (!SpecHandle.IsValid())
	{
		return;
	}
	
	// GameplayEffect 적용
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
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
	RuntimeData.BehaviorClass = UMSPB_Normal::StaticClass();
	const UMSEnemyAttributeSet* AttributeSet =  Cast<UMSEnemyAttributeSet>(Owner->GetAbilitySystemComponent()->GetAttributeSet(UMSEnemyAttributeSet::StaticClass()));
	RuntimeData.Damage =  AttributeSet->GetAttackDamage();
	RuntimeData.DamageEffect = Owner->GetDamageEffectClass();
	RuntimeData.Radius = 3.f;
	RuntimeData.CriticalChance = 0.f; // 몬스터의 공격은 치명타가 뜨지 않도록 함
	
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
