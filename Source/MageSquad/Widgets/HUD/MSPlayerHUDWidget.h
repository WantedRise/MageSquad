// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "Components/Player/MSHUDDataComponent.h"
#include "MSPlayerHUDWidget.generated.h"

// 대미지를 받았을 때 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTakeDamaged);

// 로컬 스킬 슬롯 데이터 구조체
USTRUCT(BlueprintType)
struct FMSHUDSkillSlotLocalData
{
	GENERATED_BODY()

	// 스킬 시작 시간
	UPROPERTY(BlueprintReadOnly, Category = "Custom | SkillSlotData")
	float CooldownStartTime = 0.f;

	// 스킬 쿨다운 지속시간
	UPROPERTY(BlueprintReadOnly, Category = "Custom | SkillSlotData")
	float CooldownDuration = 0.f;

	// 스킬이 현재 쿨다운 중인지 표시
	UPROPERTY(BlueprintReadOnly, Category = "Custom | SkillSlotData")
	bool bSlotOnCooldown = false;

	// 스킬 기본 쿨타임
	UPROPERTY(BlueprintReadOnly, Category = "Custom | SkillSlotData")
	float BaseCoolTime = 0.f;

	// 스킬 타입
	UPROPERTY(BlueprintReadOnly, Category = "Custom | SkillSlotData")
	uint8 SkillTypes = 0;

	// 스킬 쿨타임 태그
	UPROPERTY(BlueprintReadOnly, Category = "Custom | SkillSlotData")
	FGameplayTag SkillCooldownTags = FGameplayTag();
};


/**
 * 작성자: 김준형
 * 작성일: 25/12/17
 *
 * 플레이어 HUD 위젯
 * - 로컬 체력바
 * - 팀원 상태(자신 제외) + 팀원 스킬
 * - 공유 경험치/레벨
 * - 로컬 스킬 슬롯 + 쿨타임 시각화
 */
UCLASS()
class MAGESQUAD_API UMSPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;


	// HUD 위젯 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "Custom | HUD")
	void InitializeHUD();

	// HUD 위젯 재초기화 함수
	// Pawn/ASC가 바뀌었거나(접속/리스폰) 초기화 순서가 꼬였을 때, 현재 바인딩을 정리하고 다시 바인딩을 시도
	UFUNCTION(BlueprintCallable, Category = "Custom | HUD")
	void RequestReinitialize();


	// 관전 UI 레이아웃 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Custom | HUD | Spectate")
	void SetSpectateUI(bool bInSpectating, const FText& InTargetPlayerName);

	// 관전 상태 변경 이벤트 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Custom | HUD | Spectate")
	void BP_OnSpectateStateChanged(bool bInSpectating);


	class UMSMissionNotifyWidget* GetMissionNotifyWidget() const { return MissionNotifyWidget; }
	class UMSMissionTrackerWidget* GetMissionTrackerWidget() const { return MissionTrackerWidget; }
	class UMSGameProgressWidget* GetGameProgressWidget() const { return GameProgressWidget; }

protected:
	// 바인딩 시도 함수 (바인딩 성공 여부에 따라 결괏값 반환)
	bool TryBindLocalHealth(); // 현재/최대 체력 바인딩
	bool TryBindSharedData(); // 공유 자원 바인딩
	bool TryBindLocalSkills(); // 로컬 스킬 바인딩
	void TryBindGameState();

	// 팀 데이터 갱신 시작 함수
	void StartTeamPoll();

	// 바인딩된 델리게이트/타이머 정리 함수
	void UnbindLocalHealth(); // 현재/최대 체력 바인딩 해제
	void UnbindSharedData(); // 공유 자원 바인딩 해제
	void UnbindLocalSkills(); // 로컬 스킬 바인딩 해제

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

	// 공유 경험치/레벨 갱신 UI 반영 함수
	void RefreshSharedExperienceUI();

	// 공유 목숨 변경 콜백 함수
	void OnSharedLivesChanged(int32 NewLives);
	
	UFUNCTION()
	void SetHudVisibility(bool Result);

public:
	// 대미지를 받았을 때 이벤트 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnTakeDamaged OnTakeDamaged;

