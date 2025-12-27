#include "MSSpawnTileMap.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "NavigationSystem.h"

AMSSpawnTileMap::AMSSpawnTileMap()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // 디버그 시각화 필요할 때만 활성화

	// 네트워크 리플리케이션 불필요 (서버에서만 사용)
	bReplicates = false;
}

void AMSSpawnTileMap::BeginPlay()
{
	Super::BeginPlay();

	// 런타임에 스폰 가능 타일 캐시 빌드
	BuildSpawnableTileCache();

	// 디버그 모드면 Tick 활성화
	if (bShowDebugTiles)
	{
		SetActorTickEnabled(true);
	}
}

void AMSSpawnTileMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if !UE_BUILD_SHIPPING
	if (bShowDebugTiles)
	{
		DrawDebugTilesRuntime();
	}
#endif
}

#if WITH_EDITORONLY_DATA
void AMSSpawnTileMap::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// MaxSpawnableHeight 변경 시 자동으로 스폰 가능 상태 업데이트
	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AMSSpawnTileMap, MaxSpawnableHeight))
		{
			RefreshSpawnableStatus();
		}
	}
}
#endif

#if WITH_EDITOR
void AMSSpawnTileMap::GenerateTileMap()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[SpawnTileMap] No world found"));
		return;
	}

	AllTiles.Empty();

	const FVector ActorLocation = GetActorLocation();
	
	// 그리드 크기 계산
	const int32 GridCountX = FMath::CeilToInt(MapExtent.X * 2.f / TileSize);
	const int32 GridCountY = FMath::CeilToInt(MapExtent.Y * 2.f / TileSize);

	// 시작 위치 (좌하단)
	const float StartX = ActorLocation.X - MapExtent.X;
	const float StartY = ActorLocation.Y - MapExtent.Y;

	int32 TotalTiles = 0;
	int32 SpawnableTiles = 0;
	int32 MissedRaycasts = 0;
	int32 NoNavMeshTiles = 0;

	// NavMesh 시스템 가져오기
	UNavigationSystemV1* NavSystem = nullptr;
	if (bRequireNavMesh)
	{
		NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
		
		// 에디터에서는 다른 방식으로 시도
		if (!NavSystem)
		{
			NavSystem = UNavigationSystemV1::GetNavigationSystem(World);
		}
		
		if (!NavSystem)
		{
			UE_LOG(LogTemp, Error, TEXT("[SpawnTileMap] NavSystem not found! Make sure NavMesh is built in this level."));
			UE_LOG(LogTemp, Error, TEXT("[SpawnTileMap] Aborting generation because bRequireNavMesh is true."));
			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Starting generation: %d x %d tiles (TileSize: %.1f)"), 
		GridCountX, GridCountY, TileSize);
	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Actor Location: %s"), *ActorLocation.ToString());
	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] TraceChannel: %d, RaycastStartHeight: %.1f, RequireNavMesh: %s"), 
		(int32)TraceChannel.GetValue(), RaycastStartHeight, bRequireNavMesh ? TEXT("true") : TEXT("false"));

	for (int32 X = 0; X < GridCountX; ++X)
	{
		for (int32 Y = 0; Y < GridCountY; ++Y)
		{
			// 타일 중심 위치 계산
			FVector TileCenter;
			TileCenter.X = StartX + (X * TileSize) + (TileSize * 0.5f);
			TileCenter.Y = StartY + (Y * TileSize) + (TileSize * 0.5f);
			TileCenter.Z = RaycastStartHeight;

			// 레이캐스트 실행
			FHitResult HitResult;
			FVector TraceEnd = TileCenter - FVector(0.f, 0.f, RaycastDistance);

			FCollisionQueryParams QueryParams;
			QueryParams.bTraceComplex = false;
			QueryParams.AddIgnoredActor(this);

			bool bHit = World->LineTraceSingleByChannel(
				HitResult,
				TileCenter,
				TraceEnd,
				TraceChannel,
				QueryParams
			);

			if (bHit)
			{
				float GroundHeight = HitResult.ImpactPoint.Z;
				FVector FinalLocation = FVector(TileCenter.X, TileCenter.Y, GroundHeight);

				// NavMesh 체크
				bool bOnNavMesh = true;
				if (bRequireNavMesh && NavSystem)
				{
					FNavLocation NavLoc;
					
					// NavMeshQueryExtent가 0이면 작은 범위로 체크
					FVector QueryExtent = NavMeshQueryExtent;
					if (QueryExtent.IsNearlyZero())
					{
						QueryExtent = FVector(10.f, 10.f, 100.f);
					}
					
					bOnNavMesh = NavSystem->ProjectPointToNavigation(FinalLocation, NavLoc, QueryExtent);
					
					// 추가 검증: 투영된 위치가 원래 위치와 XY 기준으로 너무 멀면 실패
					if (bOnNavMesh)
					{
						float DistXY = FVector::DistXY(FinalLocation, NavLoc.Location);
						if (DistXY > TileSize * 0.25f) // 타일 크기의 25% 이상 떨어져 있으면 실패
						{
							bOnNavMesh = false;
						}
					}
				}

				if (!bOnNavMesh)
				{
					NoNavMeshTiles++;
					continue; // NavMesh 없으면 타일 생성하지 않음
				}

				bool bSpawnable = (GroundHeight <= MaxSpawnableHeight);
				FIntPoint GridIndex(X, Y);

				AllTiles.Add(FMSSpawnTile(FinalLocation, GroundHeight, bSpawnable, GridIndex));
				
				TotalTiles++;
				if (bSpawnable)
				{
					SpawnableTiles++;
				}
			}
			else
			{
				MissedRaycasts++;
			}
		}
	}

	// 카운트 업데이트 (에디터에서 확인용)
	TotalTileCount = TotalTiles;
	SpawnableTileCount = SpawnableTiles;

	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] ========== Generation Complete =========="));
	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Total tiles: %d"), TotalTiles);
	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Spawnable: %d"), SpawnableTiles);
	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Non-spawnable: %d"), TotalTiles - SpawnableTiles);
	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Missed raycasts (no ground hit): %d"), MissedRaycasts);
	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Skipped (no NavMesh): %d"), NoNavMeshTiles);

	if (TotalTiles > 0)
	{
		// 높이 범위 출력
		float MinHeight = FLT_MAX;
		float MaxHeight = -FLT_MAX;
		for (const FMSSpawnTile& Tile : AllTiles)
		{
			MinHeight = FMath::Min(MinHeight, Tile.GroundHeight);
			MaxHeight = FMath::Max(MaxHeight, Tile.GroundHeight);
		}
		UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Height range: %.1f ~ %.1f (MaxSpawnableHeight: %.1f)"), 
			MinHeight, MaxHeight, MaxSpawnableHeight);
	}

	// 에디터에서 변경사항 저장되도록 마킹
	Modify();

	// 타일 생성 후 자동으로 디버그 그리기
	if (bShowDebugTiles && TotalTiles > 0)
	{
		DrawDebugTilesInEditor();
	}
}

