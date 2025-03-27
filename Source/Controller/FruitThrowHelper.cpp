#include "FruitThrowHelper.h"
#include "FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

void UFruitThrowHelper::ThrowFruit(AFruitPlayerController* Controller)
{
    if (!Controller || !Controller->GetPawn())
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller 또는 Pawn이 없습니다. 공을 스폰할 수 없습니다."));
        return;
    }

    FVector PlayerLocation = Controller->GetPawn()->GetActorLocation();
    FVector PlateLocation = FVector::ZeroVector;

    // 접시 액터 검색
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        PlateLocation = PlateActors[0]->GetActorLocation();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 액터(Plate)를 찾을 수 없습니다."));
        return;
    }

    // FruitBallClass가 지정되어 있으면 스폰
    if (Controller->FruitBallClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // 플레이어 위치에서 공 스폰
        AActor* SpawnedBall = Controller->GetWorld()->SpawnActor<AActor>(
            Controller->FruitBallClass, 
            PlayerLocation, 
            FRotator::ZeroRotator, 
            SpawnParams
        );

        if (!SpawnedBall)
        {
            UE_LOG(LogTemp, Warning, TEXT("공 액터 생성에 실패했습니다."));
            return;
        }

        // 물리 컴포넌트 가져오기
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
        if (!PrimComp || !PrimComp->IsSimulatingPhysics())
        {
            UE_LOG(LogTemp, Warning, TEXT("공이 물리 시뮬레이션 상태가 아니거나 컴포넌트를 찾을 수 없습니다."));
            return;
        }

        // 포물선 비행시간(예: 1초)과 중력값
        float FlightTime = 1.0f;
        float GravityZ = Controller->GetWorld()->GetGravityZ(); // 일반적으로 -980.f

        // 간단한 탄도 공식
        FVector GravityVector(0.f, 0.f, GravityZ * 0.5f * FMath::Square(FlightTime));
        FVector Velocity = (PlateLocation - PlayerLocation - GravityVector) / FlightTime;

        PrimComp->SetPhysicsLinearVelocity(Velocity);

        UE_LOG(LogTemp, Log, TEXT("공 스폰: (%f, %f, %f) → (%f, %f, %f), 속도 (%f, %f, %f)"),
            PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z,
            PlateLocation.X, PlateLocation.Y, PlateLocation.Z,
            Velocity.X, Velocity.Y, Velocity.Z);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FruitBallClass가 설정되어 있지 않습니다."));
    }
}