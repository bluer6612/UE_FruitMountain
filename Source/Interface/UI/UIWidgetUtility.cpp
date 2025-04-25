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
FString UUIWidgetUtility::CurrentFontPath = UUIWidgetUtility::DEFAULT_FONT_PATH;

// 글로벌 폰트 설정 함수
void UUIWidgetUtility::SetGlobalFont(const FString& FontPath)
{
    if (!FontPath.IsEmpty())
    {
        CurrentFontPath = FontPath;
        UE_LOG(LogTemp, Log, TEXT("글로벌 폰트 경로가 설정됨: %s"), *CurrentFontPath);
    }
    else
    {
        CurrentFontPath = DEFAULT_FONT_PATH;
    }
}

// 폰트 적용 함수
void UUIWidgetUtility::ApplyFontToTextBlock(UTextBlock* TextBlock, const FString& FontPath, int32 FontSize, FName TypefaceName)
{
    if (!TextBlock)
    {
        return;
    }
    
    // 폰트 경로 결정
    FString EffectiveFontPath = FontPath.IsEmpty() ? CurrentFontPath : FontPath;
    
    // 폰트 로드
    UFont* FontObject = LoadObject<UFont>(nullptr, *EffectiveFontPath);
    if (!FontObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("폰트 로드 실패: %s"), *EffectiveFontPath);
        return;
    }
    
    // 기존 폰트 정보 가져오기
    FSlateFontInfo CurrentFont = TextBlock->GetFont();
    
    // 새 폰트 정보 설정
    FSlateFontInfo NewFontInfo;
    NewFontInfo.FontObject = FontObject;
    
    // 폰트 크기 결정 (0이면 기존 크기 유지)
    NewFontInfo.Size = FontSize > 0 ? FontSize : CurrentFont.Size;
    
    // 폰트 스타일 설정 (Typeface)
    if (!TypefaceName.IsNone())
    {
        NewFontInfo.TypefaceFontName = TypefaceName;
    }
    else if (!CurrentFont.TypefaceFontName.IsNone())
    {
        NewFontInfo.TypefaceFontName = CurrentFont.TypefaceFontName;
    }
    
    // 폰트 적용
    TextBlock->SetFont(NewFontInfo);
    UE_LOG(LogTemp, Display, TEXT("텍스트 블록에 폰트 적용: %s (크기: %d)"), *EffectiveFontPath, NewFontInfo.Size);
}

// 모든 텍스트 블록에 폰트 적용 (재귀적)
void UUIWidgetUtility::ApplyFontToAllTextBlocks(UWidget* RootWidget, const FString& FontPath, int32 DefaultFontSize)
{
    if (!RootWidget)
    {
        return;
    }
    
    // 텍스트 블록인 경우 폰트 적용
    UTextBlock* TextBlock = Cast<UTextBlock>(RootWidget);
    if (TextBlock)
    {
        ApplyFontToTextBlock(TextBlock, FontPath, DefaultFontSize);
    }
    
    // 패널 위젯인 경우 모든 자식에게 재귀 적용
    UPanelWidget* PanelWidget = Cast<UPanelWidget>(RootWidget);
    if (PanelWidget)
    {
        for (int32 i = 0; i < PanelWidget->GetChildrenCount(); i++)
        {
            UWidget* ChildWidget = PanelWidget->GetChildAt(i);
            if (ChildWidget)
            {
                ApplyFontToAllTextBlocks(ChildWidget, FontPath, DefaultFontSize);
            }
        }
    }
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

void UUIWidgetUtility::SetupTextBlockStyle(
    UTextBlock* TextBlock, 
    FLinearColor Color,
    int32 FontSize,
    bool bBold,
    bool bAutoWrapText,
    ESlateVisibility DefaultVisibility)
{
    if (!TextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("SetupTextBlockStyle: TextBlock이 null입니다."));
        return;
    }
    
    // 커스텀 폰트 적용
    ApplyFontToTextBlock(TextBlock, TEXT(""), FontSize, bBold ? TEXT("Bold") : NAME_None);
    
    // 색상 설정
    TextBlock->SetColorAndOpacity(Color);
    
    // 자동 줄바꿈 설정
    TextBlock->SetAutoWrapText(bAutoWrapText);
    
    // 기본 가시성 설정
    TextBlock->SetVisibility(DefaultVisibility);
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