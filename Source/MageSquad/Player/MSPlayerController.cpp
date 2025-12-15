// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerController.h"

FVector AMSPlayerController::GetServerCursor() const
{
	return ServerCursor;
}

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
		const FVector SpawnOrigin = P->GetActorLocation() + FVector(0.f, 0.f, 50.f);

		// 커서 방향
		FVector Dir = bHit ? (Hit.Location - SpawnOrigin) : P->GetActorForwardVector();

		// Z값 보정
		Dir.Z = 50.f;
		Dir = Dir.GetSafeNormal();
		if (Dir.IsNearlyZero()) Dir = FVector(1.f, 0.f, 0.f);

		if (HasAuthority())
		{
			// 호스트(리슨)면 서버 캐시 직접 갱신
			ServerCursor = FVector(Hit.ImpactPoint.X, Hit.ImpactPoint.Y, ServerCursor.Z);
			ServerCursorDir = Dir;
		}
		else
		{
			// 원격 클라는 서버로 전달
			ServerRPCSetCursorInfo(Hit.ImpactPoint, Dir);
		}
	}
}

void AMSPlayerController::ServerRPCSetCursorInfo_Implementation(const FVector_NetQuantizeNormal& InPos, const FVector_NetQuantizeNormal& InDir)
{
	// 커서 위치 저장
	FVector Pos = FVector(InPos.X, InPos.Y, ServerCursor.Z);
	if (Pos.IsNearlyZero()) Pos = FVector(1.f, 0.f, 0.f);
	ServerCursor = Pos;

	// 커서 방향 저장
	FVector Dir = FVector(InDir.X, InDir.Y, ServerCursorDir.Z);
	if (Dir.IsNearlyZero()) Dir = FVector(1.f, 0.f, 0.f);
	ServerCursorDir = Dir.GetSafeNormal();
}
