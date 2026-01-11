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

// Behavior
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "Actors/Projectile/Behaviors/MSPB_Normal.h"

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

void AMSBaseProjectile::PlaySFXAtLocation(int32 Index) const
{
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

	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), 1.f);
}

void AMSBaseProjectile::PlaySFXAttached(int32 Index, USceneComponent* AttachTo) const
{
	if (!AttachTo)
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

	UGameplayStatics::SpawnSoundAttached(Sound, AttachTo);
}
void AMSBaseProjectile::StopMovement()
{
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	}
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

	// 서버/클라 모두 Behavior 보장
	EnsureBehavior();

	// 서버 LifeTime 타이머
	if (HasAuthority())
	{
		ArmLifeTimerIfNeeded(GetEffectiveRuntimeData());
	}

	UE_LOG(
		LogTemp, Warning,
		TEXT("[Projectile] BeginPlay %s Auth=%d"),
		*GetName(), HasAuthority()
	);
}

void AMSBaseProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 서버 타이머 정리
	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(LifeTimerHandle);
		}
	}

	const FProjectileRuntimeData EffectiveData = GetEffectiveRuntimeData();
	if (EffectiveData.OnHitSFX)
	{
		// 히트 사운드 재생
		UGameplayStatics::PlaySoundAtLocation(
			this, EffectiveData.OnHitSFX, GetActorLocation(), 1.f
		);
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
}

void AMSBaseProjectile::EnsureBehavior()
{
	if (Behavior)
	{
		return;
	}

	const FProjectileRuntimeData EffectiveData = GetEffectiveRuntimeData();

	TSubclassOf<UMSProjectileBehaviorBase> ClassToUse =
		EffectiveData.BehaviorClass;

	if (!ClassToUse)
	{
		ClassToUse = UMSPB_Normal::StaticClass(); // fallback
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
	if (HasAuthority() && Behavior)
	{
		Behavior->OnEnd();
	}

	Destroy();
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
					WeakSelf->Destroy();
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
	EnsureBehavior();
}


