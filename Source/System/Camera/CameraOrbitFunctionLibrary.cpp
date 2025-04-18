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
    if (!Controller) return;
    
    APlayerCameraManager* PlayerCameraManager = Controller->PlayerCameraManager;
    if (PlayerCameraManager)
    {
        // 1. 현재 카메라 정보 저장
        FVector CurrentCameraLocation = PlayerCameraManager->GetCameraLocation();
        FRotator CurrentCameraRotation = PlayerCameraManager->GetCameraRotation();
        
        // 2. 새로운 카메라 위치 계산 - 과일 위치로부터 약간 떨어진 지점으로
        FVector NewCameraLocation = FruitLocation;
        NewCameraLocation.X -= 100.0f;  // 약간 뒤로 이동하여 과일을 보기 좋게
        NewCameraLocation.Z += 75.0f;   // 과일 높이로 이동
        
        // 3. 카메라 뷰 직접 설정 (임시 액터 사용)
        AActor* TempViewTarget = Controller->GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), NewCameraLocation, CurrentCameraRotation);
        
        if (TempViewTarget)
        {
            // 블렌드 설정 (부드러운 전환 위해)
            FViewTargetTransitionParams TransitionParams;
            TransitionParams.BlendTime = 0.5f;
            
            // 새 타겟으로 카메라 변경
            Controller->SetViewTargetWithBlend(TempViewTarget, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
            
            // 타겟 액터는 나중에 게임오버 처리 후 자동 제거되도록 설정
            TempViewTarget->SetLifeSpan(5.0f);
            
            UE_LOG(LogTemp, Warning, TEXT("카메라를 과일 관찰 위치로 이동: %s"), *NewCameraLocation.ToString());
        }
        
        // 4. 플레이어 입력 일시적 비활성화
        Controller->SetIgnoreLookInput(true);
        Controller->SetIgnoreMoveInput(true);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerCameraManager가 없습니다!"));
    }
}