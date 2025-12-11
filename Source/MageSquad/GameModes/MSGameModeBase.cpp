// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSGameModeBase.h"
#include "System/MSProjectilePoolSystem.h"
#include "Actors/Projectile/MSBaseProjectile.h"

void AMSGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (UMSProjectilePoolSystem* ProjectilePoolSystem = GetGameInstance()->GetSubsystem<UMSProjectilePoolSystem>())
	{
		// 발사체 풀링 시스템 초기화
		// 기본 발사체 클래스 지정 + 200개 풀링
		ProjectilePoolSystem->Configure(AMSBaseProjectile::StaticClass(), 200);
	}
}
