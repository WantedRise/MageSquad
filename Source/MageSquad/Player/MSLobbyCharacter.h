// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CharacterAppearanceInterface.h"
#include "MSLobbyCharacter.generated.h"

/*
* 작성자: 이상준
* 작성일: 2025-12-16
* [로비 플레이어 캐릭터 UI 동기화 처리]
* 로비 단계에서 각 플레이어의
* - 준비 상태 (Ready / Not Ready)
* - 호스트 여부
* - 닉네임
* 을 모든 클라이언트에게 동일하게 표시하기 위한 동기화 로직을 구현.
*
* PlayerState에 동기화 변수를 저장,
* 로비 캐릭터는 해당 동기화 변수를 기반으로 UI를 갱신하는 역할을 담당한다.
*
* 향후 캐릭터 선택 기능 추가를 고려하여,
* 캐릭터 메쉬 교체 또한 PlayerState 변경을 통해 적용될 예정.
*/
UCLASS()
class MAGESQUAD_API AMSLobbyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMSLobbyCharacter();
	virtual void PossessedBy(AController* NewController) override;
	//UI를 갱신 및 델리게이드 바인딩
	virtual void OnRep_PlayerState() override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	//닉네임 UI 업데이트
	void UpdateUserNickNameUI(const FString& InUserNickName);
	//호스트 표시 UI 업데이트
	void UpdateHostUI(bool bReady);
	//준비상태 UI 업데이트 
	UFUNCTION()
	void UpdateReadyStatusUI(bool bReady);

protected:
	void InitializeLobbyCharacterFromPlayerState();
    UPROPERTY()
    TObjectPtr<UMaterial> Widget3DPassThroughMaterial;

protected:
	// 로비 캐릭터 머리 위에 표시되는 플레이어 정보 UI 컴포넌트
	// (닉네임, 준비 상태, 호스트 여부 등 표시)
	UPROPERTY(EditDefaultsOnly)
	class UWidgetComponent* LobbyPlayerEntryWidgetComponent;
	// WidgetComponent에서 생성된 실제 UI 위젯 인스턴스
	// PlayerState 변경 시 UI 갱신에 사용
	UPROPERTY()
	class UMSLobbyPlayerEntryWidget* LobbyPlayerEntryWidget;
	// 지팡이 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Weapon")
	TObjectPtr<class UStaticMeshComponent> StaffMesh;
	// 지팡이 메시를 부착할 소켓 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Weapon")
	FName StaffAttachSocketName = TEXT("s_Staff");
};
