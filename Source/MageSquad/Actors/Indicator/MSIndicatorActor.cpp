#include "Actors/Indicator/MSIndicatorActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/TargetActor/MSTargetActor_IndicatorBase.h"
#include "Components/DecalComponent.h"
#include "Interfaces/MSIndicatorDamageInterface.h"
#include "Net/UnrealNetwork.h"

AMSIndicatorActor::AMSIndicatorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 네트워크 세팅
	bReplicates = true;
	bAlwaysRelevant = true;

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	RootComponent = DecalComponent;

	// 바닥을 향하도록 기본 회전 설정
	DecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
}

void AMSIndicatorActor::Initialize(const FAttackIndicatorParams& Params)
{
	CachedParams = Params;
	ElapsedTime = 0.f;
	ApplyMaterialParams();
}

void AMSIndicatorActor::SetDamageInfo(UAbilitySystemComponent* SourceASC, TSubclassOf<UGameplayEffect> InDamageEffectClass)
{
	SourceAbilitySystemComponent = SourceASC;
	DamageEffectClass = InDamageEffectClass;
}

void AMSIndicatorActor::SetIndicatorOwner(AActor* InOwner)
{
	IndicatorOwner = InOwner;
	UE_LOG(LogTemp, Warning, TEXT("SetIndicatorOwner: %s"), *IndicatorOwner->GetName());
}

void AMSIndicatorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;

	if (ElapsedTime >= CachedParams.Duration)
	{
		OnFillComplete();
		Destroy();
		return;
	}

	UpdateFillPercent();
}

void AMSIndicatorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMSIndicatorActor, CachedParams);
}

void AMSIndicatorActor::OnRep_CachedParams()
{
	ElapsedTime = 0.f;
	ApplyMaterialParams();
}

void AMSIndicatorActor::ApplyMaterialParams()
{
	if (!IndicatorMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMSIndicatorActor: IndicatorMaterial is not set!"));
		return;
	}

	if (!DynamicMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(IndicatorMaterial, this);
		DecalComponent->SetDecalMaterial(DynamicMaterial);
	}

	DynamicMaterial->SetScalarParameterValue(TEXT("ShapeType"), static_cast<float>(CachedParams.Shape));
	DynamicMaterial->SetScalarParameterValue(TEXT("FillPercent"), 0.f);

	switch (CachedParams.Shape)
	{
	case EIndicatorShape::Circle:
		DynamicMaterial->SetScalarParameterValue(TEXT("Angle"), PI * 2.f);
		DecalComponent->DecalSize = FVector(CachedParams.Radius);
		break;

	case EIndicatorShape::Cone:
		// Angle을 라디안으로 변환하여 전달
		DynamicMaterial->SetScalarParameterValue(TEXT("Angle"), CachedParams.Angle);
		DecalComponent->DecalSize = FVector(CachedParams.Radius);
		break;

	case EIndicatorShape::Rectangle:
		DynamicMaterial->SetScalarParameterValue(TEXT("AspectRatio"), CachedParams.Length / CachedParams.Width);
		//DecalComponent->DecalSize = FVector(CachedParams.Length * 0.5f, CachedParams.Width * 0.5f, 100.f);
		float HalfLength = CachedParams.Length * 0.5f;
		float HalfWidth = CachedParams.Width * 0.5f;
		SetActorLocation(GetActorLocation() + (GetActorUpVector() * HalfLength)); // 데칼 회전때문에 Forward가 아닌 UpVector 이용
		DecalComponent->DecalSize = FVector(500.f, HalfWidth, HalfLength);
		break;
	}
}

void AMSIndicatorActor::UpdateFillPercent() const
{
	if (DynamicMaterial)
	{
		const float Percent = FMath::Clamp(ElapsedTime / CachedParams.Duration, 0.f, 1.f);
		DynamicMaterial->SetScalarParameterValue(TEXT("FillPercent"), Percent);
	}
}

