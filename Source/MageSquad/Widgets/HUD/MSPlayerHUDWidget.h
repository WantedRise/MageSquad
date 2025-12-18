// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "MSPlayerHUDWidget.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/17
 *
 * 플레이어 HUD 위젯
 * - 로컬 체력바
 * - 로컬 스킬/쿨다운(예정)
 * - 팀원 상태(자신 제외)(예정)
 * - 공유 경험치/레벨(예정)
 */
UCLASS()
class MAGESQUAD_API UMSPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// HUD 위젯 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "Custom | HUD")
	void InitializeHUD();

	// HUD 위젯 재초기화 함수
	// Pawn/ASC가 바뀌었거나(접속/리스폰) 초기화 순서가 꼬였을 때, 현재 바인딩을 정리하고 다시 바인딩을 시도
	UFUNCTION(BlueprintCallable, Category = "Custom | HUD")
	void RequestReinitialize();

protected:
	// 로컬 체력 바인딩 시도 함수 (바인딩 성공 여부에 따라 결괏값 반환)
	bool TryBindLocalHealth();

	// 바인딩된 델리게이트/타이머 정리 함수
	void UnbindLocalHealth();
	void ClearRebindTimer();

	// 체력 변경 UI 반영
	void RefreshLocalHealthUI(float Health, float MaxHealth);

	// AttributeSet 갱신 콜백 함수
	void OnLocalHealthChanged(const FOnAttributeChangeData& Data);
	void OnLocalMaxHealthChanged(const FOnAttributeChangeData& Data);

	// 바인딩 재시도 함수 (클라이언트 순서 꼬임 대응용)
	void ScheduleRebind();
	void TickRebind();

protected:
	/* ======================== BindWidget ======================== */
	// 로컬 체력 바 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UProgressBar> LocalHealthBarWidget;

	// 로컬 체력 바 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> LocalHealthTextWidget;
	/* ======================== BindWidget ======================== */


	// 바인딘 재시도 간격(초)
	UPROPERTY(EditDefaultsOnly, Category = "Custom | HUD")
	float RebindRetryInterval = 0.10f;

	// 과도한 바인딩 재시도를 방지하기 위한 최대 시도 횟수 (0이면 무제한)
	UPROPERTY(EditDefaultsOnly, Category = "Custom | HUD")
	int32 MaxRebindAttempts = 200; // 0.1s * 200 = 약 20초

private:
	// 캐시된 현재/최대 체력
	float CachedHealth = 0.f;
	float CachedMaxHealth = 0.f;

	// 바인딩 델리게이트 핸들(중복 바인딩/정확한 해제 목적)
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;

	// 현재 바인딩 상태
	bool bBoundLocalASC = false;

	// 바인딩 재시도 타이머, 재시도 횟수 카운트
	FTimerHandle RebindTimer;
	int32 RebindAttemptCount = 0;

	// 현재 바인딩된 ASC (소유/생명주기는 PlayerState/Pawn이 관리)
	TWeakObjectPtr<class UAbilitySystemComponent> LocalASC;
};
