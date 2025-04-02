#include "FruitUIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Blueprint/WidgetLayoutLibrary.h" // 추가: 뷰포트 크기 관련 기능
#include "Components/CanvasPanelSlot.h" // 추가: 캔버스 패널 슬롯 접근
#include "Components/Image.h" // 추가: 이미지 관련 기능

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
    UE_LOG(LogTemp, Log, TEXT("CreateUIWidgets 시작"));
    
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateUIWidgets 실패: PlayerController가 NULL"));
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
    
    // 위젯 클래스 선택 - 명확하게 수정
    TSubclassOf<UFruitUIWidget> WidgetClass;
    if (InWidgetClass)  // 단순화된 형태로 변경
    {
        WidgetClass = InWidgetClass;
    }
    else
    {
        WidgetClass = UFruitUIWidget::StaticClass();
    }
    
    // 3개의 위젯 생성
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
            // 위젯 화면에 추가 - Z-Order 높게 설정
            NewWidget->AddToViewport(100 + i);
            
            // 가시성 명시적 설정
            NewWidget->SetVisibility(ESlateVisibility::Visible);
            
            // 위치 설정
            NewWidget->SetWidgetPosition(Positions[i]);
            
            // 배열에 저장
            UIWidgets.Add(NewWidget);
            
            UE_LOG(LogTemp, Log, TEXT("위젯 생성 완료"));
        }
    }
    
    // 모든 위젯을 화면 상단에 강제 배치 (테스트)
    for (int32 i = 0; i < UIWidgets.Num(); ++i)
    {
        if (UIWidgets[i])
        {
            if (UCanvasPanelSlot* ParentSlot = Cast<UCanvasPanelSlot>(UIWidgets[i]->Slot))
            {
                FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(PlayerController);
                ParentSlot->SetPosition(FVector2D(i * 300.0f + 100.0f, 100.0f));
                ParentSlot->SetSize(FVector2D(256.0f, 256.0f));
                ParentSlot->SetZOrder(9999);
                
                UE_LOG(LogTemp, Log, TEXT("위젯 강제 배치 설정 (테스트)"));
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("위젯 생성 완료"));
    
    // 지연 로드 설정 (0.5초로 늘림)
    if (PlayerController && PlayerController->GetWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("이미지 로드 지연 타이머 설정"));
        
        FTimerHandle TimerHandle;
        PlayerController->GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            [this]()
            {
                UE_LOG(LogTemp, Log, TEXT("지연된 이미지 로드 시작"));
                LoadDefaultImages();
            },
            0.5f,
            false
        );
    }
    
    // 테스트: 모든 위젯에 빨간색 블록 표시 시도
    for (UFruitUIWidget* Widget : UIWidgets)
    {
        if (Widget)
        {
            Widget->ShowRedBlock();
            UE_LOG(LogTemp, Warning, TEXT("위젯에 빨간색 블록 표시 요청"));
        }
    }
}

void UFruitUIManager::SetWidgetImage(int32 WidgetIndex, UTexture2D* NewTexture)
{
    if (UIWidgets.IsValidIndex(WidgetIndex) && NewTexture)
    {
        // 이미지 설정
        UIWidgets[WidgetIndex]->SetImage(NewTexture);
        
        // 가시성 명시적 설정
        UIWidgets[WidgetIndex]->SetVisibility(ESlateVisibility::Visible);
        
        // 0.1초 후 가시성 다시 확인 (안전장치)
        if (PlayerController && PlayerController->GetWorld())
        {
            FTimerHandle TimerHandle;
            PlayerController->GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                [this, WidgetIndex]()
                {
                    if (UIWidgets.IsValidIndex(WidgetIndex) && UIWidgets[WidgetIndex])
                    {
                        UIWidgets[WidgetIndex]->SetVisibility(ESlateVisibility::Visible);
                    }
                },
                0.1f,
                false
            );
        }
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
    UTexture2D* TopLeftImage = LoadUITexture("UI_Play_Score");
    UTexture2D* MiddleLeftImage = LoadUITexture("UI_Play_FruitList");
    UTexture2D* TopRightImage = LoadUITexture("UI_Play_NextFruit");
    
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
    UE_LOG(LogTemp, Log, TEXT("이미지 로드 시도"));
    
    // 여러 가능한 경로 시도
    TArray<FString> PathsToTry;
    PathsToTry.Add(FString::Printf(TEXT("/Game/Asset/UI/%s.%s"), *ImageName, *ImageName));
    PathsToTry.Add(FString::Printf(TEXT("/Game/UE_FruitMountain/Asset/UI/%s.%s"), *ImageName, *ImageName));
    PathsToTry.Add(FString::Printf(TEXT("/Game/UI/%s.%s"), *ImageName, *ImageName));
    PathsToTry.Add(TEXT("/Engine/EditorResources/S_Actor.S_Actor")); // 항상 있는 엔진 대체 이미지
    
    UTexture2D* LoadedTexture = nullptr;
    
    // 모든 경로 시도
    for (const FString& Path : PathsToTry)
    {
        LoadedTexture = LoadObject<UTexture2D>(nullptr, *Path);
        if (LoadedTexture)
        {
            // 이미지 로드 성공 - 경로와 함께 로그 출력
            FString LogMessage = FString::Printf(TEXT("이미지 로드 성공: %s"), *Path);
            UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
            break;
        }
    }
    
    if (!LoadedTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("모든 경로에서 이미지 로드 실패"));
        // 기본 텍스처 사용
        LoadedTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
        if (LoadedTexture)
        {
            UE_LOG(LogTemp, Log, TEXT("기본 엔진 텍스처 사용"));
        }
    }
    
    return LoadedTexture;
}