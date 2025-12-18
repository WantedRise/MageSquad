// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerController.h"

#include "Widgets/HUD/MSPlayerHUDWidget.h"
#include "System/MSLevelManagerSubsystem.h"

void AMSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// �Է� ��� ����
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = true;

	if (IsLocalController())
	{
		// Ŀ�� Ʈ���̽� Ÿ�̸� ����
		GetWorldTimerManager().SetTimer(
			CursorUpdateTimer,
			this,
			&AMSPlayerController::UpdateCursor,
			0.05f,
			true
		);

		// HUD ����/ǥ��
		EnsureHUDCreated();

		// BeginPlay ������ Pawn/ASC �غ� ���� ��쵵 �����Ƿ� 1ȸ ���ʱ�ȭ �õ�
		NotifyHUDReinitialize();

		// �� �ε��� ���� ������, �ε�â�� 2�ʵ� ����
		// if (UMSLevelManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
		// {
		// 	FTimerHandle MatchEntryDelayTimer;
		// 	GetWorldTimerManager().SetTimer(
		// 		MatchEntryDelayTimer,
		// 		Subsystem,
		// 		&UMSLevelManagerSubsystem::HideLoadingWidget,
		// 		2.0f,
		// 		false
		// 	);
		// }
	}
}

void AMSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// ����/ȣ��Ʈ���� Pawn�� ��� �����Ǵ� ��찡 �����Ƿ� Possess �������� HUD ���ʱ�ȭ
	if (IsLocalController())
	{
		EnsureHUDCreated();
		NotifyHUDReinitialize();
	}
}

void AMSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// OnRep_Pawn������ HUD ���ʱ�ȭ
	// Ŭ���̾�Ʈ�� Pawn�� ������ �ʰ� ������ ��찡 ���Ƽ� HUD ���ʱ�ȭ�� ���� �ʱ�ȭ Ÿ�̹� ������ ����ȭ
	if (IsLocalController())
	{
		EnsureHUDCreated();
		NotifyHUDReinitialize();
	}
}

void AMSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Ŀ�� Ʈ���̽� Ÿ�̸� �ʱ�ȭ
	if (CursorUpdateTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(CursorUpdateTimer);
	}

	// HUD ���� ����
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AMSPlayerController::EnsureHUDCreated()
{
	if (!IsLocalController() || HUDWidgetInstance || !HUDWidgetClass) return;

	// HUD ���� ���� �� �׸���
	HUDWidgetInstance = CreateWidget<UMSPlayerHUDWidget>(this, HUDWidgetClass);
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->AddToViewport();
	}
}

void AMSPlayerController::NotifyHUDReinitialize()
{
	if (!IsLocalController()) return;

	if (HUDWidgetInstance)
	{
		// ���� ���ο��� Pawn/ASC �غ� ���θ� üũ�ϰ�, �غ� �� ������ Ÿ�̸ӷ� ��õ�
		HUDWidgetInstance->RequestReinitialize();
	}
}

FVector AMSPlayerController::GetServerCursor() const
{
	return ServerCursor;
}

FVector AMSPlayerController::GetServerCursorDir(const FVector& FallbackForward) const
{
	// ȭ�� �� ������ Ŀ�� ������ ��ȿ���� ���� ���� ĳ���� �������� �߻�
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

void AMSPlayerController::UpdateCursor()
{
	// Ŀ�� Ʈ���̽�
	if (IsLocalController())
	{
		APawn* P = GetPawn();
		if (!P) return;

		// ���콺 Ŀ�� Ʈ���̽�
		FHitResult Hit;
		const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit) && Hit.bBlockingHit;

		// ĳ������ ��ġ
		const FVector SpawnOrigin = P->GetActorLocation() + FVector(0.f, 0.f, 50.f);

		// ĳ���� ����(����) - Ŀ���� ȭ���� ����� �� �������� �߻�
		FVector Forward2D = P->GetActorForwardVector();
		Forward2D.Z = 0.f;
		Forward2D = Forward2D.GetSafeNormal();
		if (Forward2D.IsNearlyZero()) Forward2D = FVector(1.f, 0.f, 0.f);

		// Ŀ�� ���� ��ġ
		// - Hit ����: �浹 ����
		// - Hit ����(Ŀ���� ����Ʈ�� ��� ��): �������� ����� �� ����(���� Ŀ��)
		const FVector CursorWorldPos = bHit ? Hit.ImpactPoint : (SpawnOrigin + Forward2D * 10000.f);

		// Ŀ�� ���� (���� ����)
		FVector Dir = (CursorWorldPos - SpawnOrigin);
		Dir.Z = 0.f;
		Dir = Dir.GetSafeNormal();
		if (Dir.IsNearlyZero()) Dir = Forward2D;

		if (HasAuthority())
		{
			// ȣ��Ʈ(����)�� ���� ĳ�� ���� ����
			ServerCursor = CursorWorldPos;
			ServerCursorDir = Dir;
		}
		else
		{
			// ���� Ŭ��� ������ ����
			ServerRPCSetCursorInfo(CursorWorldPos, Dir);
		}
	}
}

void AMSPlayerController::ServerRPCSetCursorInfo_Implementation(const FVector_NetQuantize& InPos, const FVector_NetQuantizeNormal& InDir)
{
	ServerCursor = FVector(InPos);

	// Ŀ�� ���� ���� (���� ����)
	FVector Dir = FVector(InDir);
	Dir.Z = 0.f;
	Dir = Dir.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = FVector(1.f, 0.f, 0.f);
	}
	ServerCursorDir = Dir;
}
