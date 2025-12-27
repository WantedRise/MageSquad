#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MSInkCleanerComponent.generated.h"

class AMSInkAreaActor;
class UPrimitiveComponent;

/**
 * 플레이어/스킬이 잉크 영역을 문질러 정화하는 컴포넌트
 * - Overlap은 대상 등록만
 * - 실제 정화는 Timer 기반 반복 처리
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MAGESQUAD_API UMSInkCleanerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMSInkCleanerComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
    UFUNCTION()
    void OnBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );
private:
    void ApplyClean();
protected:
    UPROPERTY(EditAnywhere, Category = "Ink")
    float CleanRadiusCm = 60.f;

    UPROPERTY(EditAnywhere, Category = "Ink")
    float CleanInterval = 0.05f;

    UPROPERTY()
    UPrimitiveComponent* OwnerCollision = nullptr;

    UPROPERTY()
    TArray<TWeakObjectPtr<AMSInkAreaActor>> OverlappingAreas;
private:
    FTimerHandle CleanTimer;
};
