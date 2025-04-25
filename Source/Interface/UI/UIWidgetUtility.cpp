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
const FString UUIWidgetUtility::DEFAULT_FONT_PATH = TEXT("/Game/UI/Font/NotoSerifKR-Regular_Font");

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

bool UUIWidgetUtility::SetupTextBlockStyle(UTextBlock* TextBlock, FLinearColor Color, float FontSize, bool bBold, bool bAutoWrapText, ESlateVisibility DefaultVisibility)
{
    if (!TextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("SetupTextBlockStyle: TextBlock이 null입니다."));
        return false;
    }
    
    // 1. 색상 설정
    TextBlock->SetColorAndOpacity(Color);
    
    // 2. 자동 줄바꿈 설정
    TextBlock->SetAutoWrapText(bAutoWrapText);
    
    // 3. 가시성 설정
    TextBlock->SetVisibility(DefaultVisibility);
    
    // 4. 폰트 설정 - 항상 DEFAULT_FONT_PATH 사용
    UFont* FontObject = LoadObject<UFont>(nullptr, *UUIWidgetUtility::DEFAULT_FONT_PATH);
    if (!FontObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("폰트 로드 실패: %s"), *UUIWidgetUtility::DEFAULT_FONT_PATH);
        return false;
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
    
    UE_LOG(LogTemp, Display, TEXT("텍스트 블록에 스타일 적용 (폰트: %s, 크기: %.1f)"), *UUIWidgetUtility::DEFAULT_FONT_PATH, FontSize);
    return true;
}

UCanvasPanelSlot* UUIWidgetUtility::SetScoreDisplayPosition(UTextBlock* TextBlock, float PosX, float PosY, float Width, float Height, bool bRightAlign)
{
    if (!TextBlock)
        return nullptr;
    
    UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(TextBlock->Slot);
    if (TextSlot)
    {
        // 앵커 설정 - bRightAlign 매개변수로 결정
        TextSlot->SetAnchors(FAnchors(bRightAlign ? 1.0f : 0.0f, 0.0f, bRightAlign ? 1.0f : 0.0f, 0.0f));
        
        // 정렬 설정
        TextSlot->SetAlignment(FVector2D(bRightAlign ? 1.0f : 0.0f, 0.5f));
        
        // 위치 설정 - 오른쪽 정렬인 경우 X가 음수여야 함
        float AdjustedX = bRightAlign ? -PosX : PosX;
        TextSlot->SetPosition(FVector2D(AdjustedX, PosY));
        
        // 크기 설정
        TextSlot->SetSize(FVector2D(Width, Height));
    }
    
    return TextSlot;
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