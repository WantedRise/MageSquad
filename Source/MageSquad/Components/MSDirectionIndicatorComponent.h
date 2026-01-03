// Copyright (c) 2025 MageSquad


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MSDirectionIndicatorComponent.generated.h"

 /*
  * 작성자: 김준형
  * 작성일: 26/01/01
  *
  * 방향 표시 인디케이터의 대상이 되는 액터에 부착하는 컴포넌트
  * 방향 표시될 액터들을 구분
  * - 플레이어 캐릭터
  * - 공동 임무
  * - 보스 몬스터
  */
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class MAGESQUAD_API UMSDirectionIndicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMSDirectionIndicatorComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 인디케이터의 월드 위치 반환 함수. (기본적으로 인디케이터 == Owner)
	UFUNCTION(BlueprintCallable, Category = "Custom | Indicator")
	FVector GetIndicatorWorldLocation() const;

	// 인디케이터의 아이콘 반환 함수
	UFUNCTION(BlueprintCallable, Category = "Custom | Indicator")
	UTexture2D* GetIndicatorIcon() const;

	// 이 인디케이터가 로컬 플레이어들에게 표시되어야 하는지 여부를 검사하는 함수
	bool ShouldShowIndicatorForPlayer(APlayerController* LocalPlayerController) const;

public:
	// 인디케이터에 표시할 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	TObjectPtr<UTexture2D> OverrideIcon;

	// 거리 텍스트 표시 여부 (ex. 300M)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	bool bShowDistance = false;

	// 대상(본인)의 인디케이터 가시화 여부 (true이면 표시함)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Indicator")
	bool bRequiresActivation = true;

private:
	// 방향 표시 인디케이터 서브시스템에 등록되었는지 여부
	bool bRegistered = false;

	// HUD 데이터 컴포넌트 캐시
	// 플레이어의 경우, 여기서 아이콘을 가져옴
	UPROPERTY(Transient)
	TObjectPtr<class UMSHUDDataComponent> CachedHUDData = nullptr;
};
