// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/MSEnemyProjectile.h"

AMSEnemyProjectile::AMSEnemyProjectile()
{
	// sphere 콜리전 설정
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_GameTraceChannel2);     // MSProjectile
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // MSPlayer
	CollisionSphere->SetGenerateOverlapEvents(true);
}
