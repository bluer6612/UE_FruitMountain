#include "FruitPhysicsHelper.h"
#include "FruitPlayerController.h"

// 던지기 파라미터 계산 함수 - 안정적인 힘 계산으로 수정
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
    
    // Controller의 ThrowAngle 값 사용 (20~70도 범위 내에서 제한)
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
    
    // --- 단순화된 힘 계산 (거리 기반) ---
    
    // 단순한 선형 관계로 힘 계산 (거리에 비례하는 힘)
    float BaseForce = 20.0f; // 매우 낮은 기본 힘
    float DistanceFactor = FMath::Clamp(Distance / 300.0f, 0.5f, 2.0f);
    
    // 질량 반영 힘 계산 - 단순하고 안정적인 공식 사용
    OutAdjustedForce = BaseForce * DistanceFactor * BallMass;
    
    // 최종 힘 제한 - 범위 축소
    OutAdjustedForce = FMath::Clamp(OutAdjustedForce, 20.0f * BallMass, 100.0f * BallMass);
    
    // 디버그 로그
    UE_LOG(LogTemp, Warning, TEXT("던지기 계산: 거리=%f, 각도=%f도, 힘=%f, 질량=%f"), 
        Distance, OptimalAngle, OutAdjustedForce, BallMass);
}