// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GA/Player/MSGA_PlayerBlink.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Player/MSPlayerController.h"
#include "GameplayTagsManager.h"

#include "MSGameplayTags.h"

UMSGA_PlayerBlink::UMSGA_PlayerBlink()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 어빌리티 태그 설정
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(MSGameplayTags::Player_Ability_Blink);
	SetAssetTags(TagContainer);

	// 트리거 이벤트 태그 설정 (Gameplay Event로 활성화)
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag(MSGameplayTags::Player_Event_Blink);
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);

	// GameplayCue 태그 설정
	const UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	Cue_BlinkStart = TagsManager.RequestGameplayTag(FName("GameplayCue.Player.Blink.Start"), false);
	Cue_BlinkEnd = TagsManager.RequestGameplayTag(FName("GameplayCue.Player.Blink.End"), false);
}

void UMSGA_PlayerBlink::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 어빌리티를 활성화해도 되는지 검사
	if (!CheckAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 필요한 객체 가져오기
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());

	if (!ASC || !Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 점멸 수행
	const bool bSuccess = PerformBlink(Character, ASC);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, !bSuccess);
}

bool UMSGA_PlayerBlink::CheckAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 액터 유효성 검사
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return false;

	// 서버에서만 로직 수행
	if (!ActorInfo->IsNetAuthority()) return false;

	// 코스트 및 쿨타임 검사
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) return false;

	return true;
}

bool UMSGA_PlayerBlink::PerformBlink(ACharacter* Character, UAbilitySystemComponent* ASC)
{
	check(Character);

	// 점멸 시작 위치 / 점멸 도착 위치
	const FVector StartLocation = Character->GetActorLocation();
	const FVector DesiredLocation = ComputeDesiredLocation(Character);
	FVector FinalLocation = DesiredLocation;

	// 점멸 가능한 위치 찾기
	if (!ResolveFinalLocation(Character, StartLocation, DesiredLocation, FinalLocation))
	{
		// 이동 불가 시 false 반환
		return false;
	}

	// 시작 VFX 재생 (GameplayCue)
	// - BlinkStart/BlinkEnd 2점을 EffectContext에 기록해서 Beam 등에 사용
	ExecuteCue(ASC, Cue_BlinkStart, StartLocation, StartLocation, FinalLocation);

	// 이동 방향 회전값
	const FRotator FacingRot = DesiredLocation.ToOrientationRotator();

	// TeleportTo를 통해 이동
	// TeleportTo는 서버에서 실행하면 캐릭터 이동이 네트워크로 복제됨
	const bool bTeleported = Character->TeleportTo(FinalLocation, FacingRot, false, false);
	if (!bTeleported) return false;

	// 종료 VFX 재생 (GameplayCue)
	ExecuteCue(ASC, Cue_BlinkEnd, FinalLocation, StartLocation, FinalLocation);

	// 위치 업데이트 즉시 반영 요청
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		Avatar->ForceNetUpdate();
	}

	return true;
}

FVector UMSGA_PlayerBlink::ComputeDesiredLocation(const ACharacter* Character) const
{
	check(Character);

	FVector CursorLocation;
	FVector CursorDirection;

	// 커서 방향 및 위치 구하기
	if (GetCurrentActorInfo()->IsNetAuthority())
	{
		if (AMSPlayerController* PC = Cast<AMSPlayerController>(GetCurrentActorInfo()->PlayerController.Get()))
		{
			CursorLocation = PC->GetServerCursor();
			CursorDirection = PC->GetServerCursorDir(Character->GetActorForwardVector());
		}
	}

	// 점멸 최대 거리로 이동
	float A = FVector::Dist(Character->GetActorLocation(), Character->GetActorLocation() + BlinkDistance);

	// 커서 위치로 이동
	float B = FVector::Dist(Character->GetActorLocation(), CursorLocation);

	// 점멸 최대 거리와 커서 위치 중에서 더 가까운 위치로 이동
	return Character->GetActorLocation() + CursorDirection * FMath::Min(A, B);;
}

bool UMSGA_PlayerBlink::ResolveFinalLocation(ACharacter* Character, const FVector& StartLocation, const FVector& DesiredLocation, FVector& OutFinalLocation) const
{
	if (!Character) return false;

	// 이동 방향 회전값
	const FRotator FacingRot = DesiredLocation.ToOrientationRotator();

	// 목표 지점으로 이동 시도
	if (CanTeleportTo(Character, DesiredLocation, FacingRot))
	{
		OutFinalLocation = DesiredLocation;
		return true;
	}

	// 이동 불가면 근처로 이동 가능한 지점 탐색
	return FindNearbyValidLocation(Character, StartLocation, DesiredLocation, OutFinalLocation);
}

