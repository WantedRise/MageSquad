// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "MSPlayerHUDWidget.generated.h"

// 대미지를 받았을 때 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTakeDamaged);

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

	class UMSMissionNotifyWidget* GetMissionNotifyWidget() const { return MissionNotifyWidget; }
	class UMSMissionTrackerWidget* GetMissionTrackerWidget() const { return MissionTrackerWidget; }
	class UGameProgressWidget* GetGameProgressWidget() const { return GameProgressWidget; }
protected:
	// 바인딩 시도 함수 (바인딩 성공 여부에 따라 결괏값 반환)
	bool TryBindLocalHealth();
	bool TryBindSharedExperience();

	// 팀 데이터 갱신 시작 함수
	void StartTeamPoll();

	// 바인딩된 델리게이트/타이머 정리 함수
	void UnbindLocalHealth();
	void UnbindSharedExperience();
	void ClearRebindTimer();
	void ClearTeamPollTimer();

	// 체력 변경 UI 반영
	void RefreshLocalHealthUI(float Health, float MaxHealth, bool InbTakeDamage);

	// AttributeSet 갱신 콜백 함수
	void OnLocalHealthChanged(const FOnAttributeChangeData& Data);
	void OnLocalMaxHealthChanged(const FOnAttributeChangeData& Data);

	// 팀 멤버 리스트 갱신 함수
	void PollTeamMembers();

	// 팀 멤버 위젯 생성/재사용 함수
	void EnsureTeamMemberWidget(class AActor* MemberActor, class UMSHUDDataComponent* HUDData);

	// 바인딩 재시도 함수 (클라이언트 순서 꼬임 대응용)
	void ScheduleRebind();
	void TickRebind();

	// 공유 경험치/레벨 갱신 콜백 함수
	void OnSharedExperienceChanged();
	void OnSharedLevelUp(int32 NewLevel);

	// 공유 경험피/레벨 갱신 UI 반영 함수
	void RefreshSharedExperienceUI();

public:
	// 대미지를 받았을 때 이벤트 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnTakeDamaged OnTakeDamaged;

protected:
	/* ======================== BindWidget ======================== */
	// 로컬 체력 바 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UProgressBar> LocalHealthBarWidget;

	// 로컬 체력 바 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> LocalHealthTextWidget;

	// 공유 레벨 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> SharedLevelTextWidget;

	// 공유 경험치 바 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UProgressBar> SharedExpBarWidget;

	// 팀 멤버 목록 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UVerticalBox> TeamMembersBoxWidget;

	// 화면 효과 이미지
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UImage> HitEffectImageWidget;

	// 미션 알림 위젯 클래스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UMSMissionNotifyWidget> MissionNotifyWidget;

	// 미션 진행 위젯 클래스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UMSMissionTrackerWidget> MissionTrackerWidget;

	//게임 진행도 위젯 클래스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UGameProgressWidget> GameProgressWidget;

	/* ======================== BindWidget ======================== */

	// 팀 멤버 데이터 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | HUD")
	TSubclassOf<class UMSTeamMemberWidget> TeamMemberWidgetClass;

	// 공유 데이터 갱신 주기(초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | HUD")
	float TeamPollInterval = 0.25f;

	// 바인딩 재시도 간격(초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | HUD")
	float RebindRetryInterval = 0.05f;

	// 과도한 바인딩 재시도를 방지하기 위한 최대 시도 횟수 (0이면 무제한)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | HUD")
	int32 MaxRebindAttempts = 200; // 0.1s * 200 = 약 20초

private:
	// 캐시된 현재/최대 체력
	float CachedHealth = 0.f;
	float CachedMaxHealth = 0.f;

	/* ======================== Delegate Handles ======================== */
	// 현재/최대 체력 바인딩 델리게이트 핸들(중복 바인딩/정확한 해제 목적)
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;

	// 공유 경험치/레벨 바인딩 델리게이트 핸들(중복 바인딩/정확한 해제 목적)
	FDelegateHandle SharedExpChangedHandle;
	FDelegateHandle SharedLevelUpHandle;
	/* ======================== Delegate Handles ======================== */

	// 현재 바인딩 상태
	bool bBoundLocalASC = false;

	// 현재 공유 데이터 갱신중 상태
	bool bTeamPolling = false;

	// 바인딩 재시도 타이머, 재시도 횟수 카운트
	FTimerHandle RebindTimer;
	int32 RebindAttemptCount = 0;

	// 공유 데이터 갱신 타이머
	FTimerHandle TeamPollTimer;

	// [팀 멤버 / 데이터 위젯 인스턴스] 캐시
	TMap<TWeakObjectPtr<AActor>, TObjectPtr<class UMSTeamMemberWidget>> TeamMembers;

	// 현재 바인딩된 ASC (소유/생명주기는 PlayerState/Pawn이 관리)
	TWeakObjectPtr<class UAbilitySystemComponent> LocalASC;

	// 캐시된 GameState
	UPROPERTY(Transient)
	TObjectPtr<class AMSGameState> CachedGameState = nullptr;
};
