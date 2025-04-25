#include "ComboCountWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "Interface/UI/Core/UIWidgetUtility.h"
#include "TimerManager.h"

// 정적 멤버 초기화
UComboCountWidget* UComboCountWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UComboCountWidget::ComboCountWidgetClass = nullptr;

// 위치 및 색상 상수 수정
const FVector2D UComboCountWidget::COMBO_IMAGE_POS = FVector2D(0.0f, 150.0f); // Y축으로 150픽셀 내려옴
const FVector2D UComboCountWidget::COMBO_TEXT_POS = FVector2D(-30.0f, 0.f); // 이미지 기준 상대 위치
const FLinearColor UComboCountWidget::COMBO_TEXT_COLOR = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // 흰색

// 생성자에 멤버 변수 초기화 추가
UComboCountWidget::UComboCountWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CurrentComboCount = 0;
    bAnimating = false;
    bFadingOut = false;
    FadeOutDuration = 0.5f; // 페이드 아웃 지속 시간
}

void UComboCountWidget::InitializeComboWidgets()
{
    // 루트 패널 찾기
    UPanelWidget* RootPanel = Cast<UPanelWidget>(GetRootWidget());
    if (!RootPanel)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: 루트 패널을 찾을 수 없습니다."));
        return;
    }
    
    // 이미지 위젯 C++에서 직접 생성
    if (!ComboImage)
    {
        // 이미지 로드
        UTexture2D* ComboTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/PlayLevel/UI_Play_ComboCount"));
        if (!ComboTexture)
        {
            UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: 콤보 텍스처 로드 실패"));
            return;
        }
        
        // 이미지 위젯 생성
        ComboImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        if (ComboImage)
        {
            // 텍스처 브러시 설정
            FSlateBrush Brush;
            Brush.SetResourceObject(ComboTexture);
            Brush.DrawAs = ESlateBrushDrawType::Image;
            Brush.ImageSize = FVector2D(353.0f, 78.0f);
            ComboImage->SetBrush(Brush);
            ComboImage->SetColorAndOpacity(FLinearColor::White);
            
            // 패널에 추가
            UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(RootPanel->AddChild(ComboImage));
            if (ImageSlot)
            {
                // 화면 상단 중앙에 앵커 설정
                ImageSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
                ImageSlot->SetAlignment(FVector2D(0.5f, 0.0f));
                ImageSlot->SetPosition(COMBO_IMAGE_POS);
                ImageSlot->SetSize(FVector2D(353.0f, 78.0f));
                
                UE_LOG(LogTemp, Display, TEXT("콤보 이미지 생성 성공 (C++ 방식)"));
            }
        }
    }
    
    // 텍스트 블록은 블루프린트에서 바인딩될 수 있지만 없으면 생성
    if (!ComboCountTextBlock)
    {
        ComboCountTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (ComboCountTextBlock)
        {
            // 스타일 설정
            UUIWidgetUtility::SetupTextBlockStyle(
                ComboCountTextBlock,
                COMBO_TEXT_COLOR,
                56.0f,
                UUIWidgetUtility::DEFAULT_NUMBER_FONT_PATH,
                true,
                false,
                ESlateVisibility::HitTestInvisible
            );
            
            // 텍스트 초기화
            ComboCountTextBlock->SetText(FText::FromString(TEXT("0")));
            
            // 이미지 위에 배치
            UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(RootPanel->AddChild(ComboCountTextBlock));
            if (TextSlot)
            {
                TextSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
                TextSlot->SetAlignment(FVector2D(0.5f, 0.0f));
                FVector2D TextPos = COMBO_IMAGE_POS + FVector2D(0, -30.0f);
                TextSlot->SetPosition(TextPos);
                TextSlot->SetSize(FVector2D(100.0f, 60.0f));
                TextSlot->SetZOrder(10); // 이미지 위에 표시
                
                UE_LOG(LogTemp, Display, TEXT("콤보 텍스트 위치 설정: Y=%f"), TextPos.Y);
            }
        }
    }
    else
    {
        // 블루프린트에서 바인딩된 텍스트 블록 스타일 설정
        UUIWidgetUtility::SetupTextBlockStyle(
            ComboCountTextBlock,
            COMBO_TEXT_COLOR,
            56.0f,
            UUIWidgetUtility::DEFAULT_NUMBER_FONT_PATH,
            true,
            false,
            ESlateVisibility::HitTestInvisible
        );
    }
    
    // 초기 숨김 상태
    SetComboCountVisibility(false);
    
    UE_LOG(LogTemp, Display, TEXT("ComboCountWidget 초기화 완료"));
}

