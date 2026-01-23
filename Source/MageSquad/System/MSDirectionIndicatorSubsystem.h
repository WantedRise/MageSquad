// Copyright Notice

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MSDirectionIndicatorSubsystem.generated.h"

class UMSDirectionIndicatorComponent;

/*
 * 작성자: 김준형
 * 작성일: 26/01/01
 *
 * 방향 표시 인디케이터 관리를 위한 서브시스템
 * 게임 월드 내에 존재하는 모든 UMSDirectionIndicatorComponent를 관리하고,
 * UI 레이어가 매 프레임 월드 액터를 스캔하지 않도록 효율적으로 목록을 제공함
 *
 * 서브시스템에 등록/해제는 UMSDirectionIndicatorComponent에서 자동으로 수행
 */
UCLASS()
class MAGESQUAD_API UMSDirectionIndicatorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 인디케이터 표시 액터 목록에 등록하는 함수
	void RegisterSource(UMSDirectionIndicatorComponent* Source);

	// 인디케이터 표시 액터 목록에 제거하는 함수
	void UnregisterSource(UMSDirectionIndicatorComponent* Source);

	// 현재 서브시스템에 등록된 표시 액터 목록을 반환하는 함수
	void GetIndicatorSources(TArray<UMSDirectionIndicatorComponent*>& OutSources) const;

protected:
	virtual void Deinitialize() override;

private:
	// 현재 서브시스템에 등록된 인디케이터 표시 액터 목록
	// WeakObjectPtr로 관리하여 자동 생명 주기 관리
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<UMSDirectionIndicatorComponent>> RegisteredSources;
};
