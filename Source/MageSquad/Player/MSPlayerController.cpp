// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerController.h"
#include <System/MSLevelManagerSubsystem.h>

FVector AMSPlayerController::GetServerCursor() const
{
	return ServerCursor;
}

FVector AMSPlayerController::GetServerCursorDir(const FVector& FallbackForward) const
{
	// 화면 밖 등으로 커서 방향이 유효하지 않을 때는 캐릭터 전방으로 발사
	FVector Fwd = FVector(FallbackForward);
	Fwd.Z = 0.f;
	Fwd = Fwd.GetSafeNormal();
	if (Fwd.IsNearlyZero())
	{
		Fwd = FVector(1.f, 0.f, 0.f);
	}

	const FVector Cursor = FVector(ServerCursorDir).GetSafeNormal();

	return Cursor.IsNearlyZero() ? Fwd : Cursor;
}

void AMSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 입력 모드 설정
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = true;

	// 커서 트레이스 타이머 설정
	if (IsLocalController())
	{
		GetWorldTimerManager().SetTimer(
			CursorUpdateTimer,
			this,
			&AMSPlayerController::UpdateCursor,
			0.05f,
			true
		);
		//맵 로딩을 위한 딜레이, 로딩창을 2초뒤 제거
		if (UMSLevelManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
		{
			FTimerHandle MatchEntryDelayTimer;
			GetWorldTimerManager().SetTimer(
				MatchEntryDelayTimer,
				Subsystem,
				&UMSLevelManagerSubsystem::HideLoadingWidget,
				2.0f,
				false
			);
		}
	}
}

void AMSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CursorUpdateTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(CursorUpdateTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void AMSPlayerController::UpdateCursor()
{
	// 커서 트레이스
	if (IsLocalController())
	{
		APawn* P = GetPawn();
		if (!P) return;

		// 마우스 커서 트레이싱
		FHitResult Hit;
		const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit) && Hit.bBlockingHit;

		// 캐릭터의 위치
		const FVector SpawnOrigin = P->GetActorLocation() + FVector(0.f, 0.f, 50.f);

		// 캐릭터 전방(수평) - 커서가 화면을 벗어나면 이 방향으로 발사
		FVector Forward2D = P->GetActorForwardVector();
		Forward2D.Z = 0.f;
		Forward2D = Forward2D.GetSafeNormal();
		if (Forward2D.IsNearlyZero()) Forward2D = FVector(1.f, 0.f, 0.f);

		// 커서 월드 위치
		// - Hit 성공: 충돌 지점
		// - Hit 실패(커서가 뷰포트를 벗어남 등): 전방으로 충분히 먼 지점(가상 커서)
		const FVector CursorWorldPos = bHit ? Hit.ImpactPoint : (SpawnOrigin + Forward2D * 10000.f);

		// 커서 방향 (수평 고정)
		FVector Dir = (CursorWorldPos - SpawnOrigin);
		Dir.Z = 0.f;
		Dir = Dir.GetSafeNormal();
		if (Dir.IsNearlyZero()) Dir = Forward2D;

		if (HasAuthority())
		{
			// 호스트(리슨)면 서버 캐시 직접 갱신
			ServerCursor = CursorWorldPos;
			ServerCursorDir = Dir;
		}
		else
		{
			// 원격 클라는 서버로 전달
			ServerRPCSetCursorInfo(CursorWorldPos, Dir);
		}
	}
}

void AMSPlayerController::ServerRPCSetCursorInfo_Implementation(const FVector_NetQuantize& InPos, const FVector_NetQuantizeNormal& InDir)
{
	ServerCursor = FVector(InPos);

	// 커서 방향 저장 (수평 고정)
	FVector Dir = FVector(InDir);
	Dir.Z = 0.f;
	Dir = Dir.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = FVector(1.f, 0.f, 0.f);
	}
	ServerCursorDir = Dir;
}
