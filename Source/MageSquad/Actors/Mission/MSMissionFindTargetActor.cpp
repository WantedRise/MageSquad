// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Mission/MSMissionFindTargetActor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"

AMSMissionFindTargetActor::AMSMissionFindTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);

	Collision->SetSphereRadius(80.f);
	Collision->SetCollisionProfileName(TEXT("MSObstacleWave"));
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
		// 수집 이벤트 브로드캐스트
		OnTargetCollected.Broadcast();
		Destroy();
	}
}