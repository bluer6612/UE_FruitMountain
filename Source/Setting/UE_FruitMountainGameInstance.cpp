#include "UE_FruitMountainGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UI/TestUIWidget.h" // 새 경로로 변경

void UUE_FruitMountainGameInstance::Init()
{
    Super::Init();
    
    // 게임 시작 시 간단한 UI 생성 테스트
    FTimerHandle TimerHandle;
    GetTimerManager().SetTimer(
        TimerHandle,
        this,
        &UUE_FruitMountainGameInstance::TestCreateSimpleUI,
        1.0f,
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