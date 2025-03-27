#include "FruitThrowHelper.h"
#include "FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

void UFruitThrowHelper::ThrowFruit(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller가 유효하지 않습니다."));
        return;
    }

    // 접시 액터 위치 검색 (스폰 기준 위치)
    FVector LocalSpawnLocation = FVector::ZeroVector;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        // Plate 위치에서 위로 100만큼 올려서 스폰
        LocalSpawnLocation = PlateActors[0]->GetActorLocation() + FVector(0, 0, 100.f);
        UE_LOG(LogTemp, Log, TEXT("접시 액터 발견: 위치 (%f, %f, %f)"),
               LocalSpawnLocation.X, LocalSpawnLocation.Y, LocalSpawnLocation.Z);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다. 기본 위치 (0,0,0)에서 스폰합니다."));
    }

    // FruitBallClass가 설정되어 있으면 공 액터 스폰 시도
    if (Controller->FruitBallClass)
    {
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FActorSpawnParameters SpawnParams;
        // 스폰 충돌 관련 설정: 항상 스폰
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AActor* SpawnedBall = Controller->GetWorld()->SpawnActor<AActor>(Controller->FruitBallClass, LocalSpawnLocation, SpawnRotation, SpawnParams);
        if (SpawnedBall)
        {
            // 미리보기 공과 동일한 타입/크기 사용
            float BaseSize = 5.f;
            float ScaleFactor = 1.f + 0.1f * (Controller->CurrentBallType - 1);
            float BallSize = BaseSize * ScaleFactor;
            SpawnedBall->SetActorScale3D(FVector(BallSize));
            
            // 물리 시뮬레이션 중이면 ThrowAngle에 따른 힘 부여
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                float RadAngle = FMath::DegreesToRadians(Controller->ThrowAngle);
                FVector ImpulseDirection = FVector(FMath::Cos(RadAngle), 0.f, FMath::Sin(RadAngle)).GetSafeNormal();
                PrimComp->AddImpulse(ImpulseDirection * Controller->ThrowForce, NAME_None, true);
            }
            UE_LOG(LogTemp, Log, TEXT("공 타입 %d 스폰됨, 크기: %f"), Controller->CurrentBallType, BallSize);
            
            // 던진 후 다음 미리보기 공 준비 (새로운 타입으로 랜덤 선택)
            Controller->CurrentBallType = FMath::RandRange(1, 11);
            Controller->UpdatePreviewBall();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("공 액터 생성에 실패했습니다."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FruitBallClass가 설정되어 있지 않습니다."));
    }
}