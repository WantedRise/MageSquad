// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSLevelUpFloaterWidget.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/23
 *
 * 레벨업 플로터 위젯
 */
UCLASS()
class MAGESQUAD_API UMSLevelUpFloaterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 풀에서 위젯을 가져오는 함수 (없으면 생성)
	static UMSLevelUpFloaterWidget* Acquire(APlayerController* OwningPC, TSubclassOf<UMSLevelUpFloaterWidget> WidgetClass);

	// 위젯 표시 시작 함수
	UFUNCTION(BlueprintCallable)
	void Start(const FVector2D& InScreenPos);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

private:
	// 풀 반환 함수
	void Recycle();

public:
	// 생명주기
	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI")
	float LifeTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI")
	float RiseSpeed = 60.0f; // px/s (위로 이동)

protected:
	// 레벨업 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> LevelUpTextWidget;

	// 최대 풀 사이즈
	UPROPERTY(EditDefaultsOnly, Category = "Custom | UI")
	int32 MaxPoolSize = 100;
private:
	// 위젯 표시 후 지난 시간
	float Elapsed = 0.f;

	// 위젯 위치
	FVector2D BaseScreenPos = FVector2D::ZeroVector;

	// 로컬 플레이어
	TWeakObjectPtr<class ULocalPlayer> OwnerLocalPlayer;

	// 위젯 풀 (로컬 플레이어 단위)
	static TMap<ULocalPlayer*, TArray<TWeakObjectPtr<UMSLevelUpFloaterWidget>>> Pools;
};
