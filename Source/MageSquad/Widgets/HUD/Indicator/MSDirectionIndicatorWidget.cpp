// Copyright (c) 2025 MageSquad

#include "Widgets/HUD/Indicator/MSDirectionIndicatorWidget.h"
#include "Widgets/HUD/Indicator/MSDirectionIndicatorItemWidget.h"

#include "Components/MSDirectionIndicatorComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

#include "Player/MSPlayerState.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "System/MSDirectionIndicatorSubsystem.h"

void UMSDirectionIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 방향 표시 인디케이터 갱신 타이머 설정
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			UpdateTimerHandle,
			this,
			&UMSDirectionIndicatorWidget::UpdateIndicator,
			UpdateInterval,
			true,
			1.f
		);
	}
}

void UMSDirectionIndicatorWidget::NativeDestruct()
{
	// 방향 표시 인디케이터 갱신 타이머 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	// 모든 인디케이터 아이템 위젯 반환
	for (auto& Pair : ActiveIndicators)
	{
		// 구조체에서 인디케이터 아이템 위젯 반환
		if (Pair.Value.IndicatorItem)
		{
			ReleaseItemWidget(Pair.Value.IndicatorItem);
		}
	}
	ActiveIndicators.Empty();

	// 풀에 있는 위젯 제거
	for (UMSDirectionIndicatorItemWidget* Widget : IndicatorItemPool)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
	}
	IndicatorItemPool.Empty();

	Super::NativeDestruct();
}

