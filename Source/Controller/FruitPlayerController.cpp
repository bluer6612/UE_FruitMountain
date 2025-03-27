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

AFruitPlayerController::AFruitPlayerController()
{
    ThrowAngle = 45.f;
    ThrowForce = 1000.f;
    AngleStep = 5.f;

    // 오빗 기본 값 설정
    CameraOrbitAngle = 0.f;
    CameraOrbitRadius = 500.f;
    
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
    
    UE_LOG(LogTemp, Log, TEXT("키 매핑 설정 시작"));
    UFruitInputMappingManager::ConfigureKeyMappings();
    UE_LOG(LogTemp, Log, TEXT("키 매핑 설정 완료"));

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
    // 접시 액터 위치 검색 (스폰 기준 위치)
    FVector LocalSpawnLocation = FVector::ZeroVector;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        // Plate 위치에서 위로 100만큼 올려서 스폰
        LocalSpawnLocation = PlateActors[0]->GetActorLocation() + FVector(0, 0, 100.f);
        UE_LOG(LogTemp, Log, TEXT("접시 액터 발견: 위치 (%f, %f, %f)"),
               LocalSpawnLocation.X, LocalSpawnLocation.Y, LocalSpawnLocation.Z);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다. 기본 위치 (0,0,0)에서 스폰합니다."));
    }

    // FruitBallClass가 설정되어 있으면 공 액터 스폰 시도
    if (FruitBallClass)
    {
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FActorSpawnParameters SpawnParams;
        // 스폰 충돌 관련 설정: 항상 스폰
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AActor* SpawnedBall = GetWorld()->SpawnActor<AActor>(FruitBallClass, LocalSpawnLocation, SpawnRotation, SpawnParams);
        if (SpawnedBall)
        {
            // 임의의 공 타입(1~11)으로 크기 조절
            int32 BallType = FMath::RandRange(1, 11);
            float BaseSize = 25.f;
            float ScaleFactor = 1.f + 0.1f * (BallType - 1);
            float BallSize = BaseSize * ScaleFactor;
            SpawnedBall->SetActorScale3D(FVector(BallSize));
            
            // 물리 시뮬레이션 중이면 ThrowAngle에 따른 힘 부여
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                float RadAngle = FMath::DegreesToRadians(ThrowAngle);
                FVector ImpulseDirection = FVector(FMath::Cos(RadAngle), 0.f, FMath::Sin(RadAngle)).GetSafeNormal();
                PrimComp->AddImpulse(ImpulseDirection * ThrowForce, NAME_None, true);
            }
            UE_LOG(LogTemp, Log, TEXT("공 타입 %d 스폰됨, 크기: %f"), BallType, BallSize);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("공 액터 생성에 실패했습니다."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FruitBallClass가 설정되어 있지 않습니다."));
    }
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
        UE_LOG(LogTemp, Log, TEXT("카메라 회전 (Axis): 현재 각도: %f"), CameraOrbitAngle);
    }
}