void AMSIndicatorActor::OnFillComplete()
{
	// 서버에서만 충돌 검사 및 데미지 처리
	if (!HasAuthority())
	{
		return;
	}

	// TargetActor 스폰 및 타겟팅 수행
	TArray<AActor*> HitActors = SpawnTargetActorAndPerformTargeting();

	// 델리게이트 브로드캐스트
	OnIndicatorComplete.Broadcast(this, HitActors);
}

bool AMSIndicatorActor::GetDamageInfo(UAbilitySystemComponent*& OutASC, TSubclassOf<UGameplayEffect>& OutDamageEffect) const
{
	// 1. 직접 설정된 값이 있으면 사용
	if (SourceAbilitySystemComponent.IsValid() && DamageEffectClass)
	{
		OutASC = SourceAbilitySystemComponent.Get();
		OutDamageEffect = DamageEffectClass;
		return true;
	}

	// 2. Owner 체크
	if (!IndicatorOwner.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetDamageInfo: IndicatorOwner is invalid"));
		return false;
	}

	// 3. 인터페이스 구현 체크
	if (!IndicatorOwner->Implements<UMSIndicatorDamageInterface>())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetDamageInfo: Owner [%s] does not implement IMSIndicatorDamageInterface"), *IndicatorOwner->GetName());
		return false;
	}

	// 4. 인터페이스로 조회
	OutASC = IMSIndicatorDamageInterface::Execute_GetIndicatorSourceASC(IndicatorOwner.Get());
	OutDamageEffect = IMSIndicatorDamageInterface::Execute_GetIndicatorDamageEffect(IndicatorOwner.Get());

	UE_LOG(LogTemp, Log, TEXT("GetDamageInfo: ASC=%s, DamageEffect=%s"), 
		OutASC ? TEXT("Valid") : TEXT("Null"),
		OutDamageEffect ? TEXT("Valid") : TEXT("Null"));

	return OutASC != nullptr && OutDamageEffect != nullptr;
}

TArray<AActor*> AMSIndicatorActor::SpawnTargetActorAndPerformTargeting()
{
	TArray<AActor*> HitActors;

	TSubclassOf<AMSTargetActor_IndicatorBase> TargetActorClass = GetTargetActorClassForShape();
	if (!TargetActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMSIndicatorActor: TargetActorClass not set for Shape %d"), static_cast<int32>(CachedParams.Shape));
		return HitActors;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return HitActors;
	}

	// TargetActor 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// 데칼의 -90도 회전을 무시하고 수평(Pitch=0)으로 스폰
	FRotator SpawnRotation = GetActorRotation();
	SpawnRotation.Pitch = 0.f;
	SpawnRotation.Roll = 0.f;

	AMSTargetActor_IndicatorBase* TargetActor = World->SpawnActor<AMSTargetActor_IndicatorBase>(
		TargetActorClass,
		GetActorLocation(),
		//GetActorRotation(),
		SpawnRotation,
		SpawnParams);

	if (TargetActor)
	{
		UAbilitySystemComponent* ASC = nullptr;
		TSubclassOf<UGameplayEffect> DamageEffect = nullptr;
		GetDamageInfo(ASC, DamageEffect); 
        		
		// Indicator 파라미터로 초기화
		TargetActor->InitializeFromIndicator(CachedParams, ASC, DamageEffect);

		// 타겟팅 + 데미지 적용 수행
		HitActors = TargetActor->ExecuteTargetingAndDamage();

		// TargetActor 즉시 제거 (일회성 사용)
		TargetActor->Destroy();
	}

	return HitActors;
}

TSubclassOf<AMSTargetActor_IndicatorBase> AMSIndicatorActor::GetTargetActorClassForShape() const
{
	switch (CachedParams.Shape)
	{
	case EIndicatorShape::Circle:
		return SphereTargetActorClass;

	case EIndicatorShape::Cone:
		return ConeTargetActorClass;

	case EIndicatorShape::Rectangle:
		return RectangleTargetActorClass;

	default:
		return nullptr;
	}
}
