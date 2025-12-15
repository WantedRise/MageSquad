
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MageSquadTypes.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/15
 *
 * MageSquad에 사용될 여러 타입 구조체들을 정의하는 클래스
 */

 /**
  * 작성자: 김준형
  * 작성일: 25/12/15
  *
  * 플레이어 시작 어빌리티 / 게임플레이 이펙트 구조체
  * Abilities : 플레이어가 소유하게 될 UGameplayAbility 클래스 목록
  * Effects   : 시작 시 한 번 적용되는 UGameplayEffect 클래스 목록
  */
USTRUCT(BlueprintType)
struct MAGESQUAD_API FPlayerStartAbilityData
{
	GENERATED_BODY()

public:
	// 플레이어가 시작 시 부여받는 어빌리티 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom | StartUp")
	TArray<TSubclassOf<class UGameplayAbility>> Abilties;

	// 플레이어가 시작 시 한 번 적용되는 게임플레이 이펙트 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom | StartUp")
	TArray<TSubclassOf<class UGameplayEffect>> Effects;
};


/**
 * 작성자: 김준형
 * 작성일: 25/12/15
 *
 * 발사체의 원본 데이터
 * 각 발사체마다 생성
 */
UCLASS(BlueprintType, Blueprintable)
class MAGESQUAD_API UProjectileStaticData : public UObject
{
	GENERATED_BODY()

public:
	// 발사체 대미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float Damage = 0;

	// 발사체 크기(피해 범위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float Radius = 1.f;

	// 방향
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	FVector Direction = FVector::ZeroVector;

	// 중력 영향력 [ 0 = 중력 없음, 1 = 기본 중력 ]
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float GravityMultiplayer = 0.f;

	// 초기 발사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float InitialSpeed = 1000.f;

	// 최대 발사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float MaxSpeed = 1000.f;

	// 생명 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float LifeTime = 3.f;

	// 자동 발사 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	bool bIsAutoAttack = true;

	// 자동 발사 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile", meta = (EditCondition = "bIsAutoAttack"))
	float AutoAttackInterval = 1.f;

	// 발사체 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	UStaticMesh* StaticMesh;

	// 적용 게임플레이 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TArray<TSubclassOf<class UGameplayEffect>> Effects;

	// 부착 VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	class UNiagaraSystem* OnAttachVFX = nullptr;

	// 피격 VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	class UNiagaraSystem* OnHitVFX = nullptr;

	// 피격 SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	USoundBase* OnHitSFX = nullptr;
};


/**
 * 작성자: 김준형
 * 작성일: 25/12/15
 *
 * 발사체의 런타임 데이터
 * 발사체 원본 데이터를 복제하여 런타임에 재정의되는 데이터
 * 발사체는 실제로 이 데이터를 사용. 발사체를 생성하는 쪽에서 자유롭게 수정 가능
 */
USTRUCT(BlueprintType)
struct MAGESQUAD_API FProjectileRuntimeData
{
	GENERATED_BODY()

public:
	// 발사체의 원본 데이터를 기준으로 모든 데이터 복사하는 함수
	void CopyFromStaticData(const UProjectileStaticData* StaticData)
	{
		if (!StaticData) return;

		Damage = StaticData->Damage;
		Radius = StaticData->Radius;
		Direction = StaticData->Direction;
		InitialSpeed = StaticData->InitialSpeed;
		GravityMultiplayer = StaticData->GravityMultiplayer;
		InitialSpeed = StaticData->InitialSpeed;
		MaxSpeed = StaticData->MaxSpeed;
		LifeTime = StaticData->LifeTime;
		bIsAutoAttack = StaticData->bIsAutoAttack;
		AutoAttackInterval = StaticData->AutoAttackInterval;
		StaticMesh = StaticData->StaticMesh;
		Effects = StaticData->Effects;
		OnAttachVFX = StaticData->OnAttachVFX;
		OnHitVFX = StaticData->OnHitVFX;
		OnHitSFX = StaticData->OnHitSFX;
	}

public:
	// 발사체 대미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float Damage = 0;

	// 발사체 크기(피해 범위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float Radius;

	// 방향
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	FVector Direction = FVector::ZeroVector;

	// 중력 영향력 [ 0 = 중력 없음, 1 = 기본 중력 ]
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float GravityMultiplayer = 0.f;

	// 초기 발사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float InitialSpeed = 1000.f;

	// 최대 발사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float MaxSpeed = 1000.f;

	// 생명 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float LifeTime = 3.f;

	// 자동 발사 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	bool bIsAutoAttack = true;

	// 자동 발사 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile", meta = (EditCondition = "bIsAutoAttack"))
	float AutoAttackInterval = 1.f;

	// 발사체 메쉬
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	UStaticMesh* StaticMesh;

	// 적용 게임플레이 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TArray<TSubclassOf<class UGameplayEffect>> Effects;

	// 부착 VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	class UNiagaraSystem* OnAttachVFX = nullptr;

	// 피격 VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	class UNiagaraSystem* OnHitVFX = nullptr;

	// 피격 SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	USoundBase* OnHitSFX = nullptr;
};
