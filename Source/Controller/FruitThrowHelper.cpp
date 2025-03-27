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

    // 접시 위치 찾기
    FVector PlateCenter = FVector::ZeroVector;
    float PlateRadius = 0.0f;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        PlateCenter = PlateActors[0]->GetActorLocation();
        PlateRadius = PlateActors[0]->FindComponentByClass<UPrimitiveComponent>()->Bounds.SphereRadius;
        UE_LOG(LogTemp, Log, TEXT("접시 위치: %s, 반경: %f"), *PlateCenter.ToString(), PlateRadius);
    }
    
    // 미리보기 공이 존재하는지 확인
    if (Controller->PreviewBall)
    {
        UE_LOG(LogTemp, Log, TEXT("미리보기 공을 발사합니다."));
        
        // 물리 컴포넌트 가져오기
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Controller->PreviewBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
        if (PrimComp)
        {
            // 현재 위치 저장 (카메라 앞)
            FVector CurrentLocation = Controller->PreviewBall->GetActorLocation();
            
            // 접시보다 50.f 높게 조정 (기존과 동일)
            CurrentLocation.Z = PlateCenter.Z + 50.f;
            
            // 먼저 부착 해제 (있는 경우)
            Controller->PreviewBall->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            
            // 새로운 높이로 위치 설정
            Controller->PreviewBall->SetActorLocation(CurrentLocation);
            
            // 물리 시뮬레이션 활성화
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetCollisionProfileName(TEXT("PhysicsActor"));
            
            // 접시 중앙까지의 정확한 벡터 계산 (직선 벡터)
            FVector ToPlateCenterExact = PlateCenter - CurrentLocation;
            float ExactDistance = ToPlateCenterExact.Size();
            
            // 접시까지의 수평 거리 계산 (포물선 계산용)
            FVector HorizontalDist = ToPlateCenterExact;
            HorizontalDist.Z = 0; // Z 성분 제거하여 수평 거리만 계산
            float HorizontalDistance = HorizontalDist.Size();
            
            // 이상적인 포물선 발사 각도 계산 (약 45도가 최대 거리)
            float OptimalAngle = 45.0f; // 기본값 45도
            float RadAngle = FMath::DegreesToRadians(OptimalAngle);
            
            // 발사 벡터 계산 - 수평 방향과 높이 방향 조합
            FVector HorizontalDir = HorizontalDist.GetSafeNormal();
            float HeightFactor = FMath::Tan(RadAngle); // 탄젠트 45도 = 1.0
            
            // 거리에 따라 각도 미세 조정 (멀면 더 높게, 가까우면 더 낮게)
            if (HorizontalDistance > 500.0f)
            {
                HeightFactor *= 1.2f; // 먼 거리일수록 조금 더 높게
            }
            else if (HorizontalDistance < 200.0f)
            {
                HeightFactor *= 0.8f; // 가까운 거리일수록 조금 더 낮게
            }
            
            // 수평 방향과 수직 방향 조합
            FVector LaunchDirection = HorizontalDir + FVector(0, 0, HeightFactor);
            LaunchDirection.Normalize();
            
            // 높은 포물선을 위해 힘 증가 (기존보다 20% 강하게)
            float AdjustedForce = Controller->ThrowForce * 1.2f;
            
            // 발사
            PrimComp->AddImpulse(LaunchDirection * AdjustedForce, NAME_None, true);
            
            UE_LOG(LogTemp, Log, TEXT("공 발사 - 위치: %s, 방향: %s, 힘: %f, 높이계수: %f"), 
                *CurrentLocation.ToString(),
                *LaunchDirection.ToString(),
                AdjustedForce,
                HeightFactor);
            
            // 발사된 공은 더 이상 미리보기 공이 아님
            Controller->PreviewBall = nullptr;
            
            // 던진 후 다음 미리보기 공 준비 (새로운 타입으로 랜덤 선택)
            Controller->CurrentBallType = FMath::RandRange(1, 11);
            UpdatePreviewBall(Controller);
        }
    }
    else
    {
        // FruitBallClass가 설정되어 있으면 공 액터 스폰 시도
        if (Controller->FruitBallClass)
        {
            // 접시보다 50.f 높은 위치에서 스폰
            FVector SpawnLocation = PlateCenter;
            SpawnLocation.Z += 50.f;
            
            AActor* SpawnedBall = SpawnBall(Controller, SpawnLocation, Controller->CurrentBallType, true);
            if (SpawnedBall)
            {
                // 물리 시뮬레이션 중이면 ThrowAngle에 따른 힘 부여
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    float RadAngle = FMath::DegreesToRadians(Controller->ThrowAngle);
                    FVector ImpulseDirection = FVector(FMath::Cos(RadAngle), 0.f, FMath::Sin(RadAngle)).GetSafeNormal();
                    PrimComp->AddImpulse(ImpulseDirection * Controller->ThrowForce, NAME_None, true);
                }
                UE_LOG(LogTemp, Log, TEXT("공 타입 %d 스폰됨"), Controller->CurrentBallType);
                
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
}

// 미리보기 공 업데이트 함수 수정
void UFruitThrowHelper::UpdatePreviewBall(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePreviewBall: Controller가 유효하지 않습니다."));
        return;
    }

    // 접시 위치 찾기
    FVector PlateCenter = FVector::ZeroVector;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        PlateCenter = PlateActors[0]->GetActorLocation();
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
    
    // 플레이어 방향 계산
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector PlayerDirection = PlayerPawn->GetActorForwardVector();
    
    // 접시보다 50.f 높게 설정된 위치 계산
    FVector PreviewLocation = FVector(
        PlayerLocation.X + PlayerDirection.X * 300.f,
        PlayerLocation.Y + PlayerDirection.Y * 300.f,
        PlateCenter.Z + 50.f  // 접시보다 50.f 높게 설정
    );
    
    // 공통 함수 호출로 공 생성 (물리 비활성화)
    Controller->PreviewBall = SpawnBall(Controller, PreviewLocation, Controller->CurrentBallType, false);
    
    if (Controller->PreviewBall)
    {
        UE_LOG(LogTemp, Warning, TEXT("미리보기 공 생성 성공 - 위치: %s"), *PreviewLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 공 생성에 실패했습니다!"));
    }
}