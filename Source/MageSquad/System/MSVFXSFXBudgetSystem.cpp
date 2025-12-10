// Fill out your copyright notice in the Description page of Project Settings.


#include "MSVFXSFXBudgetSystem.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleEmitter.h"

#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

UMSVFXSFXBudgetSystem::UMSVFXSFXBudgetSystem()
{
}

void UMSVFXSFXBudgetSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 풀링 배열 초기화 및 초기 예산 설정
	VFXPool.Reset();
	VFXPool.Reserve(10);

	SFXPool.Reset();
	SFXPool.Reserve(10);
}

UFXSystemComponent* UMSVFXSFXBudgetSystem::SpawnVFX(UFXSystemAsset* System, const FTransform& Transform)
{
	if (!System) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	UFXSystemComponent* Comp = nullptr;

	// VFX 풀링 배열에서 비활성화중인 같은 객체를 찾아서 재사용하기
	for (int32 i = 0; i < VFXPool.Num(); ++i)
	{
		UFXSystemComponent* Pooled = VFXPool[i];
		if (Pooled && !Pooled->IsActive())
		{
			// 스폰할 객체를 재사용할 객체로 초기화
			Comp = Pooled;
			VFXPool.RemoveAt(i);
			break;
		}
	}

	// 재사용할 객체가 없는 경우 새로 만들기
	if (!Comp)
	{
		// 타입에 맞는 새 객체 만들기
		if (UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(System))
		{
			// 새 나이아가라 객체를 만들어 저장
			UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(World);
			NiagaraComp->SetAutoDestroy(false);
			NiagaraComp->RegisterComponent();
			Comp = NiagaraComp;
		}
		else if (UParticleSystem* CascadeSystem = Cast<UParticleSystem>(System))
		{
			// 새 캐스케이드 객체를 만들어 저장
			UParticleSystemComponent* PSC = NewObject<UParticleSystemComponent>(World);
			PSC->bAutoDestroy = false;
			PSC->RegisterComponent();
			Comp = PSC;
		}
		else return nullptr;
	}

	// VFX 스폰(활성화)하기
	if (UNiagaraComponent* NiagaraComp = Cast<UNiagaraComponent>(Comp))
	{
		NiagaraComp->SetAsset(CastChecked<UNiagaraSystem>(System));
		NiagaraComp->SetWorldTransform(Transform);
		NiagaraComp->SetAutoDestroy(false);
		NiagaraComp->SetVisibility(true);
		NiagaraComp->Activate(true);

		// VFX 사용이 끝나면 풀링에 저장되도록 콜백 설정
		NiagaraComp->OnSystemFinished.Clear();
		NiagaraComp->OnSystemFinished.AddDynamic(this, &UMSVFXSFXBudgetSystem::OnNiagaraSystemPooled);
	}
	else if (UParticleSystemComponent* PSC = Cast<UParticleSystemComponent>(Comp))
	{
		PSC->SetTemplate(CastChecked<UParticleSystem>(System));
		PSC->SetWorldTransform(Transform);
		PSC->bAutoDestroy = false;
		PSC->SetVisibility(true);
		PSC->ActivateSystem(true);

		// VFX 사용이 끝나면 풀링에 저장되도록 콜백 설정
		PSC->OnSystemFinished.Clear();
		PSC->OnSystemFinished.AddDynamic(this, &UMSVFXSFXBudgetSystem::OnParticleSystemPooled);
	}

	return Comp;
}

UAudioComponent* UMSVFXSFXBudgetSystem::PlaySFX(USoundBase* Sound, const FVector& Location)
{
	if (!Sound) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	UAudioComponent* Comp = nullptr;

	// SFX 풀링 배열에서 비재생중인 같은 객체를 찾아서 재사용하기
	for (int32 i = 0; i < SFXPool.Num(); ++i)
	{
		UAudioComponent* Pooled = SFXPool[i];
		if (Pooled && !Pooled->IsPlaying())
		{
			// 스폰할 객체를 재사용할 객체로 초기화
			Comp = Pooled;
			SFXPool.RemoveAt(i);
			break;
		}
	}

	// 재사용할 객체가 없는 경우 새로 만들기
	if (!Comp)
	{
		Comp = NewObject<UAudioComponent>(World);
		Comp->bAutoDestroy = false;
		Comp->RegisterComponent();
	}

	// SFX 재생하기
	Comp->SetSound(Sound);
	Comp->SetWorldLocation(Location);
	Comp->bAutoDestroy = false;
	Comp->Play();

	// SFX 재생이 끝나면 풀링에 저장
	Comp->OnAudioFinished.Clear();
	Comp->OnAudioFinishedNative.Clear();
	Comp->OnAudioFinishedNative.AddLambda(
		[this](UAudioComponent* FinishedComponent) 
		{
			FinishedComponent->Stop();
			SFXPool.Add(FinishedComponent);
		});

	return Comp;
}

void UMSVFXSFXBudgetSystem::OnNiagaraSystemPooled(UNiagaraComponent* FinishedComponent)
{
	FinishedComponent->Deactivate();
	FinishedComponent->SetVisibility(false);
	VFXPool.Add(FinishedComponent);
}

void UMSVFXSFXBudgetSystem::OnParticleSystemPooled(UParticleSystemComponent* PSC)
{
	VFXPool.Add(PSC);
}
