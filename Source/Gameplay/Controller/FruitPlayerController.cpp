#include "FruitPlayerController.h"
#include "System/Input/FruitInputMappingManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "System/Camera/CameraOrbitFunctionLibrary.h"
#include "Framework/UE_FruitMountainGameMode.h"
#include "Gameplay/Physics/FruitThrowHelper.h"
#include "Gameplay/Physics/FruitTrajectoryHelper.h"
#include "Gameplay/Physics/FruitPhysicsHelper.h"
#include "Actors/FruitBall.h"
#include "Interface/HUD/FruitHUD.h"

AFruitPlayerController::AFruitPlayerController()
{
    ThrowAngle = 45.f;

    CameraOrbitRadius = 110.f;

    CurrentBallType = 1;
}

void AFruitPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // GameMode를 캐스팅하여 FruitBallClass 값을 가져옴
    AUE_FruitMountainGameMode* GM = Cast<AUE_FruitMountainGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GM && GM->FruitBallClass)
    {
        FruitBallClass = GM->FruitBallClass;
        UE_LOG(LogTemp, Log, TEXT("PlayerController 쪽 FruitBallClass를 GameMode 쪽 값으로 설정했습니다."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode의 FruitBallClass가 비어 있습니다."));
    }

    // 타이머를 사용하여 약간의 지연 후 접시 액터 검색 (타이밍 문제 해결)
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        // 접시 액터를 검색하여 회전 기준 위치로 사용
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
        if (PlateActors.Num() > 0)
        {
            // 접시 경계 구하기 (중심점 정확히 계산)
            FVector PlateOrigin;
            FVector PlateExtent;
            PlateActors[0]->GetActorBounds(false, PlateOrigin, PlateExtent);
            PlateOrigin.Z += 10.0f; // 접시 표면 위로 약간 올림
            
            // 직접 중심점 설정 (오프셋 없이)
            PlateLocation = PlateOrigin;
            
            UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾았습니다: %s"), *PlateLocation.ToString());
            
            // 카메라 위치 업데이트
            UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius);
        }
        else
        {
            PlateLocation = FVector::ZeroVector;
            UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다."));
            return;
        }
    });

    // 미리보기 공 생성 시 회전까지 적용되도록 true 매개변수 추가
    CurrentBallType = FMath::RandRange(1, AFruitBall::RandomBallTypeMax);
    UpdatePreviewBallWithDebounce();

    // 입력 매핑 설정
    UFruitInputMappingManager::ConfigureKeyMappings();
    SetInputMode(FInputModeGameAndUI());
    SetShowMouseCursor(true);
    
    // 추가된 콘솔 명령 실행
    GetLocalPlayer()->ViewportClient->ConsoleCommand(TEXT("r.TranslucentSortPolicy 0"));
    GetLocalPlayer()->ViewportClient->ConsoleCommand(TEXT("r.AllowOcclusionQueries 0"));
    
    UE_LOG(LogTemp, Log, TEXT("AFruitPlayerController::BeginPlay 호출 완료됨"));
}

void AFruitPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
        InputComponent->BindAxis("AdjustAngle", this, &AFruitPlayerController::AdjustAngle);
        InputComponent->BindAxis("RotateCamera", this, &AFruitPlayerController::RotateCamera);
        InputComponent->BindAction("ThrowFruit", IE_Pressed, this, &AFruitPlayerController::ThrowFruit);
        UE_LOG(LogTemp, Log, TEXT("입력 바인딩 완료"));
    }
}

