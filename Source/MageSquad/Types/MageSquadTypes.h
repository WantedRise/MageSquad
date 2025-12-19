
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
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

class UMSProjectileBehaviorBase;

UCLASS(BlueprintType, Blueprintable)
class MAGESQUAD_API UProjectileStaticData : public UObject
{
	GENERATED_BODY()

public:
	// 발사체 대미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float Damage = 0;

	// 치명타 확률
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float CriticalChance = 0.5f;

	// 다단히트 데미지 시퀀스: [a, b, c, d] 처럼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TArray<float> DamageSequence;

	// 다단히트 주기(초): 0.2
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float DamageInterval = 0.2f;

	// 데미지 적용 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// 발사체 크기(피해 범위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float Radius = 1.f;

	// 방향
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	FVector Direction = FVector::ZeroVector;

	// 투사체 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float ProjectileSpeed = 1000.f;

	// 생명 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float LifeTime = 3.f;

	// 관통 횟수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	int32 PenetrationCount = 0;

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

	// Behavior 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TSubclassOf<UMSProjectileBehaviorBase> BehaviorClass;
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
		CriticalChance = StaticData->CriticalChance;
		DamageSequence = StaticData->DamageSequence;
		DamageInterval = StaticData->DamageInterval;
		DamageEffect = StaticData->DamageEffect;
		Radius = StaticData->Radius;
		Direction = StaticData->Direction;
		ProjectileSpeed = StaticData->ProjectileSpeed;
		LifeTime = StaticData->LifeTime;
		PenetrationCount = StaticData->PenetrationCount;
		StaticMesh = StaticData->StaticMesh;
		Effects = StaticData->Effects;
		OnAttachVFX = StaticData->OnAttachVFX;
		OnHitVFX = StaticData->OnHitVFX;
		OnHitSFX = StaticData->OnHitSFX;
		BehaviorClass = StaticData->BehaviorClass;
	}

public:
	// 발사체 대미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float Damage = 0;

	// 치명타 확률
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float CriticalChance = 0.5f;

	// 다단히트 데미지 시퀀스: [a, b, c, d] 처럼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TArray<float> DamageSequence;

	// 다단히트 주기(초): 0.2
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float DamageInterval = 0.2f;

	// 데미지 적용 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// 발사체 크기(피해 범위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float Radius;

	// 방향
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	FVector Direction = FVector::ZeroVector;

	// 투사체 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float ProjectileSpeed = 1000.f;

	// 생명 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float LifeTime = 3.f;

	// 관통 횟수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	int32 PenetrationCount = 0;

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

	// Behavior 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TSubclassOf<UMSProjectileBehaviorBase> BehaviorClass;
};


/**
 * 작성자: 김준형
 * 작성일: 25/12/15
 *
 * 나이아가라에 FLinearColor를 넘겨주기 위한 게임플레이 이펙트 콘텍스트
 */
USTRUCT(BlueprintType)
struct MAGESQUAD_API FMSGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FLinearColor CueColor = FLinearColor::White;

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	// 복제/직렬화 지원
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
	{
		bOutSuccess = true;

		// 부모 직렬화
		if (!FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess))
		{
			bOutSuccess = false;
			return false;
		}

		Ar << CueColor;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FMSGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FMSGameplayEffectContext>
{
	enum { WithNetSerializer = true, WithCopy = true };
};
