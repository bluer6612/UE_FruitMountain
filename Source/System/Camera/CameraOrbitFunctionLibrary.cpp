#include "CameraOrbitFunctionLibrary.h"
#include "Gameplay/FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

void UCameraOrbitFunctionLibrary::UpdateCameraOrbit(APawn* ControlledPawn, const FVector& PlateLocation, float OrbitAngle, float OrbitRadius)
{
    if (!ControlledPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateCameraOrbit: 소유 중인 Pawn이 없습니다."));
        return;
    }
    
    // 현재 Pawn의 높이(Z)를 유지하기 위해 현재 위치 저장
    float CurrentHeight = ControlledPawn->GetActorLocation().Z;
    
    // OrbitAngle을 라디안으로 변환 (X,Y 평면 회전에만 사용)
    float OrbitRad = FMath::DegreesToRadians(OrbitAngle);
    
    // 오빗 위치 계산 (X,Y만 변경)
    FVector Offset;
    Offset.X = OrbitRadius * FMath::Cos(OrbitRad);
    Offset.Y = OrbitRadius * FMath::Sin(OrbitRad);
    Offset.Z = 0.0f; // Z 오프셋은 0으로 설정
    
    // Pawn의 새 위치 계산
    FVector NewLocation = PlateLocation + Offset;
    NewLocation.Z = CurrentHeight; // 원래 높이 유지
    
    // 새 위치 적용
    ControlledPawn->SetActorLocation(NewLocation);

    // 접시를 바라보도록 회전 계산 (피치만 조정하고 기존 롤 유지)
    FRotator CurrentRotation = ControlledPawn->GetActorRotation();
    FRotator LookAtRotation = (PlateLocation - NewLocation).Rotation();
    
    // 요 각도만 변경
    FRotator NewRotation = FRotator(CurrentRotation.Pitch, LookAtRotation.Yaw, CurrentRotation.Roll);
    ControlledPawn->SetActorRotation(NewRotation);
}