void AFruitPlayerController::GameOver()
{
    // 이미 게임오버 상태면 중복 처리 방지
    if (bIsGameOver) return;
    
    bIsGameOver = true;
    
    // 플레이어 입력 비활성화
    DisableInput(this);
    
    // 게임 일시정지 (선택사항)
    // SetPause(true);
    
    // 게임오버 UI 표시 (UI 클래스가 있다면)
    UE_LOG(LogTemp, Warning, TEXT("게임 오버! 과일이 접시 밖으로 떨어졌습니다."));
    
    // 게임오버 UI 표시 코드
    // 헤드업 디스플레이에 표시하거나 별도 위젯 생성
    AFruitHUD* FruitHUD = Cast<AFruitHUD>(GetHUD());
    if (FruitHUD)
    {
        // 예시: FruitHUD->ShowGameOverScreen();
    }
    
    // 새 게임 시작하는 키 바인딩 또는 타이머로 재시작
    FTimerHandle RestartTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        RestartTimerHandle,
        [this]()
        {
            // 현재 레벨 재시작
            //UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()));
        },
        3.0f,
        false
    );
}

// 과일 던지기 함수 수정
void AFruitPlayerController::ThrowFruit()
{
    // 이미 던지는 중이면 무시 - static 변수 대신 멤버 변수 사용
    if (bIsThrowingInProgress)
        return;
    
    // 던지기 시작 표시
    bIsThrowingInProgress = true;
    
    // 미리보기 공 숨기기 - 제거하되 즉시 실제 공 생성
    if (PreviewBall)
    {
        PreviewBall->Destroy();
        PreviewBall = nullptr;
    }
    
    // 입력 비활성화 - 던지는 동안 모든 조작 막기
    DisableInput(this);
    
    // 즉시 공 생성하여 던지기 실행
    UFruitThrowHelper::ThrowFruit(this);
    
    // 일정 딜레이 후 입력 다시 활성화 및 새 미리보기 공 생성
    FTimerHandle ThrowDelayTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        ThrowDelayTimerHandle,
        [this]()
        {
            // 던지기 완료 표시
            bIsThrowingInProgress = false;
            
            // 입력 다시 활성화
            EnableInput(this);
            
            // 새로운 미리보기 공 업데이트 (공 타입 바꾸기)
            CurrentBallType = FMath::RandRange(1, AFruitBall::RandomBallTypeMax); // 다음에 던질 공 타입 랜덤 변경
            UpdatePreviewBallWithDebounce();
        },
        BallThrowDelay,
        false // 반복 실행 안 함
    );
}

// 새로운 각도 조정 함수 (축 매핑용)
void AFruitPlayerController::AdjustAngle(float Value)
{
    if (FMath::Abs(Value) < KINDA_SMALL_NUMBER)
        return;
    
    // 프레임 시간과 조정 속도를 곱해 부드러운 변화 적용
    float DeltaAngle = Value * AngleAdjustSpeed * GetWorld()->DeltaTimeSeconds;
    
    // 변경 전에 최종 각도가 제한 범위를 벗어나는지 확인 (한 번의 if문으로 처리)
    float NewAngle = ThrowAngle + DeltaAngle;
    
    // 변경 전에 제한 검사 - 범위를 벗어나면 작업 수행하지 않음
    if ((NewAngle > UFruitPhysicsHelper::MaxThrowAngle && DeltaAngle > 0.0f) || 
        (NewAngle < UFruitPhysicsHelper::MinThrowAngle && DeltaAngle < 0.0f))
    {
        return;
    }
    
    // 허용 범위 내의 변경이면 실행
    ThrowAngle = NewAngle;
    
    // 안전을 위한 클램핑 (불필요하지만 추가 보호)
    ThrowAngle = FMath::Clamp(ThrowAngle, UFruitPhysicsHelper::MinThrowAngle, UFruitPhysicsHelper::MaxThrowAngle);
    
    // 각도 변경 후 미리보기 공 및 궤적 업데이트
    UpdatePreviewBallWithDebounce();
}

