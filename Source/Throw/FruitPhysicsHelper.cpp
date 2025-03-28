#include "FruitPhysicsHelper.h"
#include "FruitPlayerController.h"

// 던지기 파라미터 계산 함수 - 힘 조절 및 정확한 궤적 생성
void UFruitPhysicsHelper::CalculateThrowParameters(
    AFruitPlayerController* Controller, 
    const FVector& StartLocation, 
    const FVector& TargetLocation, 
    float& OutAdjustedForce, 
    FVector& OutLaunchDirection,
    float BallMass)
{
    // 기본값 설정
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
    
    // --- 거리 기반 힘 계산 ---
    
    // 투사체 운동 공식 사용
    float HorizontalDistance = FVector::Dist2D(StartLocation, TargetLocation);
    float HeightDifference = TargetLocation.Z - StartLocation.Z;
    
    // 투사 각도의 2배 사인 값 계산 (투사체 공식에 사용)
    float SinDoubleAngle = FMath::Sin(2 * AngleRad);
    
    // 분모가 0이 되는 것 방지 (각도가 0이나 90도일 때)
    if (SinDoubleAngle < 0.1f) SinDoubleAngle = 0.1f;
    
    // 이론적 초기 속도 계산 (투사체 운동 공식)
    // v = sqrt((d*g) / sin(2*angle) + (2*h*g) / sin^2(angle))
    // 여기서 d는 수평 거리, h는 높이 차이, g는 중력 가속도, angle은 던지기 각도
    float Term1 = (HorizontalDistance * FMath::Abs(GravityZ)) / SinDoubleAngle;
    float Term2 = 0.0f;
    
    if (HeightDifference != 0.0f) {
        float SinAngleSquared = FMath::Sin(AngleRad) * FMath::Sin(AngleRad);
        if (SinAngleSquared > 0.01f) {
            Term2 = (2.0f * HeightDifference * FMath::Abs(GravityZ)) / SinAngleSquared;
        }
    }
    
    float RequiredVelocity = FMath::Sqrt(Term1 + Term2);
    
    // 힘 계산 - 거리에 따라 동적으로 조정
    float ForceAdjustFactor = 0.05f; // 기본 계수 크게 증가
    
    // 거리에 따른 추가 보정 (먼 거리일수록 더 강한 힘 적용)
    if (Distance > 800.0f) {
        ForceAdjustFactor *= 1.5f;
    } else if (Distance > 500.0f) {
        ForceAdjustFactor *= 1.3f;
    } else if (Distance < 300.0f) {
        ForceAdjustFactor *= 0.8f;
    }
    
    // 질량 고려 힘 계산
    OutAdjustedForce = RequiredVelocity * BallMass * ForceAdjustFactor;
    
    // 힘 제한 - 더 넓은 범위로 확장
    OutAdjustedForce = FMath::Clamp(OutAdjustedForce, 20.0f * BallMass, 400.0f * BallMass);
    
    // 디버그 로그
    UE_LOG(LogTemp, Warning, TEXT("던지기 계산: 거리=%f, 각도=%f도, 속도=%f, 힘=%f, 질량=%f"), 
        Distance, OptimalAngle, RequiredVelocity, OutAdjustedForce, BallMass);
}