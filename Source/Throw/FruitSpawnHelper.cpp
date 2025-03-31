#include "FruitSpawnHelper.h"
#include "FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Actors/PlateActor.h"
#include "Actors/FruitBall.h" // 전체 헤더 포함

// 크기 계산 함수 - FruitBall 클래스 함수 사용
float UFruitSpawnHelper::CalculateBallSize(int32 BallType)
{
    // FruitBall 클래스의 함수 사용 - 월드 스케일로 계산됨
    float WorldSizeCM = AFruitBall::CalculateBallSize(BallType);
    
    // 언리얼 액터 스케일 반환 (100으로 나눔)
    return WorldSizeCM / 100.0f;
}

// 질량 계산 함수 - FruitBall 클래스 함수 사용
float UFruitSpawnHelper::CalculateBallMass(int32 BallType)
{
    // 직접 FruitBall 클래스의 함수 호출
    return AFruitBall::CalculateBallMass(BallType);
}

// 공 생성 함수 - 접시 높이 고려
AActor* UFruitSpawnHelper::SpawnBall(AFruitPlayerController* Controller, int32 BallType, bool bIsPreview)
{
    if (!Controller || !Controller->GetWorld())
        return nullptr;
        
    UWorld* World = Controller->GetWorld();
    
    // 공 클래스 가져오기
    UClass* BallClass = Controller->FruitBallClass;
    if (!BallClass)
    {
        UE_LOG(LogTemp, Error, TEXT("FruitBallClass가 설정되지 않았습니다."));
        return nullptr;
    }
    
    // 접시 정보 업데이트 (최신 정보 사용)
    Controller->UpdatePlateInfo();
    
    // 스폰 위치 계산 - 접시 높이 고려
    FVector SpawnLocation;
    
    if (bIsPreview)
    {
        // 미리보기 공은 고정된 위치에 생성
        // 카메라 앞쪽 위치 계산
        FVector CameraLocation;
        FRotator CameraRotation;
        Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
        
        // 카메라 앞쪽 방향
        FVector CameraForward = CameraRotation.Vector();
        FVector CameraRight = FRotator(0, CameraRotation.Yaw, 0).Vector();
        
        // 접시 높이를 고려하여 스폰 높이 조정
        float TargetHeight = Controller->PlateLocation.Z + Controller->SpawnHeightOffset;
        
        // 카메라 앞쪽에 위치
        SpawnLocation = CameraLocation + CameraForward * 200.0f;
        
        // 높이 조정 (접시 높이 + 오프셋)
        SpawnLocation.Z = TargetHeight;
    }
    else
    {
        // 실제 던지는 공 - 미리보기 공 위치 사용
        if (Controller->PreviewBall)
        {
            SpawnLocation = Controller->PreviewBall->GetActorLocation();
        }
        else
        {
            // 미리보기 공이 없는 경우 기본 위치 계산
            // 접시 높이를 고려하여 스폰 높이 조정
            float TargetHeight = Controller->PlateLocation.Z + Controller->SpawnHeightOffset;
            
            // 카메라 앞쪽 위치 계산
            FVector CameraLocation;
            FRotator CameraRotation;
            Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
            
            SpawnLocation = CameraLocation + CameraRotation.Vector() * 200.0f;
            SpawnLocation.Z = TargetHeight;
        }
    }
    
    // 스폰 회전 설정
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    // 공 생성
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* SpawnedBall = World->SpawnActor<AActor>(BallClass, SpawnLocation, SpawnRotation, SpawnParams);
    
    if (SpawnedBall)
    {
        // 공 크기 및 머티리얼 설정 처리
        // ... 기존 코드 유지 ...
        
        if (bIsPreview)
        {
            UE_LOG(LogTemp, Warning, TEXT("미리보기 공 생성 성공: 위치=%s"),
                *SpawnLocation.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("공 생성 실패"));
    }
    
    return SpawnedBall;
}

// 접시 가장자리 위치 계산 함수 구현
FVector UFruitSpawnHelper::CalculatePlateEdgeSpawnPosition(UWorld* World, float HeightOffset, float CameraAngle)
{
    // 접시 위치 확인
    FVector PlateCenter = FVector::ZeroVector;
    float PlateRadius = 0.0f;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);

    if (PlateActors.Num() > 0)
    {
        // 접시 정보 가져오기
        APlateActor* PlateActorRef = Cast<APlateActor>(PlateActors[0]);
        if (PlateActorRef)
        {
            PlateCenter = PlateActorRef->GetActorLocation();
            
            // 접시 반지름 계산
            FVector Bounds = PlateActorRef->GetComponentsBoundingBox().GetSize();
            PlateRadius = FMath::Max(Bounds.X, Bounds.Y) * 0.45f; // 가장자리에 더 가깝게 조정
            
            // 접시 높이 가져오기
            float PlateHeight = Bounds.Z;
            
            // 카메라 방향 벡터 계산
            float RadianAngle = FMath::DegreesToRadians(CameraAngle);
            FVector CameraDirection;
            CameraDirection.X = FMath::Cos(RadianAngle);
            CameraDirection.Y = FMath::Sin(RadianAngle);
            CameraDirection.Z = 0.0f;
            CameraDirection.Normalize();
            
            // 카메라 방향의 반대쪽 접시 가장자리 지점 계산 (카메라에서 가장 먼 곳)
            FVector EdgePoint = PlateCenter + CameraDirection * PlateRadius;
            
            // 높이 조정 - 접시 위로 HeightOffset만큼 더 높게
            EdgePoint.Z = PlateCenter.Z + PlateHeight + HeightOffset;
                
            return EdgePoint;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다."));
    return FVector::ZeroVector;
}