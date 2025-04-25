#include "PlayStartSequenceWidget.h"
#include "Interface/UI/Core/UIWidgetUtility.h"
#include "Engine/Texture2D.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"

// 정적 인스턴스 초기화
UPlayStartSequenceWidget* UPlayStartSequenceWidget::Instance = nullptr;

UPlayStartSequenceWidget::UPlayStartSequenceWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentState(ESequenceState::Inactive)
    , AnimationTimer(0.0f)
{
}

void UPlayStartSequenceWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 인스턴스 설정
    Instance = this;
    
    // 텍스처 로드
    LoadTextures();
    
    // 초기 설정
    if (ReadyImage)
    {
        ReadyImage->SetVisibility(ESlateVisibility::Hidden);
        ReadyImage->SetRenderOpacity(1.0f);
        
        // 이미지를 화면 중앙에 배치
        UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(ReadyImage->Slot);
        if (ImageSlot)
        {
            UUIWidgetUtility::SetAnchorForSlot(ImageSlot, EWidgetAnchor::Center, 0.0f, 0.0f);
        }
    }
    
    if (StartImage)
    {
        StartImage->SetVisibility(ESlateVisibility::Hidden);
        StartImage->SetRenderOpacity(1.0f);
        
        // 이미지를 화면 중앙에 배치
        UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(StartImage->Slot);
        if (ImageSlot)
        {
            UUIWidgetUtility::SetAnchorForSlot(ImageSlot, EWidgetAnchor::Center, 0.0f, 0.0f);
        }
    }
}

void UPlayStartSequenceWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    if (CurrentState == ESequenceState::Inactive || CurrentState == ESequenceState::Completed)
    {
        return;
    }
    
    // 애니메이션 타이머 업데이트
    AnimationTimer += InDeltaTime;
    float ProgressRatio = 0.0f;
    
    // 현재 상태에 따른 애니메이션 처리
    switch (CurrentState)
    {
        case ESequenceState::ReadyShrinking:
            ProgressRatio = FMath::Clamp(AnimationTimer / ReadyShrinkTime, 0.0f, 1.0f);
            if (ReadyImage)
            {
                // 1.5배 -> 1.0배로 크기 변화
                float CurrentScale = MaxReadyScaleFactor - (MaxReadyScaleFactor - 1.0f) * ProgressRatio;
                ReadyImage->SetRenderScale(FVector2D(CurrentScale, CurrentScale));
            }
            
            // 애니메이션 완료 체크
            if (AnimationTimer >= ReadyShrinkTime)
            {
                SetSequenceState(ESequenceState::ReadyFadingOut);
            }
            break;
            
        case ESequenceState::ReadyFadingOut:
            ProgressRatio = FMath::Clamp(AnimationTimer / ReadyFadeOutTime, 0.0f, 1.0f);
            if (ReadyImage)
            {
                // 페이드 아웃 (1.0 -> 0.0)
                ReadyImage->SetRenderOpacity(1.0f - ProgressRatio);
            }
            
            // StartImage 표시 시작
            if (AnimationTimer <= 0.1f && StartImage)
            {
                StartImage->SetVisibility(ESlateVisibility::Visible);
                StartImage->SetRenderOpacity(1.0f);
                StartImage->SetRenderScale(FVector2D(1.0f, 1.0f));
            }
            
            // 애니메이션 완료 체크
            if (AnimationTimer >= ReadyFadeOutTime)
            {
                if (ReadyImage)
                {
                    ReadyImage->SetVisibility(ESlateVisibility::Hidden);
                }
                SetSequenceState(ESequenceState::StartGrowing);
            }
            break;
            
        case ESequenceState::StartGrowing:
            ProgressRatio = FMath::Clamp(AnimationTimer / StartGrowTime, 0.0f, 1.0f);
            if (StartImage)
            {
                // 1.0배 -> 1.5배로 크기 변화
                float CurrentScale = 1.0f + (MaxReadyScaleFactor - 1.0f) * ProgressRatio;
                StartImage->SetRenderScale(FVector2D(CurrentScale, CurrentScale));
            }
            
            // 애니메이션 완료 체크
            if (AnimationTimer >= StartGrowTime)
            {
                SetSequenceState(ESequenceState::StartFadingOut);
            }
            break;
            
        case ESequenceState::StartFadingOut:
            ProgressRatio = FMath::Clamp(AnimationTimer / StartFadeOutTime, 0.0f, 1.0f);
            if (StartImage)
            {
                // 페이드 아웃 (1.0 -> 0.0)
                StartImage->SetRenderOpacity(1.0f - ProgressRatio);
            }
            
            // 애니메이션 완료 체크
            if (AnimationTimer >= StartFadeOutTime)
            {
                if (StartImage)
                {
                    StartImage->SetVisibility(ESlateVisibility::Hidden);
                }
                
                // 애니메이션 시퀀스 완료
                SetSequenceState(ESequenceState::Completed);
                
                // 완료 이벤트 호출
                OnSequenceFinished.Broadcast();
                
                UE_LOG(LogTemp, Display, TEXT("게임 시작 애니메이션 시퀀스 완료"));
            }
            break;
    }
}

