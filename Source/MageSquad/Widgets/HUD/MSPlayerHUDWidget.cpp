// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSPlayerHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"

#include "Player/MSPlayerController.h"
#include "Player/MSPlayerState.h"

void UMSPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// HUD 위젯 초기화
	InitializeHUD();
}

void UMSPlayerHUDWidget::NativeDestruct()
{
	// 바인딩된 델리게이트/타이머 정리
	ClearRebindTimer();
	UnbindLocalHealth();

	Super::NativeDestruct();
}

void UMSPlayerHUDWidget::InitializeHUD()
{
	// 이미 바인딩 되어 있으면 그대로 유지(재진입 안전)
	if (bBoundLocalASC && LocalASC.IsValid())
	{
		RefreshLocalHealthUI(CachedHealth, CachedMaxHealth);
		return;
	}

	// 바인딩 시도
	if (TryBindLocalHealth())
	{
		ClearRebindTimer();
		return;
	}

	// 실패하면 일정 주기로 재시도
	ScheduleRebind();
}

void UMSPlayerHUDWidget::RequestReinitialize()
{
	// 바인딩된 델리게이트/타이머 정리
	ClearRebindTimer();
	UnbindLocalHealth();

	// HUD 위젯 초기화 (재초기화)
	InitializeHUD();
}

bool UMSPlayerHUDWidget::TryBindLocalHealth()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return false;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return false;

	UAbilitySystemComponent* FoundASC = nullptr;

	// #1: Pawn이 ASC를 제공하는 경우
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
	{
		FoundASC = ASI->GetAbilitySystemComponent();
	}

	// #2: PlayerState에서 ASC 찾기
	if (!IsValid(FoundASC))
	{
		if (APlayerState* PS = PC->PlayerState)
		{
			if (AMSPlayerState* MSPS = Cast<AMSPlayerState>(PS))
			{
				FoundASC = MSPS->GetAbilitySystemComponent();
			}
		}
	}

	if (!IsValid(FoundASC)) return false;

	// ASC에서 아직 ActorInfo(Owner/Avatar) 초기화가 이뤄지지 않은 경우
	if (!FoundASC->GetOwnerActor() || !FoundASC->GetAvatarActor()) return false;

	// 기존 바인딩 정리 후 새 ASC로 바인딩
	UnbindLocalHealth();
	LocalASC = FoundASC;

	// 초기 값 반영
	CachedHealth = FoundASC->GetNumericAttribute(UMSPlayerAttributeSet::GetHealthAttribute());
	CachedMaxHealth = FoundASC->GetNumericAttribute(UMSPlayerAttributeSet::GetMaxHealthAttribute());
	RefreshLocalHealthUI(CachedHealth, CachedMaxHealth);

	// 델리게이트 바인딩(핸들을 보관해 정확히 해제)
	HealthChangedHandle = FoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UMSPlayerHUDWidget::OnLocalHealthChanged);

	MaxHealthChangedHandle = FoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UMSPlayerHUDWidget::OnLocalMaxHealthChanged);

	// 현재 바인딩 상태 설정 및 카운트 초기화
	bBoundLocalASC = true;
	RebindAttemptCount = 0;
	return true;
}

void UMSPlayerHUDWidget::UnbindLocalHealth()
{
	bBoundLocalASC = false;

	// 바인딩한 델리게이트 언바인드
	if (UAbilitySystemComponent* ASC = LocalASC.Get())
	{
		if (HealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
			HealthChangedHandle.Reset();
		}
		if (MaxHealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
			MaxHealthChangedHandle.Reset();
		}
	}

	LocalASC.Reset();
}

void UMSPlayerHUDWidget::ClearRebindTimer()
{
	// 바인딩 재시도 타이머 초기화
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RebindTimer);
	}
	RebindTimer.Invalidate();
	RebindAttemptCount = 0;
}

void UMSPlayerHUDWidget::RefreshLocalHealthUI(const float Health, const float MaxHealth)
{
	// 현재 체력 비율 계산 (ex. 0.1 = 10%)
	const float Pct = (MaxHealth > 0.f) ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 0.f;

	// 체력 바 갱신
	if (LocalHealthBarWidget)
	{
		LocalHealthBarWidget->SetPercent(Pct);
	}

	// 체력 바 텍스트 갱신
	if (LocalHealthTextWidget)
	{
		// "현재 체력/최대 체력"으로 표시 (ex. "264/600") 
		const FText Cur = FText::AsNumber(FMath::RoundToInt(Health));
		const FText Max = FText::AsNumber(FMath::RoundToInt(MaxHealth));
		LocalHealthTextWidget->SetText(FText::Format(NSLOCTEXT("MSHUD", "LocalHealthFmt", "{0}/{1}"), Cur, Max));
	}
}

void UMSPlayerHUDWidget::OnLocalHealthChanged(const FOnAttributeChangeData& Data)
{
	// 현재 체력 갱신 후 UI 업데이트
	CachedHealth = Data.NewValue;
	RefreshLocalHealthUI(CachedHealth, CachedMaxHealth);
}

void UMSPlayerHUDWidget::OnLocalMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	// 최대 체력 갱신 후 UI 업데이트
	CachedMaxHealth = Data.NewValue;
	RefreshLocalHealthUI(CachedHealth, CachedMaxHealth);
}

void UMSPlayerHUDWidget::ScheduleRebind()
{
	if (RebindTimer.IsValid()) return;

	// 바인딘 재시도 타이머 설정
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RebindTimer,
			this,
			&UMSPlayerHUDWidget::TickRebind,
			RebindRetryInterval,
			true
		);
	}
}

void UMSPlayerHUDWidget::TickRebind()
{
	// 최대 시도 제한이 있으면 초과 시 중단
	if (MaxRebindAttempts > 0 && RebindAttemptCount++ >= MaxRebindAttempts)
	{
		ClearRebindTimer();
		return;
	}

	// 바인딩 재시도
	if (TryBindLocalHealth())
	{
		ClearRebindTimer();
	}
}
