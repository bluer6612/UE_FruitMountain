#include "FruitHUD.h"
#include "Engine/Canvas.h"
#include "Blueprint/UserWidget.h"
#include "Interface/UI/UIWidgetRenderer.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h" 
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SViewport.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

AFruitHUD::AFruitHUD()
{
    TextureWidget = nullptr;
}

void AFruitHUD::BeginPlay()
{
    Super::BeginPlay();
    
    // 글로벌 폰트 설정
    UUIWidgetUtility::SetGlobalFont();
    
    // 2D 텍스쳐 위젯 그릴 위젯 생성
    CreateAndAddWidgets();
    
    // 약간의 지연 후 모든 텍스트 위젯에 폰트 적용
    FTimerHandle FontApplyHandle;
    GetWorldTimerManager().SetTimer(FontApplyHandle, [this]()
    {
        // 뷰포트의 모든 위젯 탐색
        if (GEngine && GEngine->GameViewport)
        {
            // 모든 활성 위젯 찾기
            TArray<UUserWidget*> AllWidgets = GetAllActiveWidgets();
            
            // 각 위젯에 폰트 적용
            for (UUserWidget* Widget : AllWidgets)
            {
                UUIWidgetUtility::ApplyFontToAllTextBlocks(Widget, TEXT(""), 0);
                UE_LOG(LogTemp, Display, TEXT("위젯에 폰트 적용: %s"), *Widget->GetName());
            }
        }
    }, 0.5f, false); // 0.5초 후 실행
}

void AFruitHUD::CreateAndAddWidgets()
{
    APlayerController* Controller = GetOwningPlayerController();
    if (Controller)
    {
        // 기존 위젯 제거
        if (TextureWidget)
        {
            TextureWidget->RemoveFromParent();
            TextureWidget = nullptr;
        }
        
        // UIWidgetRenderer 생성
        TextureWidget = CreateWidget<UUIWidgetRenderer>(Controller, UUIWidgetRenderer::StaticClass());
        if (TextureWidget)
        {
            // 뷰포트에 추가
            TextureWidget->AddToViewport(9999); // HUD보다 아래에 위치하도록
            
            // 로그
            UE_LOG(LogTemp, Warning, TEXT("FruitHUD: UIWidgetRenderer 생성 및 뷰포트 추가 완료"));
            
            // 이미지 설정 함수 호출
            TextureWidget->SetupAllImages();
            
            // UMG 렌더링 강제 설정 (아래 함수가 UI 그리는 공간 만드는 핵심)
            if (GEngine && GEngine->GameViewport)
            {
                // 뷰포트 설정 강제 적용
                //GEngine->GameViewport->SetForceDisableSplitscreen(true);

                UE_LOG(LogTemp, Warning, TEXT("GameViewport 검사: %s"), 
                      GEngine->GameViewport->IsValidLowLevel() ? TEXT("유효함") : TEXT("유효하지 않음"));
                
                // Z 순서 최상위로 설정
                TextureWidget->RemoveFromParent();
                GEngine->GameViewport->AddViewportWidgetContent(
                    SNew(SBox)
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Fill)
                    [
                        TextureWidget->TakeWidget()
                    ]
                );
                UE_LOG(LogTemp, Warning, TEXT("SBox에 래핑한 위젯을 뷰포트에 직접 추가"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FruitHUD: UIWidgetRenderer 생성 실패"));
        }
    }
}

TArray<UUserWidget*> AFruitHUD::GetAllActiveWidgets()
{
    TArray<UUserWidget*> AllWidgets;
    
    if (!GEngine || !GEngine->GameViewport)
    {
        return AllWidgets;
    }
    
    // UIWidgetRenderer 추가
    if (TextureWidget)
    {
        AllWidgets.Add(TextureWidget);
    }
    
    // 씬에 있는 모든 플레이어 컨트롤러 검사
    UWorld* World = GetWorld();
    if (!World)
    {
        return AllWidgets;
    }
    
    // GameMode를 통해 참조된 위젯 가져오기
    AGameModeBase* GameMode = UGameplayStatics::GetGameMode(World);
    if (GameMode)
    {
        // 여기서 GameMode 내의 특정 위젯 참조를 가져올 수 있음
        // 예: ScoreManagerComponent에서 위젯 참조를 얻을 수 있음
    }
    
    // 모든 플레이어 컨트롤러에서 HUD 위젯 찾기
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        if (!PlayerController)
            continue;
            
        // 현재 HUD 확인
        AHUD* HUD = PlayerController->GetHUD();
        if (HUD != this) // 자기 자신이 아닌 경우만 확인
        {
            // HUD에 위젯 참조가 있을 수 있음
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("활성 위젯 %d개 발견"), AllWidgets.Num());
    return AllWidgets;
}

void AFruitHUD::CollectWidgetsFromWidget(UWidget* Widget, TArray<UUserWidget*>& OutWidgets)
{
    if (!Widget)
    {
        return;
    }
    
    // UUserWidget인 경우 추가
    UUserWidget* UserWidget = Cast<UUserWidget>(Widget);
    if (UserWidget && !OutWidgets.Contains(UserWidget))
    {
        OutWidgets.Add(UserWidget);
        
        // UWidgetTree를 통해 모든 위젯 대신 검색
        UWidgetTree* WidgetTree = UserWidget->WidgetTree;
        if (WidgetTree)
        {
            WidgetTree->ForEachWidget([&OutWidgets, this](UWidget* ChildWidget) {
                UUserWidget* ChildUserWidget = Cast<UUserWidget>(ChildWidget);
                if (ChildUserWidget && !OutWidgets.Contains(ChildUserWidget))
                {
                    OutWidgets.Add(ChildUserWidget);
                    this->CollectWidgetsFromWidget(ChildUserWidget, OutWidgets);
                }
            });
        }
    }
    
    // 패널 위젯인 경우 모든 자식에게 재귀 적용
    UPanelWidget* PanelWidget = Cast<UPanelWidget>(Widget);
    if (PanelWidget)
    {
        for (int32 i = 0; i < PanelWidget->GetChildrenCount(); i++)
        {
            UWidget* ChildWidget = PanelWidget->GetChildAt(i);
            if (ChildWidget)
            {
                CollectWidgetsFromWidget(ChildWidget, OutWidgets);
            }
        }
    }
}