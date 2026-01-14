// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Projectile/MSBaseProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"

#include "Net/UnrealNetwork.h"

#include "MSFunctionLibrary.h"
#include "GameFramework/GameStateBase.h"

// Behavior
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "Actors/Projectile/Behaviors/MSPB_Normal.h"
#include "Actors/Projectile/Behaviors/MSPB_ChainBolt.h"

AMSBaseProjectile::AMSBaseProjectile()
{
	// Tick 비활성화
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 리플리케이션 활성화
	bReplicates = true;
	SetReplicateMovement(true);

	// Overlap 판정을 위한 Sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere.Get());

	// Sphere Collision 설정
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_GameTraceChannel2);                    // MSProjectile
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap); // MSEnemy
	CollisionSphere->SetGenerateOverlapEvents(true);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(
		this, &AMSBaseProjectile::OnHitOverlap
	);

	// 시각적 Mesh를 Sphere에 부착
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere.Get());
	ProjectileMesh->SetIsReplicated(true);
	ProjectileMesh->SetCollisionProfileName(TEXT("MSProjectile"));
	ProjectileMesh->bReceivesDecals = false;

	// Projectile Movement
	ProjectileMovementComponent =
		CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	ProjectileMovementComponent->OnProjectileStop.AddDynamic(
		this, &AMSBaseProjectile::OnProjectileStop
	);

	// 이동 기준 컴포넌트 지정
	ProjectileMovementComponent->UpdatedComponent = CollisionSphere.Get();

	// 기본 태그 설정
	Tags.AddUnique(TEXT("Projectile"));
}

void AMSBaseProjectile::InitProjectileRuntimeDataFromClass(
	TSubclassOf<UProjectileStaticData> InProjectileDataClass
)
{
	// 발사체 기본 데이터 클래스 설정
	ProjectileDataClass = InProjectileDataClass;

	const UProjectileStaticData* StaticData =
		UMSFunctionLibrary::GetProjectileStaticData(ProjectileDataClass);

	ProjectileRuntimeData.CopyFromStaticData(StaticData);
	bRuntimeDataInitialized = true;
}

void AMSBaseProjectile::SetProjectileRuntimeData(
	const FProjectileRuntimeData& InRuntimeData
)
{
	// 발사체 런타임 데이터 초기화
	ProjectileRuntimeData = InRuntimeData;
	bRuntimeDataInitialized = true;

	// 이미 BeginPlay 이후라면 즉시 반영
	// (서버에서 런타임 데이터를 늦게 받는 경우)
	if (HasActorBegunPlay())
	{
		ApplyProjectileRuntimeData(true);
		EnsureBehavior();

		if (HasAuthority())
		{
			ArmLifeTimerIfNeeded(GetEffectiveRuntimeData());
		}
	}
}

FProjectileRuntimeData AMSBaseProjectile::GetEffectiveRuntimeData() const
{
	FProjectileRuntimeData EffectiveData = ProjectileRuntimeData;

	if (!bRuntimeDataInitialized)
	{
		const UProjectileStaticData* StaticData =
			UMSFunctionLibrary::GetProjectileStaticData(ProjectileDataClass);
		EffectiveData.CopyFromStaticData(StaticData);
	}

	return EffectiveData;
}

void AMSBaseProjectile::SetCollisionRadius(float Radius)
{
	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(Radius);
	}
}

void AMSBaseProjectile::EnableCollision(bool bEnable)
{
	if (CollisionSphere)
	{
		CollisionSphere->SetGenerateOverlapEvents(bEnable);
		CollisionSphere->SetCollisionEnabled(
			bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision
		);
	}
}

void AMSBaseProjectile::AddIgnoredActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	IgnoredActors.Add(Actor);
}

bool AMSBaseProjectile::IsIgnoredActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	return IgnoredActors.Contains(Actor);
}

void AMSBaseProjectile::AddSimPathPoint(const FVector& Point)
{
	if (!HasAuthority())
	{
		return;
	}

	SimPathPoints.Add(Point);
	ForceNetUpdate();
}

void AMSBaseProjectile::ClearSimPathPoints()
{
	if (!HasAuthority())
	{
		return;
	}

	SimPathPoints.Reset();
	ForceNetUpdate();
}

void AMSBaseProjectile::PlaySFXAtLocation(int32 Index)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	Multicast_PlaySFXAtLocation_Implementation(Index, GetActorLocation());
}

