#include "MSInkAreaActor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include "Components/MSDirectionIndicatorComponent.h"

AMSInkAreaActor::AMSInkAreaActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AreaMesh"));
    RootComponent = AreaMesh;

    DirectionIndicatorComponent = CreateDefaultSubobject<UMSDirectionIndicatorComponent>(TEXT("DirectionIndicatorComponent"));
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

    // 플레이어 방향 표시 인디케이터 설정
    if (DirectionIndicatorComponent)
    {
        // 거리 표기 비활성화
        DirectionIndicatorComponent->bShowDistance = false;
    }
}

void AMSInkAreaActor::InitGrid()
{
    if (!HasAuthority()) return;

    InkGrid.SetNum(GridSize * GridSize);
    BakeBlockedCells();
}

void AMSInkAreaActor::BakeBlockedCells()
{
    UWorld* World = GetWorld();
    if (!World || InkGrid.Num() == 0) return;

    // 변수 초기화
    TotalPlayableCells = 0;
    CurrentDirtyCount = 0;

  

    for (int32 y = 0; y < GridSize; ++y)
    {
        for (int32 x = 0; x < GridSize; ++x)
        {
            int32 Index = y * GridSize + x;
            FVector Start = GetCellWorldCenter(x, y) + FVector(0, 0, 5.f);
            FVector End = Start + FVector(0, 0, 300.f);

            FHitResult Hit;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);

            bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params);
            
            if (bHit && Hit.GetComponent() != AreaMesh)
            {
                // 1. 장애물 영역 처리
                InkGrid[Index] = EInkGridState::Blocked;
            }
            else
            {
                // 2. 실제 플레이 가능한 영역 처리
                InkGrid[Index] = EInkGridState::Dirty; // 초기 상태는 더러움
                TotalPlayableCells++;
                CurrentDirtyCount++;
            }

            //const FBoxSphereBounds B = AreaMesh->CalcBounds(FTransform::Identity);
            //// 그리드 한 칸의 절반 크기 (박스 크기용)
            //FVector CellExtent = FVector(B.BoxExtent.X / GridSize, B.BoxExtent.Y / GridSize, 5.f);

            //FColor DebugColor;
            //if (InkGrid[Index] == EInkGridState::Blocked)
            //{
            //    DebugColor = FColor::Red; // 장애물 영역 (비율 계산 제외)
            //}
            //else if (InkGrid[Index] == EInkGridState::Dirty)
            //{
            //    DebugColor = FColor::Yellow; // 청소해야 할 영역
            //}
            //else
            //{
            //    DebugColor = FColor::Green; // 청소 완료 영역
            //}
      
            //FVector Center = GetCellWorldCenter(x, y);
            //
            //// 월드에 박스 그리기 (10초 동안 유지)
            //DrawDebugBox(World, Center, CellExtent, DebugColor, false, 10.f, 0, 2.f);
        }
    }
}

FVector AMSInkAreaActor::GetCellWorldCenter(int32 X, int32 Y) const
{
    if (!AreaMesh || GridSize <= 0)
    {
        return FVector::ZeroVector;
    }

    // 1️⃣ Grid index → UV 중심 좌표
    const float U = (X + 0.5f) / GridSize;
    const float V = (Y + 0.5f) / GridSize;

    const FBoxSphereBounds LocalBounds = AreaMesh->CalcBounds(FTransform::Identity);

    // 2️⃣ AreaMesh 로컬 공간에서의 크기
    // (Plane 기준: -Half ~ +Half)
    const FVector BoundsExtent = LocalBounds.BoxExtent;
    // 3️⃣ UV(0~1) → Local Position
    FVector LocalPos;
    LocalPos.X = (U - 0.5f) * 2.f * BoundsExtent.X;
    LocalPos.Y = (V - 0.5f) * 2.f * BoundsExtent.Y;
    LocalPos.Z = 0.f;

    // 4️⃣ Local → World
    return AreaMesh->GetComponentTransform().TransformPosition(LocalPos);
}

float AMSInkAreaActor::GetCleanRatio() const
{
    if (TotalPlayableCells <= 0) return 1.f;

    // (전체 칸 - 남은 칸) / 전체 칸 = 청소된 비율
    float Ratio = (float)(TotalPlayableCells - CurrentDirtyCount) / (float)TotalPlayableCells;

    // 정밀도 문제로 인한 오버플로우 방지 (0.0 ~ 1.0 사이로 클램프)
    return FMath::Clamp(Ratio, 0.f, 1.f);
}

void AMSInkAreaActor::CleanAtWorldPos(const FVector& WorldPos, float RadiusCm)
{
    float U, V;
    if (!WorldPosToUV(WorldPos, U, V))
        return;
    // 스케일 1.0 기준의 순수 로컬 크기를 가져옴
    const FBoxSphereBounds LocalBounds = AreaMesh->CalcBounds(FTransform::Identity);
    // 여기에 현재 액터의 X 스케일을 곱해 실제 월드 너비(cm)를 구함
    const float RealWorldWidth = LocalBounds.BoxExtent.X * 2.0f * GetActorScale3D().X;
    // 이제 스케일과 회전에 상관없이 정확한 UV 반경이 계산됨
    const float RadiusUV = RadiusCm / FMath::Max(RealWorldWidth, 1.f);
    //const float RadiusUV = RadiusCm / AreaWidthCm;

    // RT에 “지나간 길” 기록 (누적)
    PaintRT(U, V, RadiusUV);

    // Grid 판정 (지나간 영역 기록)
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
    if (GridSize <= 0 || !HasAuthority()) return;

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

            if (InkGrid[Idx] == EInkGridState::Dirty)
            {
                InkGrid[Idx] = EInkGridState::Clean;
                CurrentDirtyCount = FMath::Max(0, CurrentDirtyCount - 1); // 지워질 때만 카운트 감소
                OnProgressChanged.Broadcast(GetCleanRatio());
            }
        }
    }
}
