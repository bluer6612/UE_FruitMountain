#include "UE_FruitMountainGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "GameFramework/Actor.h"
#include "Actors/PlateActor.h"
#include "Actors/PlayerPawn.h"
#include "Actors/FruitBall.h"
#include "Interface/HUD/FruitHUD.h"
#include "Interface/UI/TextureDisplayWidget.h"
#include "Interface/UI/ScoreDisplayWidget.h"
#include "Interface/UI/TotalScoreWidget.h"
#include "Gameplay/Physics/FruitTrajectoryHelper.h"
#include "Gameplay/Merging/FruitMergeHelper.h"
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
    
    // 게임 시작 시 모든 과일 메시 사전 로드
    UFruitMergeHelper::PreloadAllFruitMeshes(GetWorld());
    
    // UI 위젯 초기화
    UTextureDisplayWidget* TextureWidget = UTextureDisplayWidget::CreateDisplayWidget(this);
    UScoreDisplayWidget* ScoreWidget = UScoreDisplayWidget::CreateScoreWidget(this);
    UTotalScoreWidget* TotalScoreWidget = UTotalScoreWidget::CreateTotalScoreWidget(this);

    // ScoreManager 컴포넌트 가져오기 또는 생성
    UScoreManagerComponent* ScoreManagerComp = FindComponentByClass<UScoreManagerComponent>();
    if (!ScoreManagerComp)
    {
        ScoreManagerComp = NewObject<UScoreManagerComponent>(this, UScoreManagerComponent::StaticClass());
        ScoreManagerComp->RegisterComponent();
    }
    
    // 위젯 연결
    if (ScoreManagerComp)
    {
        ScoreManagerComp->ScoreWidgetInstance = ScoreWidget;
        ScoreManagerComp->TotalScoreWidgetInstance = TotalScoreWidget;
        ScoreManagerComp->bWidgetCreated = true;
    }
}

void AUE_FruitMountainGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // 모든 UI 위젯 정리 (기존 코드를 확장)
    
    // 1. ScoreDisplayWidget 정리 
    if (UScoreDisplayWidget::IsInstanceValid())
    {
        UScoreDisplayWidget* ScoreWidget = UScoreDisplayWidget::GetInstance();
        if (ScoreWidget)
        {
            ScoreWidget->RemoveFromParent();
            UScoreDisplayWidget::ClearInstance();
        }
    }
    
    // 2. TextureDisplayWidget 정리
    if (UTextureDisplayWidget::IsInstanceValid())
    {
        UTextureDisplayWidget* TextureWidget = UTextureDisplayWidget::GetInstance();
        if (TextureWidget)
        {
            TextureWidget->RemoveFromParent();
            UTextureDisplayWidget::ClearInstance();
        }
    }
    
    // 3. TotalScoreWidget 정리
    if (UTotalScoreWidget::IsInstanceValid())
    {
        UTotalScoreWidget* TotalScoreWidget = UTotalScoreWidget::GetInstance();
        if (TotalScoreWidget)
        {
            TotalScoreWidget->RemoveFromParent();
            UTotalScoreWidget::ClearInstance();
        }
    }
    
    // 4. 스코어 매니저 참조 해제
    UScoreManagerComponent* ScoreManagerComp = FindComponentByClass<UScoreManagerComponent>();
    if (ScoreManagerComp)
    {
        // 위젯 참조 해제
        ScoreManagerComp->ScoreWidgetInstance = nullptr;
        ScoreManagerComp->TotalScoreWidgetInstance = nullptr;
        ScoreManagerComp->bWidgetCreated = false;
    }
    
    // 5. 배처 및 시스템 정리
    UFruitTrajectoryHelper::ResetTrajectorySystem();
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