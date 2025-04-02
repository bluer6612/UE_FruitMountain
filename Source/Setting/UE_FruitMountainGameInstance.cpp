#include "UE_FruitMountainGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UI/TestUIWidget.h" // 새 경로로 변경

void UUE_FruitMountainGameInstance::Init()
{
    Super::Init();
    
    UE_LOG(LogTemp, Error, TEXT("FruitMountain 게임 인스턴스 초기화"));
    
    // UI 표시는 약간 지연
    FTimerHandle TimerHandle;
    GetTimerManager().SetTimer(
        TimerHandle,
        this,
        &UUE_FruitMountainGameInstance::ShowPersistentUI,
        0.5f,  // 0.5초 후
        false
    );
}

void UUE_FruitMountainGameInstance::TestCreateSimpleUI()
{
    UE_LOG(LogTemp, Error, TEXT("간단한 테스트 UI 생성 시도"));
    
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("플레이어 컨트롤러가 없습니다"));
        return;
    }
    
    // 기존 위젯 제거 (혹시 있다면)
    TArray<UUserWidget*> FoundWidgets;
    for (TObjectIterator<UUserWidget> Itr; Itr; ++Itr)
    {
        UUserWidget* Widget = *Itr;
        // IsPendingKill 대신 IsValid를 사용
        if (Widget && IsValid(Widget) && Widget->GetClass() == UTestUIWidget::StaticClass())
        {
            FoundWidgets.Add(Widget);
        }
    }
    
    // 기존 위젯 모두 제거
    for (UUserWidget* Widget : FoundWidgets)
    {
        if (Widget->IsInViewport())
        {
            Widget->RemoveFromParent();
        }
    }
    
    // 새 위젯 생성
    UTestUIWidget* TestWidget = CreateWidget<UTestUIWidget>(PC, UTestUIWidget::StaticClass());
    if (TestWidget)
    {
        // 가시성 명시적 설정
        TestWidget->SetVisibility(ESlateVisibility::Visible);
        
        // 화면에 추가 (z-order 최대)
        TestWidget->AddToViewport(9999);
        
        // 빨간색 블록 표시
        TestWidget->ShowRedBlock();
        
        UE_LOG(LogTemp, Error, TEXT("테스트 위젯이 화면에 추가되었습니다"));
    }
    
    // 모든 UI 가시성 강제 업데이트
    PC->FlushPressedKeys();
    
    // 확인용 추가 타이머
    FTimerHandle DoubleCheckTimer;
    GetWorld()->GetTimerManager().SetTimer(
        DoubleCheckTimer,
        [TestWidget, PC]()
        {
            // 다시 한번 확인
            if (TestWidget)
            {
                if (TestWidget->IsInViewport())
                {
                    UE_LOG(LogTemp, Error, TEXT("1초 후 확인: 위젯이 뷰포트에 있습니다"));
                    
                    // 가시성 다시 설정
                    TestWidget->SetVisibility(ESlateVisibility::Visible);
                    TestWidget->ShowRedBlock();
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("1초 후 확인: 위젯이 뷰포트에 없어서 다시 추가합니다"));
                    TestWidget->AddToViewport(10000);
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("1초 후 확인: 위젯 인스턴스가 제거되었습니다"));
            }
        },
        1.0f,
        false
    );
}