protected:
	/* ======================== BindWidget ======================== */
	// HUD의 요소들을 가지고 있는 메인 캔버스
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UCanvasPanel> MainCanvas;
	
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

	// 남은 공유 목숨 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> SharedLivesTextWidget;

	// 화면 효과 이미지
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UImage> HitEffectImageWidget;

	// 현재 관전 대상 플레이어 이름 텍스트 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UTextBlock> SpectateTargetNameTextWidget;

	// 방향 표시 인디케이터 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UMSDirectionIndicatorWidget> DirectionIndicatorWidget;

	// 미션 알림 위젯 클래스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UMSMissionNotifyWidget> MissionNotifyWidget;

	// 미션 진행 위젯 클래스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UMSMissionTrackerWidget> MissionTrackerWidget;

	//게임 진행도 위젯 클래스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UMSGameProgressWidget> GameProgressWidget;
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

	// 현재 바인딩 상태
	bool bBoundLocalASC = false;

	// 현재 공유 데이터 갱신중 상태
	bool bTeamPolling = false;

	// 현재 관전 상태
	bool bSpectateUIActive = false;

	// 현재 관전 대상 이름
	FText SpectateTargetPlayerName;

	// 바인딩 재시도 타이머, 재시도 횟수 카운트
	FTimerHandle RebindTimer;
	int32 RebindAttemptCount = 0;

	// 공유 데이터 갱신 타이머
	FTimerHandle TeamPollTimer;

	/* ======================== Delegate Handles ======================== */
	// 현재/최대 체력 바인딩 델리게이트 핸들(중복 바인딩/정확한 해제 목적)
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;

	// 공유 경험치/레벨 바인딩 델리게이트 핸들(중복 바인딩/정확한 해제 목적)
	FDelegateHandle SharedExpChangedHandle;
	FDelegateHandle SharedLevelUpHandle;

	// 공유 목숨 변경 이벤트 델리게이트 핸들
	FDelegateHandle SharedLivesChangedHandle;
	/* ======================== Delegate Handles ======================== */

	// 로컬 플레이어 캐릭터 캐시
	TWeakObjectPtr<class AMSPlayerCharacter> CachedLocalCharacter;

	// [팀 멤버 / 데이터 위젯 인스턴스] 캐시
	TMap<TWeakObjectPtr<AActor>, TObjectPtr<class UMSTeamMemberWidget>> TeamMembers;

	// 현재 바인딩된 ASC (소유/생명주기는 PlayerState/Pawn이 관리)
	TWeakObjectPtr<class UAbilitySystemComponent> LocalASC;

	// 캐시된 GameState
	UPROPERTY(Transient)
	TObjectPtr<class AMSGameState> CachedGameState = nullptr;



	/*****************************************************
	* Skill Slot Section
	*****************************************************/
protected:
	// Tick에서 호출하여 쿨다운 진행률을 계산하고 이벤트를 전달하는 함수
	void UpdateCooldowns(float DeltaTime);

	// 슬롯 위젯 인스턴스 배열 초기화 함수
	void InitializeSkillBar();

	// HUD에 표시하는 스킬 슬롯 데이터 변경 시 호출되는 콜백 함수
	void HandleSkillSlotDataUpdated();

	// 스킬 슬롯 배열 변경 시 호출되는 콜백 함수 (내부 쿨타임/타입/태그 정보 갱신)
	void HandleSkillSlotsUpdated();

	// 능력치 쿨타임 감소 속성 변경 콜백 함수
	void OnLocalCooldownReductionChanged(const FOnAttributeChangeData& Data);

	// 스킬 쿨다운 시작 콜백 함수
	void StartCooldownForSlot(uint8 SlotIndex, float Duration);

	// 블링크 스킬 쿨다운 시작 콜백 함수
	void HandleBlinkSkillCooldownStarted();

	// 로컬 스킬 슬롯 데이터를 기반으로 액티브/패시브 스킬 슬롯의 인덱스 목록을 갱신하는 함수
	void BuildSlotTypeIndices();

	// 현재 캐시된 쿨다운 감소 비율(CurrentCDR)을 기반으로 패시브 슬롯의 쿨타임 지속시간을 재계산하는 함수
	void RecalculatePassiveDurations();

protected:
	// 로컬 HUD용 개별 스킬 슬롯 위젯
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UMSSkillSlotWidget> SlotActiveLeftWidget;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UMSSkillSlotWidget> SlotActiveRightWidget;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UMSSkillSlotWidget> SlotPassive01Widget;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UMSSkillSlotWidget> SlotPassive02Widget;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UMSSkillSlotWidget> SlotPassive03Widget;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UMSSkillSlotWidget> SlotPassive04Widget;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Transient)
	TObjectPtr<class UMSSkillSlotWidget> SlotBlinkWidget;

	// 슬롯 위젯 인스턴스 배열
	// 0: ActiveLeft, 1: ActiveRight, 2~5: Passive01~04
	TArray<TObjectPtr<class UMSSkillSlotWidget>> SkillSlotWidgets;

	// 블링크 슬롯 위젯 인스턴스
	TObjectPtr<class UMSSkillSlotWidget> BlinkSkillSlotWidget;

	// 블링크 스킬 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Skill")
	TSoftObjectPtr<UMaterialInterface> BlinkSkillIcon;

private:
	// 스킬 슬롯 크기. 항상 6칸 (0~1번 슬롯은 액티브, 2~5번 슬롯은 패시브)
	const int32 TotalSlots = 6;

	// 스킬 슬롯 바인딩 상태
	bool bBoundLocalSkills = false;

	// 현재 스킬 쿨다운 감소 비율 캐시. (0 ~ 0.95 사이)
	float CurrentCDR = 0.f;

	// 로컬 스킬 슬롯 데이터
	TArray<FMSHUDSkillSlotLocalData> SkillSlotLocalDatas;

	// 액티브 스킬 슬롯의 인덱스 배열
	TArray<int32> ActiveSlotIndices;

	// 패시브 스킬 슬롯의 인덱스 배열
	TArray<int32> PassiveSlotIndices;

	// 스킬 슬롯 업데이트, 쿨다운 시작, 스킬 데이터 업데이트, 쿨다운 변경 델리게이트 핸들
	FDelegateHandle SkillSlotsUpdatedHandle;
	FDelegateHandle SkillCooldownStartedHandle;
	FDelegateHandle SkillSlotDataUpdatedHandle;
	FDelegateHandle CooldownReductionChangedHandle;

	FDelegateHandle BlinkSkillCooldownStartedHandle;

	FMSHUDSkillSlotData BlinkSkillSlotData;
};
