// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Mission/MSMissionFindTargetActor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Components/MSDirectionIndicatorComponent.h"
#include <Kismet/GameplayStatics.h>

AMSMissionFindTargetActor::AMSMissionFindTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);

	Collision->SetSphereRadius(80.f);
	Collision->SetCollisionProfileName(TEXT("MSObstacleWave"));

	DirectionIndicatorComponent = CreateDefaultSubobject<UMSDirectionIndicatorComponent>(TEXT("DirectionIndicatorComponent"));
}

void AMSMissionFindTargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	const float OffsetZ = FMath::Sin(RunningTime * FloatSpeed) * FloatHeight;

	SetActorLocation(BaseLocation + FVector(0.f, 0.f, OffsetZ));
}


void AMSMissionFindTargetActor::BeginPlay()
{
	Super::BeginPlay();

	BaseLocation = GetActorLocation();

	if (HasAuthority())
	{
		Collision->OnComponentBeginOverlap.AddDynamic(this,&AMSMissionFindTargetActor::OnOverlapBegin);
	}


	// 플레이어 방향 표시 인디케이터 설정
	if (DirectionIndicatorComponent)
	{
		// 거리 표기 비활성화
		DirectionIndicatorComponent->bShowDistance = false;
	}
}

void AMSMissionFindTargetActor::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!HasAuthority())
		return;

	if (OtherActor && OtherActor->IsA<ACharacter>())
	{
		MulticastPlaySound();

		// 수집 이벤트 브로드캐스트
		OnTargetCollected.Broadcast();
		Destroy();
	}
}

void AMSMissionFindTargetActor::MulticastPlaySound_Implementation()
{
	if (StartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, StartSound, GetActorLocation());
	}

	// 추가로 파티클 효과가 있다면 여기서 함께 처리하면 좋습니다.
	// UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupEffect, GetActorLocation());
}