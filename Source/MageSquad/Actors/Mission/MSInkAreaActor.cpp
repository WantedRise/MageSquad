#include "MSInkAreaActor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMathLibrary.h"

AMSInkAreaActor::AMSInkAreaActor()
{
    PrimaryActorTick.bCanEverTick = false;

    AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AreaMesh"));
    RootComponent = AreaMesh;
}

void AMSInkAreaActor::BeginPlay()
{
    Super::BeginPlay();

    // 1) Grid 초기화
    InitGrid();

    // 2) RT 2장 생성 + 초기화(검정=더러움)
    RT_A = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), RTSize, RTSize, RTF_RGBA8);
    RT_B = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), RTSize, RTSize, RTF_RGBA8);

    UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), RT_A, FLinearColor::Black);
    UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), RT_B, FLinearColor::Black);

    CurrentRT = RT_A;
    WriteRT = RT_B;

    // 3) AreaMesh 머티리얼 인스턴스 만들고 CurrentRT 연결
    AreaMID = AreaMesh->CreateDynamicMaterialInstance(0);
    if (AreaMID)
    {
        AreaMID->SetTextureParameterValue(RTParamName, CurrentRT);
    }

    // 4) 브러시 MID 생성
    if (BrushPaintMaterial)
    {
        BrushMID = UMaterialInstanceDynamic::Create(BrushPaintMaterial, this);
    }
}

void AMSInkAreaActor::InitGrid()
{
    InkGrid.SetNum(GridSize * GridSize);
    for (uint8& C : InkGrid) C = 1;
    DirtyCount = InkGrid.Num();
}

float AMSInkAreaActor::GetCleanRatio() const
{
    if (InkGrid.Num() == 0) return 0.f;
    return 1.f - (float)DirtyCount / (float)InkGrid.Num();
}

void AMSInkAreaActor::CleanAtWorldPos(const FVector& WorldPos, float RadiusCm)
{
    float U, V;
    if (!WorldPosToUV(WorldPos, U, V))
        return;

    const float RadiusUV = RadiusCm / AreaWidthCm;

    // 1) RT에 “지나간 길” 기록 (누적)
    PaintRT(U, V, RadiusUV);

    // 2) Grid 판정 (지나간 영역 기록)
    CleanGridAtUV(U, V, RadiusUV); 
}

bool AMSInkAreaActor::WorldPosToUV(const FVector& WorldPos, float& OutU, float& OutV) const
{
    if (!AreaMesh) return false;

    const FVector Local = AreaMesh->GetComponentTransform().InverseTransformPosition(WorldPos);

    // Bounds는 “로컬 공간” 기준으로도 쓸 수 있게 Extent를 가져오는 방식이 안전
    const FBoxSphereBounds B = AreaMesh->CalcBounds(FTransform::Identity);
    const FVector Ext = B.BoxExtent;   // 로컬 기준 반폭/반높이

    // Plane이 XY라면 X/Y를 사용
    const float HalfW = FMath::Max(Ext.X, KINDA_SMALL_NUMBER);
    const float HalfH = FMath::Max(Ext.Y, KINDA_SMALL_NUMBER);

    // 로컬 X,Y가 범위 밖이면 false
    if (Local.X < -HalfW || Local.X > HalfW || Local.Y < -HalfH || Local.Y > HalfH)
        return false;

    OutU = (Local.X + HalfW) / (2.f * HalfW);
    OutV = (Local.Y + HalfH) / (2.f * HalfH);

    return true;
}

void AMSInkAreaActor::PaintRT(float U, float V, float RadiusUV)
{
    if (!BrushMID || !CurrentRT || !WriteRT || !AreaMID)
        return;

    // 브러시 파라미터
    BrushMID->SetVectorParameterValue(BrushCenterParam, FLinearColor(U, V, 0, 0));
    BrushMID->SetScalarParameterValue(BrushRadiusParam, RadiusUV);
    BrushMID->SetScalarParameterValue(BrushHardnessParam, BrushHardness);
    BrushMID->SetScalarParameterValue(BrushStrengthParam, BrushStrength);

    // 누적 핵심: 이전 RT를 읽어서 합친 결과를 WriteRT에 기록
    BrushMID->SetTextureParameterValue(PrevRTParam, CurrentRT);
    UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), WriteRT, BrushMID);

    // 핑퐁 스왑
    Swap(CurrentRT, WriteRT);

    // 바닥 머티리얼이 바라보는 RT 갱신
    AreaMID->SetTextureParameterValue(RTParamName, CurrentRT);
}

void AMSInkAreaActor::CleanGridAtUV(float U, float V, float RadiusUV)
{
    if (GridSize <= 0) return;

    const int32 CX = FMath::RoundToInt(U * (GridSize - 1));
    const int32 CY = FMath::RoundToInt(V * (GridSize - 1));
    const int32 R = FMath::CeilToInt(RadiusUV * GridSize);
    const int32 R2 = R * R;

    const int32 MinX = FMath::Clamp(CX - R, 0, GridSize - 1);
    const int32 MaxX = FMath::Clamp(CX + R, 0, GridSize - 1);
    const int32 MinY = FMath::Clamp(CY - R, 0, GridSize - 1);
    const int32 MaxY = FMath::Clamp(CY + R, 0, GridSize - 1);

    for (int32 y = MinY; y <= MaxY; ++y)
    {
        for (int32 x = MinX; x <= MaxX; ++x)
        {
            const int32 dx = x - CX;
            const int32 dy = y - CY;
            if (dx * dx + dy * dy > R2) continue;

            const int32 Idx = y * GridSize + x;
            if (InkGrid[Idx] == 1)
            {
                InkGrid[Idx] = 0;
                DirtyCount = FMath::Max(0, DirtyCount - 1);
            }
        }
    }
}