void UPlayStartSequenceWidget::LoadTextures()
{
    // Ready 텍스처 로드
    UTexture2D* ReadyTexture = LoadObject<UTexture2D>(nullptr, *ReadyTexturePath);
    if (ReadyTexture && ReadyImage)
    {
        ReadyImage->SetBrushFromTexture(ReadyTexture);
        UE_LOG(LogTemp, Display, TEXT("Ready 텍스처 로드 완료: %s"), *ReadyTexturePath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Ready 텍스처 로드 실패: %s"), *ReadyTexturePath);
    }
    
    // Start 텍스처 로드
    UTexture2D* StartTexture = LoadObject<UTexture2D>(nullptr, *StartTexturePath);
    if (StartTexture && StartImage)
    {
        StartImage->SetBrushFromTexture(StartTexture);
        UE_LOG(LogTemp, Display, TEXT("Start 텍스처 로드 완료: %s"), *StartTexturePath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Start 텍스처 로드 실패: %s"), *StartTexturePath);
    }
}

void UPlayStartSequenceWidget::StartSequence()
{
    if (CurrentState != ESequenceState::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("시작 시퀀스가 이미 실행 중입니다."));
        return;
    }
    
    // Ready 이미지 표시 준비
    if (ReadyImage)
    {
        ReadyImage->SetVisibility(ESlateVisibility::Visible);
        ReadyImage->SetRenderOpacity(1.0f);
        ReadyImage->SetRenderScale(FVector2D(MaxReadyScaleFactor, MaxReadyScaleFactor));
    }
    
    // Start 이미지 초기화
    if (StartImage)
    {
        StartImage->SetVisibility(ESlateVisibility::Hidden);
        StartImage->SetRenderOpacity(1.0f);
        StartImage->SetRenderScale(FVector2D(1.0f, 1.0f));
    }
    
    // 애니메이션 시작
    SetSequenceState(ESequenceState::ReadyShrinking);
    
    UE_LOG(LogTemp, Display, TEXT("게임 시작 애니메이션 시퀀스 시작"));
}

void UPlayStartSequenceWidget::SetSequenceState(ESequenceState NewState)
{
    CurrentState = NewState;
    AnimationTimer = 0.0f;
}

UPlayStartSequenceWidget* UPlayStartSequenceWidget::GetInstance()
{
    return Instance;
}

UPlayStartSequenceWidget* UPlayStartSequenceWidget::CreatePlayStartSequence(UObject* WorldContextObject)
{
    // 이미 인스턴스가 있고 유효하면 반환
    if (Instance && IsValid(Instance) && Instance->IsInViewport())
    {
        return Instance;
    }
    
    // 플레이어 컨트롤러 가져오기
    APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("CreatePlayStartSequence: 유효한 플레이어 컨트롤러가 없습니다."));
        return nullptr;
    }
    
    // 위젯 생성
    UPlayStartSequenceWidget* Widget = CreateWidget<UPlayStartSequenceWidget>(PC, UPlayStartSequenceWidget::StaticClass());
    if (!Widget)
    {
        UE_LOG(LogTemp, Error, TEXT("CreatePlayStartSequence: 위젯 생성 실패"));
        return nullptr;
    }
    
    // 뷰포트에 추가
    Widget->AddToViewport(100); // 최상위 Z-순서로 표시
    
    return Widget;
}