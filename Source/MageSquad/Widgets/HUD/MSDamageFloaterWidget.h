// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSDamageFloaterWidget.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/19
 *
 * 대미지 플로터 위젯
 * 체력 변화량만큼 로컬 UI에 텍스트를 표시
 */
UCLASS()
class MAGESQUAD_API UMSDamageFloaterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 풀에서 위젯을 가져오는 함수 (없으면 생성)
	static UMSDamageFloaterWidget* Acquire(APlayerController* OwningPC, TSubclassOf<UMSDamageFloaterWidget> WidgetClass);

	// 위젯 표시 시작 함수
	UFUNCTION(BlueprintCallable)
	void Start(float InAmount, bool bInCritical, bool bInHeal, const FVector2D& InScreenPos);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

private:
	// 풀 반환 함수
	void Recycle();

public:
	// 기본 파라미터(코드에서 필요하면 외부에서 수정 가능)
	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI")
	float LifeTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI")
	float RiseSpeed = 60.0f; // px/s (위로 이동)

protected:
	// 대미지 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> DamageTextWidget;

	// 최대 풀 사이즈
	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI")
	int32 MaxPoolSize = 256;

	// 기본 색상
	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI | Color")
	FSlateColor BaseColor = FSlateColor(FLinearColor(0.823f, 0.745f, 0.571f, 1.0f));

	// 회복 색상
	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI | Color")
	FSlateColor HealColor = FSlateColor(FLinearColor(0.207f, 0.823f, 0.068f, 1.0f));

	// 치명타 색상
	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI | Color")
	FSlateColor CriticalColor = FSlateColor(FLinearColor(1.0f, 0.1602f, 0.0f, 1.0f));

private:
	float Amount = 0.f;			// 대미지 수치
	float Elapsed = 0.f;		// 위젯 표시 후 지난 시간
	bool bCritical = false;		// 치명타 여부
	bool bHeal = false;			// 힐 여부

	// 위젯 위치
	FVector2D BaseScreenPos = FVector2D::ZeroVector;

	// 로컬 플레이어
	TWeakObjectPtr<class ULocalPlayer> OwnerLocalPlayer;

	// 위젯 풀 (로컬 플레이어 단위)
	static TMap<ULocalPlayer*, TArray<TWeakObjectPtr<UMSDamageFloaterWidget>>> Pools;
};