void UUE_FruitMountainGameInstance::ShowPersistentUI()
{
    UE_LOG(LogTemp, Error, TEXT("슬레이트 방식으로 직접 UI 생성 시도"));
    
    // 이전 위젯 정리
    if (PersistentUIWidget && PersistentUIWidget->IsInViewport())
    {
        PersistentUIWidget->RemoveFromViewport();
    }
    
    if (PersistentUIWidget && PersistentUIWidget->IsRooted())
    {
        PersistentUIWidget->RemoveFromRoot();
    }
    
    // 새로운 방식: 슬레이트 이미지 위젯 생성
    if (GEngine && GEngine->GameViewport)
    {
        // 색상 정의 - 빨간색
        const FLinearColor RedColor(1.0f, 0.0f, 0.0f, 1.0f);
        
        // 슬레이트 이미지 생성
        TSharedRef<SImage> RedImage = SNew(SImage)
            .ColorAndOpacity(RedColor);
        
        // 슬레이트 보더 생성 (패딩과 함께)
        TSharedRef<SBorder> RedBorder = SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("Border"))
            .BorderBackgroundColor(FLinearColor::Black)
            .Padding(FMargin(50))
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                RedImage
            ];
        
        // 크기 박스로 감싸서 크기 지정
        TSharedRef<SBox> SizedBox = SNew(SBox)
            .WidthOverride(400)
            .HeightOverride(400)
            [
                RedBorder
            ];
        
        // 화면 중앙에 배치하기 위한 오버레이
        TSharedRef<SOverlay> CenteredOverlay = SNew(SOverlay)
            + SOverlay::Slot()
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SizedBox
            ];
        
        // 게임 뷰포트에 직접 추가
        GEngine->GameViewport->AddViewportWidgetContent(
            CenteredOverlay,
            10000  // 높은 Z-Order
        );
        
        UE_LOG(LogTemp, Error, TEXT("슬레이트 위젯 직접 추가 완료"));
        
        // 저장된 참조 - 필요하면 나중에 삭제하기 위해
        LastAddedViewportContent = CenteredOverlay;
        
        // 확인용 로그
        FTimerHandle CheckTimer;
        GetTimerManager().SetTimer(
            CheckTimer,
            [this]()
            {
                UE_LOG(LogTemp, Error, TEXT("타이머 확인: 위젯을 다시 추가합니다"));
                
                // 이전에 추가한 위젯 존재하면 제거
                if (LastAddedViewportContent.IsValid() && GEngine && GEngine->GameViewport)
                {
                    GEngine->GameViewport->RemoveViewportWidgetContent(LastAddedViewportContent.ToSharedRef());
                }
                
                // 새 위젯 추가
                if (GEngine && GEngine->GameViewport)
                {
                    // 완전히 새로운 위젯 생성
                    TSharedRef<STextBlock> TextWidget = SNew(STextBlock)
                        .Text(FText::FromString(TEXT("빨간색 텍스트")))
                        .ColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 48));
                    
                    TSharedRef<SBox> TextBox = SNew(SBox)
                        .WidthOverride(600)
                        .HeightOverride(100)
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        [
                            TextWidget
                        ];
                    
                    // 게임 뷰포트에 직접 추가
                    LastAddedViewportContent = TextBox;
                    GEngine->GameViewport->AddViewportWidgetContent(TextBox, 10000);
                    
                    UE_LOG(LogTemp, Error, TEXT("새 텍스트 위젯이 추가됨"));
                }
            },
            3.0f,  // 3초마다
            true   // 반복
        );
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GEngine 또는 GameViewport가 NULL입니다!"));
    }
}

// 위젯 상태 확인 함수 추가
void UUE_FruitMountainGameInstance::CheckPersistentUI()
{
    FTimerHandle CheckTimer;
    GetTimerManager().SetTimer(
        CheckTimer,
        [this]()
        {
            if (!PersistentUIWidget)
            {
                UE_LOG(LogTemp, Error, TEXT("CheckPersistentUI: 위젯이 NULL입니다"));
                return;
            }
            
            if (PersistentUIWidget->IsInViewport())
            {
                UE_LOG(LogTemp, Error, TEXT("CheckPersistentUI: 위젯이 뷰포트에 있습니다"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("CheckPersistentUI: 위젯이 뷰포트에 없어 다시 추가합니다"));
                PersistentUIWidget->AddToViewport(10000);
            }
        },
        0.5f,
        true  // 반복
    );
}