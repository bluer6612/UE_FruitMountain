#include "FruitPhysicsHelper.h"
#include "FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Actors/FruitBall.h" // FruitBall 헤더 포함

// 던지기 파라미터 계산 함수
void UFruitPhysicsHelper::CalculateThrowParameters(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float& OutAdjustedForce, FVector& OutLaunchDirection, float BallMass)
{
    // 방향 및 거리 계산
    FVector Direction = TargetLocation - StartLocation;
    float Distance = Direction.Size();
    
    // --- 발사 각도 계산 ---
    float BaseAngle = 20.0f; // 기본 발사 각도 (도)
    float AngleRad = FMath::DegreesToRadians(BaseAngle); // 라디안 변환
    
    // 방향 벡터 정규화 및 높이 조정
    Direction.Normalize();
    OutLaunchDirection = FVector(Direction.X, Direction.Y, 0.0f);
    OutLaunchDirection.Normalize();
    
    // Z 성분 추가 (발사 각도 적용)
    OutLaunchDirection.Z = FMath::Tan(AngleRad);
    OutLaunchDirection.Normalize();
    
    // --- 질량 기반 힘 계산 (질량이 높을수록 더 적은 힘 적용) ---
    
    // 기본 힘 계산
    float BaseForce = 2000.0f;
    float DistanceFactor = FMath::Clamp(Distance / 300.0f, 0.8f, 3.0f);

    // 중요: 질량 역수 계수 적용 - FruitBall 클래스의 표준 질량 사용
    float StandardMass = AFruitBall::DensityFactor * 50.0f / 0.3f; // 원래 표준 질량에 밀도 비율 적용
    float MassRatio = StandardMass / BallMass; // 질량이 클수록 작은 값이 됨

    // 최종 힘 계산 (질량이 높을수록 힘 감소)
    OutAdjustedForce = BaseForce * DistanceFactor * MassRatio;

    // 추가 제한: 최소/최대 힘 설정
    float MinForce = 1000.0f;
    float MaxForce = 10000.0f;
    OutAdjustedForce = FMath::Clamp(OutAdjustedForce, MinForce, MaxForce);
    
    // 디버그 로깅
    UE_LOG(LogTemp, Warning, TEXT("던지기 계산: 거리=%f, 각도=%f도, 힘=%f, 질량=%f, 질량비율=%f"), 
        Distance, BaseAngle, OutAdjustedForce, BallMass, MassRatio);
}