//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "Subsystems/GameInstanceSubsystem.h"
//#include "MSVFXSFXBudgetSystem.generated.h"
//
//// VFX 풀 데이터 구조체
//USTRUCT()
//struct FVFXPoolData
//{
//	GENERATED_BODY()
//
//	// 재사용 가능한 비활성화된 VFX
//	UPROPERTY(Transient)
//	TArray<UNiagaraComponent*> Pool;
//
//	// 현재 활성화된 VFX 수
//	UPROPERTY(Transient)
//	int32 ActiveCount = 0;
//
//	// 미리 생성할 인스턴스 수
//	UPROPERTY(EditAnywhere, Category = "Budget")
//	int32 InitialSize = 16;
//
//	// 동시에 활성화되어있을 수 있는 최대 VFX 수
//	UPROPERTY(EditAnywhere, Category = "Budget")
//	int32 Budget = 100;
//};
//
//// SFX 풀 데이터 구조체
//USTRUCT()
//struct FSFXPoolData
//{
//	GENERATED_BODY()
//
//	// 재사용할 수 있는 풀
//	UPROPERTY(Transient)
//	TArray<UAudioComponent*> Pool;
//
//	// 현재 활성화된 SFX 수
//	UPROPERTY(Transient)
//	int32 ActiveCount = 0;
//
//	// 미리 생성할 인스턴스 수
//	UPROPERTY(EditAnywhere, Category = "Budget")
//	int32 InitialSize = 16;
//
//	// 동시에 활성화되어있을 수 있는 최대 SFX 수
//	UPROPERTY(EditAnywhere, Category = "Budget")
//	int32 Budget = 50;
//};
//
///**
// * 작성자: 김준형
// * 작성일: 25/12/09
// *
// * 시각 효과(VFX) 및 음향 효과(SFX)에 대한 예산 및 풀링을 관리하는 서브시스템
// * 재생이 끝나면 풀로 반환하여 재사용
// */
//UCLASS()
//class MAGESQUAD_API UMSVFXSFXBudgetSystem : public UGameInstanceSubsystem
//{
//	GENERATED_BODY()
//
//public:
//	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
//	virtual void Deinitialize() override;
//
//	// 서브시스템 Getter
//	UFUNCTION(BlueprintCallable, Category = "Custom | System", meta = (WorldContext = "WorldContextObject"))
//	static UMSVFXSFXBudgetSystem* Get(UObject* WorldContextObject);
//
//	/*
//	* 지정된 트랜스폼에 VFX 컴포넌트를 생성하는 함수
//	* System	: 재생할 나이아가라
//	* Transform	: 재생할 월드 트랜스폼 값
//	* @return	: 생성된 나이아가라를 반환
//	*/
//	UFUNCTION(BlueprintCallable, Category = "Custom | System")
//	class UNiagaraComponent* SpawnVFX(class UNiagaraSystem* System, const FTransform& Transform);
//
//	/*
//	* 지정된 위치에서 SFX 컴포넌트를 재생하는 함수
//	* Sound		: 재생할 사운드
//	* Location	: 사운드의 월드 위치
//	* @return	: 공통 SFX 기본 컴포넌트 클래스를 반환. 용도에 따라 캐스팅해서 사용
//	*/
//	UFUNCTION(BlueprintCallable, Category = "Custom | System")
//	class UAudioComponent* PlaySFX(USoundBase* Sound, const FVector& Location);
//
//	/*
//	* 지정된 VFX 에셋에 대한 초기 풀 사이즈 설정 함수
//	* System	: 재생할 나이아가라
//	* NewBudget	: 새 풀 사이즈
//	*/
//	UFUNCTION(BlueprintCallable, Category = "Custom | System")
//	void SetVFXBudget(UNiagaraSystem* System, int32 NewBudget);
//
//	/*
//	* 지정된 SFX 에셋에 대한 초기 풀 사이즈 설정 함수
//	* Sound		: 재생할 사운드
//	* NewBudget	: 새 풀 사이즈
//	*/
//	UFUNCTION(BlueprintCallable, Category = "Custom | System")
//	void SetSFXBudget(USoundBase* Sound, int32 NewBudget);
//
//protected:
//	/*
//	* 새 VFX 인스턴스를 생성하고 구성하는 함수
//	* System	: 나이아가라 클래스
//	* @return	: 생성된 나이아가라를 반환
//	*/
//	UNiagaraComponent* CreateNewVFX(UNiagaraSystem* System);
//
//	/*
//	* 새 SFX 인스턴스를 생성하고 구성하는 함수
//	* System	: 사운드 클래스
//	* @return	: 생성된 사운드를 반환
//	*/
//	UAudioComponent* CreateNewSFX(USoundBase* Sound);
//
//	// 초기 인스턴스를 생성하여 풀 초기화하는 함수
//	void PrewarmVFXPool(UNiagaraSystem* System);
//	void PrewarmSFXPool(USoundBase* Sound);
//
//	// VFX 재생이 끝나서 풀로 반환될 때, 호출되는 콜백 함수
//	UFUNCTION()
//	void OnVFXFinished(UNiagaraComponent* FinishedComponent);
//
//	// SFX 재생이 끝나서 풀로 반환될 때, 호출되는 콜백 함수
//	void OnSFXFinished(UAudioComponent* FinishedComponent);
//
//protected:
//	// VFX와 해당 풀 데이터 간의 맵
//	UPROPERTY(Transient)
//	TMap<TObjectPtr<UNiagaraSystem>, FVFXPoolData> VFXPools;
//
//	// SFX와 해당 풀 데이터 간의 맵
//	UPROPERTY(Transient)
//	TMap<TObjectPtr<USoundBase>, FSFXPoolData> SFXPools;
//
//	// VFX 인스턴스에서 해당 클래스 키로의 역방향 조회 맵
//	UPROPERTY(Transient)
//	TMap<TObjectPtr<UNiagaraComponent>, TObjectPtr<UNiagaraSystem>> VFXComponentToAsset;
//
//	// SFX 인스턴스에서 해당 클래스 키로의 역방향 조회 맵
//	UPROPERTY(Transient)
//	TMap<TObjectPtr<UAudioComponent>, TObjectPtr<USoundBase>> SFXComponentToAsset;
//};
