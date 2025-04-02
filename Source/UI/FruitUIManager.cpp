#include "FruitUIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// 정적 인스턴스 초기화
UFruitUIManager* UFruitUIManager::Instance = nullptr;

UFruitUIManager::UFruitUIManager()
{
    // 생성자에서는 특별한 초기화 작업 없음
}

UFruitUIManager* UFruitUIManager::GetInstance()
{
    // 인스턴스가 없으면 생성
    if (!Instance)
    {
        Instance = NewObject<UFruitUIManager>();
        Instance->AddToRoot(); // 가비지 컬렉션으로부터 보호
    }
    
    return Instance;
}

void UFruitUIManager::Initialize(APlayerController* InController)
{
    // 컨트롤러 설정
    PlayerController = InController;
    
    // 기존 위젯 정리
    UIWidgets.Empty();
    
    // 위젯 자동 생성
    CreateUIWidgets();
    
    // 기본 이미지 로드
    LoadDefaultImages();
    
    UE_LOG(LogTemp, Log, TEXT("FruitUIManager 초기화 완료"));
}

void UFruitUIManager::CreateUIWidgets(TSubclassOf<UFruitUIWidget> InWidgetClass)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("UI 위젯 생성 실패: PlayerController가 없습니다"));
        return;
    }
    
    // 기존 위젯 제거
    for (UFruitUIWidget* Widget : UIWidgets)
    {
        if (Widget)
        {
            Widget->RemoveFromParent();
        }
    }
    UIWidgets.Empty();
    
    // 위젯 클래스가 설정되지 않은 경우 기본 클래스 찾기
    TSubclassOf<UFruitUIWidget> WidgetClass = InWidgetClass;
    if (!WidgetClass)
    {
        // 컨텐츠 브라우저에서 위젯 클래스 찾기 시도
        WidgetClass = LoadClass<UFruitUIWidget>(nullptr, TEXT("/Game/UI/BP_FruitUIWidget.BP_FruitUIWidget_C"));
        
        // 찾지 못했다면 기본 C++ 클래스 사용
        if (!WidgetClass)
        {
            WidgetClass = UFruitUIWidget::StaticClass();
        }
    }
    
    // 3개의 위젯 생성 (왼쪽 상단, 왼쪽 중앙, 오른쪽 상단)
    EWidgetPosition Positions[] = {
        EWidgetPosition::LeftTop,
        EWidgetPosition::LeftMiddle,
        EWidgetPosition::RightTop
    };
    
    for (int32 i = 0; i < 3; ++i)
    {
        UFruitUIWidget* NewWidget = CreateWidget<UFruitUIWidget>(PlayerController, WidgetClass);
        if (NewWidget)
        {
            // 위젯 화면에 추가
            NewWidget->AddToViewport();
            
            // 위치 설정
            NewWidget->SetWidgetPosition(Positions[i]);
            
            // 배열에 저장
            UIWidgets.Add(NewWidget);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("UI 위젯 %d개 생성 완료"), UIWidgets.Num());
}

void UFruitUIManager::SetWidgetImage(int32 WidgetIndex, UTexture2D* NewTexture)
{
    if (UIWidgets.IsValidIndex(WidgetIndex) && NewTexture)
    {
        UIWidgets[WidgetIndex]->SetImage(NewTexture);
    }
}

void UFruitUIManager::SetAllWidgetsVisibility(bool bVisible)
{
    for (UFruitUIWidget* Widget : UIWidgets)
    {
        if (Widget)
        {
            Widget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        }
    }
}

void UFruitUIManager::SetWidgetVisibility(int32 WidgetIndex, bool bVisible)
{
    if (UIWidgets.IsValidIndex(WidgetIndex))
    {
        UIWidgets[WidgetIndex]->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UFruitUIManager::LoadDefaultImages()
{
    // 기본 이미지 로드 및 설정
    UTexture2D* TopLeftImage = LoadUITexture("UI_Score");
    UTexture2D* MiddleLeftImage = LoadUITexture("UI_FruitList");
    UTexture2D* TopRightImage = LoadUITexture("UI_NextFruit");
    
    // 위젯에 설정
    if (UIWidgets.Num() >= 3)
    {
        if (TopLeftImage) SetWidgetImage(0, TopLeftImage);
        if (MiddleLeftImage) SetWidgetImage(1, MiddleLeftImage);
        if (TopRightImage) SetWidgetImage(2, TopRightImage);
    }
}

UTexture2D* UFruitUIManager::LoadUITexture(const FString& ImageName)
{
    // 전체 경로 구성 - 제공된 UI 경로 사용
    FString FullPath = FString::Printf(TEXT("/Game/Content/Asset/UI/%s.%s"), *ImageName, *ImageName);
    
    // 텍스처 로드
    UTexture2D* LoadedTexture = LoadObject<UTexture2D>(nullptr, *FullPath);
    
    return LoadedTexture;
}