void AMSSpawnTileMap::ClearTileMap()
{
	AllTiles.Empty();
	SpawnableTileIndices.Empty();
	CachedSpawnableTiles.Empty();
	bCacheBuilt = false;

	TotalTileCount = 0;
	SpawnableTileCount = 0;

	// 기존 디버그 라인 지우기
	FlushPersistentDebugLines(GetWorld());

	Modify();

	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] TileMap cleared"));
}

void AMSSpawnTileMap::RefreshSpawnableStatus()
{
	int32 SpawnableCount = 0;

	for (FMSSpawnTile& Tile : AllTiles)
	{
		Tile.bIsSpawnable = (Tile.GroundHeight <= MaxSpawnableHeight);
		if (Tile.bIsSpawnable)
		{
			SpawnableCount++;
		}
	}

	SpawnableTileCount = SpawnableCount;

	Modify();

	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Refreshed spawnable status: %d / %d tiles are spawnable (MaxHeight: %.1f)"),
		SpawnableCount, AllTiles.Num(), MaxSpawnableHeight);

	// 디버그 타일 다시 그리기
	if (bShowDebugTiles)
	{
		DrawDebugTilesInEditor();
	}
}

void AMSSpawnTileMap::DrawDebugTilesInEditor()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (AllTiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnTileMap] No tiles to draw. Generate TileMap first!"));
		return;
	}

	// 기존 디버그 라인 지우기
	FlushPersistentDebugLines(World);

	const float HalfTileSize = TileSize * 0.5f;
	int32 DrawnCount = 0;

	for (const FMSSpawnTile& Tile : AllTiles)
	{
		FColor DrawColor = Tile.bIsSpawnable ? SpawnableColor : NonSpawnableColor;

		// 타일 경계선 그리기 (지면에서 약간 위로)
		float DrawZ = Tile.Location.Z + 10.f;

		FVector Corners[4];
		Corners[0] = FVector(Tile.Location.X - HalfTileSize, Tile.Location.Y - HalfTileSize, DrawZ);
		Corners[1] = FVector(Tile.Location.X + HalfTileSize, Tile.Location.Y - HalfTileSize, DrawZ);
		Corners[2] = FVector(Tile.Location.X + HalfTileSize, Tile.Location.Y + HalfTileSize, DrawZ);
		Corners[3] = FVector(Tile.Location.X - HalfTileSize, Tile.Location.Y + HalfTileSize, DrawZ);

		for (int32 i = 0; i < 4; ++i)
		{
			DrawDebugLine(
				World,
				Corners[i],
				Corners[(i + 1) % 4],
				DrawColor,
				true,  // bPersistentLines = true (에디터에서 계속 보임)
				EditorDebugLineDuration,  // 지속 시간
				0,     // DepthPriority
				DebugTileThickness
			);
		}
		DrawnCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Drew %d debug tiles (Duration: %.1f seconds)"), 
		DrawnCount, EditorDebugLineDuration);
}
#endif

