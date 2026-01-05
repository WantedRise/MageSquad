#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "MSIndicatorTypes.h"
#include "Particles/ParticleSystem.h"
#include "SkillData/MSSkillList.h"
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
	// 초기 스탯 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FText> InitialStatInfo;
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
	void InitFromRow(const FMSSkillList& InRow, int32 InSlotIndex)
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
	FMSSkillList SkillRow;
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
 * 블링크 전달용 게임플레이 이펙트 콘텍스트
 * 
 * 수정자 : 임희섭
 * 수정일 : 2025/12/31
 * Indicator용 데이터 추가
 */
USTRUCT(BlueprintType)
struct MAGESQUAD_API FMSGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
#pragma region Blink Section
	// GameplayCue용 컬러 파라미터 (LinearColor)
	UPROPERTY()
	FLinearColor CueColor = FLinearColor::White;

	// Blink(텔레포트) 세그먼트 시작/끝 (Beam 연출용)
	UPROPERTY()
	FVector BlinkStart = FVector::ZeroVector;

	UPROPERTY()
	FVector BlinkEnd = FVector::ZeroVector;

	// BlinkStart/BlinkEnd 값이 유효한지 여부
	UPROPERTY()
	bool bHasBlinkSegment = false;

	FORCEINLINE void SetBlinkSegment(const FVector& InStart, const FVector& InEnd)
	{
		BlinkStart = InStart;
		BlinkEnd = InEnd;
		bHasBlinkSegment = true;
	}

	FORCEINLINE bool HasBlinkSegment() const { return bHasBlinkSegment; }
#pragma endregion 
	
#pragma region Indicator Section
	FAttackIndicatorParams IndicatorParams;

	UPROPERTY()
	bool bHasIndicatorParams = false;
	
	FORCEINLINE void SetIndicatorParams(const FAttackIndicatorParams& InParams)
	{
		IndicatorParams = InParams;
		bHasIndicatorParams = true;
	}
	
	FORCEINLINE const FAttackIndicatorParams& GetIndicatorParams() const { return IndicatorParams; }
	FORCEINLINE bool HasIndicatorParams() const { return bHasIndicatorParams; }
#pragma endregion

#pragma region Effect Asset Section
	UPROPERTY()
	TObjectPtr<UParticleSystem> ParticleAsset = nullptr;

	UPROPERTY()
	TObjectPtr<USoundBase> SoundAsset = nullptr;

	bool bHasEffectAssets = false;

	FORCEINLINE void SetEffectAssets(UParticleSystem* InParticle, USoundBase* InSound)
	{
		ParticleAsset = InParticle;
		SoundAsset = InSound;
		bHasEffectAssets = (InParticle || InSound);
	}
#pragma endregion
	
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	/**
	 * 컨텍스트 핸들 복제 시 파생 타입이 유지되도록 Duplicate 오버라이드
	 * (HitResult 등 내부 포인터도 안전 복사)
	 */
	virtual FGameplayEffectContext* Duplicate() const override
	{
		FMSGameplayEffectContext* NewCtx = new FMSGameplayEffectContext();
		*NewCtx = *this;
		if (GetHitResult())
		{
			NewCtx->AddHitResult(*GetHitResult(), true);
		}
		return NewCtx;
	}

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

		// 추가 데이터는 최소화된 플래그 + 조건부 직렬화
		uint8 Flags = 0;
		if (Ar.IsSaving())
		{
			// 0x01: CueColor 포함
			Flags |= 0x01;
			// 0x02: Blink 세그먼트 포함
			if (bHasBlinkSegment)
			{
				Flags |= 0x02;
			}
			// 0x04: Indicator
			if (bHasIndicatorParams)
			{
				Flags |= 0x04;
			}
		}

		Ar.SerializeBits(&Flags, 4); // 2->4로 확장 - 임희섭

		if (Flags & 0x01)
		{
			Ar << CueColor;
		}
		else if (Ar.IsLoading())
		{
			CueColor = FLinearColor::White;
		}

		if (Flags & 0x02)
		{
			Ar << BlinkStart;
			Ar << BlinkEnd;
			bHasBlinkSegment = true;
		}
		else if (Ar.IsLoading())
		{
			BlinkStart = FVector::ZeroVector;
			BlinkEnd = FVector::ZeroVector;
			bHasBlinkSegment = false;
		}
		
		// Indicator
		if (Flags & 0x04)
		{
			uint8 ShapeValue = static_cast<uint8>(IndicatorParams.Shape);
			Ar << ShapeValue;
			if (Ar.IsLoading())
			{
				IndicatorParams.Shape = static_cast<EIndicatorShape>(ShapeValue);
			}

			Ar << IndicatorParams.Duration;

			switch (IndicatorParams.Shape)
			{
			case EIndicatorShape::Circle:
				Ar << IndicatorParams.Radius;
				break;
			case EIndicatorShape::Cone:
				Ar << IndicatorParams.Radius;
				Ar << IndicatorParams.Angle;
				break;
			case EIndicatorShape::Rectangle:
				Ar << IndicatorParams.Width;
				Ar << IndicatorParams.Length;
				break;
			}
			bHasIndicatorParams = true;
		}
		else if (Ar.IsLoading())
		{
			IndicatorParams = FAttackIndicatorParams();
			bHasIndicatorParams = false;
		}
		
		// EffectAsset
		if (Flags & 0x08)
		{
			// Object 포인터 직렬화 (네트워크 전송용)
			Ar << ParticleAsset; 
			Ar << SoundAsset;
			bHasEffectAssets = true;
		}
		return true;
	}
};

template <>
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
USTRUCT(BlueprintType)
struct FMSLevelUpChoicePair
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag SkillTag;

	UPROPERTY()
	FGameplayTag UpgradeTag;
};