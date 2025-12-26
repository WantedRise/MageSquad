#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MSInkAreaActor.generated.h"

UCLASS()
class MAGESQUAD_API AMSInkAreaActor : public AActor
{
    GENERATED_BODY()

public:
    AMSInkAreaActor();
    virtual void BeginPlay() override;

    /** 플레이어 위치를 “지나간 자리”로 기록 (RT + Grid) */
    UFUNCTION(BlueprintCallable, Category = "Ink")
    void CleanAtWorldPos(const FVector& WorldPos, float RadiusCm);

    /** 0~1 (1 = 완전 정화) */
    UFUNCTION(BlueprintCallable, Category = "Ink")
    float GetCleanRatio() const;

protected:
    bool WorldPosToUV(const FVector& WorldPos, float& OutU, float& OutV) const;

    /** RT 누적 기록 */
    void PaintRT(float U, float V, float RadiusUV);

    /** Grid 판정(내부) */
    void InitGrid();
    void CleanGridAtUV(float U, float V, float RadiusUV);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ink")
    UStaticMeshComponent* AreaMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Area")
    float AreaWidthCm = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Area")
    float AreaHeightCm = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Logic")
    int32 GridSize = 64;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    int32 RTSize = 1024;

    /** 브러시 머티리얼 (M_InkBrush_Paint) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    UMaterialInterface* BrushPaintMaterial = nullptr;

    /** AreaMesh에 적용된 바닥 머티리얼 파라미터 이름 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    FName RTParamName = TEXT("InkMaskRT");

    /** 브러시 파라미터 이름들 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    FName BrushCenterParam = TEXT("BrushCenter");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    FName BrushRadiusParam = TEXT("BrushRadius");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    FName BrushHardnessParam = TEXT("BrushHardness");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    FName BrushStrengthParam = TEXT("BrushStrength");
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    FName PrevRTParam = TEXT("PrevRT");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    float BrushHardness = 0.7f;

    /** 1이면 “한 번에 완전 기록” */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ink|Visual")
    float BrushStrength = 1.0f;

private:
    // Grid 상태
    TArray<uint8> InkGrid; // 1=Dirty, 0=Clean
    int32 DirtyCount = 0;

    // RT 2장 (핑퐁)
    UPROPERTY()
    UTextureRenderTarget2D* RT_A = nullptr;
    UPROPERTY()
    UTextureRenderTarget2D* RT_B = nullptr;

    UPROPERTY()
    UTextureRenderTarget2D* CurrentRT = nullptr; // 읽는 RT
    UPROPERTY()
    UTextureRenderTarget2D* WriteRT = nullptr;   // 쓰는 RT

    UPROPERTY()
    UMaterialInstanceDynamic* AreaMID = nullptr;

    UPROPERTY()
    UMaterialInstanceDynamic* BrushMID = nullptr;
};