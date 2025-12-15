// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerController.h"

FVector AMSPlayerController::GetServerCursorDir(const FVector& FallbackForward) const
{
	const FVector Fwd = FVector(FallbackForward).GetSafeNormal();
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
		const FVector SpawnOrigin = P->GetActorLocation() + FVector(0, 0, 50);

		// 커서 방향
		FVector Dir = bHit ? (Hit.Location - SpawnOrigin) : P->GetActorForwardVector();

		// Z값 보정
		Dir.Z = 50.f;
		Dir = Dir.GetSafeNormal();
		if (Dir.IsNearlyZero()) Dir = FVector(1, 0, 50.f);

		if (HasAuthority())
		{
			// 호스트(리슨)면 서버 캐시 직접 갱신
			ServerCursorDir = Dir;
		}
		else
		{
			// 원격 클라는 서버로 전달
			ServerSetCursorDir(Dir);
		}
	}
}

void AMSPlayerController::ServerSetCursorDir_Implementation(const FVector_NetQuantizeNormal& InDir2D)
{
	// 커서 방향 저장
	FVector Dir = FVector(InDir2D.X, InDir2D.Y, InDir2D.Z);
	if (Dir.IsNearlyZero()) Dir = FVector(1.f, 0.f, 50.f);
	ServerCursorDir = Dir.GetSafeNormal();
}
