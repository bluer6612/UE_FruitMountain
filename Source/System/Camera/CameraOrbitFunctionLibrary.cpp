#include "CameraOrbitFunctionLibrary.h"
#include "Gameplay/Controller/FruitPlayerController.h"
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

void UCameraOrbitFunctionLibrary::MoveViewToFallingFruit(APlayerController* Controller, const FVector& FruitLocation, const FRotator& CameraRotation)
{
    if (!Controller || !Controller->PlayerCameraManager) return;
    
    // 접시 찾기
    FVector PlateLocation = FVector::ZeroVector;
    
    // 과일 -> 접시 방향 벡터 계산
    FVector FruitToPlateDirection = (PlateLocation - FruitLocation).GetSafeNormal();
    
    // 카메라 위치는 과일 반대쪽에서 일직선상에 배치
    FVector NewCameraLocation = FruitLocation - FruitToPlateDirection * 100.0f; //카메라-과일 거리
    
    // 높이 조정
    NewCameraLocation.Z -= 75.0f;
    
    // 카메라 회전 - 과일을 향하도록
    FRotator NewRotation = (FruitLocation - NewCameraLocation).Rotation();
    
    UE_LOG(LogTemp, Warning, TEXT("새 카메라 위치: %s, 회전: %s"), 
           *NewCameraLocation.ToString(), *NewRotation.ToString());
    
    // 카메라 뷰 조작
    AActor* DefaultCameraActor = Controller->GetViewTarget();
    if (DefaultCameraActor)
    {
        // 기존 뷰 타겟 사용
        DefaultCameraActor->SetActorLocation(NewCameraLocation);
        DefaultCameraActor->SetActorRotation(NewRotation);
        UE_LOG(LogTemp, Warning, TEXT("뷰 타겟 위치 이동: %s"), *NewCameraLocation.ToString());

        // 블렌드 설정
        FViewTargetTransitionParams TransitionParams;
        TransitionParams.BlendTime = 0.5f;

        // 뷰 타겟 변경
        Controller->SetViewTargetWithBlend(DefaultCameraActor, 0.5f,
            EViewTargetBlendFunction::VTBlend_Linear,
            0.0f, true);
    }
    
    // 플레이어 입력 비활성화
    Controller->SetIgnoreLookInput(true);
    Controller->SetIgnoreMoveInput(true);
}