void AMSSpawnTileMap::BuildSpawnableTileCache()
{
	SpawnableTileIndices.Empty();
	CachedSpawnableTiles.Empty();

	for (int32 i = 0; i < AllTiles.Num(); ++i)
	{
		if (AllTiles[i].bIsSpawnable)
		{
			SpawnableTileIndices.Add(i);
			CachedSpawnableTiles.Add(AllTiles[i]);
		}
	}

	bCacheBuilt = true;

	UE_LOG(LogTemp, Log, TEXT("[SpawnTileMap] Cache built: %d spawnable tiles"), CachedSpawnableTiles.Num());
}

const TArray<FMSSpawnTile>& AMSSpawnTileMap::GetAllSpawnableTiles() const
{
	return CachedSpawnableTiles;
}

bool AMSSpawnTileMap::GetNearestSpawnableTile(const FVector& FromLocation, FMSSpawnTile& OutTile) const
{
	if (CachedSpawnableTiles.Num() == 0)
	{
		return false;
	}

	float MinDistSq = FLT_MAX;
	int32 NearestIndex = -1;

	for (int32 i = 0; i < CachedSpawnableTiles.Num(); ++i)
	{
		float DistSq = FVector::DistSquared2D(FromLocation, CachedSpawnableTiles[i].Location);
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			NearestIndex = i;
		}
	}

	if (NearestIndex >= 0)
	{
		OutTile = CachedSpawnableTiles[NearestIndex];
		return true;
	}

	return false;
}

TArray<FMSSpawnTile> AMSSpawnTileMap::GetSpawnableTilesInRadius(const FVector& Center, float Radius) const
{
	TArray<FMSSpawnTile> Result;
	const float RadiusSq = Radius * Radius;

	for (const FMSSpawnTile& Tile : CachedSpawnableTiles)
	{
		if (FVector::DistSquared2D(Center, Tile.Location) <= RadiusSq)
		{
			Result.Add(Tile);
		}
	}

	return Result;
}

bool AMSSpawnTileMap::IsLocationVisibleToPlayer(APlayerController* PC, const FVector& Location) const
{
	if (!PC)
	{
		return false;
	}

	// 스크린 좌표로 변환
	FVector2D ScreenPosition;
	if (PC->ProjectWorldLocationToScreen(Location, ScreenPosition, false))
	{
		int32 ViewportSizeX, ViewportSizeY;
		PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

		// 화면 내부에 있는지 체크 (약간의 마진 포함)
		constexpr float Margin = 150.0f;

		if (ScreenPosition.X >= -Margin && ScreenPosition.X <= ViewportSizeX + Margin &&
			ScreenPosition.Y >= -Margin && ScreenPosition.Y <= ViewportSizeY + Margin)
		{
			return true;
		}
	}

	return false;
}

