// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Tasks/MSAT_ChaseAndSpawnMeteor.h"

#include "AbilitySystemComponent.h"
#include "MSGameplayTags.h"
#include "Actors/Indicator/MSIndicatorActor.h"
#include "Player/MSPlayerCharacter.h"

UMSAT_ChaseAndSpawnMeteor* UMSAT_ChaseAndSpawnMeteor::CreateTask(UGameplayAbility* OwningAbility, float InTotalDuration,
                                                                 float InSpawnInterval, TSubclassOf<AMSIndicatorActor> IndicatorClass, const FAttackIndicatorParams& IndicatorParams,
                                                                 TSubclassOf<UGameplayEffect> DamageEffect, UParticleSystem* CompleteParticle, USoundBase* CompleteSound)
{
	UMSAT_ChaseAndSpawnMeteor* Task = NewAbilityTask<UMSAT_ChaseAndSpawnMeteor>(OwningAbility);
	Task->TotalDuration = InTotalDuration;
	Task->SpawnInterval = InSpawnInterval;
	Task->IndicatorActorClass = IndicatorClass;
	Task->CachedIndicatorParams = IndicatorParams;
	Task->DamageEffectClass = DamageEffect;
	Task->bTickingTask = true;
	Task->CompleteParticle = CompleteParticle;
	Task->CompleteSound = CompleteSound;
	return Task;
}

void UMSAT_ChaseAndSpawnMeteor::Activate()
{
	Super::Activate();
	
	// 즉시 첫 스폰을 위해 간격만큼 설정
	TimeSinceLastSpawn = SpawnInterval;
}

void UMSAT_ChaseAndSpawnMeteor::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!Ability || !AbilitySystemComponent.IsValid())
	{
		EndTask();
		return;
	}
	
	if (bSpawningComplete)
	{
		if (ActiveIndicatorCount <= 0)
		{
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnChaseComplete.Broadcast();
			}
			EndTask();
		}
		return;
	}

	ElapsedTime += DeltaTime;
	TimeSinceLastSpawn += DeltaTime;

	// 스폰 페이즈 종료 체크
	if (ElapsedTime >= TotalDuration)
	{
		bSpawningComplete = true;
        
		// 활성 Indicator가 없으면 즉시 종료
		if (ActiveIndicatorCount <= 0)
		{
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnChaseComplete.Broadcast();
			}
			EndTask();
		}
		return;
	}
	// 스폰 간격 체크
	if (TimeSinceLastSpawn >= SpawnInterval)
	{
		TimeSinceLastSpawn = 0.f;
		SpawnIndicatorsOnAllPlayers();
	}
}

void UMSAT_ChaseAndSpawnMeteor::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
}

void UMSAT_ChaseAndSpawnMeteor::SpawnIndicatorsOnAllPlayers()
{
	// 서버에서만 실행
	AActor* AvatarActor = GetAvatarActor();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 모든 플레이어에게 Indicator 스폰
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		APawn* PlayerPawn = PC->GetPawn();
		if (!PlayerPawn)
		{
			continue;
		}
		
		if (AMSPlayerCharacter* Player = Cast<AMSPlayerCharacter>(PlayerPawn))
		{
			if (Player->GetAbilitySystemComponent()->HasMatchingGameplayTag(MSGameplayTags::Player_State_Dead))
			{
				continue;
			}
		}

		const FVector PlayerLocation = PlayerPawn->GetActorLocation();
		AMSIndicatorActor* SpawnedIndicator = SpawnIndicatorAtLocation(PlayerLocation);

		if (SpawnedIndicator && ShouldBroadcastAbilityTaskDelegates())
		{
			++ActiveIndicatorCount;
			OnIndicatorSpawned.Broadcast(SpawnedIndicator, PlayerLocation);
			
			SpawnedIndicator->OnIndicatorComplete.AddDynamic(
			this,
			&UMSAT_ChaseAndSpawnMeteor::HandleIndicatorComplete);
		}
	}
}

AMSIndicatorActor* UMSAT_ChaseAndSpawnMeteor::SpawnIndicatorAtLocation(const FVector& Location) const
{
	UWorld* World = GetWorld();
	if (!World || !IndicatorActorClass)
	{
		return nullptr;
	}

	// 바닥 높이 계산
	FVector SpawnLocation = Location;
	SpawnLocation.Z = GetGroundZ(Location);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMSIndicatorActor* Indicator = World->SpawnActor<AMSIndicatorActor>(
		IndicatorActorClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams);

	if (Indicator)
	{
		Indicator->Initialize(CachedIndicatorParams);
		
		// 데미지 정보 설정
		if (DamageEffectClass && AbilitySystemComponent.IsValid())
		{
			Indicator->SetDamageInfo(AbilitySystemComponent.Get(), DamageEffectClass);
		}
	}

	return Indicator;
}

float UMSAT_ChaseAndSpawnMeteor::GetGroundZ(const FVector& Location) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return Location.Z;
	}

	FHitResult HitResult;
	const FVector TraceStart = Location + FVector(0.f, 0.f, 500.f);
	const FVector TraceEnd = Location - FVector(0.f, 0.f, 1000.f);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	// 바닥 채널로 트레이스 (ECC_WorldStatic 또는 커스텀 채널)
	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		// 약간 위로 오프셋 (데칼이 바닥에 묻히지 않도록)
		return HitResult.Location.Z + 1.f;
	}

	return Location.Z;
}

void UMSAT_ChaseAndSpawnMeteor::HandleIndicatorComplete(AMSIndicatorActor* Indicator, const TArray<AActor*>& HitActors)
{
	// 컨텍스트 객체 생성 (New로 생성하지만 핸들이 관리하므로 메모리는 안전)
	FMSGameplayEffectContext* Context = new FMSGameplayEffectContext();
	
	Context->SetEffectAssets(CompleteParticle, CompleteSound);
	// Context->AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo()); // 필요한 경우
	
	FGameplayEffectContextHandle ContextHandle(Context);
	
	FGameplayCueParameters Params;
	Params.EffectContext = ContextHandle;
	Params.Location = Indicator->GetActorLocation(); // 재생될 위치
	Params.RawMagnitude = 1.0f;       // 필요시 강도 전달
	
	// 5. 실행
	AbilitySystemComponent->ExecuteGameplayCue(
		FGameplayTag::RequestGameplayTag("GameplayCue.IndicatorComplete"), 
		Params
	);
	
	--ActiveIndicatorCount;
    
	if (bSpawningComplete && ActiveIndicatorCount <= 0)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnChaseComplete.Broadcast();
		}
		EndTask();
	}
}
