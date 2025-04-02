#include "UE_FruitMountainGameMode.h"
#include "Throw/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"  // 액터 검색 및 스폰을 위한 include
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Actors/PlateActor.h"
#include "Actors/PlayerPawn.h"
#include "Actors/FruitBall.h"
#include "UI/FruitUIManager.h"
#include "UE_FruitMountainGameInstance.h"

AUE_FruitMountainGameMode::AUE_FruitMountainGameMode()
{
    // 기본 플레이어 컨트롤러를 AFruitPlayerController로 명시적으로 설정
    PlayerControllerClass = AFruitPlayerController::StaticClass();

    // DefaultPawnClass 지정 (적절한 Pawn 클래스로 교체)
    DefaultPawnClass = APlayerPawn::StaticClass();

    // Blueprint 없이 코드로 만든 PlateActor를 기본값으로 할당
    PlateClass = APlateActor::StaticClass();

    FruitBallClass = AFruitBall::StaticClass();

    UE_LOG(LogTemp, Log, TEXT("AUE_FruitMountainGameMode 생성자 호출됨 - 기본 컨트롤러가 명시적으로 설정되었습니다."));
}

void AUE_FruitMountainGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Error, TEXT("==== 게임 모드 BeginPlay 시작 ===="));
    
    // UI 매니저 초기화 - 테스트 UI와 함께 Fruit UI도 표시
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        // 테스트 UI 먼저 생성 (이게 잘 작동함을 확인)
        UUE_FruitMountainGameInstance* GameInstance = 
            Cast<UUE_FruitMountainGameInstance>(GetGameInstance());
        if (GameInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("테스트 UI 생성 시작"));
            GameInstance->TestCreateSimpleUI();
            UE_LOG(LogTemp, Error, TEXT("테스트 UI 생성 완료"));
        }
        
        // FruitUIManager 초기화를 명시적으로 수행
        UE_LOG(LogTemp, Error, TEXT("FruitUIManager 초기화 시작"));
        UFruitUIManager* UIManager = UFruitUIManager::GetInstance();
        UIManager->Initialize(PC);
        
        // 초기화 후 추가 확인 및 위젯 표시 강제
        FTimerHandle FruitUITimer;
        GetWorld()->GetTimerManager().SetTimer(
            FruitUITimer,
            [UIManager]()
            {
                UE_LOG(LogTemp, Error, TEXT("FruitUI 위젯 상태 확인 타이머"));
                
                // 위젯이 제대로 생성되었는지 확인
                if (UIManager->GetWidgetCount() > 0)
                {
                    UE_LOG(LogTemp, Error, TEXT("FruitUI 위젯 %d개 감지됨"), UIManager->GetWidgetCount());
                    
                    // 모든 위젯의 가시성 강제 설정
                    UIManager->SetAllWidgetsVisibility(true);
                    
                    // 기본 이미지 다시 로드
                    UIManager->LoadDefaultImages();
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("FruitUI 위젯이 없음 - 다시 생성"));
                    // 위젯이 없으면 다시 생성
                    APlayerController* PC = UGameplayStatics::GetPlayerController(UIManager->GetWorld(), 0);
                    if (PC)
                    {
                        UIManager->Initialize(PC);
                    }
                }
            },
            2.0f,
            false
        );
    }
    
    UE_LOG(LogTemp, Error, TEXT("==== 게임 모드 BeginPlay 완료 ===="));
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
}

void AUE_FruitMountainGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // 매 프레임마다 업데이트되어야 하는 게임 로직을 구현합니다.
}