void AMSBaseProjectile::PlaySFXAttached(int32 Index, USceneComponent* AttachTo)
{
	if (!AttachTo)
	{
		return;
	}
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	Multicast_PlaySFXAttached_Implementation(Index);
}

void AMSBaseProjectile::Multicast_PlaySFXAtLocation_Implementation(
	int32 Index,
	const FVector& Location
)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FProjectileRuntimeData EffectiveData = GetEffectiveRuntimeData();
	if (!EffectiveData.SFX.IsValidIndex(Index))
	{
		return;
	}

	USoundBase* Sound = EffectiveData.SFX[Index];
	if (!Sound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, 1.f);
}

void AMSBaseProjectile::Multicast_PlaySFXAttached_Implementation(int32 Index)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FProjectileRuntimeData EffectiveData = GetEffectiveRuntimeData();
	if (!EffectiveData.SFX.IsValidIndex(Index))
	{
		return;
	}

	USoundBase* Sound = EffectiveData.SFX[Index];
	if (!Sound)
	{
		return;
	}

	USceneComponent* AttachTo = GetRootComponent();
	if (!AttachTo)
	{
		return;
	}

	UGameplayStatics::SpawnSoundAttached(Sound, AttachTo);
}

void AMSBaseProjectile::Multicast_SpawnVFXAtLocation_Implementation(
	UNiagaraSystem* Vfx,
	const FVector& Location,
	float Scale
)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!Vfx)
	{
		return;
	}

	const float SafeScale = FMath::Max(0.01f, Scale);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		Vfx,
		Location,
		FRotator::ZeroRotator,
		FVector(SafeScale)
	);
}

