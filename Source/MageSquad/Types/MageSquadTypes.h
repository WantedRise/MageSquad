
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "SkillData/MSSkillDataRow.h"
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
  * 플레이어 시작 스킬 데이터
  * [스킬 어빌리티 / 스킬ID / 스킬 레벨]
  */
USTRUCT(BlueprintType)
struct MAGESQUAD_API FStartSkillData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UGameplayAbility> SkillAbilty;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SkillId = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SkillLevel = 1;
};


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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | StartUp")
	TArray<TSubclassOf<class UGameplayAbility>> Abilties;

	// 시작 스킬 데이터 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | StartUp")
	TArray<FStartSkillData> StartSkillDatas;

	// 플레이어가 시작 시 한 번 적용되는 게임플레이 이펙트 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | StartUp")
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
 * 작성일: 25/12/20
 *
 * 스킬 슬롯 런타임 데이터
 * 스킬 데이터테이블의 행을 캐싱하여, 어빌리티가 매번 DT를 조회하지 않도록 함
 * GameplayEvent의 OptionalObject로 전달하기 위해 UObject를 상속
 */
UCLASS(BlueprintType)
class MAGESQUAD_API UMSSkillSlotRuntimeData : public UObject
{
	GENERATED_BODY()

public:
	// 스킬 행 초기화 함수
	void InitFromRow(const FMSSkillDataRow& InRow, int32 InSlotIndex)
	{
		SkillRow = InRow;
		SlotIndex = InSlotIndex;
	}

public:
	// 어떤 슬롯에서 발생한 이벤트인지(디버깅/확장용)
	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;

	// 해당 스킬의 최종 데이터(레벨 반영된 한 행)
	UPROPERTY(BlueprintReadOnly)
	FMSSkillDataRow SkillRow;
};


/**
 * 작성자: 김준형
 * 작성일: 25/12/20
 *
 * 네트워크로 전달되는 최소 스킬 슬롯 정보
 * 플레이어는 "어떤 스킬을 갖고 있는지"와 "쿨타임"만 알고, 실제 스킬 동작은 어빌리티에서 처리
 */
USTRUCT(BlueprintType)
struct MAGESQUAD_API FMSPlayerSkillSlotNet
{
	GENERATED_BODY()

	// 스킬 종류 (1: 자동(패시브), 2: 좌클릭 액티브, 3: 우클릭 액티브)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom | Skill")
	int32 SkillType = 0;

	// 스킬 ID. 스킬 식별용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom | Skill")
	int32 SkillID = 0;

	// 스킬 레벨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom | Skill")
	int32 SkillLevel = 0;

	// 스킬 어빌리티를 활성화할 이벤트 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom | Skill")
	FGameplayTag SkillEventTag;

	// 스킬의 원본 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom | Skill")
	float BaseCoolTime = 0.f;

	bool IsValid() const
	{
		return SkillID > 0 && SkillEventTag.IsValid();
	}
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

/**
 * 작성자: 박세찬
 * 작성일: 25/12/22
 *
 * 플레이어 스테이트에서 스킬 레벨업 후보를 저장할 때 사용하는 구조체
 */
USTRUCT()
struct FMSLevelUpChoicePair
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag SkillTag;

	UPROPERTY()
	FGameplayTag UpgradeTag;
};