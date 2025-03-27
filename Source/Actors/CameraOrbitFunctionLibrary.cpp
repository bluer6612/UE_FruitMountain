#include "CameraOrbitFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

void UCameraOrbitFunctionLibrary::UpdateCameraOrbit(APawn* ControlledPawn, const FVector& PlateLocation, float OrbitAngle, float OrbitRadius, float ElevationAngle)
{
    if (!ControlledPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateCameraOrbit: 소유 중인 Pawn이 없습니다."));
        return;
    }
    
    // ElevationAngle(높이 각도)와 OrbitAngle을 라디안으로 변환
    float ElevationRad = FMath::DegreesToRadians(ElevationAngle);
    float OrbitRad = FMath::DegreesToRadians(OrbitAngle);
    
    // 오빗 위치 계산
    FVector Offset;
    Offset.X = OrbitRadius * FMath::Cos(ElevationRad) * FMath::Cos(OrbitRad);
    Offset.Y = OrbitRadius * FMath::Cos(ElevationRad) * FMath::Sin(OrbitRad);
    Offset.Z = OrbitRadius * FMath::Sin(ElevationRad);
    
    // Pawn의 새 위치 = 접시 위치 + 계산된 오프셋
    FVector NewLocation = PlateLocation + Offset;
    ControlledPawn->SetActorLocation(NewLocation);

    // 접시를 바라보도록 회전 계산
    FRotator NewRotation = (PlateLocation - NewLocation).Rotation();
    ControlledPawn->SetActorRotation(NewRotation);

    UE_LOG(LogTemp, Log, TEXT("카메라 위치 업데이트 (라이브러리): 위치=(%f, %f, %f), 각도=(%f, %f, %f)"),
           NewLocation.X, NewLocation.Y, NewLocation.Z,
           NewRotation.Pitch, NewRotation.Yaw, NewRotation.Roll);
}