void AMSBaseProjectile::Multicast_SpawnClientProjectiles_Implementation(
	TSubclassOf<UProjectileStaticData> DataClass,
	const FProjectileRuntimeData& BaseData,
	const FVector& Origin,
	const TArray<FVector_NetQuantizeNormal>& Directions
)
{
	if (HasAuthority())
	{
		return;
	}
	if (!DataClass)
	{
		DataClass = ProjectileDataClass;
	}
	if (!DataClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const UProjectileStaticData* StaticData = UMSFunctionLibrary::GetProjectileStaticData(DataClass);

	for (const FVector_NetQuantizeNormal& Dir : Directions)
	{
		FVector LaunchDir = FVector(Dir);
		if (LaunchDir.IsNearlyZero())
		{
			continue;
		}

		FProjectileRuntimeData Data = BaseData;
		if (!Data.StaticMesh && StaticData)
		{
			Data.StaticMesh = StaticData->StaticMesh;
		}
		if (!Data.OnAttachVFX && StaticData)
		{
			Data.OnAttachVFX = StaticData->OnAttachVFX;
		}
		Data.Direction = LaunchDir;

		const float SpawnOffset = 50.f;
		const FVector SpawnLocation = Origin + (LaunchDir * SpawnOffset);
		const FTransform SpawnTransform(LaunchDir.Rotation(), SpawnLocation);

		AMSBaseProjectile* SplitProjectile = World->SpawnActorDeferred<AMSBaseProjectile>(
			AMSBaseProjectile::StaticClass(),
			SpawnTransform,
			GetOwner(),
			GetInstigator(),
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

		if (!SplitProjectile)
		{
			continue;
		}

		SplitProjectile->SetReplicates(false);
		SplitProjectile->SetReplicateMovement(false);
		SplitProjectile->ProjectileDataClass = DataClass;
		SplitProjectile->SetProjectileRuntimeData(Data);
		SplitProjectile->FinishSpawning(SpawnTransform);

		const float LifeTime = (Data.LifeTime > 0.f) ? Data.LifeTime : 3.f;
		SplitProjectile->SetLifeSpan(LifeTime);
	}
}

void AMSBaseProjectile::Multicast_ChainBoltStep_Implementation(
	const FVector& Start,
	const FVector& Target,
	float Speed,
	float Interval,
	int32 StepId
)
{
	if (HasAuthority())
	{
		return;
	}

	if (UMSPB_ChainBolt* ChainBehavior = Cast<UMSPB_ChainBolt>(Behavior))
	{
		ChainBehavior->ClientReceiveChainStep(Start, Target, Speed, Interval, StepId);
	}
}

void AMSBaseProjectile::Multicast_ServerStop_Implementation(const FVector& InLocation)
{
	if (HasAuthority())
	{
		return;
	}

	SetActorLocation(InLocation);
	StopMovement();
	EnableCollision(false);
	SetActorHiddenInGame(true);
}
void AMSBaseProjectile::StopMovement()
{
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	}
}

FVector AMSBaseProjectile::GetClientSimCorrectionOffset(float DeltaSeconds, const FVector& SimulatedLocation)
{
	if (!bHasSimCorrection || DeltaSeconds <= 0.f)
	{
		return FVector::ZeroVector;
	}

	SimCorrectionAlpha = FMath::Min(1.f, SimCorrectionAlpha + (DeltaSeconds / SimCorrectionDuration));
	const FVector DesiredLocation = FMath::Lerp(SimCorrectionStart, SimCorrectionTarget, SimCorrectionAlpha);
	const FVector Offset = DesiredLocation - SimulatedLocation;

	if (SimCorrectionAlpha >= 1.f)
	{
		bHasSimCorrection = false;
	}

	return Offset;
}

void AMSBaseProjectile::TriggerSplitEvent(
	const FVector& Origin,
	const FVector& DirA,
	const FVector& DirB,
	int32 PenetrationCount
)
{
	if (!HasAuthority())
	{
		return;
	}

	SplitEvent.bValid = true;
	SplitEvent.Origin = Origin;
	SplitEvent.DirA = DirA;
	SplitEvent.DirB = DirB;
	SplitEvent.NumDirs = DirB.IsNearlyZero() ? 1 : 2;
	SplitEvent.PenetrationCount = PenetrationCount;
	SplitEventId++;

	ForceNetUpdate();
}

float AMSBaseProjectile::GetClientSimStartTime() const
{
	if (HasAuthority())
	{
		return SimStartServerTime;
	}

	if (!bClientSimEnabled || SimStartServerTime <= 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			return World->GetTimeSeconds();
		}
		return 0.f;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* GS = World ? World->GetGameState() : nullptr;
	if (!GS)
	{
		return World ? World->GetTimeSeconds() : 0.f;
	}

	const float ServerNow = GS->GetServerWorldTimeSeconds();
	const float Elapsed = FMath::Max(0.f, ServerNow - SimStartServerTime);
	return World->GetTimeSeconds() - Elapsed;
}

void AMSBaseProjectile::UpdateSimServerLocation()
{
	if (!HasAuthority())
	{
		return;
	}

	SimServerLocation = GetActorLocation();
	ForceNetUpdate();
}

void AMSBaseProjectile::RequestDestroy()
{
	if (bDestroyRequested)
	{
		return;
	}
	bDestroyRequested = true;

	if (HasAuthority())
	{
		bServerStop = true;
		SimServerLocation = GetActorLocation();
		Multicast_ServerStop(SimServerLocation);
		ForceNetUpdate();
	}

	Destroy();
}

void AMSBaseProjectile::SpawnAttachVFXOnce()
{
	if (bAttachVfxSpawned)
	{
		return;
	}

	const FProjectileRuntimeData EffectiveData = GetEffectiveRuntimeData();
	if (EffectiveData.OnAttachVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			EffectiveData.OnAttachVFX,
			GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false
		);

		bAttachVfxSpawned = true;
	}
}

void AMSBaseProjectile::Multicast_StopAndHide_Implementation(
	const FVector& InLocation
)
{
	SetActorLocation(InLocation);
	StopMovement();
	EnableCollision(false);
	SetActorHiddenInGame(true);
}

void AMSBaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서 RuntimeData가 아직 없으면 StaticData로 초기화
	if (HasAuthority() && !bRuntimeDataInitialized)
	{
		InitProjectileRuntimeDataFromClass(ProjectileDataClass);
	}

	// 런타임 데이터 적용
	ApplyProjectileRuntimeData(true);

	if (HasAuthority())
	{
		const FProjectileRuntimeData EffectiveData = GetEffectiveRuntimeData();
		bClientSimEnabled = true;
		SimStartServerTime = GetWorld()->GetTimeSeconds();
		SimStartLocation = GetActorLocation();
		SimDirection = EffectiveData.Direction.IsNearlyZero()
			? GetActorForwardVector()
			: EffectiveData.Direction.GetSafeNormal();
		SimSpeed = EffectiveData.ProjectileSpeed;
		SimNoiseSeed = FMath::Rand();

		SetNetUpdateFrequency(3.33f);
		SetMinNetUpdateFrequency(3.33f);

		SimServerLocation = GetActorLocation();
		GetWorldTimerManager().SetTimer(
			SimCorrectionTimerHandle,
			this,
			&AMSBaseProjectile::UpdateSimServerLocation,
			0.5f,
			true
		);

		SetReplicateMovement(false);
	}

	// 서버/클라 모두 Behavior 보장
	EnsureBehavior();

	// 서버 LifeTime 타이머
	if (HasAuthority())
	{
		ArmLifeTimerIfNeeded(GetEffectiveRuntimeData());
	}

	// UE_LOG(
	// 	LogTemp, Warning,
	// 	TEXT("[Projectile] BeginPlay %s Auth=%d"),
	// 	*GetName(), HasAuthority()
	// );
}

void AMSBaseProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 클라이언트에서도 종료 처리를 통해 루프 SFX 등을 정리.
	if (Behavior)
	{
		Behavior->OnEnd();
	}

	// 서버 타이머 정리
	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(LifeTimerHandle);
			World->GetTimerManager().ClearTimer(SimCorrectionTimerHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AMSBaseProjectile::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSBaseProjectile, ProjectileDataClass);
	DOREPLIFETIME(AMSBaseProjectile, bRuntimeDataInitialized);
	DOREPLIFETIME(AMSBaseProjectile, ProjectileRuntimeData);
	DOREPLIFETIME(AMSBaseProjectile, SplitEvent);
	DOREPLIFETIME(AMSBaseProjectile, SplitEventId);
	DOREPLIFETIME(AMSBaseProjectile, bServerStop);
	DOREPLIFETIME(AMSBaseProjectile, bClientSimEnabled);
	DOREPLIFETIME(AMSBaseProjectile, SimStartServerTime);
	DOREPLIFETIME(AMSBaseProjectile, SimStartLocation);
	DOREPLIFETIME(AMSBaseProjectile, SimDirection);
	DOREPLIFETIME(AMSBaseProjectile, SimSpeed);
	DOREPLIFETIME(AMSBaseProjectile, SimNoiseSeed);
	DOREPLIFETIME(AMSBaseProjectile, SimPathPoints);
	DOREPLIFETIME(AMSBaseProjectile, SimServerLocation);
}

void AMSBaseProjectile::EnsureBehavior()
{
	const FProjectileRuntimeData EffectiveData = GetEffectiveRuntimeData();

	TSubclassOf<UMSProjectileBehaviorBase> ClassToUse =
		EffectiveData.BehaviorClass;

	if (!ClassToUse)
	{
		ClassToUse = UMSPB_Normal::StaticClass(); // fallback
	}

	if (Behavior)
	{
		if (Behavior->GetClass() == ClassToUse)
		{
			return;
		}

		Behavior->OnEnd();
		Behavior = nullptr;
	}

	Behavior = NewObject<UMSProjectileBehaviorBase>(this, ClassToUse);

	if (Behavior)
	{
		Behavior->Initialize(this, EffectiveData);
		Behavior->ApplyCollisionRadius(this, EffectiveData);
		Behavior->OnBegin();
	}
}

void AMSBaseProjectile::OnHitOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (IsIgnoredActor(OtherActor))
	{
		return;
	}

	EnsureBehavior();
	if (!Behavior)
	{
		return;
	}

	Behavior->OnTargetEnter(OtherActor, SweepResult);
}

void AMSBaseProjectile::OnProjectileStop(
	const FHitResult& ImpactResult
)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Behavior)
	{
		Behavior->OnEnd();
	}

	RequestDestroy();
}

void AMSBaseProjectile::ApplyProjectileRuntimeData(bool bSpawnAttachVFX)
{
	const FProjectileRuntimeData EffectiveData = GetEffectiveRuntimeData();

	// Mesh
	if (ProjectileMesh && EffectiveData.StaticMesh)
	{
		ProjectileMesh->SetStaticMesh(EffectiveData.StaticMesh);
	}

	// Radius (handled by Behavior)
	if (Behavior)
	{
		Behavior->ApplyCollisionRadius(this, EffectiveData);
	}

	// Mesh Scale
	if (ProjectileMesh)
	{
		// 기존 코드 유지: Radius 값으로 스케일 조정
		ProjectileMesh->SetWorldScale3D(FVector(EffectiveData.Radius));
	}

	// Attach VFX
	if (bSpawnAttachVFX)
	{
		SpawnAttachVFXOnce();
	}
}