UMSDirectionIndicatorItemWidget* UMSDirectionIndicatorWidget::AcquireItemWidget()
{
	if (!IndicatorItemWidgetClass) return nullptr;

	// 반환할 인디케이터 아이템 위젯
	UMSDirectionIndicatorItemWidget* Result = nullptr;

	if (IndicatorItemPool.Num() > 0)
	{
		// 풀에 데이터가 있으면 가져오기
		Result = IndicatorItemPool.Pop();
	}
	else
	{
		// 없으면 새로 생성
		if (UWorld* World = GetWorld())
		{
			Result = CreateWidget<UMSDirectionIndicatorItemWidget>(World, IndicatorItemWidgetClass);
		}
	}

	if (Result && RootCanvasWidget)
	{
		// 캔버스에 위젯 추가
		if (!Result->GetParent())
		{
			RootCanvasWidget->AddChild(Result);
		}

		// 화면에 보이게 하고, 입력은 받지 않도록 설정
		Result->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	return Result;
}

void UMSDirectionIndicatorWidget::ReleaseItemWidget(UMSDirectionIndicatorItemWidget* Widget)
{
	if (!Widget) return;

	// 위젯 제거
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	if (Widget->GetParent())
	{
		Widget->RemoveFromParent();
	}

	// 풀에 추가
	IndicatorItemPool.Add(Widget);
}

void UMSDirectionIndicatorWidget::UpdateWidgetVisibility()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	// 플레이어 생존 여부
	bool bLocalAlive = true;
	if (const APlayerState* PS = PC->PlayerState)
	{
		if (const AMSPlayerState* MSPC = Cast<AMSPlayerState>(PS))
		{
			bLocalAlive = MSPC->IsAlive();
		}
	}

	// 가시성 업데이트
	if (!bLocalAlive)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UMSDirectionIndicatorWidget::UpdateIndicator()
{
	// ============================================================
	// #1: 위젯 가시성 업데이트 (로컬 생존 여부에 따른 전체 ON/OFF)
	// ============================================================
	UpdateWidgetVisibility();

	// 위젯이 숨김 상태인 경우 화면에 표시할 필요가 없음
	if (GetVisibility() == ESlateVisibility::Collapsed)
	{
		// 현재 활성화된 인디케이터 아이템 위젯들을 전부 풀로 반환
		for (auto& Pair : ActiveIndicators)
		{
			if (Pair.Value.IndicatorItem)
			{
				ReleaseItemWidget(Pair.Value.IndicatorItem);
				Pair.Value.IndicatorItem = nullptr;
			}
		}
		ActiveIndicators.Empty();
		return;
	}


	// ============================================================
	// #2: 뷰포트 기본 정보 확보 / 뷰포트 정보 계산
	// ============================================================
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	// 뷰포트 크기(픽셀) 가져오기
	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());

	// 유효하지 않은 뷰포트 방어
	if (ViewportSize.X <= 0.f || ViewportSize.Y <= 0.f) return;

	// 화면 중앙(레이 투영 기준점)
	const FVector2D Center(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f);


	// ============================================================
	// #3: 히스테리시스(깜빡임 방지)용 사각형 2개 정의
	// 
	//  - OnRect(ActivationInsetPx): "이 영역 안이면 인디케이터 OFF로 전환" 기준
	//  - OffRect(DeactivationInsetPx): "이 영역 안이면 계속 OFF 유지" 기준
	// 
	// 즉, OffRect가 OnRect보다 보통 더 넓거나/다르게 설정되어 경계 근처에서 ON/OFF가 빠르게 토글되는 현상을 줄임
	// ============================================================
	// 인디케이터 표시 시작 경계선 좌표
	const float MinXOn = ActivationInsetPixel;
	const float MaxXOn = ViewportSize.X - ActivationInsetPixel;
	const float MinYOn = ActivationInsetPixel;
	const float MaxYOn = ViewportSize.Y - ActivationInsetPixel;

	// 인디케이터 표시 해제 경계선 좌표
	const float MinXOff = DeactivationInsetPixel;
	const float MaxXOff = ViewportSize.X - DeactivationInsetPixel;
	const float MinYOff = DeactivationInsetPixel;
	const float MaxYOff = ViewportSize.Y - DeactivationInsetPixel;


	// ============================================================
	// #4: 인디케이터를 실제로 붙일 엣지(가장자리) 사각형 정의
	// ============================================================
	// 인디케이터 화면 가장자리 경계선 마진값 좌표
	const float EdgeMinX = EdgeMarginPixel;
	const float EdgeMaxX = ViewportSize.X - EdgeMarginPixel;
	const float EdgeMinY = EdgeMarginPixel;
	const float EdgeMaxY = ViewportSize.Y - EdgeMarginPixel;


	// ============================================================
	// #5: 인디케이터 표시 액터 목록 수집
	// ============================================================
	TArray<UMSDirectionIndicatorComponent*> Sources;
	if (UWorld* World = GetWorld())
	{
		// 현재 서브시스템에 등록된 표시 액터 목록을 가져와 저장
		if (UMSDirectionIndicatorSubsystem* Subsystem = World->GetSubsystem<UMSDirectionIndicatorSubsystem>())
		{
			Subsystem->GetIndicatorSources(Sources);
		}
	}


	// ============================================================
	// #6: 이번 프레임에 정상 처리된 소스 키를 기록
	//  - 프레임 마지막 정리 단계에서, 안 보이거나 사라진 항목을 제거하기 위함
	// ============================================================
	TSet<TWeakObjectPtr<UMSDirectionIndicatorComponent>> SeenKeys;


	// ============================================================
	// #7: 각 소스(인디케이터 표시 액터)에 대해:
	// 1. 표시 가능 여부 필터링
	// 2. 월드 좌표 -> 스크린 좌표 프로젝션
	// 3. 히스테리시스 기반으로 표시해야 하는지 결정
	// 4. 표시해야 하는 경우, 화면에 표시할 가장자리 위치 계산
	// 5. 소스 거리 계산(m)
	// 6. 최종 소스를 SeenKeys에 기록
	// ============================================================
	for (UMSDirectionIndicatorComponent* Source : Sources)
	{
		if (!Source) continue;

		// ============================================================
		// #7-1: 이 인디케이터가 로컬 플레이어들에게 표시되지 않아도 되는 경우
		// ============================================================
		//if (!Source->ShouldShowIndicatorForPlayer(PC))
		//{
		//	// 기존에 활성화 되어있으면 숨김 처리
		//	FIndicatorItem* Existing = ActiveIndicators.Find(Source);
		//	if (Existing)
		//	{
		//		Existing->bVisible = false;
		//	}
		//	continue;
		//}


		// ============================================================
		// #7-2: 월드 위치 -> 스크린(위젯) 좌표로 프로젝션
		// ============================================================
		// FIndicatorItem 준비 (없으면 생성, 있으면 재사용)
		FIndicatorItem& Item = ActiveIndicators.FindOrAdd(Source);
		Item.IndicatorComponent = Source;

		// 프로젝션: 플레이어의 화면 위치에 투영된 월드를 가져온 다음, 이를 위젯 위치로 변환
		FVector WorldLocation = Source->GetIndicatorWorldLocation();
		FVector2D ScreenPos;
		bool bProjected = false;
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldLocation, ScreenPos, false))
		{
			bProjected = true;
		}
		else
		{
			// 프로젝션 실패(카메라 뒤/특이 케이스 고려)
			// 안전하게 Center로 두고, 엣지 사각형 내부로 Clamp
			ScreenPos = Center;
			ScreenPos.X = FMath::Clamp(ScreenPos.X, EdgeMinX, EdgeMaxX);
			ScreenPos.Y = FMath::Clamp(ScreenPos.Y, EdgeMinY, EdgeMaxY);
		}


		// ============================================================
		// #7-3: 히스테리시스 기반 표시 여부 결정
		// 
		//  - Item.bVisible == false (현재 숨김 상태):
		//		OnRect 안쪽이면 계속 숨김
		//		OnRect 바깥이면 표시로 전환
		// 
		//  - Item.bVisible == true (현재 표시 상태):
		//		OffRect 안쪽이면 숨김으로 전환
		//		OffRect 바깥이면 계속 표시 유지
		// ============================================================
		// 표시 시작 경계 안에 있는지 여부
		const bool bInsideOnRect = (
			ScreenPos.X >= MinXOn &&
			ScreenPos.X <= MaxXOn &&
			ScreenPos.Y >= MinYOn &&
			ScreenPos.Y <= MaxYOn);

		// 표시 해제 경계 안에 있는지 여부
		const bool bInsideOffRect = (
			ScreenPos.X >= MinXOff &&
			ScreenPos.X <= MaxXOff &&
			ScreenPos.Y >= MinYOff &&
			ScreenPos.Y <= MaxYOff);

		bool bShouldBeVisible = true;

		if (!Item.bVisible)
		{
			// 현재 숨김이면, OnRect 밖으로 나갔을 때만 표시
			bShouldBeVisible = !bInsideOnRect;
		}
		else
		{
			// 현재 표시면, OffRect 안으로 충분히 들어오면 숨김
			bShouldBeVisible = !bInsideOffRect;
		}

		// 숨김 상태 저장
		Item.bVisible = bShouldBeVisible;

		// 히스테리시스 기반 표시 여부 결정후에도 숨김 상태면 스킵
		if (!bShouldBeVisible) continue;


		// ============================================================
		// #7-4: 표시해야 하는 경우, 화면에 표시할 가장자리 위치 계산
		// 
		//  - Center에서 Screen 방향으로 쏜 레이가 표시 가장자리와 만나는 첫 번째 교점을 구함
		// +-----------------+
		// |				 |
		// |				 |
		// |        + -> -> "|
		// |				 |
		// |				 |
		// +-----------------+
		// ============================================================
		// 가장자리 위치 계산: Center -> ScreenPos 방향 벡터
		FVector2D Delta = ScreenPos - Center;
		float Dx = Delta.X; // X방향 진행량
		float Dy = Delta.Y; // Y방향 진행량

		// 각각 X경계 / Y경계에 닿기 위해 필요한 스케일
		float TX = FLT_MAX;
		float TY = FLT_MAX;

		if (FMath::Abs(Dx) > UE_KINDA_SMALL_NUMBER)
		{
			// 오른쪽이면, 우측 경계(EdgeMaxX), 왼쪽이면 좌측 경계(EdgeMinX)
			TX = (Dx > 0.f) ?
				((EdgeMaxX - Center.X) / Dx) :
				((EdgeMinX - Center.X) / Dx);
		}
		if (FMath::Abs(Dy) > UE_KINDA_SMALL_NUMBER)
		{
			// 아래면, 하단 경계(EdgeMaxY), 위로면 상단 경계(EdgeMinY)
			TY = (Dy > 0.f) ?
				((EdgeMaxY - Center.Y) / Dy) :
				((EdgeMinY - Center.Y) / Dy);
		}

		// 더 작은 t가 사각형 테두리에 먼저 닿는 교점
		float T = FMath::Min(TX, TY);

		if (!FMath::IsFinite(T) || T <= 0.f)
		{
			// 예외/안전 처리: 단순 Clamp
			Item.DesiredPos.X = FMath::Clamp(ScreenPos.X, EdgeMinX, EdgeMaxX);
			Item.DesiredPos.Y = FMath::Clamp(ScreenPos.Y, EdgeMinY, EdgeMaxY);
		}
		else
		{
			// 교점 계산 후, 혹시 튀는 값 방지를 위해 최종 Clamp
			Item.DesiredPos = Center + Delta * T;
			Item.DesiredPos.X = FMath::Clamp(Item.DesiredPos.X, EdgeMinX, EdgeMaxX);
			Item.DesiredPos.Y = FMath::Clamp(Item.DesiredPos.Y, EdgeMinY, EdgeMaxY);
		}


		// ============================================================
		// #7-5: 소스(인디케이터 표시 액터) 거리 계산(m)
		// ============================================================
		float DistMeter = 0.f;
		if (APawn* LocalPawn = PC->GetPawn())
		{
			DistMeter = FVector::Dist(LocalPawn->GetActorLocation(), WorldLocation) / 100.f; /* cm -> m로 변환*/
		}
		Item.Distance = DistMeter;


		// ============================================================
		// #7-6: 이번 프레임에 정상 처리된 소스 키를 기록
		// ============================================================
		SeenKeys.Add(Source);
	}


	// ============================================================
	// #8: 정리
	// ============================================================
	TArray<TWeakObjectPtr<UMSDirectionIndicatorComponent>> KeysToRemove;

	for (auto& Pair : ActiveIndicators)
	{
		TWeakObjectPtr<UMSDirectionIndicatorComponent> Key = Pair.Key;
		FIndicatorItem& Item = Pair.Value;

		/*
		* 1. 소스가 Destroy 된 경우
		* 2. 이번 프레임에 SeenKeys에 포함되지 않았거나
		* 3, bVisible == false인 경우
		*/
		if (!Key.IsValid() || !SeenKeys.Contains(Key) || !Item.bVisible)
		{
			// 위젯 풀 반환
			if (Item.IndicatorItem)
			{
				ReleaseItemWidget(Item.IndicatorItem);
				Item.IndicatorItem = nullptr;
			}
			KeysToRemove.Add(Key);
		}
	}

	// ActiveIndicators에서 제거
	for (const auto& Key : KeysToRemove)
	{
		ActiveIndicators.Remove(Key);
	}


	// ============================================================
	// #9: 슬롯팅(겹침 방지) 준비
	// ============================================================
	// 가장자리 방향별 그룹화
	TArray<FIndicatorItem*> TopGroup;
	TArray<FIndicatorItem*> RightGroup;
	TArray<FIndicatorItem*> BottomGroup;
	TArray<FIndicatorItem*> LeftGroup;

	for (auto& Pair : ActiveIndicators)
	{
		// 숨김 처리되어있으면 스킵
		FIndicatorItem& Item = Pair.Value;
		if (!Item.bVisible) continue;

		// 각 인디케이터가 가장 가까운 엣지에 속하도록 분류
		float DistLeft = FMath::Abs(Item.DesiredPos.X - EdgeMinX);
		float DistRight = FMath::Abs(Item.DesiredPos.X - EdgeMaxX);
		float DistTop = FMath::Abs(Item.DesiredPos.Y - EdgeMinY);
		float DistBottom = FMath::Abs(Item.DesiredPos.Y - EdgeMaxY);

		// 가장 가까웃 엣지의 거리 찾기
		float MinDist = FMath::Min(FMath::Min(DistLeft, DistRight), FMath::Min(DistTop, DistBottom));

		// 해당 그룹에 추가
		if (MinDist == DistLeft)
		{
			LeftGroup.Add(&Item);
		}
		else if (MinDist == DistRight)
		{
			RightGroup.Add(&Item);
		}
		else if (MinDist == DistTop)
		{
			TopGroup.Add(&Item);
		}
		else
		{
			BottomGroup.Add(&Item);
		}
	}


	// ============================================================
	// #10: 슬롯팅(겹침 방지) 수행
	// ============================================================
	// 슬롯팅용 람다 함수 생성
	auto SlottingPass =
		[this, &EdgeMinX, &EdgeMaxX, &EdgeMinY, &EdgeMaxY]
		(TArray<FIndicatorItem*>& Group, bool bHorizontal)
		{
			if (Group.Num() <= 1) return;

			// #10-1: 축 기준 정렬 (원점에 더 가까운 순서로 정렬)
			Group.Sort(
				[bHorizontal](const FIndicatorItem& A, const FIndicatorItem& B)
				{
					return bHorizontal ?
						(A.DesiredPos.X < B.DesiredPos.X) :
						(A.DesiredPos.Y < B.DesiredPos.Y);
				}
			);

			// #10-2: 축 Clamp 범위 설정
			const float MinAxis = bHorizontal ? EdgeMinX : EdgeMaxY;
			const float MaxAxis = bHorizontal ? EdgeMaxX : EdgeMaxY;

			// #10-3: 이전 항목과의 최소 간격 보장
			float PrevAxis = -FLT_MAX;

			for (int32 i = 0; i < Group.Num(); ++i)
			{
				FIndicatorItem* Item = Group[i];
				if (!Item) continue;

				// 현재 위젯 크기 가져오기
				FVector2D Size(0.f, 0.f);
				if (Item->IndicatorItem)
				{
					Size = Item->IndicatorItem->GetDesiredSize();
				}

				// 현재 아이템의 축 값(정렬 축 기준)
				float Axis = bHorizontal ? Item->DesiredPos.X : Item->DesiredPos.Y;

				// i>0이면 이전 아이템과 겹치지 않도록 최소 요구 축 값 계산
				if (i > 0)
				{
					// 이전 아이템 위젯의 크기
					float PrevSizeAlongAxis = 0.f;

					// 이전 아이템 유효 검사
					if (Group[i - 1] && Group[i - 1]->IndicatorItem)
					{
						FVector2D PrevSize = Group[i - 1]->IndicatorItem->GetDesiredSize();
						PrevSizeAlongAxis = bHorizontal ? PrevSize.X : PrevSize.Y;
					}

					// 최소 간격 계산
					float RequiredMin = PrevAxis + PrevSizeAlongAxis + SlotGapPixel;

					// 현재 아이템의 축 값이 최소 간격보다 작으면 최소 간격으로 보정
					if (Axis < RequiredMin)
					{
						Axis = RequiredMin;
					}
				}

				// 화면 가장자리 마진값을 벗어나지 않도록 Clamp
				Axis = FMath::Clamp(Axis, MinAxis, MaxAxis);

				// 결과를 아이템 위젯 위치에 반영
				if (bHorizontal)
				{
					Item->DesiredPos.X = Axis;
				}
				else
				{
					Item->DesiredPos.Y = Axis;
				}

				PrevAxis = Axis;
			}
		};

	// 가장자리 방향별 그룹에 대해서 슬롯팅(겹침 방지) 수행
	SlottingPass(LeftGroup, false);
	SlottingPass(RightGroup, false);
	SlottingPass(TopGroup, true);
	SlottingPass(BottomGroup, true);


	// ============================================================
	// #11: UI(위젯) 적용
	// ============================================================
	for (auto& Pair : ActiveIndicators)
	{
		FIndicatorItem& Item = Pair.Value;
		if (!Item.bVisible) continue;

		// #11-1: 아이템 위젯이 없으면 풀에서 가져오고, 대상 소스 바인딩
		if (!Item.IndicatorItem)
		{
			Item.IndicatorItem = AcquireItemWidget();
			if (Item.IndicatorItem)
			{
				Item.IndicatorItem->SetIndicatorComponent(Item.IndicatorComponent.Get());
			}
		}

		// 풀에서 가져오지 못한 경우 방어
		if (!Item.IndicatorItem) continue;

		// #11-2: Canvas 상에서 위치/정렬 설정
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Item.IndicatorItem->Slot))
		{
			// 절대 좌표 기반 배치
			CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));

			// 중앙 정렬
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));

			// 앞에서 계산한 가장자리 위치로 이동
			CanvasSlot->SetPosition(Item.DesiredPos);
		}

		// #11-3: 아이콘 텍스쳐 반영
		UTexture2D* Icon = Item.IndicatorComponent.IsValid() ? Item.IndicatorComponent->GetIndicatorIcon() : nullptr;
		Item.IndicatorItem->SetIcon(Icon);

		// #11-5: 거리 표시 여부 검사 + 값 반영
		const bool bShowDist = bShowDistance && Item.IndicatorComponent.IsValid() && Item.IndicatorComponent->bShowDistance;
		Item.IndicatorItem->SetDistance(bShowDist, Item.Distance);

		// #11-6: 사망 오버레이 표시 설정
		Item.IndicatorItem->SetDeadOverlayVisible(false);
	}
}
