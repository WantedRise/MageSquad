
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
 * 발사체가 가지는 데이터
 */
UCLASS(BlueprintType, Blueprintable)
class MAGESQUAD_API UProjectileStaticData : public UObject
{
	GENERATED_BODY()

public:
	// 발사체 대미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float BaseDamage;

	// 피해 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float DamageRadius;

	// 중력 영향력 [ 0 = 중력 없음, 1 = 기본 중력 ]
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float GravityMultiplayer = 0.f;

	// 초기 발사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float InitialSpeed = 1000.f;

	// 최대 발사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float MaxSpeed = 1000.f;

	// 최대 발사 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float LifeTime = 3.f;

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
