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
    UE_LOG(LogTemp, Error, TEXT("FruitUIManager - Initialize 호출"));
    
    // 컨트롤러 설정
    PlayerController = InController;
    
    // 기존 위젯 정리
    for (UFruitUIWidget* Widget : UIWidgets)
    {
        if (Widget)
        {
            Widget->RemoveFromParent();
        }
    }
    UIWidgets.Empty();
    
    UE_LOG(LogTemp, Error, TEXT("FruitUIManager - 위젯 생성 시작"));
    
    // 위젯 자동 생성
    CreateUIWidgets();
    
    // 1초 후에 추가 체크 (대기 시간 늘림)
    if (PlayerController && PlayerController->GetWorld())
    {
        FTimerHandle TimerHandle;
        PlayerController->GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            [this]()
            {
                UE_LOG(LogTemp, Error, TEXT("FruitUIManager - 타이머에서 상태 확인"));
                
                // 위젯 개수 확인
                UE_LOG(LogTemp, Error, TEXT("FruitUIManager - 현재 위젯 개수: %d"), UIWidgets.Num());
                
                // 기본 이미지 로드 (다시 한번)
                LoadDefaultImages();
                
                // 가시성 재설정
                for (UFruitUIWidget* Widget : UIWidgets)
                {
                    if (Widget)
                    {
                        Widget->SetVisibility(ESlateVisibility::Visible);
                        
                        // 위젯 로그 확인
                        if (Widget->IsInViewport())
                        {
                            UE_LOG(LogTemp, Error, TEXT("FruitUIManager - 위젯이 뷰포트에 있음"));
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("FruitUIManager - 위젯이 뷰포트에 없음"));
                            Widget->AddToViewport(9999); // 강제로 다시 추가
                        }
                    }
                }
            },
            1.0f,
            false
        );
    }
    
    UE_LOG(LogTemp, Error, TEXT("FruitUIManager - Initialize 완료"));
}

void UFruitUIManager::CreateUIWidgets(TSubclassOf<UFruitUIWidget> InWidgetClass)
{
    UE_LOG(LogTemp, Warning, TEXT("FruitUIManager - CreateUIWidgets 시작"));
    
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("FruitUIManager - CreateUIWidgets 실패: PlayerController가 NULL"));
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
    
    // 위젯 클래스 선택 - 모호성 해결
    TSubclassOf<UFruitUIWidget> WidgetClass;
    if (InWidgetClass)  // 단순 불리언 체크로 변경 (내부적으로 UClass* 캐스팅 수행)
    {
        WidgetClass = InWidgetClass;
    }
    else
    {
        WidgetClass = UFruitUIWidget::StaticClass();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("FruitUIManager - 위젯 클래스 선택 완료, 위젯 생성 시작"));
    
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
            
            UE_LOG(LogTemp, Warning, TEXT("FruitUIManager - 위젯 %d 생성 및 설정 완료"), i);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FruitUIManager - 위젯 %d 생성 실패"), i);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("FruitUIManager - 위젯 생성 완료, 로드 타이머 설정"));
    
    // 지연 로드 설정 (0.5초로 늘림)
    if (PlayerController && PlayerController->GetWorld())
    {
        FTimerHandle TimerHandle;
        PlayerController->GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            [this]()
            {
                UE_LOG(LogTemp, Warning, TEXT("FruitUIManager - 지연된 이미지 로드 시작"));
                LoadDefaultImages();
                
                // 이미지 로드 후 다시 한번 가시성 확인
                for (UFruitUIWidget* Widget : UIWidgets)
                {
                    if (Widget)
                    {
                        Widget->SetVisibility(ESlateVisibility::Visible);
                        Widget->UpdateWidgetPosition();
                    }
                }
            },
            0.5f,
            false
        );
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
    UE_LOG(LogTemp, Warning, TEXT("FruitUIManager - LoadUITexture 시작: %s"), *ImageName);
    
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
            UE_LOG(LogTemp, Warning, TEXT("FruitUIManager - 이미지 로드 성공: %s"), *Path);
            break;
        }
    }
    
    if (!LoadedTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("FruitUIManager - 모든 경로에서 이미지 로드 실패"));
        // 기본 텍스처 사용
        LoadedTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
        if (LoadedTexture)
        {
            UE_LOG(LogTemp, Warning, TEXT("FruitUIManager - 기본 엔진 텍스처 사용"));
        }
    }
    
    return LoadedTexture;
}