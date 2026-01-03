// Fill out your copyright notice in the Description page of Project Settings.

#include "System/MSDirectionIndicatorSubsystem.h"

#include "Components/MSDirectionIndicatorComponent.h"

void UMSDirectionIndicatorSubsystem::RegisterSource(UMSDirectionIndicatorComponent* Source)
{
	if (!Source) return;

	// 인디케이터 표시 액터 목록에 등록
	RegisteredSources.Add(Source);
}

void UMSDirectionIndicatorSubsystem::UnregisterSource(UMSDirectionIndicatorComponent* Source)
{
	if (!Source) return;

	// 인디케이터 표시 액터 목록에 제거
	RegisteredSources.Remove(Source);
}

void UMSDirectionIndicatorSubsystem::GetIndicatorSources(TArray<UMSDirectionIndicatorComponent*>& OutSources) const
{
	OutSources.Reset();

	// 현재 서브시스템에 등록된 인디케이터 표시 액터 목록을 순회하며 추가
	for (const TWeakObjectPtr<UMSDirectionIndicatorComponent>& WeakPtr : RegisteredSources)
	{
		if (WeakPtr.IsValid())
		{
			OutSources.Add(WeakPtr.Get());
		}
	}
}

void UMSDirectionIndicatorSubsystem::Deinitialize()
{
	// 월드 종료 시 모든 레퍼런스 정리
	RegisteredSources.Empty();

	Super::Deinitialize();
}