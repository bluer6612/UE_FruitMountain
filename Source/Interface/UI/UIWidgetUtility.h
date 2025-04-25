#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"
#include "UIWidgetUtility.generated.h"

// 앵커 위치 열거형
UENUM(BlueprintType)
enum class EWidgetAnchor : uint8
{
    TopLeft,
    TopRight,
    MiddleLeft,
    MiddleRight,
    BottomLeft,
    BottomRight,
    Center
};

class UCanvasPanelSlot;
class UImage;
class UTexture2D;
class UTextBlock;

UCLASS()
class UE_FRUITMOUNTAIN_API UUIWidgetUtility : public UObject
{
    GENERATED_BODY()
    
public:
    static const FString DEFAULT_FONT_PATH;

    // 글로벌 폰트 설정 함수
    UFUNCTION(BlueprintCallable, Category = "Font")
    static void SetGlobalFont();

    // 앵커 기반 슬롯 위치 설정
    static void SetAnchorForSlot(UCanvasPanelSlot* CanvasSlot, EWidgetAnchor Anchor, float PaddingX, float PaddingY);
    
    // 텍스트 블록 스타일 설정 헬퍼 함수
    UFUNCTION(BlueprintCallable, Category = "UI Helper")
    static void SetupTextBlockStyle(UTextBlock* TextBlock, FLinearColor Color, int32 FontSize, bool bBold = false, bool bAutoWrapText = true, ESlateVisibility DefaultVisibility = ESlateVisibility::HitTestInvisible);
    
    // 텍스트 블록 위치 설정 헬퍼 함수
    UFUNCTION(BlueprintCallable, Category = "UI Helper")
    static UCanvasPanelSlot* SetScoreDisplayPosition(UTextBlock* TextBlock, 
                                                   float PosX, 
                                                   float PosY,
                                                   float Width = 200.0f,
                                                   float Height = 80.0f,
                                                   bool bRightAlign = true);
    
    // 유효한 플레이어 컨트롤러 가져오기 (공통 함수)
    UFUNCTION(BlueprintCallable, Category = "UI Helper")
    static APlayerController* GetValidPlayerController(UObject* WorldContextObject);
    
    // 위젯 클래스 로드 함수
    template<class T>
    static TSubclassOf<UUserWidget> LoadWidgetClassIfNeeded(TSubclassOf<UUserWidget>& WidgetClass, const FString& BlueprintPath);
    
    // 싱글톤 위젯 생성 및 관리 템플릿 함수 (인스턴스 관리용)
    template<class T>
    static T* CreateSingletonWidget(
        T*& Instance, 
        TSubclassOf<UUserWidget>& WidgetClass, 
        UObject* WorldContextObject, 
        const FString& BlueprintPath, 
        int32 ZOrder = 10,
        ESlateVisibility Visibility = ESlateVisibility::HitTestInvisible)
    {
        // 기존 유효 인스턴스 확인
        if (Instance && IsValid(Instance) && Instance->IsInViewport())
        {
            return Instance;
        }
        
        // 기존 인스턴스가 무효하면 null로 설정
        if (Instance)
        {
            Instance = nullptr;
        }
        
        // 플레이어 컨트롤러 가져오기
        APlayerController* Controller = GetValidPlayerController(WorldContextObject);
        if (!Controller)
        {
            return nullptr;
        }
        
        // 위젯 클래스 로드
        if (!WidgetClass)
        {
            WidgetClass = LoadClass<UUserWidget>(nullptr, *BlueprintPath);
            if (!WidgetClass)
            {
                UE_LOG(LogTemp, Error, TEXT("CreateSingletonWidget: 블루프린트를 찾을 수 없습니다: %s"), *BlueprintPath);
                return nullptr;
            }
        }
        
        // 인스턴스 생성 및 뷰포트에 추가
        Instance = CreateWidget<T>(Controller, WidgetClass);
        if (Instance)
        {
            Instance->AddToViewport(ZOrder);
            Instance->SetVisibility(Visibility);
        }
        
        return Instance;
    }
        
    // 특정 위젯 유효성 검사 함수
    template<class T>
    static bool IsWidgetInstanceValid(T* Instance);
};

// 위젯 클래스 로드 템플릿 함수
template<class T>
TSubclassOf<UUserWidget> UUIWidgetUtility::LoadWidgetClassIfNeeded(TSubclassOf<UUserWidget>& WidgetClass, const FString& BlueprintPath)
{
    if (WidgetClass)
    {
        return WidgetClass;
    }
    
    WidgetClass = LoadClass<UUserWidget>(nullptr, *BlueprintPath);
    
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("블루프린트를 찾을 수 없습니다: %s"), *BlueprintPath);
    }
    
    return WidgetClass;
}

// 위젯 유효성 검사 템플릿 함수
template<class T>
bool UUIWidgetUtility::IsWidgetInstanceValid(T* Instance)
{
    return Instance && IsValid(Instance) && Instance->IsInViewport();
}