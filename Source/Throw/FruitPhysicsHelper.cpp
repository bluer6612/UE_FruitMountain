#include "FruitPhysicsHelper.h"
#include "FruitPlayerController.h"

// 던지기 파라미터 계산 함수 - 모든 공 던지기 로직에서 공통으로 사용
void UFruitPhysicsHelper::CalculateThrowParameters(
    AFruitPlayerController* Controller, 
    const FVector& StartLocation, 
    const FVector& TargetLocation, 
    float& OutAdjustedForce, 
    FVector& OutLaunchDirection,
    float BallMass)
{
    // 기본값 설정
    OutAdjustedForce = 1000.0f;
    OutLaunchDirection = FVector::ForwardVector;
    
    if (!Controller)
        return;
    
    // 시작점과 목표점 사이의 방향과 거리 계산
    FVector Direction = TargetLocation - StartLocation;
    float Distance = Direction.Size();
    
    // 목표 지점이 너무 가까우면 기본값 반환
    // if (Distance < 100.0f)
    //     return;
    
    // 방향 벡터 정규화
    Direction.Normalize();

    // Z 방향 조정 - 거리에 따른 최적 투사 각도 계산
    float VerticalAdjustment = FMath::Clamp(Distance / 1500.0f, 0.3f, 0.7f);
    
    // 수평 방향과 수직 방향 분리
    FVector HorizontalDir = Direction;
    HorizontalDir.Z = 0.0f;
    HorizontalDir.Normalize();
    
    // 거리에 따라 궤적 높이 조정 (Z 성분)
    Direction = HorizontalDir * (1.0f - VerticalAdjustment) + FVector(0, 0, 1) * VerticalAdjustment;
    Direction.Normalize();
    
    // 중력 상수
    const float GravityZ = -980.0f;
    
    // 거리 기반 힘 계산 - 포물선 운동 공식에 기반
    float DistanceFactor = FMath::Clamp(Distance / 500.0f, 0.8f, 3.0f);
    
    // 기본 힘 - 질량과 중력을 고려한 기본값
    float BaseForce = 500.0f;
    
    // 질량, 거리, 중력을 고려한 최종 힘 계산
    OutAdjustedForce = BallMass * BaseForce * FMath::Sqrt(DistanceFactor);
    
    // 최종 방향 설정
    OutLaunchDirection = Direction;
}