// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Player/MSGA_PlayerDefaultAttack.h"

#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"

UMSGA_PlayerDefaultAttack::UMSGA_PlayerDefaultAttack()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 태그 설정
	FGameplayTagContainer TagContainer;
	//SetAssetTags()
}

void UMSGA_PlayerDefaultAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 로컬 플레이어에서만 로직 수행
	// 커서 방향과 같은 로컬 입력/시점 정보를 필요로 하기 때문
	if (!ActorInfo->IsLocallyControlled())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	//if (AMSPlayerCharacter* Player = Cast<AMSPlayerCharacter>(ActorInfo->AvatarActor.Get()))
	//{
	//	APlayerController* PC = Cast<APlayerController>(ActorInfo->PlayerController.Get());
	//	if (PC)
	//	{
	//		FHitResult Hit;

	//		// 커서 위치 가져오기
	//		if (!PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	//		{
	//			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	//			return;
	//		}

	//		// 발사체 위치, 방향 구하기
	//		FVector SpawnLocation = Player->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	//		FVector Direction = Hit.Location - SpawnLocation;
	//		if (!Direction.Normalize())
	//		{
	//			Direction = Player->GetActorForwardVector();
	//		}

	//		// 서버에 발사체 생성 요청
	//		ServerRPCSpawnProjectile(SpawnLocation, Direction);
	//	}
	//}

	// 능력 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
