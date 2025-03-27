#include "FruitPlayerController.h"
#include "Controller/FruitInputMappingManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/InputSettings.h"
#include "Engine/Engine.h"
#include "Actors/CameraOrbitFunctionLibrary.h"
#include "FruitThrowHelper.h"
#include "GameMode/UE_FruitMountainGameMode.h"
#include "Camera/CameraComponent.h"

AFruitPlayerController::AFruitPlayerController()
{
    ThrowAngle = 45.f;
    ThrowForce = 1000.f;
    AngleStep = 5.f;

    // 오빗 기본 값 설정
    CameraOrbitAngle = 0.f;
    CameraOrbitRadius = 700.f;
    
    // 카메라 회전 속도 (도/초)
    RotateCameraSpeed = 180.f;
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
    
    UE_LOG(LogTemp, Log, TEXT("AFruitPlayerController::BeginPlay 호출됨"));

    // 입력 모드를 게임 전용으로 설정하여 키 입력이 제대로 전달되는지 확인
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;
    
    UFruitInputMappingManager::ConfigureKeyMappings();

    // 접시 액터를 검색하여 회전 기준 위치로 사용 (접시가 있을 경우)
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        PlateLocation = PlateActors[0]->GetActorLocation();
    }
    else
    {
        PlateLocation = FVector::ZeroVector;
        UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다. 기본 위치 (0,0,0) 사용."));
    }
    
    // Pawn이 확실히 할당된 후 카메라 위치를 업데이트하여 접시를 바라보게 함
    // (다음 틱에서 호출)
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius, 30.f);
    });

    // 미리보기 공 생성
    CurrentBallType = FMath::RandRange(1, 11);
    UpdatePreviewBall();
}

void AFruitPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
        InputComponent->BindAction("IncreaseAngle", IE_Pressed, this, &AFruitPlayerController::IncreaseAngle);
        InputComponent->BindAction("DecreaseAngle", IE_Pressed, this, &AFruitPlayerController::DecreaseAngle);
        InputComponent->BindAction("ThrowFruit", IE_Pressed, this, &AFruitPlayerController::ThrowFruit);
        // 기존 RotateCameraLeft/Right 액션 대신 Axis 매핑 사용 (키를 누른 채로 부드럽게 회전)
        InputComponent->BindAxis("RotateCamera", this, &AFruitPlayerController::RotateCamera);
        UE_LOG(LogTemp, Log, TEXT("입력 바인딩 완료"));
    }
}

void AFruitPlayerController::IncreaseAngle()
{
    ThrowAngle += AngleStep;
    UE_LOG(LogTemp, Log, TEXT("각도 증가: 현재 각도 %f"), ThrowAngle);
}

void AFruitPlayerController::DecreaseAngle()
{
    ThrowAngle -= AngleStep;
    UE_LOG(LogTemp, Log, TEXT("각도 감소: 현재 각도 %f"), ThrowAngle);
}

void AFruitPlayerController::ThrowFruit()
{
    UFruitThrowHelper::ThrowFruit(this);
}

void AFruitPlayerController::HandleThrow()
{
    // 모든 코드가 FruitThrowHelper로 이동했으므로 이 함수는 비워둡니다.
    // 다른 코드에서 이 함수를 직접 호출하는 곳이 있다면 
    // UFruitThrowHelper::ThrowFruit(this); 로 리다이렉트할 수 있습니다.
}

void AFruitPlayerController::RotateCamera(float Value)
{
    if (FMath::Abs(Value) > KINDA_SMALL_NUMBER)
    {
        // 프레임 시간과 회전 속도를 곱해 부드러운 변화 적용
        float DeltaAngle = Value * RotateCameraSpeed * GetWorld()->DeltaTimeSeconds;
        CameraOrbitAngle += DeltaAngle;
        
        if(CameraOrbitAngle >= 360.f)
        {
            CameraOrbitAngle -= 360.f;
        }
        else if(CameraOrbitAngle < 0.f)
        {
            CameraOrbitAngle += 360.f;
        }

        UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius, 30.f);
        
        // 카메라 회전 후 미리보기 공도 함께 업데이트
        UpdatePreviewBall();
    }
}

void AFruitPlayerController::UpdatePreviewBall()
{
    // 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("UpdatePreviewBall 호출됨"));

    // 이전 미리보기 공 제거
    if (PreviewBall)
    {
        PreviewBall->Destroy();
        PreviewBall = nullptr;
    }
    
    if (!FruitBallClass)
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 실패: FruitBallClass가 NULL입니다!"));
        return;
    }
    
    // 플레이어 카메라 기준 위치 계산
    APawn* PlayerPawn = GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 실패: PlayerPawn이 NULL입니다!"));
        return;
    }
    
    // 더 간단한 방식으로 미리보기 공 배치 - 카메라 위치 앞쪽으로 고정 거리
    FVector CameraLocation = PlayerPawn->GetActorLocation();
    FRotator CameraRotation = PlayerPawn->GetActorRotation();
    
    // 카메라 앞쪽으로 더 멀리 배치 (100→200)
    FVector PreviewLocation = CameraLocation + CameraRotation.Vector() * 200.f;
    
    // 디버그 스피어로 위치 시각화 (크기 키움)
    DrawDebugSphere(GetWorld(), PreviewLocation, 100.f, 12, FColor::Red, false, 3.0f);
    UE_LOG(LogTemp, Warning, TEXT("미리보기 생성 위치: %s"), *PreviewLocation.ToString());
    
    // 미리보기 공 생성
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = this;
    
    PreviewBall = GetWorld()->SpawnActor<AActor>(FruitBallClass, PreviewLocation, FRotator::ZeroRotator, SpawnParams);
    if (PreviewBall)
    {
        UE_LOG(LogTemp, Warning, TEXT("미리보기 공 생성 성공!"));
        
        float BaseSize = 2.5f; // 기존 5.f에서 2배 증가
        float ScaleFactor = 1.f + 0.05f * (CurrentBallType - 1);
        float BallSize = BaseSize * ScaleFactor;
        PreviewBall->SetActorScale3D(FVector(BallSize));
        
        // 미리보기 공은 물리 비활성화
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(PreviewBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
        if (PrimComp)
        {
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        
        // 부착 시도를 별도로 수행
        UCameraComponent* CameraComp = PlayerPawn->FindComponentByClass<UCameraComponent>();
        if (CameraComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("카메라 컴포넌트 찾음, 부착 시도"));
            // 방법 1: 월드 기준 위치 유지하며 부착
            PreviewBall->AttachToComponent(CameraComp, FAttachmentTransformRules::KeepWorldTransform);
            
            // 방법 2: 카메라 기준 고정 위치에 배치
            // PreviewBall->SetActorRelativeLocation 대신 아래 사용
            // PreviewBall->SetRelativeLocation(FVector(200.f, 0.f, -50.f));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("카메라 컴포넌트를 찾을 수 없습니다!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 공 생성에 실패했습니다!"));
    }
}