#include "PlayGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"
#include "System/Input/FruitInputMappingManager.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Actors/PlayerPawn.h"
#include "Actors/FruitBall.h"
#include "Interface/HUD/FruitHUD.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Gameplay/Physics/FruitTrajectoryHelper.h"
#include "Gameplay/Merging/Core/FruitMergeHelper.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

APlayGameMode::APlayGameMode()
{
    // FruitHUD 명시적 설정
    HUDClass = AFruitHUD::StaticClass();
    
    // PlayerController 설정 확인
    PlayerControllerClass = AFruitPlayerController::StaticClass();

    // DefaultPawnClass 지정 (적절한 Pawn 클래스로 교체)
    DefaultPawnClass = APlayerPawn::StaticClass();

    FruitBallClass = AFruitBall::StaticClass();
    
    UE_LOG(LogTemp, Log, TEXT("APlayGameMode 생성자 호출됨"));
}

void APlayGameMode::BeginPlay()
{
    Super::BeginPlay();

    UUIWidgetRenderer::CreateDisplayWidget(GetWorld());

    // 현재 레벨 이름이 PlayLevel일 때만 Play UI 생성
    UWorld* World = GetWorld();
    if (World && World->GetMapName().Contains(TEXT("PlayLevel")))
    {
        // 입력 매핑은 여기서 한 번만!
        UFruitInputMappingManager::ConfigureKeyMappings();
        
        // PlayLevel 시작 후 0.66초 후 시작 시퀀스 실행
        FTimerHandle DelayHandle;
        GetWorldTimerManager().SetTimer(DelayHandle, [this]() {
            // 1. 먼저 HUD 참조 가져오기
            AFruitHUD* FruitHUD = Cast<AFruitHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
            FruitHUD->CreateAndAddWidgets();
            
            // 2. 게임 시작 시퀀스 생성 및 실행
            UPlayStartSequenceManager* SequenceManager = UPlayStartSequenceManager::CreateInstance(this);
            if (SequenceManager && FruitHUD && FruitHUD->GetTextureWidget())
            {
                // HUD의 UIWidgetRenderer 사용
                SequenceManager->SetExistingWidgetRenderer(FruitHUD->GetTextureWidget());
                
                // 시퀀스 완료 이벤트 바인딩
                SequenceManager->OnSequenceCompleted.AddDynamic(this, &APlayGameMode::OnGameStartSequenceFinished);
                
                // 시퀀스 시작
                SequenceManager->StartSequence(this);
                UE_LOG(LogTemp, Display, TEXT("게임 시작 시퀀스 시작"));
            }
        }, 0.66f, false);
    }
}

// 시퀀스 완료 이벤트 핸들러
void APlayGameMode::OnGameStartSequenceFinished()
{
    if (AFruitPlayerController* PC = Cast<AFruitPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
    {
        // 게임 시작 시 입력 해제
        PC->EnableInput(PC);
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeGameAndUI());
    }

    // 게임 시작 로직 실행
    UE_LOG(LogTemp, Display, TEXT("게임 시작 시퀀스 완료 - 게임 플레이 시작"));
    
    InitializeGameWidgets(); // 위젯 초기화 함수 호출

    // HUD의 UIWidgetRenderer에 이미지 설정
    AFruitHUD* FruitHUD = Cast<AFruitHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
    if (FruitHUD && FruitHUD->GetTextureWidget())
    {
        // 여기서 명시적으로 이미지 설정
        FruitHUD->GetTextureWidget()->SetupPlayImages();
        UE_LOG(LogTemp, Display, TEXT("게임 UI 이미지 설정 완료"));
    }
}

void APlayGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
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
    
    // 2. UIWidgetRenderer 정리
    if (UUIWidgetRenderer::IsInstanceValid())
    {
        UUIWidgetRenderer* TextureWidget = UUIWidgetRenderer::GetInstance();
        if (TextureWidget)
        {
            TextureWidget->RemoveFromParent();
            UUIWidgetRenderer::ClearInstance();
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
    
    // 4. UComboCountWidget 정리
    if (UComboCountWidget::IsInstanceValid())
    {
        UComboCountWidget* ComboCountWidget = UComboCountWidget::GetInstance();
        if (ComboCountWidget)
        {
            ComboCountWidget->ClearInstance();
        }
    }
    
    // 5. 스코어 매니저 참조 해제
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

void APlayGameMode::InitializeGameWidgets()
{
    UE_LOG(LogTemp, Display, TEXT("게임 위젯 초기화 시작"));
    
    // UI 위젯 초기화
    UScoreDisplayWidget* ScoreWidget = UScoreDisplayWidget::CreateScoreWidget(this);
    UTotalScoreWidget* TotalScoreWidget = UTotalScoreWidget::CreateTotalScoreWidget(this);
    
    // 새 콤보 카운트 위젯 생성
    UComboCountWidget* ComboWidget = UComboCountWidget::CreateComboCountWidget(this);

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
        ScoreManagerComp->ComboCountWidgetInstance = ComboWidget;
        ScoreManagerComp->bWidgetCreated = true;
        
        UE_LOG(LogTemp, Display, TEXT("게임 위젯 초기화 완료"));
    }
}