void AMSBaseProjectile::ArmLifeTimerIfNeeded(
	const FProjectileRuntimeData& EffectiveData
)
{
	if (EffectiveData.LifeTime <= 0.f)
	{
		return; // 무한 지속
	}

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<AMSBaseProjectile> WeakSelf(this);

		World->GetTimerManager().ClearTimer(LifeTimerHandle);
		World->GetTimerManager().SetTimer(
			LifeTimerHandle,
			FTimerDelegate::CreateLambda([WeakSelf]()
			{
				if (WeakSelf.IsValid())
				{
					if (WeakSelf->HasAuthority() && WeakSelf->Behavior)
					{
						WeakSelf->Behavior->OnEnd();
					}
					WeakSelf->RequestDestroy();
				}
			}),
			EffectiveData.LifeTime,
			false
		);
	}
}

void AMSBaseProjectile::OnRep_ProjectileRuntimeData()
{
	// 런타임 데이터 리플리케이션 시 클라이언트 반영
	ApplyProjectileRuntimeData(true);
	if (Behavior)
	{
		Behavior->OnEnd();
		Behavior = nullptr;
	}
	if (!HasAuthority() && bClientSimEnabled)
	{
		SetReplicateMovement(false);
	}
	EnsureBehavior();
}

void AMSBaseProjectile::OnRep_SplitEvent()
{
	if (HasAuthority())
	{
		return;
	}

	if (SplitEventId <= LastHandledSplitEventId)
	{
		return;
	}
	LastHandledSplitEventId = SplitEventId;

	if (!SplitEvent.bValid)
	{
		return;
	}

	TSubclassOf<UProjectileStaticData> DataClass = ProjectileDataClass;
	if (!DataClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FProjectileRuntimeData SplitData = GetEffectiveRuntimeData();
	SplitData.PenetrationCount = SplitEvent.PenetrationCount;
	SplitData.BehaviorClass = UMSPB_Normal::StaticClass();

	const int32 NumDirs = FMath::Clamp(static_cast<int32>(SplitEvent.NumDirs), 1, 2);
	for (int32 Index = 0; Index < NumDirs; ++Index)
	{
		const FVector Dir = (Index == 0) ? FVector(SplitEvent.DirA) : FVector(SplitEvent.DirB);
		if (Dir.IsNearlyZero())
		{
			continue;
		}

		FVector LaunchDir = Dir.GetSafeNormal();
		SplitData.Direction = LaunchDir;

		const float SpawnOffset = 50.f;
		const FVector SpawnLocation = FVector(SplitEvent.Origin) + (LaunchDir * SpawnOffset);
		const FTransform SpawnTransform(LaunchDir.Rotation(), SpawnLocation);

		AMSBaseProjectile* SplitProjectile = World->SpawnActorDeferred<AMSBaseProjectile>(
			AMSBaseProjectile::StaticClass(),
			SpawnTransform,
			GetOwner(),
			GetInstigator(),
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

		if (!SplitProjectile)
		{
			continue;
		}

		SplitProjectile->SetReplicates(false);
		SplitProjectile->SetReplicateMovement(false);
		SplitProjectile->ProjectileDataClass = DataClass;
		SplitProjectile->SetProjectileRuntimeData(SplitData);
		SplitProjectile->EnableCollision(false);
		SplitProjectile->FinishSpawning(SpawnTransform);

		const float LifeTime = (SplitData.LifeTime > 0.f) ? SplitData.LifeTime : 3.f;
		SplitProjectile->SetLifeSpan(LifeTime);
	}
}

void AMSBaseProjectile::OnRep_SimServerLocation()
{
	if (HasAuthority())
	{
		return;
	}

	SimCorrectionStart = GetActorLocation();
	SimCorrectionTarget = SimServerLocation;
	SimCorrectionAlpha = 0.f;
	bHasSimCorrection = true;
}

void AMSBaseProjectile::OnRep_ClientSimEnabled()
{
	if (HasAuthority())
	{
		return;
	}

	if (!bClientSimEnabled)
	{
		return;
	}

	SetReplicateMovement(false);

	if (Behavior)
	{
		Behavior->OnEnd();
		Behavior = nullptr;
	}

	EnsureBehavior();
}
void AMSBaseProjectile::OnRep_ServerStop()
{
	if (!bServerStop)
	{
		return;
	}

	SetActorLocation(SimServerLocation);
	StopMovement();
	EnableCollision(false);
	SetActorHiddenInGame(true);
}


