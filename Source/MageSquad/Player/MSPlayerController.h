// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MSPlayerController.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어 컨트롤러 클래스
 * HUD 생성 및 유지 관리
 * 각 컨트롤러의 커서 위치를 서버에게 알림
 */
UCLASS()
class MAGESQUAD_API AMSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 서버에서 읽는 최신 커서 위치
	FVector GetServerCursor() const;

	// 서버에서 읽는 최신 커서 방향
	FVector GetServerCursorDir(const FVector& FallbackForward) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 마우스 커서 업데이트 함수
	void UpdateCursor();

	// 커서 정보 업데이트 RPC
	UFUNCTION(Server, Unreliable)
	void ServerRPCSetCursorInfo(const FVector_NetQuantize& InPos, const FVector_NetQuantizeNormal& InDir);

	UFUNCTION(Server, Reliable)
	void ServerRPCReportReady();
private:
	// 마우스 커서 업데이트 타이머
	FTimerHandle CursorUpdateTimer;

	// 마우스 커서 위치
	UPROPERTY()
	FVector_NetQuantize ServerCursor = FVector(0.f, 0.f, 0.f);

	// 마우스 커서 방향
	UPROPERTY()
	FVector_NetQuantize ServerCursorDir = FVector(1.f, 0.f, 0.f);
};
