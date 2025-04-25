#include "UIWidgetUtility.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Font.h"
#include "Components/PanelWidget.h"

// 정적 멤버 변수 초기화
const FString UUIWidgetUtility::DEFAULT_KOREAN_FONT_PATH = TEXT("/Game/UI/Font/NotoSerifKR-Regular_Font");
const FString UUIWidgetUtility::DEFAULT_NUMBER_FONT_PATH = TEXT("/Game/UI/Font/LiberationSans-Bold_Font");

void UUIWidgetUtility::SetupTextBlockStyle(UTextBlock* TextBlock, FLinearColor Color, int32 FontSize, bool bBold, bool bAutoWrapText, ESlateVisibility DefaultVisibility)
{
    if (!TextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("SetupTextBlockStyle: TextBlock이 null입니다."));
        return;
    }
    
    // 1. 색상 설정
    TextBlock->SetColorAndOpacity(Color);
    
    // 2. 자동 줄바꿈 설정
    TextBlock->SetAutoWrapText(bAutoWrapText);
    
    // 3. 가시성 설정
    TextBlock->SetVisibility(DefaultVisibility);
    
    // 4. 폰트 설정 - 항상 DEFAULT_NUMBER_FONT_PATH 사용
    UFont* FontObject = LoadObject<UFont>(nullptr, *UUIWidgetUtility::DEFAULT_NUMBER_FONT_PATH);
    if (!FontObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("폰트 로드 실패: %s"), *UUIWidgetUtility::DEFAULT_NUMBER_FONT_PATH);
        return;
    }
    
    // 폰트 정보 설정
    FSlateFontInfo FontInfo;
    FontInfo.FontObject = FontObject;
    FontInfo.Size = FontSize;
    
    // Bold 설정 적용
    if (bBold)
    {
        FontInfo.TypefaceFontName = FName("Bold");
    }
    
    // 폰트 적용
    TextBlock->SetFont(FontInfo);
    
    UE_LOG(LogTemp, Display, TEXT("텍스트 블록에 스타일 적용 (폰트: %s, 크기: %d)"), *UUIWidgetUtility::DEFAULT_NUMBER_FONT_PATH, FontSize);
}

void UUIWidgetUtility::SetAnchorForSlot(UCanvasPanelSlot* CanvasSlot, EWidgetAnchor Anchor, float PaddingX, float PaddingY)
{
    if (!CanvasSlot) return;
    
    // 앵커 타입에 따라 위치와 정렬 설정
    switch (Anchor)
    {
        case EWidgetAnchor::TopLeft:
            CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
            CanvasSlot->SetPosition(FVector2D(PaddingX, PaddingY));
            CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
            break;
            
        case EWidgetAnchor::TopRight:
            CanvasSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
            CanvasSlot->SetPosition(FVector2D(-PaddingX, PaddingY));
            CanvasSlot->SetAlignment(FVector2D(1.0f, 0.0f));
            break;
            
        case EWidgetAnchor::MiddleLeft:
            CanvasSlot->SetAnchors(FAnchors(0.0f, 0.5f, 0.0f, 0.5f));
            CanvasSlot->SetPosition(FVector2D(PaddingX, 0.0f));
            CanvasSlot->SetAlignment(FVector2D(0.0f, 0.5f));
            break;
            
        case EWidgetAnchor::MiddleRight:
            CanvasSlot->SetAnchors(FAnchors(1.0f, 0.5f, 1.0f, 0.5f));
            CanvasSlot->SetPosition(FVector2D(-PaddingX, 0.0f));
            CanvasSlot->SetAlignment(FVector2D(1.0f, 0.5f));
            break;
            
        case EWidgetAnchor::BottomLeft:
            CanvasSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
            CanvasSlot->SetPosition(FVector2D(PaddingX, -PaddingY));
            CanvasSlot->SetAlignment(FVector2D(0.0f, 1.0f));
            break;
            
        case EWidgetAnchor::BottomRight:
            CanvasSlot->SetAnchors(FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
            CanvasSlot->SetPosition(FVector2D(-PaddingX, -PaddingY));
            CanvasSlot->SetAlignment(FVector2D(1.0f, 1.0f));
            break;
            
        case EWidgetAnchor::Center:
            CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
            CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
            CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            break;
    }
}

APlayerController* UUIWidgetUtility::GetValidPlayerController(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("GetValidPlayerController: World가 유효하지 않음!"));
        return nullptr;
    }
    
    APlayerController* Controller = World->GetFirstPlayerController();
    if (!Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("GetValidPlayerController: PlayerController가 유효하지 않음!"));
        return nullptr;
    }
    
    return Controller;
}