TArray<FMSSpawnTile> AMSSpawnTileMap::GetSpawnableTilesNotVisibleToPlayers(
	const TArray<APlayerController*>& PlayerControllers) const
{
	TArray<FMSSpawnTile> Result;

	for (const FMSSpawnTile& Tile : CachedSpawnableTiles)
	{
		bool bVisibleToAnyPlayer = false;

		for (APlayerController* PC : PlayerControllers)
		{
			if (PC && IsLocationVisibleToPlayer(PC, Tile.Location))
			{
				bVisibleToAnyPlayer = true;
				break;
			}
		}

		if (!bVisibleToAnyPlayer)
		{
			Result.Add(Tile);
		}
	}

	return Result;
}

bool AMSSpawnTileMap::GetBestSpawnTileForPlayer(
	APlayerController* TargetPlayer,
	const TArray<APlayerController*>& AllPlayers,
	FMSSpawnTile& OutTile) const
{
	if (!TargetPlayer || CachedSpawnableTiles.Num() == 0)
	{
		return false;
	}

	APawn* TargetPawn = TargetPlayer->GetPawn();
	if (!TargetPawn)
	{
		return false;
	}

	const FVector PlayerLocation = TargetPawn->GetActorLocation();

	// 모든 플레이어에게 보이지 않는 타일들 필터링
	TArray<FMSSpawnTile> CandidateTiles = GetSpawnableTilesNotVisibleToPlayers(AllPlayers);

	if (CandidateTiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnTileMap] No tiles invisible to all players"));
		return false;
	}

	// 타깃 플레이어와 가장 가까운 타일 찾기
	float MinDistSq = FLT_MAX;
	int32 BestIndex = -1;

	for (int32 i = 0; i < CandidateTiles.Num(); ++i)
	{
		float DistSq = FVector::DistSquared2D(PlayerLocation, CandidateTiles[i].Location);
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			BestIndex = i;
		}
	}

	if (BestIndex >= 0)
	{
		OutTile = CandidateTiles[BestIndex];
		return true;
	}

	return false;
}

FIntPoint AMSSpawnTileMap::WorldToGridIndex(const FVector& WorldLocation) const
{
	const FVector ActorLocation = GetActorLocation();
	const float StartX = ActorLocation.X - MapExtent.X;
	const float StartY = ActorLocation.Y - MapExtent.Y;

	int32 GridX = FMath::FloorToInt((WorldLocation.X - StartX) / TileSize);
	int32 GridY = FMath::FloorToInt((WorldLocation.Y - StartY) / TileSize);

	return FIntPoint(GridX, GridY);
}

FVector AMSSpawnTileMap::GridIndexToWorld(const FIntPoint& GridIndex) const
{
	const FVector ActorLocation = GetActorLocation();
	const float StartX = ActorLocation.X - MapExtent.X;
	const float StartY = ActorLocation.Y - MapExtent.Y;

	FVector WorldLocation;
	WorldLocation.X = StartX + (GridIndex.X * TileSize) + (TileSize * 0.5f);
	WorldLocation.Y = StartY + (GridIndex.Y * TileSize) + (TileSize * 0.5f);
	WorldLocation.Z = 0.f; // 실제 사용 시 타일 데이터에서 높이 가져와야 함

	return WorldLocation;
}

void AMSSpawnTileMap::DrawDebugTilesRuntime()
{
#if !UE_BUILD_SHIPPING
	const float HalfTileSize = TileSize * 0.5f;

	for (const FMSSpawnTile& Tile : AllTiles)
	{
		FColor DrawColor = Tile.bIsSpawnable ? SpawnableColor : NonSpawnableColor;
		float DrawZ = Tile.Location.Z + 10.f;

		FVector Corners[4];
		Corners[0] = FVector(Tile.Location.X - HalfTileSize, Tile.Location.Y - HalfTileSize, DrawZ);
		Corners[1] = FVector(Tile.Location.X + HalfTileSize, Tile.Location.Y - HalfTileSize, DrawZ);
		Corners[2] = FVector(Tile.Location.X + HalfTileSize, Tile.Location.Y + HalfTileSize, DrawZ);
		Corners[3] = FVector(Tile.Location.X - HalfTileSize, Tile.Location.Y + HalfTileSize, DrawZ);

		for (int32 i = 0; i < 4; ++i)
		{
			DrawDebugLine(
				GetWorld(),
				Corners[i],
				Corners[(i + 1) % 4],
				DrawColor,
				false,  // bPersistentLines = false (매 프레임 그림)
				-1.f,
				0,
				DebugTileThickness
			);
		}
	}
#endif
}