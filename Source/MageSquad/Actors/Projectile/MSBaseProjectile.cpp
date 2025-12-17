// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/MSBaseProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"

#include "Net/UnrealNetwork.h"

#include "MSFunctionLibrary.h"

// Behavior
#include "Actors/Projectile/Behaviors/MSProjectileBehaviorBase.h"
#include "Actors/Projectile/Behaviors/MSProjectileBehavior_Normal.h"

AMSBaseProjectile::AMSBaseProjectile()
{
	// Tick 비활성화
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 리플리케이션 활성화
	bReplicates = true;
	SetReplicateMovement(true);

	// overlap 판정용 sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere.Get());
	
	// sphere 콜리전 설정
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_GameTraceChannel2);     // MSProjectile
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap); // MSEnemy
	CollisionSphere->SetGenerateOverlapEvents(true);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AMSBaseProjectile::OnHitOverlap);
	
	// 시각용 Mesh는 Sphere에 부착
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere.Get());
	ProjectileMesh->SetIsReplicated(true);
	ProjectileMesh->SetCollisionProfileName(TEXT("MSProjectile"));
	ProjectileMesh->bReceivesDecals = false;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &AMSBaseProjectile::OnProjectileStop);

	// 액터 태그 설정
	Tags.AddUnique(TEXT("Projectile"));
}

void AMSBaseProjectile::InitProjectileRuntimeDataFromClass(TSubclassOf<UProjectileStaticData> InProjectileDataClass)
{
	// 발사체 원본 데이터 초기화
	ProjectileDataClass = InProjectileDataClass;

	const UProjectileStaticData* StaticData = UMSFunctionLibrary::GetProjectileStaticData(ProjectileDataClass);
	ProjectileRuntimeData.CopyFromStaticData(StaticData);
	bRuntimeDataInitialized = true;
}

void AMSBaseProjectile::SetProjectileRuntimeData(const FProjectileRuntimeData& InRuntimeData)
{
	// 발사체 런타임 데이터 초기화
	ProjectileRuntimeData = InRuntimeData;
	bRuntimeDataInitialized = true;

	// 이미 실행 중인 경우 즉시 적용 (서버 측 지연 생성때문에 늦게 나온 경우)
	if (HasActorBegunPlay())
	{
		ApplyProjectileRuntimeData(false);
	}
}

void AMSBaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 런타임 데이터의 초기화가 되지 않은 경우
	// 발사체 원본 데이터를 복제하여 발사체 재정의 데이터를 초기화
	if (HasAuthority() && !bRuntimeDataInitialized)
	{
		InitProjectileRuntimeDataFromClass(ProjectileDataClass);
	}

	// 런타임 데이터 적용
	ApplyProjectileRuntimeData(true);
	
	// 서버에서만 Behavior 준비
	if (HasAuthority())
	{
		InitializeBehavior();
	}
}

void AMSBaseProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 수명 타이머 정리(중복/누수 방지)
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(LifeTimerHandle);
	}
	
	// 런타임 데이터 가져오기
	FProjectileRuntimeData EffectiveData = ProjectileRuntimeData;

	// 런타임 데이터가 초기화되지 않았으면, 원본 데이터를 가져와 런타임 데이터로 초기화
	if (!bRuntimeDataInitialized)
	{
		const UProjectileStaticData* StaticData = UMSFunctionLibrary::GetProjectileStaticData(ProjectileDataClass);
		EffectiveData.CopyFromStaticData(StaticData);
	}

	if (EffectiveData.OnHitVFX)
	{
		// 폭발 나이아가라 재생
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, EffectiveData.OnHitVFX, GetActorLocation());
	}

	if (EffectiveData.OnHitSFX)
	{
		// 폭발 사운드 재생
		UGameplayStatics::PlaySoundAtLocation(this, EffectiveData.OnHitSFX, GetActorLocation(), 1.f);

	}

	Super::EndPlay(EndPlayReason);
}

void AMSBaseProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSBaseProjectile, ProjectileDataClass);
	DOREPLIFETIME(AMSBaseProjectile, bRuntimeDataInitialized);
	DOREPLIFETIME(AMSBaseProjectile, ProjectileRuntimeData);
}

void AMSBaseProjectile::InitializeBehavior()
{
	if (Behavior) return;

	TSubclassOf<UMSProjectileBehaviorBase> ClassToUse = ProjectileRuntimeData.BehaviorClass;
	if (!ClassToUse)
	{
		ClassToUse = UMSProjectileBehavior_Normal::StaticClass(); // fallback
	}

	Behavior = NewObject<UMSProjectileBehaviorBase>(this, ClassToUse);

	if (Behavior)
	{
		Behavior->Initialize(this, ProjectileRuntimeData);
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

	if (!Behavior)
	{
		InitializeBehavior();
		if (!Behavior) return;
	}

	// Enemy 채널로 이미 거른 상태지만, 안전하게 nullptr만 방지
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	Behavior->OnTargetEnter(OtherActor, SweepResult);
}

void AMSBaseProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	if (HasAuthority() && Behavior)
	{
		Behavior->OnEnd();
	}
	// 발사체 파괴 (EndPlay 호출)
	Destroy();
}

void AMSBaseProjectile::ApplyProjectileRuntimeData(bool bSpawnAttachVFX)
{
	if (!ProjectileMovementComponent) return;

	// 런타임 데이터 가져오기
	FProjectileRuntimeData EffectiveData = ProjectileRuntimeData;

	// 런타임 데이터가 초기화되지 않았으면, 원본 데이터를 가져와 런타임 데이터로 초기화
	if (!bRuntimeDataInitialized)
	{
		const UProjectileStaticData* StaticData = UMSFunctionLibrary::GetProjectileStaticData(ProjectileDataClass);
		EffectiveData.CopyFromStaticData(StaticData);
	}

	// 스태틱 메시 초기화
	if (ProjectileMesh && EffectiveData.StaticMesh)
	{
		ProjectileMesh->SetStaticMesh(EffectiveData.StaticMesh);
	}

	// 범위는 CollisionSphere 반경으로 맞추기
	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(EffectiveData.Radius);
	}
	
	// 메시 크기 설정
	ProjectileMesh->SetWorldScale3D(FVector(EffectiveData.Radius));
		
	// 발사체 무브먼트 설정
	ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;
	ProjectileMovementComponent->InitialSpeed = EffectiveData.InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = EffectiveData.MaxSpeed;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->Bounciness = 0.f;
	ProjectileMovementComponent->ProjectileGravityScale = EffectiveData.GravityMultiplayer;

	// 발사체 방향 설정
	FVector Dir = EffectiveData.Direction.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector();
	}
	ProjectileMovementComponent->Velocity = EffectiveData.InitialSpeed * Dir;

	// 부착 VFX 부착(한 번만)
	if (bSpawnAttachVFX && EffectiveData.OnAttachVFX)
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
	}

	// 서버에서 생명주기 타이머 관리
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(LifeTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			LifeTimerHandle,
			FTimerDelegate::CreateLambda([this]()
			{
				Destroy();
			}),
			EffectiveData.LifeTime,
			false
		);
	}
}

void AMSBaseProjectile::OnRep_ProjectileRuntimeData()
{
	// 런타임 데이터 리플리케이션 시, VFX만 제외하고 런타임 데이터 적용
	ApplyProjectileRuntimeData(false);
}