// 카메라 회전 처리 함수 - 과일 회전 보정 수정
void AFruitPlayerController::RotateCamera(float Value)
{
    if (FMath::IsNearlyZero(Value))
        return;
    
    // 회전 속도 적용
    float DeltaAngle = Value * RotateCameraSpeed * GetWorld()->GetDeltaSeconds();
    
    // 각도 업데이트
    CameraOrbitAngle += DeltaAngle;
    
    // 360도 범위 내로 제한
    CameraOrbitAngle = FMath::Fmod(CameraOrbitAngle, 360.0f);
    if (CameraOrbitAngle < 0.0f)
        CameraOrbitAngle += 360.0f;
    
    // 카메라 위치 업데이트
    UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius);
    
    // 위치만 업데이트하고 회전은 별도로 처리
    UFruitThrowHelper::UpdatePreviewBall(this, false);
    
    // 과일이 항상 카메라를 바라보도록 회전 조정
    if (PreviewBall)
    {
        SetFruitRotation(PreviewBall);
    }
}

// 실제 업데이트 수행 함수 (무한 루프 방지 용으로 이중으로 거침)
void AFruitPlayerController::ExecutePreviewBallUpdate()
{
    bPreviewBallUpdatePending = false;
    
    // 공 위치 업데이트 (회전은 별도 처리)
    UFruitThrowHelper::UpdatePreviewBall(this, false);
    
    // 과일 각도 업데이트
    if (PreviewBall)
    {
        SetFruitRotation(PreviewBall);
    }
}

// 미리보기 공 업데이트 함수 - 연속 호출 방지 (디바운싱) 처리
void AFruitPlayerController::UpdatePreviewBallWithDebounce()
{
    // 이미 업데이트가 예약되어 있으면 중복 실행하지 않음
    if (!bPreviewBallUpdatePending)
    {
        bPreviewBallUpdatePending = true;
        
        // 타이머 설정: 일정 시간 후에만 업데이트 실행
        GetWorld()->GetTimerManager().SetTimer(
            PreviewBallUpdateTimerHandle,
            this,
            &AFruitPlayerController::ExecutePreviewBallUpdate,
            PreviewBallUpdateDelay,
            false // 반복 실행 안 함
        );
    }
}

// 과일 회전 설정 함수 구현 - 항상 카메라 각도 고려
void AFruitPlayerController::SetFruitRotation(AActor* Fruit)
{
    if (!Fruit)
    {
        return;
    }
    
    // 던지기 각도에 따른 피치 회전 계산
    float PitchAngle = ThrowAngle - 30.0f; // 30도라는 적절한 각도값 찾아 놓았음
    
    // 요(Yaw) 회전 - 항상 카메라가 바라보는 방향을 보도록 조정
    float YawAngle = CameraOrbitAngle - 180.0f;
    
    // 360도 범위 내로 제한
    YawAngle = FMath::Fmod(YawAngle, 360.0f);
    if (YawAngle < 0.0f)
        YawAngle += 360.0f;
    
    // 새로운 회전 설정
    FRotator NewRotation = FRotator(PitchAngle, YawAngle, 0.0f);
    Fruit->SetActorRotation(NewRotation);
}

void AFruitPlayerController::MoveViewToFallingFruit(const FVector& FruitLocation, const FRotator& CameraRotation)
{
    if (PlayerCameraManager)
    {
        // 현재 카메라 위치에서 과일을 바라보도록 회전만 계산
        FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
        FVector DirectionToFruit = FruitLocation - CameraLocation;
        FRotator NewRotation = DirectionToFruit.Rotation();
        
        // 카메라 회전만 설정 (위치는 변경하지 않음)
        // 부드러운 전환을 위한 블렌드 설정
        PlayerCameraManager->SetManualCameraFade(0.0f, FLinearColor::Black, false);
        PlayerCameraManager->SetViewTarget(this, FViewTargetTransitionParams());
        
        // 새 시점으로 조정 (0.5초 동안 블렌드)
        FViewTargetTransitionParams TransitionParams;
        TransitionParams.BlendTime = 0.5f;
        PlayerCameraManager->LockCameraView(NewRotation);
        
        // 플레이어 입력 일시적 비활성화
        SetIgnoreLookInput(true);
        SetIgnoreMoveInput(true);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerCameraManager가 없습니다!"));
    }
}