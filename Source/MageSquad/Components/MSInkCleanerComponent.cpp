#include "MSInkCleanerComponent.h"
#include "Actors/Mission/MSInkAreaActor.h"
#include <Kismet/GameplayStatics.h>
#include "Components/PrimitiveComponent.h"

UMSInkCleanerComponent::UMSInkCleanerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMSInkCleanerComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    OwnerCollision = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (OwnerCollision)
    {
        OwnerCollision->OnComponentBeginOverlap.AddDynamic(this, &UMSInkCleanerComponent::OnBeginOverlap);
        OwnerCollision->OnComponentEndOverlap.AddDynamic(this, &UMSInkCleanerComponent::OnEndOverlap);
    }

    
    UE_LOG(LogTemp, Verbose,
        TEXT("UMSInkCleanerComponent::BeginPlay")
    );
}

void UMSInkCleanerComponent::OnEndOverlap(
    UPrimitiveComponent*,
    AActor* OtherActor,
    UPrimitiveComponent*,
    int32
)
{
    if (AMSInkAreaActor* Area = Cast<AMSInkAreaActor>(OtherActor))
    {
        OverlappingAreas.Remove(Area);
    }
}

void UMSInkCleanerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(CleanTimer);
    }
    OverlappingAreas.Empty();
    Super::EndPlay(EndPlayReason);
}


void UMSInkCleanerComponent::OnBeginOverlap(
    UPrimitiveComponent*,
    AActor* OtherActor,
    UPrimitiveComponent*,
    int32,
    bool,
    const FHitResult&
)
{
    if (AMSInkAreaActor* Area = Cast<AMSInkAreaActor>(OtherActor))
    {
        OverlappingAreas.AddUnique(Area);

        // 일정 주기로 정화 호출
        GetWorld()->GetTimerManager().SetTimer(
            CleanTimer,
            this,
            &UMSInkCleanerComponent::ApplyClean,
            CleanInterval,
            true
        );
    }
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

    for (const TWeakObjectPtr<AMSInkAreaActor>& AreaPtr : OverlappingAreas)
    {
        AMSInkAreaActor* Area = AreaPtr.Get();
        if (!Area) continue;
        // 플레이어 현재 위치
        const FVector CleanPos = Owner->GetActorLocation();
        Area->CleanAtWorldPos(CleanPos, CleanRadiusCm);
    }
}