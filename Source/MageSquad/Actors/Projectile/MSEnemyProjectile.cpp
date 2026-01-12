// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile/MSEnemyProjectile.h"

AMSEnemyProjectile::AMSEnemyProjectile()
{
	// sphere 콜리전 설정
	// 부모에서 MSEnemy에 반응하도록 설정됨 → MSPlayer로 변경
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);  // MSEnemy 무시
	CollisionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // MSPlayer
}
