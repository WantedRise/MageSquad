// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_LevelUpFloater_Burst.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Widgets/HUD/MSLevelUpFloaterWidget.h"

UMSGC_LevelUpFloater_Burst::UMSGC_LevelUpFloater_Burst()
{
}

bool UMSGC_LevelUpFloater_Burst::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	if (!Target || !LevelUpFloaterWidget) return false;

	UWorld* World = Target->GetWorld();
	if (!World) return false;

	// 표시 위치 설정. 액터 머리 위 느낌을 위해 월드 Z 오프셋 적용
	const FVector WorldLocation = FVector(Parameters.Location) + FVector(0.f, 0.f, WorldZOffset);

	// 각 클라이언트는 자기 로컬 컨트롤러에만 UI를 띄움
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController()) continue;

		// 나이아가라 스폰
		if (LevelUpNiagara)
		{
			AActor* InstigatorActor = Parameters.Instigator.Get();
			if (!InstigatorActor) continue;

			APawn* InstigatorPawn = Cast<APawn>(InstigatorActor);
			if (!InstigatorPawn)
			{
				// Pawn이 아닌 Actor가 들어왔을 가능성까지 방어적으로 처리
				InstigatorPawn = Cast<APawn>(InstigatorActor->GetOwner());
			}
			if (!InstigatorPawn) continue;

			// 나이아가라를 부착시킬 SkeletalMeshComponent 찾기
			USkeletalMeshComponent* SkelMeshComp = nullptr;

			if (ACharacter* Character = Cast<ACharacter>(Parameters.Instigator.Get()))
			{
				SkelMeshComp = Character->GetMesh();
			}
			else
			{
				SkelMeshComp = InstigatorPawn->FindComponentByClass<USkeletalMeshComponent>();
			}

			if (!SkelMeshComp) continue;

			// 레벨업 나이아가라 부착
			UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
				LevelUpNiagara,
				SkelMeshComp,
				NAME_None,
				Parameters.Location + FVector(0.f, 0.f, -100.f),
				FRotator::ZeroRotator,
				EAttachLocation::KeepWorldPosition,
				true
			);

			// 서운드 재생
			if (StartSound)
			{
				UGameplayStatics::SpawnSoundAttached(StartSound, SkelMeshComp);
			}

			// 나이아가라의 회전 절댓값 설정
			if (Niagara)
			{
				Niagara->SetAbsolute(false, true, false);
			}
		}

		// 월드 좌표를 스크린 좌표로 변환
		FVector2D ScreenPos;
		if (!UGameplayStatics::ProjectWorldToScreen(PC, WorldLocation, ScreenPos, true))
		{
			continue;
		}

		// 레벨업 플로터 위젯 풀링 or 생성
		UMSLevelUpFloaterWidget* Widget = UMSLevelUpFloaterWidget::Acquire(PC, LevelUpFloaterWidget);
		if (!Widget) continue;

		// 위젯 표시 시작
		Widget->Start(ScreenPos);
	}

	return true;
}
