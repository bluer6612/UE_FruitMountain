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
    
    // UI 매니저 초기화
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    UFruitUIManager::GetInstance()->Initialize(PC);
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