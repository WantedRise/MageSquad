// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/MSBaseProjectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMSBaseProjectile::AMSBaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = false; // 리플리케이트 안 함. RPC를 통해 동기화

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	CollisionComp->SetBoxExtent(FVector(10.f, 10.f, 10.f));
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AMSBaseProjectile::OnProjectileOverlap);
	RootComponent = CollisionComp;

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->ProjectileGravityScale = 0.f;
	MovementComp->bAutoActivate = false;

	// 액터 태그 설정
	Tags.AddUnique(TEXT("Projectile"));
}

void AMSBaseProjectile::InitProjectile(const FTransform& SpawnTransform, const FVector_NetQuantize& Direction, float Speed, float LifeTime)
{
	LifeDuration = LifeTime;
	SetActorTransform(SpawnTransform);
	MovementComp->Velocity = Direction * Speed;
	MovementComp->Activate();

	// 서버에서 자동 삭제(발사체 풀로 반환)를 예약
	// 클라이언트는 서버의 RPC를 통해 삭제 작업 수행
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(LifeTimerHandle,
			FTimerDelegate::CreateLambda(
				[&]()
				{
					// 발사체 생명주기 종료 이벤트 호출
					// 발사체 풀링 시스템에 의해 관리
					OnProjectileFinished.Broadcast(this);
				}
			), LifeDuration, false
		);
	}
}

void AMSBaseProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AMSBaseProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (OtherActor && OtherActor != this)
		{
			// TODO: 김준형 | 발사체 대미지 로직
			// 대미지 적용하기. 일단 로그만
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(0, 10.f, FColor::Blue, FString("Projectile Hit"));
			}

			// 발사체 생명주기 종료 이벤트 호출
			// 발사체 풀링 시스템에 의해 관리
			OnProjectileFinished.Broadcast(this);
		}
	}
}
