// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Player/MSGA_PlayerDefaultAttack.h"
#include "Player/MSPlayerController.h"

#include "Types/MageSquadTypes.h"
#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"

UMSGA_PlayerDefaultAttack::UMSGA_PlayerDefaultAttack()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 어빌리티 태그 설정
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Player_Ability_DefaultAttack);
	SetAssetTags(TagContainer);

	// 트리거 이벤트 태그 설정 (Gameplay Event로 활성화)
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Player_Event_DefaultAttack);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UMSGA_PlayerDefaultAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 어빌리티를 활성화해도 되는지 검사
	if (!CheckAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 아바타 액터 가져오기
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 발사체의 원본 데이터를 기반으로 런타임 데이터 생성
	FProjectileRuntimeData RuntimeData = UMSFunctionLibrary::MakeProjectileRuntimeData(ProjectileDataClass);

	// 발사체 스폰 위치 및 방향 계산
	const FVector SpawnLocation = Avatar->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	FVector Direction = Avatar->GetActorForwardVector();

	// 컨트롤러의 커서 방향으로 설정
	if (AMSPlayerController* PC = Cast<AMSPlayerController>(ActorInfo->PlayerController.Get()))
	{
		// 발사체 방향을 커서 방향으로 설정
		RuntimeData.Direction = PC->GetServerCursorDir(Avatar->GetActorForwardVector());
	}

	// 스폰 위치 설정
	FTransform SpawnTransform = FTransform();
	SpawnTransform.SetLocation(SpawnLocation);

	// 발사체 런타임 데이터를 통해 발사체 생성 및 발사
	UMSFunctionLibrary::LaunchProjectile(this, ProjectileDataClass, RuntimeData, SpawnTransform, Avatar, Cast<APawn>(Avatar));

	// 능력 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UMSGA_PlayerDefaultAttack::CheckAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 액터 유효성 검사
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return false;

	// 서버에서만 로직 수행
	if (!ActorInfo->IsNetAuthority()) return false;

	// 코스트 및 쿨타임 검사
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) return false;

	return true;
}
