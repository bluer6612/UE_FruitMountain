#include "UE_FruitMountainGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Actors/PlateActor.h"
#include "Actors/PlayerPawn.h"
#include "Actors/FruitBall.h"
#include "Interface/UI/FruitUIManager.h"
#include "UE_FruitMountainGameInstance.h"
#include "FruitHUD.h"

AUE_FruitMountainGameMode::AUE_FruitMountainGameMode()
{
    // 반드시 HUD를 FruitHUD로 지정
    HUDClass = AFruitHUD::StaticClass();
    
    // 기본 플레이어 컨트롤러를 AFruitPlayerController로 명시적으로 설정
    PlayerControllerClass = AFruitPlayerController::StaticClass();

    // DefaultPawnClass 지정 (적절한 Pawn 클래스로 교체)
    DefaultPawnClass = APlayerPawn::StaticClass();

    // Blueprint 없이 코드로 만든 PlateActor를 기본값으로 할당
    PlateClass = APlateActor::StaticClass();

    FruitBallClass = AFruitBall::StaticClass();

    UE_LOG(LogTemp, Log, TEXT("AUE_FruitMountainGameMode 생성자 호출됨 - 기본 컨트롤러와 HUD가 설정되었습니다."));
}

void AUE_FruitMountainGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Error, TEXT("==== 게임 모드 BeginPlay 시작 ===="));
    
    // 게임 인스턴스를 통해 UI 생성 (먼저 실행)
    UUE_FruitMountainGameInstance* GameInstance = Cast<UUE_FruitMountainGameInstance>(GetGameInstance());
    if (GameInstance)
    {
        // UI 위젯 먼저 생성
        GameInstance->ShowFruitUIWidget();
        
        // 약간의 딜레이 후 확인 타이머 시작
        FTimerHandle UICheckTimer;
        GetWorldTimerManager().SetTimer(
            UICheckTimer,
            [GameInstance]()
            {
                GameInstance->CheckPersistentUI();
            },
            1.0f,  // 1초 후 시작
            false
        );
    }
}

void AUE_FruitMountainGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    UE_LOG(LogTemp, Error, TEXT("==== 게임 모드 EndPlay - 위젯 메모리 정리 ===="));
    
    // 게임 종료 시 위젯 정리
    UUE_FruitMountainGameInstance* GameInstance = Cast<UUE_FruitMountainGameInstance>(GetGameInstance());
    if (GameInstance && GameInstance->PersistentUIWidget)
    {
        // 루트 세트에서 제거
        if (GameInstance->PersistentUIWidget->IsRooted())
        {
            GameInstance->PersistentUIWidget->RemoveFromRoot();
            UE_LOG(LogTemp, Error, TEXT("위젯을 루트 세트에서 제거함"));
        }
    }
}

void AUE_FruitMountainGameMode::StartPlay()
{
    Super::StartPlay();

    // 레벨에 "Plate" 태그가 부여된 액터가 있는지 확인
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() == 0)
    {
        if (PlateClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            // 접시 액터의 위치는 원하는 좌표로 수정 가능 (여기서는 원점 사용)
            FVector PlateLocation = FVector::ZeroVector;
            FRotator PlateRotation = FRotator::ZeroRotator;
            AActor* NewPlate = GetWorld()->SpawnActor<AActor>(PlateClass, PlateLocation, PlateRotation, SpawnParams);
            if (NewPlate)
            {
                // 스폰된 액터에 "Plate" 태그 추가
                NewPlate->Tags.Add(FName("Plate"));
                UE_LOG(LogTemp, Log, TEXT("접시 액터가 생성되었습니다."));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("접시 액터 생성에 실패했습니다."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PlateClass가 설정되어 있지 않습니다."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("이미 접시 액터가 존재합니다."));
    }

    // 게임 시작 시점에 UI가 여전히 존재하는지 확인
    UUE_FruitMountainGameInstance* GameInstance = Cast<UUE_FruitMountainGameInstance>(GetGameInstance());
    if (GameInstance && GameInstance->PersistentUIWidget)
    {
        if (!GameInstance->PersistentUIWidget->IsInViewport())
        {
            UE_LOG(LogTemp, Error, TEXT("StartPlay에서 UI가 사라짐을 감지 - 다시 표시"));
            GameInstance->PersistentUIWidget->AddToViewport(10000);
        }
    }
}

void AUE_FruitMountainGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // 매 프레임마다 업데이트되어야 하는 게임 로직을 구현합니다.
}