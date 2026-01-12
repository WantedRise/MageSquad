//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "System/MSVFXSFXBudgetSystem.h"
//
//#include "NiagaraComponent.h"
//#include "NiagaraSystem.h"
//
//#include "Sound/SoundBase.h"
//#include "Components/AudioComponent.h"
//
//void UMSVFXSFXBudgetSystem::Initialize(FSubsystemCollectionBase& Collection)
//{
//	Super::Initialize(Collection);
//
//	// 맵 초기화
//	VFXPools.Empty();
//	SFXPools.Empty();
//	VFXComponentToAsset.Empty();
//	SFXComponentToAsset.Empty();
//}
//
//void UMSVFXSFXBudgetSystem::Deinitialize()
//{
//	// 풀링된 인스턴스들을 모두 Destroy
//	for (auto& Pair : VFXPools)
//	{
//		for (UNiagaraComponent* Comp : Pair.Value.Pool)
//		{
//			if (Comp)
//			{
//				Comp->DestroyComponent();
//			}
//		}
//	}
//	for (auto& Pair : SFXPools)
//	{
//		for (UAudioComponent* Comp : Pair.Value.Pool)
//		{
//			if (Comp)
//			{
//				Comp->DestroyComponent();
//			}
//		}
//	}
//
//	// 맵 초기화
//	VFXPools.Empty();
//	SFXPools.Empty();
//	VFXComponentToAsset.Empty();
//	SFXComponentToAsset.Empty();
//
//	Super::Deinitialize();
//}
//
//UMSVFXSFXBudgetSystem* UMSVFXSFXBudgetSystem::Get(UObject* WorldContextObject)
//{
//	if (!WorldContextObject) return nullptr;
//
//	if (UWorld* World = WorldContextObject->GetWorld())
//	{
//		if (UGameInstance* GI = World->GetGameInstance())
//		{
//			return GI->GetSubsystem<UMSVFXSFXBudgetSystem>();
//		}
//	}
//	return nullptr;
//}
//
//UNiagaraComponent* UMSVFXSFXBudgetSystem::SpawnVFX(UNiagaraSystem* System, const FTransform& Transform)
//{
//	if (!System) return nullptr;
//
//	FVFXPoolData& Data = VFXPools.FindOrAdd(System);
//
//	// 처음 생성 시, 초기 풀 세팅
//	PrewarmVFXPool(System);
//
//	// 최대 개수가 넘어가는 경우 종료
//	if (Data.ActiveCount >= Data.Budget)
//	{
//		return nullptr;
//	}
//
//	// 생성된 VFX
//	UNiagaraComponent* Comp = nullptr;
//
//	// 풀에서 사용 가능한 VFX 찾기
//	for (int32 i = 0; i < Data.Pool.Num(); ++i)
//	{
//		UNiagaraComponent* Candidate = Data.Pool[i];
//		if (Candidate && !Candidate->IsActive())
//		{
//			Comp = Candidate;
//			Data.Pool.RemoveAtSwap(i);
//			break;
//		}
//	}
//
//	// 사용 가능한 인스턴스가 없으면 새로 생성
//	if (!Comp)
//	{
//		Comp = CreateNewVFX(System);
//	}
//	if (!Comp) return nullptr;
//
//	// VFX 활성화 설정
//	Comp->SetAsset(System);
//	Comp->SetWorldTransform(Transform);
//	Comp->SetAutoDestroy(false);
//	Comp->SetVisibility(true);
//	Comp->Activate(true);
//
//	// VFX 생명주기 종료 콜백 함수 바인딩
//	Comp->OnSystemFinished.Clear();
//	Comp->OnSystemFinished.AddDynamic(this, &UMSVFXSFXBudgetSystem::OnVFXFinished);
//
//	// 활성화 카운트 증가 및 맵에 저장
//	Data.ActiveCount++;
//	VFXComponentToAsset.Add(Comp, System);
//	return Comp;
//}
//
//UAudioComponent* UMSVFXSFXBudgetSystem::PlaySFX(USoundBase* Sound, const FVector& Location)
//{
//	if (!Sound) return nullptr;
//
//	FSFXPoolData& Data = SFXPools.FindOrAdd(Sound);
//
//	// 처음 생성 시, 초기 풀 세팅
//	PrewarmSFXPool(Sound);
//
//	// 최대 개수가 넘어가는 경우 종료
//	if (Data.ActiveCount >= Data.Budget)
//	{
//		return nullptr;
//	}
//
//	// 생성된 SFX
//	UAudioComponent* Comp = nullptr;
//
//	// 풀에서 사용 가능한 SFX 찾기
//	for (int32 i = 0; i < Data.Pool.Num(); ++i)
//	{
//		UAudioComponent* Candidate = Data.Pool[i];
//		if (Candidate && !Candidate->IsPlaying())
//		{
//			Comp = Candidate;
//			Data.Pool.RemoveAtSwap(i);
//			break;
//		}
//	}
//
//	// 사용 가능한 인스턴스가 없으면 새로 생성
//	if (!Comp)
//	{
//		Comp = CreateNewSFX(Sound);
//	}
//	if (!Comp) return nullptr;
//
//	// SFX 활성화 설정
//	Comp->SetSound(Sound);
//	Comp->SetWorldLocation(Location);
//	Comp->bAutoDestroy = false;
//	Comp->Play();
//
//	// SFX 생명주기 종료 콜백 함수 바인딩
//	Comp->OnAudioFinished.Clear();
//	Comp->OnAudioFinishedNative.Clear();
//	Comp->OnAudioFinishedNative.AddLambda([this](UAudioComponent* Finished)
//		{
//			OnSFXFinished(Finished);
//		});
//
//	// 활성화 카운트 증가 및 맵에 저장
//	Data.ActiveCount++;
//	SFXComponentToAsset.Add(Comp, Sound);
//	return Comp;
//}
//
//void UMSVFXSFXBudgetSystem::SetVFXBudget(UNiagaraSystem* System, int32 NewBudget)
//{
//	if (!System || NewBudget < 0) return;
//
//	// 예산 초기화
//	FVFXPoolData& Data = VFXPools.FindOrAdd(System);
//	Data.Budget = NewBudget;
//}
//
//void UMSVFXSFXBudgetSystem::SetSFXBudget(USoundBase* Sound, int32 NewBudget)
//{
//	if (!Sound || NewBudget < 0) return;
//
//	// 예산 초기화
//	FSFXPoolData& Data = SFXPools.FindOrAdd(Sound);
//	Data.Budget = NewBudget;
//}
//
//UNiagaraComponent* UMSVFXSFXBudgetSystem::CreateNewVFX(UNiagaraSystem* System)
//{
//	UWorld* World = GetWorld();
//	if (!World || !System) return nullptr;
//
//	// 새 VFX 생성 및 설정
//	UNiagaraComponent* Comp = NewObject<UNiagaraComponent>(World);
//	Comp->SetAutoDestroy(false);
//	Comp->RegisterComponent();
//	return Comp;
//}
//
//UAudioComponent* UMSVFXSFXBudgetSystem::CreateNewSFX(USoundBase* Sound)
//{
//	UWorld* World = GetWorld();
//	if (!World) return nullptr;
//
//	// 새 SFX 생성 및 설정
//	UAudioComponent* Comp = NewObject<UAudioComponent>(World);
//	Comp->bAutoDestroy = false;
//	Comp->RegisterComponent();
//	return Comp;
//}
//
//void UMSVFXSFXBudgetSystem::PrewarmVFXPool(UNiagaraSystem* System)
//{
//	if (!System) return;
//	FVFXPoolData& Data = VFXPools.FindOrAdd(System);
//
//	UWorld* World = GetWorld();
//	if (!World) return;
//
//	// 초기 인스턴스를 생성하여 풀 초기화
//	int32 CurrentCount = Data.Pool.Num() + Data.ActiveCount;
//	for (int32 i = CurrentCount; i < Data.InitialSize; ++i)
//	{
//		UNiagaraComponent* Comp = CreateNewVFX(System);
//		if (Comp)
//		{
//			Comp->SetAsset(System);
//			Comp->SetVisibility(false);
//			Data.Pool.Add(Comp);
//		}
//	}
//}
//
//void UMSVFXSFXBudgetSystem::PrewarmSFXPool(USoundBase* Sound)
//{
//	if (!Sound) return;
//	FSFXPoolData& Data = SFXPools.FindOrAdd(Sound);
//
//	UWorld* World = GetWorld();
//	if (!World) return;
//
//	// 초기 인스턴스를 생성하여 풀 초기화
//	int32 CurrentCount = Data.Pool.Num() + Data.ActiveCount;
//	for (int32 i = CurrentCount; i < Data.InitialSize; ++i)
//	{
//		UAudioComponent* Comp = CreateNewSFX(Sound);
//		if (Comp)
//		{
//			Data.Pool.Add(Comp);
//		}
//	}
//}
//
//void UMSVFXSFXBudgetSystem::OnVFXFinished(UNiagaraComponent* FinishedComponent)
//{
//	if (!FinishedComponent) return;
//
//	// VFX 비활성화
//	FinishedComponent->Deactivate();
//	FinishedComponent->SetVisibility(false);
//
//	// 풀 클래스 찾기
//	if (auto* AssetPtr = VFXComponentToAsset.Find(FinishedComponent))
//	{
//		UNiagaraSystem* Asset = *AssetPtr;
//		if (FVFXPoolData* Data = VFXPools.Find(Asset))
//		{
//			Data->ActiveCount = FMath::Max(0, Data->ActiveCount - 1);
//			Data->Pool.Add(FinishedComponent);
//		}
//		VFXComponentToAsset.Remove(FinishedComponent);
//	}
//	else
//	{
//		// 예외 VFX의 경우 제거
//		FinishedComponent->DestroyComponent();
//	}
//}
//
//void UMSVFXSFXBudgetSystem::OnSFXFinished(UAudioComponent* FinishedComponent)
//{
//	if (!FinishedComponent) return;
//
//	// SFX 비활성화
//	FinishedComponent->Stop();
//
//	// 풀 클래스 찾기
//	if (auto* AssetPtr = SFXComponentToAsset.Find(FinishedComponent))
//	{
//		USoundBase* Asset = *AssetPtr;
//		if (FSFXPoolData* Data = SFXPools.Find(Asset))
//		{
//			Data->ActiveCount = FMath::Max(0, Data->ActiveCount - 1);
//			Data->Pool.Add(FinishedComponent);
//		}
//		SFXComponentToAsset.Remove(FinishedComponent);
//	}
//	else
//	{
//		// 예외 SFX의 경우 제거
//		FinishedComponent->DestroyComponent();
//	}
//}
