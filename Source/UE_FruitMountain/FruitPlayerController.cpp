#include "FruitPlayerController.h"
#include "FruitInputMappingManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/InputSettings.h"
#include "Engine/Engine.h"

AFruitPlayerController::AFruitPlayerController()
{
    ThrowAngle = 45.f;
    ThrowForce = 1000.f;
    AngleStep = 5.f;
}

void AFruitPlayerController::BeginPlay()
{
    Super::BeginPlay();
    UFruitInputMappingManager::ConfigureKeyMappings();
}

void AFruitPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
        InputComponent->BindAction("IncreaseAngle", IE_Pressed, this, &AFruitPlayerController::IncreaseAngle);
        InputComponent->BindAction("DecreaseAngle", IE_Pressed, this, &AFruitPlayerController::DecreaseAngle);
        InputComponent->BindAction("ThrowFruit", IE_Pressed, this, &AFruitPlayerController::ThrowFruit);
    }
}

void AFruitPlayerController::IncreaseAngle()
{
    ThrowAngle += AngleStep;
    UE_LOG(LogTemp, Log, TEXT("각도 증가: 현재 각도 %f"), ThrowAngle);
}

void AFruitPlayerController::DecreaseAngle()
{
    ThrowAngle -= AngleStep;
    UE_LOG(LogTemp, Log, TEXT("각도 감소: 현재 각도 %f"), ThrowAngle);
}

void AFruitPlayerController::ThrowFruit()
{
    UE_LOG(LogTemp, Log, TEXT("과일 발사! 각도: %f, 힘: %f"), ThrowAngle, ThrowForce);
    HandleThrow();
}

void AFruitPlayerController::HandleThrow()
{
    if (!FruitBallClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("FruitBallClass가 설정되어 있지 않습니다."));
        return;
    }

    int32 BallType = FMath::RandRange(1, 11);
    float BaseSize = 25.f;
    float ScaleFactor = 1.f + 0.1f * (BallType - 1);
    float BallSize = BaseSize * ScaleFactor;

    // 플레이레벨에 배치된 접시(Plate) 액터의 위치를 찾습니다.
    FVector SpawnLocation = FVector(0.f, 0.f, 0.f);
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        SpawnLocation = PlateActors[0]->GetActorLocation();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 액터(Plate)를 찾을 수 없습니다. 기본 위치에서 스폰합니다."));
    }

    FRotator SpawnRotation = FRotator::ZeroRotator;
    FActorSpawnParameters SpawnParams;

    AActor* SpawnedBall = GetWorld()->SpawnActor<AActor>(FruitBallClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (SpawnedBall)
    {
        SpawnedBall->SetActorScale3D(FVector(BallSize, BallSize, BallSize));

        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            float RadAngle = FMath::DegreesToRadians(ThrowAngle);
            FVector ImpulseDirection = FVector(FMath::Cos(RadAngle), 0.f, FMath::Sin(RadAngle)).GetSafeNormal();
            PrimComp->AddImpulse(ImpulseDirection * ThrowForce, NAME_None, true);
        }
        UE_LOG(LogTemp, Log, TEXT("공 타입 %d 스폰됨, 크기: %f"), BallType, BallSize);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("공 액터 생성 실패"));
    }
}