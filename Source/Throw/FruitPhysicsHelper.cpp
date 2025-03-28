#include "FruitPhysicsHelper.h"
#include "FruitPlayerController.h"

// 던지기 파라미터 계산 함수 - 힘을 크게 감소하여 적절한 포물선 구현
void UFruitPhysicsHelper::CalculateThrowParameters(
    AFruitPlayerController* Controller, 
    const FVector& StartLocation, 
    const FVector& TargetLocation, 
    float& OutAdjustedForce, 
    FVector& OutLaunchDirection,
    float BallMass)
{
    // 기본값 설정 - 매우 낮은 기본 힘
    OutAdjustedForce = 100.0f;
    OutLaunchDirection = FVector::ForwardVector;
    
    if (!Controller)
        return;
    
    // 시작점과 목표점 사이의 방향과 거리 계산
    FVector Direction = TargetLocation - StartLocation;
    float Distance = Direction.Size();
    
    // 목표 지점이 너무 가까우면 기본값 반환
    if (Distance < 50.0f)
        return;
    
    // 방향 벡터 정규화
    Direction.Normalize();

    // --- 투사체 운동 공식 기반 계산 ---
    
    // 중력 상수
    const float GravityZ = -980.0f;
    
    // Controller의 ThrowAngle 값 사용 (0~90도 범위 내에서 제한)
    float OptimalAngle = FMath::Clamp(Controller->ThrowAngle, 20.0f, 70.0f);
    
    // 각도를 라디안으로 변환
    float AngleRad = FMath::DegreesToRadians(OptimalAngle);
    
    // 수평 방향 계산
    FVector HorizontalDir = Direction;
    HorizontalDir.Z = 0.0f;
    HorizontalDir.Normalize();
    
    // Z 방향 조정 - 컨트롤러의 던지기 각도 적용
    float ZComponent = FMath::Sin(AngleRad);
    float HorizontalComponent = FMath::Cos(AngleRad);
    
    // 최종 방향 벡터 구성
    OutLaunchDirection = HorizontalDir * HorizontalComponent + FVector(0, 0, 1) * ZComponent;
    OutLaunchDirection.Normalize();
    
    // --- 대폭 감소된 힘 계산 ---
    
    // 투사체 운동 공식 사용하되 매우 약한 힘으로 조정
    float HorizontalDistance = FVector::Dist2D(StartLocation, TargetLocation);
    
    // 투사 각도의 2배 사인 값 계산 (투사체 공식에 사용)
    float SinDoubleAngle = FMath::Sin(2 * AngleRad);
    
    // 분모가 0이 되는 것 방지 (각도가 0이나 90도일 때)
    if (SinDoubleAngle < 0.1f) SinDoubleAngle = 0.1f;
    
    // 이론적 초기 속도 계산 (투사체 운동 공식 기반) - 매우 작은 계수 적용
    float RequiredVelocity = FMath::Sqrt((HorizontalDistance * FMath::Abs(GravityZ)) / SinDoubleAngle);
    
    // 힘 계산 - 매우 작은 계수 적용 (0.01f)
    float ForceAdjustFactor = 0.01f;
    
    // 질량 고려 힘 계산
    OutAdjustedForce = RequiredVelocity * BallMass * ForceAdjustFactor;
    
    // 힘 제한 - 매우 낮은 범위로 제한
    OutAdjustedForce = FMath::Clamp(OutAdjustedForce, 10.0f * BallMass, 200.0f * BallMass);
    
    // 디버그 로그
    UE_LOG(LogTemp, Warning, TEXT("던지기 계산: 거리=%f, 각도=%f도, 힘=%f, 질량=%f"), 
        Distance, OptimalAngle, OutAdjustedForce, BallMass);
}