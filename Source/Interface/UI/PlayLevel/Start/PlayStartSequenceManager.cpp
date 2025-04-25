#include "PlayStartSequenceManager.h"
#include "Interface/UI/Core/UIWidgetUtility.h"
#include "Engine/Engine.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"

// 정적 인스턴스 초기화
UPlayStartSequenceManager* UPlayStartSequenceManager::Instance = nullptr;

UPlayStartSequenceManager::UPlayStartSequenceManager()
    : CurrentStep(ESequenceStep::None)
    , ElapsedTime(0.0f)
    , TotalDuration(0.0f)
    , ReadyTexturePath(TEXT("/Game/UI/PlayLevel/UI_Play_Ready"))
    , StartTexturePath(TEXT("/Game/UI/PlayLevel/UI_Play_Start"))
    , MaxScaleFactor(1.5f)
    , WorldContextObject(nullptr)
{
}

void UPlayStartSequenceManager::StartSequence(UObject* InWorldContextObject)
{
    // 월드 컨텍스트 저장
    WorldContextObject = InWorldContextObject;
    
    // 이미 실행 중인 시퀀스가 있다면 정리
    if (WidgetRenderer)
    {
        APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
        if (PC)
        {
            PC->GetWorldTimerManager().ClearTimer(SequenceTimerHandle);
        }
    }
    
    // 위젯 생성
    CreateSequenceWidgets();
    
    // 시퀀스 초기화
    InitializeSequence();
    
    // 시퀀스 시작
    CurrentStep = ESequenceStep::ReadyShrink;
    ElapsedTime = 0.0f;
    TotalDuration = 1.5f; // Ready 줄어드는 시간
    
    // 타이머 설정
    APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
    if (PC)
    {
        PC->GetWorldTimerManager().SetTimer(SequenceTimerHandle, 
                                           this, 
                                           &UPlayStartSequenceManager::UpdateSequence, 
                                           0.016f, // 약 60fps
                                           true);
    }
    
    UE_LOG(LogTemp, Display, TEXT("PlayStartSequence: 시작 시퀀스 시작됨"));
}

void UPlayStartSequenceManager::CreateSequenceWidgets()
{
    // UIWidgetRenderer 생성
    WidgetRenderer = UUIWidgetRenderer::CreateDisplayWidget(WorldContextObject);
    if (!WidgetRenderer)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayStartSequence: UIWidgetRenderer 생성 실패"));
        return;
    }
    
    // Ready 이미지 생성
    ReadyImage = nullptr;
    WidgetRenderer->RenderUIImage(ReadyImage, 
                                 EWidgetAnchor::Center, 
                                 ReadyTexturePath, 
                                 FVector2D(0, 0),  // 원본 크기 사용
                                 0.0f, 0.0f);
    
    // Start 이미지 생성
    StartImage = nullptr;
    WidgetRenderer->RenderUIImage(StartImage, 
                                 EWidgetAnchor::Center, 
                                 StartTexturePath, 
                                 FVector2D(0, 0),  // 원본 크기 사용
                                 0.0f, 0.0f);
    
    // 초기 설정
    if (ReadyImage)
    {
        ReadyImage->SetRenderScale(FVector2D(MaxScaleFactor, MaxScaleFactor));
        ReadyImage->SetRenderOpacity(1.0f);
    }
    
    if (StartImage)
    {
        StartImage->SetVisibility(ESlateVisibility::Hidden);
    }
    
    UE_LOG(LogTemp, Display, TEXT("PlayStartSequence: 위젯 생성 완료"));
}

void UPlayStartSequenceManager::UpdateSequence()
{
    // 월드/플레이어 컨트롤러 확인
    APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayStartSequence: 유효한 PlayerController 없음"));
        return;
    }
    
    // 델타 타임 계산
    float DeltaTime = PC->GetWorld()->GetDeltaSeconds();
    ElapsedTime += DeltaTime;
    
    // 현재 단계에 따른 처리
    switch (CurrentStep)
    {
        case ESequenceStep::ReadyShrink:
            ProcessReadyShrink(DeltaTime);
            break;
            
        case ESequenceStep::ReadyFadeOut:
            ProcessReadyFadeOut(DeltaTime);
            break;
            
        case ESequenceStep::StartGrow:
            ProcessStartGrow(DeltaTime);
            break;
            
        case ESequenceStep::StartFadeOut:
            ProcessStartFadeOut(DeltaTime);
            break;
            
        case ESequenceStep::Complete:
        case ESequenceStep::None:
            // 아무것도 안함
            break;
    }
    
    // 단계 완료 확인
    if (ElapsedTime >= TotalDuration && CurrentStep != ESequenceStep::None && CurrentStep != ESequenceStep::Complete)
    {
        AdvanceToNextStep();
    }
}

