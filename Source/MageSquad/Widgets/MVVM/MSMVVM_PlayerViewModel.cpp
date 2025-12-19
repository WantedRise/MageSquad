// Fill out your copyright notice in the Description page of Project Settings.


// MSMVVM_PlayerViewModel.cpp
#include "MSMVVM_PlayerViewModel.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"

void UMSMVVM_PlayerViewModel::InitializeWithASC(UAbilitySystemComponent* ASC)
{
    if (!ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("MSMVVM_PlayerViewModel: ASC is null"));
        return;
    }

    // 기존 바인딩 정리
    UninitializeFromASC();

    CachedASC = ASC;

    // AttributeSet 가져오기
    const UMSPlayerAttributeSet* PlayerAttributeSet = ASC->GetSet<UMSPlayerAttributeSet>();
    if (!PlayerAttributeSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("MSMVVM_PlayerViewModel: PlayerAttributeSet not found"));
        return;
    }

    // Health Attribute 변경 델리게이트 바인딩
    HealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(
        PlayerAttributeSet->GetHealthAttribute()
    ).AddUObject(this, &UMSMVVM_PlayerViewModel::OnHealthChanged);

    // MaxHealth Attribute 변경 델리게이트 바인딩
    MaxHealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(
        PlayerAttributeSet->GetMaxHealthAttribute()
    ).AddUObject(this, &UMSMVVM_PlayerViewModel::OnMaxHealthChanged);

    // 초기값 설정
    SetCurrentHealth(PlayerAttributeSet->GetHealth());
    SetMaxHealth(PlayerAttributeSet->GetMaxHealth());
    UpdateHealthValues();

    UE_LOG(LogTemp, Log, TEXT("MSMVVM_PlayerViewModel: Initialized with ASC. Initial Health: %f/%f"),
        CurrentHealth, MaxHealth);
}

void UMSMVVM_PlayerViewModel::UninitializeFromASC()
{
    if (CachedASC)
    {
        if (HealthChangedDelegateHandle.IsValid())
        {
            if (const UMSPlayerAttributeSet* PlayerAttributeSet = CachedASC->GetSet<UMSPlayerAttributeSet>())
            {
                CachedASC->GetGameplayAttributeValueChangeDelegate(
                    PlayerAttributeSet->GetHealthAttribute()
                ).Remove(HealthChangedDelegateHandle);
            }
            HealthChangedDelegateHandle.Reset();
        }

        if (MaxHealthChangedDelegateHandle.IsValid())
        {
            if (const UMSPlayerAttributeSet* PlayerAttributeSet = CachedASC->GetSet<UMSPlayerAttributeSet>())
            {
                CachedASC->GetGameplayAttributeValueChangeDelegate(
                    PlayerAttributeSet->GetMaxHealthAttribute()
                ).Remove(MaxHealthChangedDelegateHandle);
            }
            MaxHealthChangedDelegateHandle.Reset();
        }

        CachedASC = nullptr;
    }

    StopBlinking();
}

FLinearColor UMSMVVM_PlayerViewModel::GetDamageOverlayColor() const
{
    // 원하는 기본 오버레이 색상을 설정합니다 (예: 빨간색)
    FLinearColor OverlayColor = FLinearColor::White;
    
    // 계산된 Alpha 값을 적용합니다.
    OverlayColor.A = DamageOverlayAlpha;
    
    return OverlayColor;
}

void UMSMVVM_PlayerViewModel::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    SetCurrentHealth(Data.NewValue);
    UpdateHealthValues();

    UE_LOG(LogTemp, Verbose, TEXT("MSMVVM_PlayerViewModel: Health changed to %f"), Data.NewValue);
}

void UMSMVVM_PlayerViewModel::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
    SetMaxHealth(Data.NewValue);
    UpdateHealthValues();

    UE_LOG(LogTemp, Verbose, TEXT("MSMVVM_PlayerViewModel: MaxHealth changed to %f"), Data.NewValue);
}

void UMSMVVM_PlayerViewModel::UpdateHealthValues()
{
    // 정규화된 Health 계산
    float NormalizedHealth = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
    SetHealth(NormalizedHealth);
    
    UpdateLowHealthState();
}

void UMSMVVM_PlayerViewModel::UpdateLowHealthState()
{
    // 30% 이하 체크
    bool bShouldBeLowHealth = Health <= 0.3f && Health > 0.0f;
    
    if (bIsLowHealth != bShouldBeLowHealth)
    {
        SetbIsLowHealth(bShouldBeLowHealth);
        
        if (bShouldBeLowHealth)
        {
            StartBlinking();
        }
        else
        {
            StopBlinking();
        }
    }
}

// Setter 구현들
void UMSMVVM_PlayerViewModel::SetHealth(float NewHealth)
{
    UE_MVVM_SET_PROPERTY_VALUE(Health, NewHealth);
}

void UMSMVVM_PlayerViewModel::SetMaxHealth(float NewMaxHealth)
{
    UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, NewMaxHealth);
}

void UMSMVVM_PlayerViewModel::SetCurrentHealth(float NewCurrentHealth)
{
    UE_MVVM_SET_PROPERTY_VALUE(CurrentHealth, NewCurrentHealth);
}

void UMSMVVM_PlayerViewModel::SetbIsLowHealth(bool bNewIsLowHealth)
{
    UE_MVVM_SET_PROPERTY_VALUE(bIsLowHealth, bNewIsLowHealth);
}

void UMSMVVM_PlayerViewModel::SetDamageOverlayAlpha(float NewAlpha)
{
    UE_MVVM_SET_PROPERTY_VALUE(DamageOverlayAlpha, FMath::Clamp(NewAlpha, 0.0f, 1.0f));
}

// 깜빡임 구현
void UMSMVVM_PlayerViewModel::StartBlinking()
{
    if (UWorld* World = GetWorld())
    {
        if (BlinkTimerHandle.IsValid())
        {
            World->GetTimerManager().ClearTimer(BlinkTimerHandle);
        }

        bBlinkIncreasing = true;
        SetDamageOverlayAlpha(0.0f);
        
        World->GetTimerManager().SetTimer(
            BlinkTimerHandle,
            this,
            &UMSMVVM_PlayerViewModel::UpdateDamageOverlayBlink,
            0.016f,
            true
        );
    }
}

void UMSMVVM_PlayerViewModel::StopBlinking()
{
    if (UWorld* World = GetWorld())
    {
        if (BlinkTimerHandle.IsValid())
        {
            World->GetTimerManager().ClearTimer(BlinkTimerHandle);
            BlinkTimerHandle.Invalidate();
        }
        
        SetDamageOverlayAlpha(0.0f);
    }
}

void UMSMVVM_PlayerViewModel::UpdateDamageOverlayBlink()
{
    float DeltaTime = 0.016f;
    float AlphaChange = BlinkSpeed * DeltaTime;
    
    if (bBlinkIncreasing)
    {
        float NewAlpha = DamageOverlayAlpha + AlphaChange;
        if (NewAlpha >= 1.0f)
        {
            NewAlpha = 1.0f;
            bBlinkIncreasing = false;
        }
        SetDamageOverlayAlpha(NewAlpha);
    }
    else
    {
        float NewAlpha = DamageOverlayAlpha - AlphaChange;
        if (NewAlpha <= 0.0f)
        {
            NewAlpha = 0.0f;
            bBlinkIncreasing = true;
        }
        SetDamageOverlayAlpha(NewAlpha);
    }
}

