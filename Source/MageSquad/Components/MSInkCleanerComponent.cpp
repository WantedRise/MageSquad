#include "MSInkCleanerComponent.h"
#include "Actors/Mission/MSInkAreaActor.h"
#include <Kismet/GameplayStatics.h>
#include "Components/PrimitiveComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Components/AudioComponent.h"

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
        StopCleanSound();
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
    StopCleanSound();
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
        //클린 처리
        if (Area->CleanAtWorldPos(CleanPos, CleanRadiusCm))
        {
            //성공시 사운드
            StartCleanSound();
        }
        else
        {
            StopCleanSound();
        }
    }
}

void UMSInkCleanerComponent::StartCleanSound()
{
    if (bIsCleaningSoundPlaying)
        return;

    if (!CleanLoopSound)
        return;

    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    CleanLoopAudioComp =
        UGameplayStatics::SpawnSoundAttached(
            CleanLoopSound,
            Owner->GetRootComponent(),
            FName(TEXT("")),          // 소켓 이름 (특정 소켓에 첨부하려면 소켓 이름 지정, 없으면 빈 이름)
            FVector(ForceInit),       // 상대 위치 오프셋 (FVector::ZeroVector 사용 가능)
            FRotator(ForceInit),      // 상대 회전 오프셋 (FRotator::ZeroRotator 사용 가능)
            EAttachLocation::SnapToTarget, // 첨부 방식 (타겟 위치/회전에 맞춤)
            true,                     // 자동 활성화 여부
            1.0f,                     // 볼륨 멀티플라이어
            1.0f,                     // 피치 멀티플라이어
            0.0f,                     // 시작 시간 (초)
            nullptr,                  // 감쇠 설정 (USoundAttenuation* 타입, nullptr 시 애셋 기본값)
            nullptr,                  // 동시성 설정 (USoundConcurrency* 타입, nullptr 시 애셋 기본값)
            false                     // 레벨 전환 시 지속 여부
        );

    if (CleanLoopAudioComp)
    {
        CleanLoopAudioComp->bAutoDestroy = false;
        CleanLoopAudioComp->Play();
        bIsCleaningSoundPlaying = true;
    }
}

void UMSInkCleanerComponent::StopCleanSound()
{
    if (!bIsCleaningSoundPlaying)
        return;

    if (CleanLoopAudioComp)
    {
        CleanLoopAudioComp->FadeOut(0.15f, 0.f);
        CleanLoopAudioComp = nullptr;
    }

    bIsCleaningSoundPlaying = false;
}
