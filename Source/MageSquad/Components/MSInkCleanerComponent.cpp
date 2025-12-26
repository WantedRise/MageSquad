#include "MSInkCleanerComponent.h"
#include "Actors/Mission/MSInkAreaActor.h"
#include <Kismet/GameplayStatics.h>


UMSInkCleanerComponent::UMSInkCleanerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMSInkCleanerComponent::BeginPlay()
{
    Super::BeginPlay();
    // 일정 주기로 정화 호출
    GetWorld()->GetTimerManager().SetTimer(
        CleanTimer,
        this,
        &UMSInkCleanerComponent::ApplyClean,
        CleanInterval,
        true
    );
}
/*
void UMSInkCleanerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime,TickType, ThisTickFunction);
    UE_LOG(LogTemp, Log, TEXT("TickComponent"));
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // 플레이어 현재 위치
    const FVector CleanPos = Owner->GetActorLocation();

    // 월드에 존재하는 모든 InkArea 검색 (예제용)
    TArray<AActor*> FoundAreas;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AMSInkAreaActor::StaticClass(),
        FoundAreas
    );

    for (AActor* Actor : FoundAreas)
    {
        if (AMSInkAreaActor* Area = Cast<AMSInkAreaActor>(Actor))
        {
            Area->CleanAtWorldPos(CleanPos, CleanRadiusCm);
        }
    }
}
*/
void UMSInkCleanerComponent::ApplyClean()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // 플레이어 현재 위치
    const FVector CleanPos = Owner->GetActorLocation();

    // 월드에 존재하는 모든 InkArea 검색 (예제용)
    TArray<AActor*> FoundAreas;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AMSInkAreaActor::StaticClass(),
        FoundAreas
    );

    for (AActor* Actor : FoundAreas)
    {
        if (AMSInkAreaActor* Area = Cast<AMSInkAreaActor>(Actor))
        {
            Area->CleanAtWorldPos(CleanPos, CleanRadiusCm);
        }
    }
}