bool UMSGA_PlayerBlink::FindNearbyValidLocation(ACharacter* Character, const FVector& StartLocation, const FVector& DesiredLocation, FVector& OutLocation) const
{
	if (!Character || !Character->GetWorld()) return false;

	UWorld* World = Character->GetWorld();
	const FRotator FacingRot = DesiredLocation.ToOrientationRotator();

	// #1: 엔진 내장 기능을 통해 근처 텔레포트 위치 찾기
	{
		FVector Candidate = DesiredLocation;
		if (World->FindTeleportSpot(Character, Candidate, FacingRot))
		{
			OutLocation = Candidate;
			return true;
		}
	}

	// #2: 원형 탐색: DesiredLocation 주변을 반경 증가시키며 샘플링
	{
		const int32 Steps = FMath::Max(4, FallbackAngleSteps);
		const float MaxR = FMath::Max(0.f, FallbackMaxRadius);
		const float StepR = FMath::Max(1.f, FallbackRingStep);

		for (float R = StepR; R <= MaxR; R += StepR)
		{
			for (int32 i = 0; i < Steps; ++i)
			{
				const float Angle = (2.f * PI) * (static_cast<float>(i) / static_cast<float>(Steps));
				const FVector Offset(FMath::Cos(Angle) * R, FMath::Sin(Angle) * R, 0.f);
				const FVector Candidate = DesiredLocation + Offset;

				if (CanTeleportTo(Character, Candidate, FacingRot))
				{
					OutLocation = Candidate;
					return true;
				}
			}
		}
	}

	// #3: 이진 탐색: 시작 위치에서 목표 위치까지의 선분에서 가장 멀리 가능한 위치 찾기
	// - 목표 지점 자체는 불가이므로, 가능한 최대 t를 찾는 형태
	{
		float Low = 0.f;
		float High = 1.f;
		FVector Best = StartLocation;

		// 10회면 충분히 근사
		for (int32 Iter = 0; Iter < 10; ++Iter)
		{
			const float Mid = (Low + High) * 0.5f;
			const FVector Candidate = FMath::Lerp(StartLocation, DesiredLocation, Mid);

			if (CanTeleportTo(Character, Candidate, FacingRot))
			{
				Best = Candidate;
				Low = Mid;    // 더 멀리 시도
			}
			else
			{
				High = Mid;   // 덜 멀리
			}
		}

		// Best가 Start와 거의 같으면 실패로 처리
		if (!Best.Equals(StartLocation, 1.0f))
		{
			OutLocation = Best;
			return true;
		}
	}

	return false;
}

bool UMSGA_PlayerBlink::CanTeleportTo(ACharacter* Character, const FVector& Location, const FRotator& Rot) const
{
	if (!Character) return false;

	// bIsATest 옵션을 true로 하고 텔레포트 수행 (실제 이동 없이 체크한 수행)
	return Character->TeleportTo(Location, Rot, true, false);
}

void UMSGA_PlayerBlink::ExecuteCue(UAbilitySystemComponent* ASC, const FGameplayTag& CueTag, const FVector& CueLocation, const FVector& BlinkStart, const FVector& BlinkEnd) const
{
	if (!ASC || !CueTag.IsValid()) return;

	// 서버에서만 Cue 실행
	AActor* OwnerActor = ASC->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	/*
	* 파라미터를 안전하게 복제하기 위해 EffectContext에 기록
	* 프로젝트에서 AbilitySystemGlobals를 커스텀 컨텍스트로 세팅하지 않았다면,
	* ASC->MakeEffectContext()는 기본(FGameplayEffectContext) 타입을 반환할 수 있음
	* 이 경우 무조건 static_cast 하면 메모리 오염/크래시로 이어질 수 있음
	*/

	// #1: 타입 체크 후 캐스팅
	FGameplayEffectContextHandle CtxHandle = ASC->MakeEffectContext();
	FMSGameplayEffectContext* MSCtx = nullptr;

	if (FGameplayEffectContext* BaseCtx = CtxHandle.Get())
	{
		if (BaseCtx->GetScriptStruct() == FMSGameplayEffectContext::StaticStruct())
		{
			MSCtx = static_cast<FMSGameplayEffectContext*>(BaseCtx);
		}
	}

	// #2: 커스텀 컨텍스트가 아니라면, 직접 파생 컨텍스트를 생성해서 교체
	if (!MSCtx)
	{
		FMSGameplayEffectContext* NewCtx = new FMSGameplayEffectContext();

		if (FGameplayEffectContext* BaseCtx = CtxHandle.Get())
		{
			// Base 부분 복사 (Instigator/SourceObject/Ability/HitResult 등)
			*static_cast<FGameplayEffectContext*>(NewCtx) = *BaseCtx;

			// HitResult는 내부 포인터를 안전 복사
			if (BaseCtx->GetHitResult())
			{
				NewCtx->AddHitResult(*BaseCtx->GetHitResult(), true);
			}
		}

		CtxHandle = FGameplayEffectContextHandle(NewCtx);
		MSCtx = NewCtx;
	}

	// 커스텀 데이터 기록
	MSCtx->CueColor = BlinkColor;
	MSCtx->SetBlinkSegment(BlinkStart, BlinkEnd);

	// 큐 파라미터 설정
	FGameplayCueParameters Params;
	Params.Location = CueLocation;
	Params.EffectContext = CtxHandle;

	// Cue 실행
	ASC->ExecuteGameplayCue(CueTag, Params);
}