void UComboCountWidget::UpdateComboCount(int32 NewComboCount)
{
    // 이전 콤보 카운트와 다르면 애니메이션 재생
    if (CurrentComboCount != NewComboCount)
    {
        CurrentComboCount = NewComboCount;
        PlayComboAnimation(CurrentComboCount);
    }
    
    // 콤보가 2 이상일 때만 보이게
    if (CurrentComboCount >= 2)
    {
        SetComboCountVisibility(true);
        
        // 텍스트 업데이트
        if (ComboCountTextBlock)
        {
            ComboCountTextBlock->SetText(FText::AsNumber(CurrentComboCount));
        }
    }
    else
    {
        SetComboCountVisibility(false);
    }
}

void UComboCountWidget::SetComboCountVisibility(bool bVisible)
{
    ESlateVisibility InVisibility = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
    
    if (ComboImage)
    {
        ComboImage->SetVisibility(InVisibility);
    }
    
    if (ComboCountTextBlock)
    {
        ComboCountTextBlock->SetVisibility(InVisibility);
    }
}

void UComboCountWidget::PlayComboAnimation(int32 ComboCount)
{
    // 이미 애니메이션 중이거나 페이드 아웃 중이면 취소
    if ((bAnimating || bFadingOut) && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboAnimTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);
        bAnimating = false;
        bFadingOut = false;
    }
    
    // 콤보가 없으면 애니메이션 필요 없음
    if (ComboCount < 2)
    {
        return;
    }
    
    // 애니메이션 시작
    bAnimating = false; // 크기 애니메이션 없으므로 false로 설정
    
    // 초기 상태 설정 - 크기 애니메이션 없이 기본 크기로 표시
    if (ComboCountTextBlock)
    {
        ComboCountTextBlock->SetRenderScale(FVector2D(1.0f, 1.0f)); // 기본 크기
        ComboCountTextBlock->SetRenderOpacity(1.0f);
    }
    
    if (ComboImage)
    {
        ComboImage->SetRenderScale(FVector2D(1.0f, 1.0f));
        ComboImage->SetRenderOpacity(1.0f);
    }
}

void UComboCountWidget::ResetComboCount()
{
    CurrentComboCount = 0;
    
    // 즉시 숨기는 대신 페이드 아웃 애니메이션 실행
    PlayFadeOutAnimation();
    
    // 애니메이션 취소 코드는 필요 없으나 bAnimating 변수 초기화를 위해 유지
    if (bAnimating && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboAnimTimerHandle);
        bAnimating = false;
    }
}

bool UComboCountWidget::IsInstanceValid()
{
    return Instance && IsValid(Instance) && Instance->IsInViewport();
}

void UComboCountWidget::ClearInstance()
{
    if (Instance)
    {
        if (Instance->IsInViewport())
        {
            Instance->RemoveFromParent();
        }
        Instance = nullptr;
    }
}

UComboCountWidget* UComboCountWidget::CreateComboCountWidget(UObject* WorldContextObject)
{
    // 이미 유효한 인스턴스가 있는지 확인
    if (Instance && IsValid(Instance) && Instance->IsInViewport())
    {
        return Instance;
    }
    
    // 기존 인스턴스 정리
    if (Instance)
    {
        Instance = nullptr;
    }
    
    // 위젯 클래스 로드
    if (!LoadWidgetClassIfNeeded())
    {
        UE_LOG(LogTemp, Error, TEXT("ComboCountWidget 클래스 로드 실패"));
        return nullptr;
    }
    
    APlayerController* Controller = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    if (!Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: 유효한 플레이어 컨트롤러가 없습니다"));
        return nullptr;
    }
    
    // 위젯 생성 및 뷰포트에 추가
    Instance = CreateWidget<UComboCountWidget>(Controller, ComboCountWidgetClass);
    if (Instance)
    {
        Instance->AddToViewport(10002); // 점수 위젯보다 높은 Z-Order
        UE_LOG(LogTemp, Display, TEXT("ComboCountWidget 생성 완료"));
    }
    
    return Instance;
}

