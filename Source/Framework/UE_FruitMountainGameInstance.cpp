#include "UE_FruitMountainGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "TimerManager.h"
#include "Interface/UI/FruitUIWidget.h"

UUE_FruitMountainGameInstance::UUE_FruitMountainGameInstance()
{
    // 생성자 로직 (필요 시)
}

void UUE_FruitMountainGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogTemp, Error, TEXT("GameInstance Init 호출됨"));
}

#pragma region ShowPersistentUI (슬레이트 직접 추가 예시)
void UUE_FruitMountainGameInstance::ShowPersistentUI()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowPersistentUI: World가 NULL임"));
        return;
    }

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

    // 슬레이트 콘텐츠도 제거
    if (LastAddedViewportContent.IsValid() && GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(LastAddedViewportContent.ToSharedRef());
        LastAddedViewportContent.Reset();
    }

    // 게임 뷰포트가 없으면 실패
    if (!(GEngine && GEngine->GameViewport))
    {
        UE_LOG(LogTemp, Error, TEXT("GEngine 또는 GameViewport가 NULL입니다!"));
        return;
    }

    // 빨간색 박스 예시
    const FLinearColor RedColor(1.f, 0.f, 0.f, 1.f);

    // 간단한 보더 위젯
    TSharedRef<SBorder> RedBorder = SNew(SBorder)
        .Padding(FMargin(10))
        .BorderBackgroundColor(RedColor);

    TSharedRef<SBox> SizedBox = SNew(SBox)
        .WidthOverride(300)
        .HeightOverride(300)
        [
            RedBorder
        ];

    TSharedRef<SOverlay> CenteredOverlay = SNew(SOverlay)
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SizedBox
        ];

    // 뷰포트에 추가
    GEngine->GameViewport->AddViewportWidgetContent(CenteredOverlay, 10000);
    LastAddedViewportContent = CenteredOverlay;

    UE_LOG(LogTemp, Error, TEXT("ShowPersistentUI: 슬레이트 위젯 직접 추가 완료"));
}

void UUE_FruitMountainGameInstance::CheckPersistentUI()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("CheckPersistentUI: World가 NULL임"));
        return;
    }

    // PersistentUIWidget이 NULL인 경우 UI 다시 생성
    if (!PersistentUIWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("CheckPersistentUI: PersistentUIWidget이 NULL, 다시 생성합니다"));
        
        // UI 다시 생성 시도
        ShowFruitUIWidget();
        
        // 타이머 설정 후 리턴
        FTimerHandle RetryTimer;
        World->GetTimerManager().SetTimer(
            RetryTimer,
            [this]()
            {
                CheckPersistentUI(); // 재귀 호출
            },
            1.0f,
            false
        );
        return;
    }

    // 이미 존재하는 경우 뷰포트 확인
    if (PersistentUIWidget->IsInViewport())
    {
        UE_LOG(LogTemp, Error, TEXT("CheckPersistentUI: PersistentUIWidget이 뷰포트에 있음"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CheckPersistentUI: 뷰포트에 없어 다시 추가"));
        PersistentUIWidget->AddToViewport(10000);
    }
}
#pragma endregion

#pragma region ShowFruitUIWidget
void UUE_FruitMountainGameInstance::ShowFruitUIWidget()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    // 기존 FruitWidget 제거
    if (FruitWidget && FruitWidget->IsInViewport())
    {
        FruitWidget->RemoveFromViewport();
        FruitWidget = nullptr;
    }

    // 새 FruitUI 생성 후 중앙에 표시
    UFruitUIWidget* NewFruitUI = CreateWidget<UFruitUIWidget>(PC, UFruitUIWidget::StaticClass());
    if (NewFruitUI)
    {
        NewFruitUI->AddToViewport(9999);
        NewFruitUI->SetVisibility(ESlateVisibility::Visible);
        FruitWidget = NewFruitUI;
    }
}

void UUE_FruitMountainGameInstance::ShowFruitUIWidgetAtPosition(EWidgetPosition InPosition)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowFruitUIWidgetAtPosition: World가 NULL임"));
        return;
    }
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowFruitUIWidgetAtPosition: 플레이어 컨트롤러가 NULL입니다"));
        return;
    }
    
    // 기존 위젯 제거 (단일 FruitWidget 참조를 재활용 중이라면)
    if (FruitWidget && FruitWidget->IsInViewport())
    {
        FruitWidget->RemoveFromViewport();
        FruitWidget = nullptr;
    }

    // 새 FruitUI 생성
    UFruitUIWidget* NewFruitUI = CreateWidget<UFruitUIWidget>(PC, UFruitUIWidget::StaticClass());
    if (NewFruitUI)
    {
        // 원하는 위치로 설정
        NewFruitUI->SetWidgetPosition(InPosition);

        // 뷰포트에 추가
        NewFruitUI->AddToViewport(9999);
        NewFruitUI->SetVisibility(ESlateVisibility::Visible);

        FruitWidget = NewFruitUI; // 단일 참조 예시

        UE_LOG(LogTemp, Error, TEXT("ShowFruitUIWidgetAtPosition: %s 위치로 FruitUIWidget 추가 완료"), *UEnum::GetValueAsString(InPosition));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ShowFruitUIWidgetAtPosition: FruitUIWidget 생성 실패"));
    }
}

// 여러 위치를 동시에 표시하는 함수
void UUE_FruitMountainGameInstance::ShowAllFruitUI()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 위치 배열
    TArray<EWidgetPosition> Positions = {
        EWidgetPosition::LeftTop,
        EWidgetPosition::LeftMiddle,
        EWidgetPosition::RightTop
    };

    for (EWidgetPosition Pos : Positions)
    {
        // 각 위치마다 새 위젯 스폰 (서로 다른 참조로 관리)
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (!PC) return;

        UFruitUIWidget* MultiFruitUI = CreateWidget<UFruitUIWidget>(PC, UFruitUIWidget::StaticClass());
        if (MultiFruitUI)
        {
            MultiFruitUI->SetWidgetPosition(Pos);
            MultiFruitUI->AddToViewport(9000); 
            MultiFruitUI->SetVisibility(ESlateVisibility::Visible);

            UE_LOG(LogTemp, Error, TEXT("ShowAllFruitUI: %s 위치로 FruitUIWidget 추가"), *UEnum::GetValueAsString(Pos));
        }
    }
}
#pragma endregion