#include "FruitUIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"

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
    UE_LOG(LogTemp, Error, TEXT("FruitUIManager - LoadUITexture 시작: %s"), *ImageName);
    
    // 실제 존재하는 경로로 추가 테스트
    TArray<FString> PathsToTry;
    
    // Content 폴더 내 다양한 경로 시도
    PathsToTry.Add(FString::Printf(TEXT("/Game/Asset/UI/%s.%s"), *ImageName, *ImageName));
    PathsToTry.Add(FString::Printf(TEXT("/Game/UI/%s.%s"), *ImageName, *ImageName));
    PathsToTry.Add(FString::Printf(TEXT("/Game/UI/Images/%s.%s"), *ImageName, *ImageName));
    PathsToTry.Add(FString::Printf(TEXT("/Game/Textures/UI/%s.%s"), *ImageName, *ImageName));
    
    // 엔진에서 확실히 찾을 수 있는 텍스처 (디버깅용)
    PathsToTry.Add(TEXT("/Engine/EditorResources/SequenceRecorder.SequenceRecorder"));
    PathsToTry.Add(TEXT("/Engine/EditorResources/S_Actor.S_Actor"));
    
    UTexture2D* LoadedTexture = nullptr;
    
    // 모든 경로 시도하며 상세 로그 출력
    for (const FString& Path : PathsToTry)
    {
        UE_LOG(LogTemp, Error, TEXT("경로 시도: %s"), *Path);
        LoadedTexture = LoadObject<UTexture2D>(nullptr, *Path);
        
        if (LoadedTexture)
        {
            UE_LOG(LogTemp, Error, TEXT("이미지 로드 성공: %s"), *Path);
            break;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("경로에서 이미지 찾지 못함: %s"), *Path);
        }
    }
    
    // 테스트: 프로젝트에 포함된 다른 이미지를 직접 불러오기
    if (!LoadedTexture)
    {
        // "/Engine/EngineResources/DefaultTexture.DefaultTexture"는 매우 작고 보이지 않을 수 있음
        // 대신 눈에 띄는 텍스처 사용
        LoadedTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture"));
        if (LoadedTexture)
        {
            UE_LOG(LogTemp, Error, TEXT("기본 엔진 하얀색 텍스처 사용 (확실히 보이는 텍스처)"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("하얀색 텍스처도 로드 실패 - 엔진/경로 문제"));
        }
    }
    
    return LoadedTexture;
}

void UFruitUIManager::TestUIToggle()
{
    static bool bShowWhite = false;
    bShowWhite = !bShowWhite;
    
    UE_LOG(LogTemp, Error, TEXT("UI 토글 - 모드: %s"), bShowWhite ? TEXT("흰색") : TEXT("아이콘"));
    
    for (int32 i = 0; i < UIWidgets.Num(); ++i)
    {
        UTexture2D* TestTexture = nullptr;
        if (bShowWhite)
        {
            // 하얀색 텍스처
            TestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture"));
        }
        else
        {
            // 색상이 다른 텍스처 3개
            if (i == 0) 
                TestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/S_Actor"));
            else if (i == 1)
                TestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/ActorIcons/LightActor_16x"));
            else
                TestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/ActorIcons/Camera_16x"));
        }
        
        if (TestTexture)
        {
            SetWidgetImage(i, TestTexture);
        }
    }
}

void UFruitUIManager::SetDebugTexture(int32 WidgetIndex, int32 TextureType)
{
    if (!UIWidgets.IsValidIndex(WidgetIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("SetDebugTexture: 유효하지 않은 위젯 인덱스 %d"), WidgetIndex);
        return;
    }
    
    UTexture2D* TestTexture = nullptr;
    
    // 텍스처 타입에 따라 다른 엔진 기본 텍스처 사용
    switch (TextureType)
    {
        case 0: // 흰색
            TestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture"));
            break;
        case 1: // 아이콘
            TestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/S_Actor"));
            break;
        case 2: // 라이트 아이콘
            TestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/ActorIcons/LightActor_16x"));
            break;
        case 3: // 카메라 아이콘
            TestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/ActorIcons/Camera_16x"));
            break;
        default:
            TestTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/S_Actor"));
            break;
    }
    
    if (TestTexture)
    {
        SetWidgetImage(WidgetIndex, TestTexture);
        UE_LOG(LogTemp, Error, TEXT("위젯 %d에 디버그 텍스처 타입 %d 설정됨"), WidgetIndex, TextureType);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("디버그 텍스처 로드 실패"));
    }
}