// 페이드 아웃 애니메이션 함수 추가
void UComboCountWidget::PlayFadeOutAnimation()
{
    // 이미 페이드 아웃 중이면 중복 실행 방지
    if (bFadingOut)
        return;
        
    bFadingOut = true;
    
    // 애니메이션 진행 중이면 취소
    if (bAnimating && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboAnimTimerHandle);
        bAnimating = false;
    }
    
    // 초기 상태 설정
    if (ComboCountTextBlock)
        ComboCountTextBlock->SetRenderOpacity(1.0f);
        
    if (ComboImage)
        ComboImage->SetRenderOpacity(1.0f);
    
    // 타이머로 페이드 아웃 실행
    FTimerDelegate FadeOutDelegate;
    FadeOutDelegate.BindUObject(this, &UComboCountWidget::ExecuteFadeOutStep);
    
    GetWorld()->GetTimerManager().SetTimer(
        FadeOutTimerHandle, 
        FadeOutDelegate, 
        0.016f, // ~60fps
        true
    );
    
    UE_LOG(LogTemp, Display, TEXT("콤보 카운트 페이드 아웃 시작"));
}

// 페이드 아웃 단계별 실행 함수
void UComboCountWidget::ExecuteFadeOutStep()
{
    static float ElapsedTime = 0.0f;
    
    if (!GetWorld())
        return;
        
    ElapsedTime += 0.016f;
    
    // 진행률 계산 (0.0 ~ 1.0)
    float Progress = FMath::Clamp(ElapsedTime / FadeOutDuration, 0.0f, 1.0f);
    
    // 불투명도 계산 (1.0 -> 0.0)
    float CurrentOpacity = 1.0f - Progress;
    
    // 위젯에 적용
    if (ComboCountTextBlock)
        ComboCountTextBlock->SetRenderOpacity(CurrentOpacity);
        
    if (ComboImage) 
        ComboImage->SetRenderOpacity(CurrentOpacity);
    
    // 페이드 아웃 완료
    if (Progress >= 1.0f)
    {
        // 타이머 중지
        GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);
        
        // 상태 초기화
        ElapsedTime = 0.0f;
        bFadingOut = false;
        
        // 위젯 완전히 숨기기
        SetComboCountVisibility(false);
        
        UE_LOG(LogTemp, Display, TEXT("콤보 카운트 페이드 아웃 완료"));
    }
}

void UComboCountWidget::BeginDestroy()
{
    // 인스턴스 참조 해제
    if (Instance == this)
    {
        Instance = nullptr;
    }
    
    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ComboAnimTimerHandle);
        World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
    }
    
    Super::BeginDestroy();
}

bool UComboCountWidget::LoadWidgetClassIfNeeded()
{
    if (!ComboCountWidgetClass)
    {
        ComboCountWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/PlayLevel/BP_UI_Play_ComboCount.BP_UI_Play_ComboCount_C"));
    }
    return ComboCountWidgetClass != nullptr;
}

void UComboCountWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 인스턴스 업데이트
    Instance = this;
    
    // 위젯 초기화 호출
    InitializeComboWidgets();
    
    UE_LOG(LogTemp, Display, TEXT("ComboCountWidget NativeConstruct 완료"));
}

void UComboCountWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 애니메이션 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ComboAnimTimerHandle);
        World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
    }
    
    // 인스턴스 참조 해제
    if (Instance == this)
    {
        Instance = nullptr;
    }
    
    UE_LOG(LogTemp, Display, TEXT("ComboCountWidget NativeDestruct 완료"));
}