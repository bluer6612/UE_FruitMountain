#include "UE_FruitMountainGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Actors/PlateActor.h"
#include "Actors/PlayerPawn.h"
#include "Actors/FruitBall.h"
#include "Interface/UI/SimpleTextureWidget.h"
#include "Interface/UI/TestWidget.h"
#include "Interface/HUD/FruitHUD.h"
#include "UE_FruitMountainGameInstance.h"
#include "Components/WidgetComponent.h"

AUE_FruitMountainGameMode::AUE_FruitMountainGameMode()
{
    // 반드시 HUD를 FruitHUD로 지정
    // HUDClass = AFruitHUD::StaticClass(); // 이 부분 주석 처리
    
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
    
    UE_LOG(LogTemp, Warning, TEXT("==== 게임 모드 BeginPlay 시작 - UI 생성 시도 ===="));
    
    // UI 생성 - SimpleTextureWidget 직접 생성
    USimpleTextureWidget::CreateSimpleUI(this);
    
    // 딜레이 후 UI 생성 다시 시도 (만약의 경우를 위해)
    FTimerHandle UITimer;
    GetWorldTimerManager().SetTimer(
        UITimer,
        [this]()
        {
            UE_LOG(LogTemp, Warning, TEXT("UI 생성 타이머 실행됨"));
            USimpleTextureWidget::CreateSimpleUI(this);
        },
        1.0f,
        false
    );

    // GameMode BeginPlay에 추가
    GetWorld()->GetGameViewport()->SetForceDisableSplitscreen(true);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("화면 디버그 메시지 테스트"));
    }

    // 사용: GameMode.cpp의 BeginPlay에서
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        UTestWidget* TestWidget = CreateWidget<UTestWidget>(PC, UTestWidget::StaticClass());
        TestWidget->AddToViewport(99999);
        PC->SetInputMode(FInputModeGameAndUI());
        PC->SetShowMouseCursor(true);
    }

    // 3D 위젯 생성 (화면에 UI가 안 보이는 경우 대체 방법)
    CreateWorldUI();
}

void AUE_FruitMountainGameMode::CreateWorldUI()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // 3D 위젯 컴포넌트 생성
    UWidgetComponent* WidgetComp = NewObject<UWidgetComponent>(this);
    WidgetComp->RegisterComponent();
    WidgetComp->SetWidgetClass(USimpleTextureWidget::StaticClass());
    WidgetComp->SetDrawSize(FVector2D(500, 500));
    WidgetComp->SetWidgetSpace(EWidgetSpace::Screen); // 화면에 고정
    WidgetComp->SetRelativeLocation(FVector(300, 0, 200)); // 카메라 앞에 위치
    
    // 위젯 생성 및 UI 설정
    WidgetComp->InitWidget();
    USimpleTextureWidget* WorldUI = Cast<USimpleTextureWidget>(WidgetComp->GetUserWidgetObject());
    if (WorldUI)
    {
        WorldUI->SetupAllImages();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("3D 월드 위젯이 생성됨"));
}

void AUE_FruitMountainGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    UE_LOG(LogTemp, Error, TEXT("==== 게임 모드 EndPlay ===="));
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