#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/MageSquadTypes.h"
#include "Engine/EngineTypes.h"
#include "MSProjectileBehaviorBase.generated.h"

class AMSBaseProjectile;

/**
*  작성자: 박세찬
 * 작성일: 25/12/17
 *
 * 투사체 행동 베이스
 * 투사체의 행동을 스킬 유형에 맞게 정의하기 위한 클래스
 */

UCLASS(Abstract, BlueprintType, Blueprintable)
class MAGESQUAD_API UMSProjectileBehaviorBase : public UObject
{
    GENERATED_BODY()

public:
    /** 소유자(공격 액터) + 런타임 데이터 초기화 */
    virtual void Initialize(AMSBaseProjectile* InOwner, const FProjectileRuntimeData& InRuntimeData);

    /** 시작 시 호출 (BeginPlay 이후, 세팅 완료 후) */
    UFUNCTION(BlueprintNativeEvent)
    void OnBegin();

    virtual void OnBegin_Implementation() {}

    /** 타겟이 공격 범위/충돌에 들어왔을 때 */
    UFUNCTION(BlueprintNativeEvent)
    void OnTargetEnter(AActor* Target, const FHitResult& HitResult);

    virtual void OnTargetEnter_Implementation(AActor* Target, const FHitResult& HitResult) {}

    /** 타겟이 공격 범위에서 나갔을 때 (장판/지속형에서 주로 사용) */
    UFUNCTION(BlueprintNativeEvent)
    void OnTargetExit(AActor* Target);

    virtual void OnTargetExit_Implementation(AActor* Target) {}

    /** 수명 만료/파괴 직전 등 종료 시점에 호출 (선택) */
    UFUNCTION(BlueprintNativeEvent)
    void OnEnd();

    virtual void OnEnd_Implementation() {}

public:
    /** 런타임 데이터 Getter */
    const FProjectileRuntimeData& GetRuntimeData() const { return RuntimeData; }

    /** 소유자 Getter */
    AMSBaseProjectile* GetOwnerActor() const { return Owner.Get(); }

protected:
    /** 서버에서만 처리해야 하는 로직인지 체크 */
    bool IsAuthority() const;

    /** 월드 접근 헬퍼 */
    UWorld* GetWorldSafe() const;

protected:
    UPROPERTY(Transient)
    TWeakObjectPtr<AMSBaseProjectile> Owner;

    UPROPERTY(Transient)
    FProjectileRuntimeData RuntimeData;
};