void UPlayStartSequenceManager::ProcessReadyShrink(float DeltaTime)
{
    if (!ReadyImage) return;
    
    // 1.5배 -> 1.0배로 줄이는 애니메이션
    float Progress = FMath::Clamp(ElapsedTime / TotalDuration, 0.0f, 1.0f);
    float CurrentScale = MaxScaleFactor - ((MaxScaleFactor - 1.0f) * Progress);
    
    ReadyImage->SetRenderScale(FVector2D(CurrentScale, CurrentScale));
}

void UPlayStartSequenceManager::ProcessReadyFadeOut(float DeltaTime)
{
    if (!ReadyImage || !StartImage) return;
    
    // READY 페이드 아웃
    float Progress = FMath::Clamp(ElapsedTime / TotalDuration, 0.0f, 1.0f);
    float Opacity = 1.0f - Progress;
    
    ReadyImage->SetRenderOpacity(Opacity);
    
    // START 표시 시작
    if (ElapsedTime >= TotalDuration * 0.5f && StartImage->GetVisibility() == ESlateVisibility::Hidden)
    {
        StartImage->SetVisibility(ESlateVisibility::HitTestInvisible);
        StartImage->SetRenderOpacity(1.0f);
        StartImage->SetRenderScale(FVector2D(1.0f, 1.0f));
    }
}

void UPlayStartSequenceManager::ProcessStartGrow(float DeltaTime)
{
    if (!StartImage) return;
    
    // 1.0배 -> 1.5배로 키우는 애니메이션
    float Progress = FMath::Clamp(ElapsedTime / TotalDuration, 0.0f, 1.0f);
    float CurrentScale = 1.0f + ((MaxScaleFactor - 1.0f) * Progress);
    
    StartImage->SetRenderScale(FVector2D(CurrentScale, CurrentScale));
}

void UPlayStartSequenceManager::ProcessStartFadeOut(float DeltaTime)
{
    if (!StartImage) return;
    
    // START 페이드 아웃
    float Progress = FMath::Clamp(ElapsedTime / TotalDuration, 0.0f, 1.0f);
    float Opacity = 1.0f - Progress;
    
    StartImage->SetRenderOpacity(Opacity);
}

void UPlayStartSequenceManager::AdvanceToNextStep()
{
    // 다음 단계로 진행
    switch (CurrentStep)
    {
        case ESequenceStep::ReadyShrink:
            CurrentStep = ESequenceStep::ReadyFadeOut;
            TotalDuration = 0.5f; // Ready 페이드 아웃 시간
            break;
            
        case ESequenceStep::ReadyFadeOut:
            CurrentStep = ESequenceStep::StartGrow;
            TotalDuration = 1.5f; // Start 커지는 시간
            break;
            
        case ESequenceStep::StartGrow:
            CurrentStep = ESequenceStep::StartFadeOut;
            TotalDuration = 0.5f; // Start 페이드 아웃 시간
            break;
            
        case ESequenceStep::StartFadeOut:
            CurrentStep = ESequenceStep::Complete;
            CompleteSequence();
            break;
            
        default:
            break;
    }
    
    // 타이머 초기화
    ElapsedTime = 0.0f;
}

void UPlayStartSequenceManager::InitializeSequence()
{
    // Ready 이미지 초기화
    if (ReadyImage)
    {
        ReadyImage->SetRenderScale(FVector2D(MaxScaleFactor, MaxScaleFactor));
        ReadyImage->SetRenderOpacity(1.0f);
        ReadyImage->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    
    // Start 이미지 초기화
    if (StartImage)
    {
        StartImage->SetRenderScale(FVector2D(1.0f, 1.0f));
        StartImage->SetRenderOpacity(1.0f);
        StartImage->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayStartSequenceManager::CompleteSequence()
{
    // 타이머 중지
    APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
    if (PC)
    {
        PC->GetWorldTimerManager().ClearTimer(SequenceTimerHandle);
    }
    
    // 위젯 정리
    if (ReadyImage)
    {
        ReadyImage->SetVisibility(ESlateVisibility::Hidden);
    }
    
    if (StartImage)
    {
        StartImage->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // 완료 이벤트 발생
    OnSequenceCompleted.Broadcast();
    
    UE_LOG(LogTemp, Display, TEXT("PlayStartSequence: 시퀀스 완료"));
}

UPlayStartSequenceManager* UPlayStartSequenceManager::GetInstance()
{
    return Instance;
}

UPlayStartSequenceManager* UPlayStartSequenceManager::CreateInstance(UObject* WorldContextObject)
{
    if (!Instance)
    {
        Instance = NewObject<UPlayStartSequenceManager>();
        Instance->AddToRoot(); // 가비지 컬렉션 방지
    }
    
    return Instance;
}