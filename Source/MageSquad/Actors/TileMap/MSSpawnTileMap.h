#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSSpawnTileMap.generated.h"


/*
 * 작성자 : 임희섭
 * 작성일 : 2025/12/27
 * 몬스터 스폰 가능 영역을 관리하는 타일맵 액터
 * 에디터에서 레이캐스트로 타일을 생성하고, 런타임에 스폰 위치 조회에 사용
 */

// 개별 타일 데이터
USTRUCT(BlueprintType)
struct FMSSpawnTile
{
	GENERATED_BODY()

	// 타일 중심 위치 (월드 좌표)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Location = FVector::ZeroVector;

	// 타일의 지형 높이 (레이캐스트로 감지된 Z값)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GroundHeight = 0.f;

	// 스폰 가능 여부 (높이 조건 충족 시 true)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsSpawnable = false;

	// 그리드 인덱스 (빠른 조회용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntPoint GridIndex = FIntPoint::ZeroValue;

	FMSSpawnTile() {}
	FMSSpawnTile(const FVector& InLocation, float InHeight, bool bSpawnable, FIntPoint InIndex)
		: Location(InLocation), GroundHeight(InHeight), bIsSpawnable(bSpawnable), GridIndex(InIndex) {}
};

UCLASS()
class MAGESQUAD_API AMSSpawnTileMap : public AActor
{
	GENERATED_BODY()

public:
	AMSSpawnTileMap();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	// 스폰 가능 타일 인덱스 캐시 빌드
	void BuildSpawnableTileCache();

	// 특정 위치가 플레이어 뷰포트에 보이는지 체크
	bool IsLocationVisibleToPlayer(APlayerController* PC, const FVector& Location) const;
	
	// Frustum 기반 가시성 체크 (원격 플레이어용)
	bool IsLocationInPlayerFrustum(APlayerController* PC, const FVector& Location) const;
	
	// 디버그 드로잉 (런타임)
	void DrawDebugTilesRuntime();

	// 캐시된 스폰 가능 타일 배열 (런타임용)
	UPROPERTY(Transient)
	TArray<FMSSpawnTile> CachedSpawnableTiles;

	
public:
		// ========== 에디터 설정 ==========
	// 타일 크기 (가로, 세로 동일)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Settings", meta = (ClampMin = "50.0"))
	float TileSize = 300.f;

	// 타일맵 전체 범위 (중심에서 각 방향으로의 거리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Settings")
	FVector2D MapExtent = FVector2D(10000.f, 10000.f);

	// 스폰 가능 최대 높이 (이 Z값 이하만 스폰 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Settings")
	float MaxSpawnableHeight = 200.f;

	// 레이캐스트 시작 높이 (맵 최상단보다 높게 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Settings")
	float RaycastStartHeight = 5000.f;

	// 레이캐스트 최대 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Settings")
	float RaycastDistance = 10000.f;

	// 레이캐스트 트레이스 채널
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Settings")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// NavMesh 위에 있는 타일만 생성할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Settings")
	bool bRequireNavMesh = true;

	// NavMesh 투영 허용 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Settings", meta = (EditCondition = "bRequireNavMesh"))
	FVector NavMeshQueryExtent = FVector(100.f, 100.f, 500.f);
	
	// 모든 타일 데이터 (에디터에서 생성, 레벨에 저장됨)
	// AdvancedDisplay: Details 패널에서 숨김 (대량 데이터로 인한 에디터 렉 방지)
	UPROPERTY(BlueprintReadOnly, Category = "TileMap|Data", meta = (AdvancedDisplay))
	TArray<FMSSpawnTile> AllTiles;

	// 타일 개수 (에디터에서 확인용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TileMap|Data")
	int32 TotalTileCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TileMap|Data")
	int32 SpawnableTileCount = 0;

	// 스폰 가능한 타일만 캐싱 (런타임 최적화용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TileMap|Data", Transient)
	TArray<int32> SpawnableTileIndices;

	// ========== 디버그 시각화 ==========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Debug")
	bool bShowDebugTiles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Debug")
	FColor SpawnableColor = FColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Debug")
	FColor NonSpawnableColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Debug")
	float DebugTileThickness = 2.f;

	// 에디터에서 디버그 라인 지속 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileMap|Debug", meta = (ClampMin = "1.0"))
	float EditorDebugLineDuration = 30.f;

	// ========== 에디터 기능 ==========
	
#if WITH_EDITOR
	// 에디터에서 타일맵 생성 버튼
	UFUNCTION(CallInEditor, Category = "TileMap")
	void GenerateTileMap();

	// 타일맵 클리어
	UFUNCTION(CallInEditor, Category = "TileMap")
	void ClearTileMap();

	// 높이 기준 재적용 (타일 재생성 없이 스폰 가능 여부만 업데이트)
	UFUNCTION(CallInEditor, Category = "TileMap")
	void RefreshSpawnableStatus();

	// 에디터에서 디버그 타일 다시 그리기
	UFUNCTION(CallInEditor, Category = "TileMap|Debug")
	void DrawDebugTilesInEditor();
#endif

	// ========== 런타임 API ==========
	
	// 스폰 가능한 모든 타일 위치 반환
	UFUNCTION(BlueprintCallable, Category = "TileMap")
	const TArray<FMSSpawnTile>& GetAllSpawnableTiles() const;

	// 특정 위치에서 가장 가까운 스폰 가능 타일 찾기
	UFUNCTION(BlueprintCallable, Category = "TileMap")
	bool GetNearestSpawnableTile(const FVector& FromLocation, FMSSpawnTile& OutTile) const;

	// 특정 위치 주변의 스폰 가능 타일들 찾기 (반경 내)
	UFUNCTION(BlueprintCallable, Category = "TileMap")
	TArray<FMSSpawnTile> GetSpawnableTilesInRadius(const FVector& Center, float Radius) const;

	// 특정 뷰포트들에 보이지 않는 스폰 가능 타일들 찾기
	UFUNCTION(BlueprintCallable, Category = "TileMap")
	TArray<FMSSpawnTile> GetSpawnableTilesNotVisibleToPlayers(const TArray<APlayerController*>& PlayerControllers) const;

	// 특정 플레이어 기준, 다른 모든 플레이어에게 보이지 않으면서 가장 가까운 타일 찾기
	UFUNCTION(BlueprintCallable, Category = "TileMap")
	bool GetBestSpawnTileForPlayer(
		APlayerController* TargetPlayer,
		const TArray<APlayerController*>& AllPlayers,
		FMSSpawnTile& OutTile) const;

	// 월드 좌표를 그리드 인덱스로 변환
	UFUNCTION(BlueprintCallable, Category = "TileMap")
	FIntPoint WorldToGridIndex(const FVector& WorldLocation) const;

	// 그리드 인덱스를 월드 좌표로 변환
	UFUNCTION(BlueprintCallable, Category = "TileMap")
	FVector GridIndexToWorld(const FIntPoint& GridIndex) const;
	
private:
	bool bCacheBuilt = false;
};