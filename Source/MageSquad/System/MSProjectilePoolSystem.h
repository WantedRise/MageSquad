//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "Subsystems/GameInstanceSubsystem.h"
//#include "MSProjectilePoolSystem.generated.h"
//
///*
//* 발사체 풀 데이터 구조체
//*/
//USTRUCT()
//struct FProjectilePoolData
//{
//	GENERATED_BODY()
//
//	// 재사용 가능한 비활성화된 발사체
//	UPROPERTY(Transient)
//	TArray<AMSBaseProjectile*> Pool;
//
//	// 현재 활성화된 발사체 수
//	UPROPERTY(Transient)
//	int32 ActiveCount = 0;
//
//	// 미리 생성할 인스턴스 수
//	UPROPERTY(EditAnywhere, Category = "Budget")
//	int32 InitialSize = 32;
//
//	// 동시에 활성화되어있을 수 있는 최대 발사체 수
//	UPROPERTY(EditAnywhere, Category = "Budget")
//	int32 Budget = 100;
//};
//
///**
// * 작성자: 김준형
// * 작성일: 25/12/11
// *
// * 발사체 클래스를 키로 사용하는 발사체 액터 풀을 관리하는 서브시스템
// * 발사체 수명이 끝나면 풀로 반환하여 재사용
// */
//UCLASS()
//class MAGESQUAD_API UMSProjectilePoolSystem : public UGameInstanceSubsystem
//{
//	GENERATED_BODY()
//
//public:
//	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
//	virtual void Deinitialize() override;
//
//	// 서브시스템 Getter
//	UFUNCTION(BlueprintCallable, Category = "Custom | System", meta = (WorldContext = "WorldContextObject"))
//	static UMSProjectilePoolSystem* GetProjectilePoolSystem(UObject* WorldContextObject);
//
//	// 풀링에 사용할 발사체 클래스와 초기 풀 사이즈 설정 함수
//	UFUNCTION(BlueprintCallable, Category = "Custom | System")
//	void Configure(TSubclassOf<AMSBaseProjectile> InProjectileClass, int32 InInitialSize = 32, int32 InBudget = 200);
//
//	/*
//	* 지정된 트랜스폼에 발사체를 생성하는 함수
//	* InProjectileClass	: 발사체 클래스
//	* SpawnTransform	: 재생할 월드 FTransform 값
//	* Instigator		: 발사체를 생성하는 주체
//	* @return			: 기본 발사체 클래스를 반환
//	*/
//	UFUNCTION(BlueprintCallable, Category = "Custom | System")
//	class AMSBaseProjectile* SpawnProjectile(TSubclassOf<AMSBaseProjectile> InProjectileClass, const FTransform& Transform, AActor* Instigator);
//
//	// 발사체를 해당 발사체 풀로 반환하는 함수
//	UFUNCTION(BlueprintCallable, Category = "Custom | System")
//	void ReturnProjectile(AMSBaseProjectile* InProjectile);
//
//	/*
//	* 새 발사체 클래스에 대한 예산 초기화 함수
//	* InProjectileClass	: 발사체 클래스
//	* NewBudget			: 새 풀 예산
//	*/
//	UFUNCTION(BlueprintCallable, Category = "Custom | System")
//	void SetProjectileBudget(TSubclassOf<AMSBaseProjectile> InProjectileClass, int32 NewBudget);
//
//protected:
//	/*
//	* 새 발사체 인스턴스를 생성하고 구성하는 함수
//	* InProjectileClass	: 발사체 클래스
//	* Instigator		: 발사체를 생성하는 주체
//	* @return			: 기본 발사체 클래스를 반환
//	*/
//	AMSBaseProjectile* CreateNewProjectile(TSubclassOf<AMSBaseProjectile> InProjectileClass, AActor* Instigator);
//
//	// 초기 인스턴스를 생성하여 풀 초기화하는 함수
//	void PrewarmPool(TSubclassOf<AMSBaseProjectile> InProjectileClass);
//
//	// 발사체가 풀로 반환될 때, 호출되는 콜백 함수
//	void HandleProjectileFinished(AMSBaseProjectile* InProjectile);
//
//protected:
//	// 발사체 클래스와 해당 풀 데이터 간의 맵
//	UPROPERTY(Transient)
//	TMap<TSubclassOf<AMSBaseProjectile>, FProjectilePoolData> ProjectilePools;
//
//	// 발사체 인스턴스에서 해당 클래스 키로의 역방향 조회 맵
//	UPROPERTY(Transient)
//	TMap<AMSBaseProjectile*, TSubclassOf<AMSBaseProjectile>> ProjectileToClassMap;
//};
