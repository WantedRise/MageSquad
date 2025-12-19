// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSDamageFloaterWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

TMap<ULocalPlayer*, TArray<TWeakObjectPtr<UMSDamageFloaterWidget>>> UMSDamageFloaterWidget::Pools;

UMSDamageFloaterWidget* UMSDamageFloaterWidget::Acquire(APlayerController* OwningPC, TSubclassOf<UMSDamageFloaterWidget> WidgetClass)
{
	if (!OwningPC || !WidgetClass) return nullptr;

	// 로컬 플레이어 가져오기
	ULocalPlayer* LP = OwningPC->GetLocalPlayer();
	if (!LP) return nullptr;

	// 대미지 플로터 위젯
	UMSDamageFloaterWidget* Widget = nullptr;

	// 풀에 재사용 가능한 위젯이 있는지 탐색
	TArray<TWeakObjectPtr<UMSDamageFloaterWidget>>& Pool = Pools.FindOrAdd(LP);
	while (Pool.Num() > 0)
	{
		// 풀을 하나씩 pop하며 검사. 재사용 가능하면 재사용
		TWeakObjectPtr<UMSDamageFloaterWidget> Candidate = Pool.Pop(EAllowShrinking::No);
		if (Candidate.IsValid())
		{
			Widget = Candidate.Get();
			break;
		}
	}

	// 풀링 가능한 위젯이 없으면 새로 생성
	if (!Widget)
	{
		Widget = CreateWidget<UMSDamageFloaterWidget>(OwningPC, WidgetClass);
	}

	// 위젯을 그릴 로컬 플레이어 설정
	if (Widget)
	{
		Widget->OwnerLocalPlayer = LP;
	}

	return Widget;
}

void UMSDamageFloaterWidget::Start(float InAmount, bool bInCritical, bool bInHeal, const FVector2D& InScreenPos)
{
	// 대미지, 치명타 여부, 힐 여부 초기화
	Amount = InAmount;
	bCritical = bInCritical;
	bHeal = bInHeal;

	// 위젯 생성 후 지난 시간
	Elapsed = 0.f;

	// 위젯 위치: 
	BaseScreenPos = InScreenPos;

	// 초기 배치(레이아웃 변경은 여기서만)
	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(1.f);

	// 레이아웃 비용을 줄이기 위해 PositionInViewport는 1회만 수행
	SetPositionInViewport(BaseScreenPos, true);

	// 이후 프레임은 RenderTranslation만 갱신(레이아웃 비용 최소화)
	SetRenderTranslation(FVector2D::ZeroVector);

	// 텍스트 위젯 설정
	if (DamageTextWidget)
	{
		const int32 IntAmount = FMath::Max(0, FMath::RoundToInt(Amount));
		const FString Prefix = bHeal ? TEXT("+") : TEXT("");
		DamageTextWidget->SetText(FText::FromString(FString::Printf(TEXT("%s%d"), *Prefix, IntAmount)));
	
		if (bHeal) DamageTextWidget->SetColorAndOpacity(HealColor);
		else if (bCritical) DamageTextWidget->SetColorAndOpacity(CriticalColor);
		else DamageTextWidget->SetColorAndOpacity(BaseColor);
	}

	// 뷰포트에 그리기 (이미 그려져있으면 중복 안 함)
	if (!IsInViewport())
	{
		AddToViewport(1000);
	}
}

void UMSDamageFloaterWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Elapsed += InDeltaTime;

	// 위로 떠오르기(렌더 변환만 사용)
	const float Rise = RiseSpeed * Elapsed;
	SetRenderTranslation(FVector2D(0.f, -Rise));

	// 페이드아웃
	const float Alpha = 1.f - (Elapsed / FMath::Max(0.05f, LifeTime));
	SetRenderOpacity(FMath::Clamp(Alpha, 0.f, 1.f));

	// 위젯 생명주기가 지났으면 풀로 반환
	if (Elapsed >= LifeTime)
	{
		Recycle();
	}
}

void UMSDamageFloaterWidget::NativeDestruct()
{
	// 파괴되는 위젯은 풀에서 자연스럽게 약한 참조가 무효화됨
	Super::NativeDestruct();
}

void UMSDamageFloaterWidget::Recycle()
{
	// 뷰포트에서 제거
	RemoveFromParent();
	SetVisibility(ESlateVisibility::Collapsed);

	// 로컬 플레이어 가져오기
	ULocalPlayer* LP = OwnerLocalPlayer.Get();
	if (!LP) return;

	// 약한 참조 풀로 반환(중복 반환 방지)
	TArray<TWeakObjectPtr<UMSDamageFloaterWidget>>& Pool = Pools.FindOrAdd(LP);

	// 풀에 추가
	if (Pool.Num() < MaxPoolSize)
	{
		Pool.Add(this);
	}
}
