#include "FruitThrowHelper.h"
#include "FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

// 공통 함수로 볼 생성 로직 통합
AActor* UFruitThrowHelper::SpawnBall(AFruitPlayerController* Controller, const FVector& Location, int32 BallType, bool bEnablePhysics)
{
    if (!Controller || !Controller->FruitBallClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnBall: Controller 또는 FruitBallClass가 유효하지 않습니다."));
        return nullptr;
    }

    // 공 액터 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = Controller;
    
    AActor* SpawnedBall = Controller->GetWorld()->SpawnActor<AActor>(
        Controller->FruitBallClass, Location, FRotator::ZeroRotator, SpawnParams);
        
    if (SpawnedBall)
    {
        // 크기 설정 - 모든 공에 동일한 계산식 적용
        float BaseSize = 0.5f;
        float ScaleFactor = 1.f + 0.1f * (BallType - 1);
        float BallSize = BaseSize * ScaleFactor;
        SpawnedBall->SetActorScale3D(FVector(BallSize));
        
        // 물리 컴포넌트 설정
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(
            SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
            
        if (PrimComp)
        {
            // 물리 활성화 여부 설정
            PrimComp->SetSimulatePhysics(bEnablePhysics);
            
            if (bEnablePhysics)
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetCollisionProfileName(TEXT("PhysicsActor"));
            }
            else
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("공 스폰 성공 - 타입: %d, 크기: %f, 물리: %s"), 
            BallType, BallSize, bEnablePhysics ? TEXT("활성화") : TEXT("비활성화"));
    }
    
    return SpawnedBall;
}

void UFruitThrowHelper::ThrowFruit(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller가 유효하지 않습니다."));
        return;
    }

    // 접시 액터 위치 검색
    FVector PlateLocation = FVector::ZeroVector;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        PlateLocation = PlateActors[0]->GetActorLocation();
    }
    
    // 발사 시작 위치를 접시 앞쪽 낮은 곳으로 설정 (포물선 효과를 위해)
    // 접시 높이에서 약간 아래, 플레이어 방향쪽에서 스폰
    APawn* PlayerPawn = Controller->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    // 플레이어에서 접시 방향 벡터
    FVector PlayerToPlate = (PlateLocation - PlayerPawn->GetActorLocation()).GetSafeNormal();
    // 접시 앞쪽 낮은 위치에서 공을 스폰 (약 200 유닛 앞, 50유닛 아래)
    FVector SpawnLocation = PlateLocation - PlayerToPlate * 200.f;
    // 높이를 접시보다 낮게 설정
    SpawnLocation.Z = PlateLocation.Z - 50.f;
    
    // 미리보기 공이 존재하는지 확인
    int32 CurrentType = Controller->CurrentBallType;
    
    // 미리보기 공은 단순히 표시용으로만 사용하고 실제 발사는 새 공으로
    if (Controller->PreviewBall)
    {
        // 미리보기 공 제거 (실제 공을 새로 만들 것이므로)
        Controller->PreviewBall->Destroy();
        Controller->PreviewBall = nullptr;
    }
    
    // 새 공을 스폰하여 던지기
    AActor* SpawnedBall = SpawnBall(Controller, SpawnLocation, CurrentType, true);
    if (SpawnedBall)
    {
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(
            SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
            
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            // ThrowAngle 사용하여 포물선 발사
            float RadAngle = FMath::DegreesToRadians(Controller->ThrowAngle);
            FVector ImpulseDirection = FVector(FMath::Cos(RadAngle), 0.f, FMath::Sin(RadAngle)).GetSafeNormal();
            PrimComp->AddImpulse(ImpulseDirection * Controller->ThrowForce, NAME_None, true);
            
            UE_LOG(LogTemp, Log, TEXT("공 발사 - 위치: %s, 각도: %f, 힘: %f"), 
                *SpawnLocation.ToString(), Controller->ThrowAngle, Controller->ThrowForce);
        }
    }
    
    // 다음 미리보기 공 준비
    Controller->CurrentBallType = FMath::RandRange(1, 11);
    UpdatePreviewBall(Controller);
}

// 미리보기 공 업데이트 함수 수정
void UFruitThrowHelper::UpdatePreviewBall(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePreviewBall: Controller가 유효하지 않습니다."));
        return;
    }

    // 이전 미리보기 공 제거
    if (Controller->PreviewBall)
    {
        Controller->PreviewBall->Destroy();
        Controller->PreviewBall = nullptr;
    }
    
    // 플레이어 카메라 정보 가져오기
    APawn* PlayerPawn = Controller->GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 실패: PlayerPawn이 NULL입니다!"));
        return;
    }
    
    // 카메라 위치와 방향 계산
    FVector CameraLocation = PlayerPawn->GetActorLocation();
    FRotator CameraRotation = PlayerPawn->GetActorRotation();
    FVector PreviewLocation = CameraLocation + CameraRotation.Vector() * 100.f;
    
    // 공통 함수 호출로 공 생성 (물리 비활성화)
    Controller->PreviewBall = SpawnBall(Controller, PreviewLocation, Controller->CurrentBallType, false);
    
    if (Controller->PreviewBall)
    {
        // 부착 시도를 별도로 수행
        UCameraComponent* CameraComp = PlayerPawn->FindComponentByClass<UCameraComponent>();
        if (CameraComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("카메라 컴포넌트 찾음, 부착 시도"));
            Controller->PreviewBall->AttachToComponent(CameraComp, FAttachmentTransformRules::KeepWorldTransform);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("카메라 컴포넌트를 찾을 수 없습니다!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 공 생성에 실패했습니다!"));
    }
}