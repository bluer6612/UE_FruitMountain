#include "UE_FruitMountainGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "GameFramework/Actor.h"
#include "Actors/PlateActor.h"
#include "Actors/PlayerPawn.h"
#include "Actors/FruitBall.h"
#include "Interface/HUD/FruitHUD.h"
#include "Interface/UI/TextureDisplayWidget.h"
#include "Gameplay/Physics/FruitTrajectoryHelper.h"
#include "Gameplay/Fruit/FruitMergeHelper.h"
#include "Logging/LogMacros.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

AUE_FruitMountainGameMode::AUE_FruitMountainGameMode()
{
    // FruitHUD 명시적 설정
    HUDClass = AFruitHUD::StaticClass();
    
    // PlayerController 설정 확인
    PlayerControllerClass = AFruitPlayerController::StaticClass();

    // DefaultPawnClass 지정 (적절한 Pawn 클래스로 교체)
    DefaultPawnClass = APlayerPawn::StaticClass();

    // Blueprint 없이 코드로 만든 PlateActor를 기본값으로 할당
    PlateClass = APlateActor::StaticClass();

    FruitBallClass = AFruitBall::StaticClass();
    
    UE_LOG(LogTemp, Log, TEXT("AUE_FruitMountainGameMode 생성자 호출됨"));
}

void AUE_FruitMountainGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 게임 시작 시 파티클 에셋 미리 로드
    UFruitMergeHelper::PreloadAssets(GetWorld());
    
    UTextureDisplayWidget::CreateDisplayWidget(this);
}

void AUE_FruitMountainGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 종료 시점에 정리
    UFruitTrajectoryHelper::ResetTrajectorySystem();
    
    Super::EndPlay(EndPlayReason);
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