// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/MSBaseProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Net/UnrealNetwork.h"

#include "MSFunctionLibrary.h"

AMSBaseProjectile::AMSBaseProjectile()
{
	// Tick 비활성화
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 리플리케이션 활성화
	bReplicates = true;
	SetReplicateMovement(true);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(GetRootComponent());
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

const UProjectileStaticData* AMSBaseProjectile::GetProjectileStaticData() const
{
	if (IsValid(ProjectileDataClass))
	{
		// 클래스의 CDO 반환
		// 클래스의 기본 객체를 반환하므로, 여러 개의 발사체가 같은 데이터 객체를 공유
		// 즉, 메모리에 새로운 객체를 생성하지 않고, 미리 존재하는 기본 객체를 사용
		return GetDefault<UProjectileStaticData>(ProjectileDataClass);
	}
	return nullptr;
}

void AMSBaseProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 발사체 데이터 가져오기
	const UProjectileStaticData* ProjectileData = GetProjectileStaticData();

	if (ProjectileData && ProjectileMovementComponent)
	{
		// 발사체 메쉬 설정
		if (ProjectileData->StaticMesh)
		{
			ProjectileMesh->SetStaticMesh(ProjectileData->StaticMesh);
			SetRootComponent(ProjectileMesh);
		}

		// 발사체의 초기 속도를 월드 좌표계 기준으로 적용
		ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;

		// 발사체의 초기 속도와 최대 속도를 설정
		ProjectileMovementComponent->InitialSpeed = ProjectileData->InitialSpeed;
		ProjectileMovementComponent->MaxSpeed = ProjectileData->MaxSpeed;

		// 발사체의 방향이 이동하는 속도 벡터를 따라가도록 설정
		// 발사체가 이동 방향에 맞춰 자동으로 회전
		ProjectileMovementComponent->bRotationFollowsVelocity = true;

		// 충돌 시 튕기지 않도록 설정, 반발 계수를 0으로 설정하여 완전히 충돌 후 정지
		ProjectileMovementComponent->bShouldBounce = false;
		ProjectileMovementComponent->Bounciness = 0.f;

		// 발사체의 중력 영향력을 설정
		ProjectileMovementComponent->ProjectileGravityScale = ProjectileData->GravityMultiplayer;

		// 발사체의 초기 속도를 설정
		// 발사체가 생성될 때 바라보는 방향으로 초기 속도만큼의 속도를 적용
		ProjectileMovementComponent->Velocity = ProjectileData->InitialSpeed * GetActorForwardVector();

		// 부착 나이아가라 재생
		UNiagaraFunctionLibrary::SpawnSystemAttached(ProjectileData->OnAttachVFX, GetRootComponent(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);

		//FTimerHandle LifeTimeTimer;
		//// 서버에서 생명주기 타이머 관리
		//if (HasAuthority())
		//{
		//	GetWorld()->GetTimerManager().SetTimer(LifeTimeTimer,
		//		FTimerDelegate::CreateLambda(
		//			[this]()
		//			{
		//				Destroy();
		//			}
		//		), ProjectileData, false
		//	);
		//}
	}
}

void AMSBaseProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 발사체 데이터 가져오기
	const UProjectileStaticData* ProjectileData = GetProjectileStaticData();

	if (ProjectileData)
	{
		// 폭발 사운드 재생
		UGameplayStatics::PlaySoundAtLocation(this, ProjectileData->OnHitSFX, GetActorLocation(), 1.f);

		// 폭발 나이아가라 재생
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ProjectileData->OnHitVFX, GetActorLocation());
	}

	Super::EndPlay(EndPlayReason);
}

void AMSBaseProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSBaseProjectile, ProjectileDataClass);
}

void AMSBaseProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	// 발사체 데이터 가져오기
	const UProjectileStaticData* ProjectileData = GetProjectileStaticData();

	if (ProjectileData)
	{
		// 대미지 로직
	}

	// 발사체 파괴 (EndPlay 호출)
	Destroy();
}
