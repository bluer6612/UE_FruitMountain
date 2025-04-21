#include "ScoreManagerComponent.h"
#include "Framework/UE_FruitMountainGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/UI/TextureDisplayWidget.h"
#include "Interface/UI/ScoreDisplayWidget.h"

UScoreManagerComponent::UScoreManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // 기본값 초기화
    CurrentScore = 0;
    ComboCount = 0;
    ComboRemainingTime = 0.0f;
    bComboActive = false;
    
    // 위젯 초기화
    ScoreWidgetInstance = nullptr;
    bWidgetCreated = false;
}

void UScoreManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 게임 시작 시 위젯 확인
    if (!ScoreWidgetInstance && GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("BeginPlay: 위젯 초기화 시작"));
        
        // 이미 생성된 정적 인스턴스가 있는지 확인
        ScoreWidgetInstance = UScoreDisplayWidget::Instance;
        
        // 없으면 새로 생성 시도
        if (!ScoreWidgetInstance)
        {
            ScoreWidgetInstance = UScoreDisplayWidget::CreateScoreWidget(GetWorld());
            if (ScoreWidgetInstance)
            {
                UE_LOG(LogTemp, Display, TEXT("BeginPlay: 점수 위젯 초기 생성 완료"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("BeginPlay: 위젯 생성 실패 - 블루프린트를 확인하세요"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Display, TEXT("BeginPlay: 기존 위젯 인스턴스 사용"));
        }
    }
}

void UScoreManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // 콤보 시간 업데이트
    if (bComboActive)
    {
        ComboRemainingTime -= DeltaTime;
        
        // 콤보 타임 종료
        if (ComboRemainingTime <= 0.0f)
        {
            UE_LOG(LogTemp, Display, TEXT("콤보 시간 종료! 최종 콤보 카운트: %d"), ComboCount);
            
            // 콤보 종료 이벤트 발생
            OnComboEnded.Broadcast(ComboCount);
            
            // 콤보 초기화
            ResetCombo();
        }
    }
}

int32 UScoreManagerComponent::AddScore(int32 BallType)
{
    // 1. 기본 점수 계산
    int32 BaseScore = CalculateBaseScore(BallType);
    
    // 2. 콤보 상태 확인 및 업데이트
    bool bIsNewCombo = !bComboActive;
    
    if (bComboActive)
    {
        // 기존 콤보 연장
        ComboCount++;
        ExtendComboTime();
    }
    else
    {
        // 새 콤보 시작
        ComboCount = 1;
        bComboActive = true;
        ExtendComboTime();
    }
    
    // 3. 연쇄 보너스 계산
    float ComboMultiplier = CalculateComboMultiplier();
    
    // 4. 최종 점수 계산
    int32 FinalScore = FMath::RoundToInt(BaseScore * ComboMultiplier);
    
    // 5. 점수 추가
    CurrentScore += FinalScore;
    
    // 6. 로그 출력
    if (ComboCount >= 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("%d연쇄 병합! 기본점수: %d, 보너스율: %.1f배, 최종점수: %d"),
               ComboCount, BaseScore, ComboMultiplier, FinalScore);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("과일 병합 점수: %d (레벨 %d)"), FinalScore, BallType);
    }
    
    // 7. 이벤트 발생
    OnScoreAdded.Broadcast(FinalScore, ComboCount, ComboMultiplier);
    
    // UI에 점수 표시 - 간소화된 방법
    if (!ScoreWidgetInstance)
    {
        // 정적 인스턴스가 있는지 확인만 함
        ScoreWidgetInstance = UScoreDisplayWidget::Instance;
        
        // 로그만 출력하고 위젯 없이도 계속 진행
        if (!ScoreWidgetInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("점수 위젯이 없음 - 점수만 누적"));
        }
    }
    
    // 위젯이 있을 때만 점수 표시 시도
    if (IsValid(ScoreWidgetInstance))
    {
        UE_LOG(LogTemp, Display, TEXT("점수 표시: %d점 (콤보 %d, 배율 %.1f)"), 
               FinalScore, ComboCount, ComboMultiplier);
        
        ScoreWidgetInstance->DisplayScoreGain(FinalScore, ComboCount, ComboMultiplier);
    }
    
    return FinalScore;
}

void UScoreManagerComponent::ResetCombo()
{
    ComboCount = 0;
    ComboRemainingTime = 0.0f;
    bComboActive = false;
}

void UScoreManagerComponent::ExtendComboTime()
{
    ComboRemainingTime = ComboTimeLimit;
    bComboActive = true;
}

int32 UScoreManagerComponent::CalculateBaseScore(int32 BallType) const
{
    // 등차수열의 합 공식: n*(n+1)/2
    return (BallType * (BallType + 1)) / 2;
}

float UScoreManagerComponent::CalculateComboMultiplier() const
{
    if (ComboCount < 2)
        return 1.0f;
    
    // 연쇄 보너스 계산 (2연쇄: 1.1배, 4연쇄: 1.2배, 6연쇄: 1.3배, ...)
    int32 BonusTiers = ComboCount / 2;
    return 1.0f + (BonusTiers * 0.1f);
}

// 정적 헬퍼 함수 구현
void UScoreManagerComponent::AddScoreStatic(UWorld* World, int32 BallType)
{
    if (!World) return;
    
    // 게임모드에서 ScoreManagerComponent 찾기
    AUE_FruitMountainGameMode* GameMode = Cast<AUE_FruitMountainGameMode>(UGameplayStatics::GetGameMode(World));
    if (!GameMode) 
    {
        UE_LOG(LogTemp, Error, TEXT("AddScoreStatic: 게임모드를 찾을 수 없음"));
        return;
    }
    
    // 컴포넌트 가져오기
    UScoreManagerComponent* ScoreManager = GameMode->FindComponentByClass<UScoreManagerComponent>();
    
    // 없으면 생성 및 등록
    if (!ScoreManager)
    {
        UE_LOG(LogTemp, Display, TEXT("AddScoreStatic: ScoreManagerComponent 새로 생성"));
        ScoreManager = NewObject<UScoreManagerComponent>(GameMode, UScoreManagerComponent::StaticClass());
        ScoreManager->RegisterComponent();
        
        // 컴포넌트 생성 직후 위젯도 초기화
        ScoreManager->ScoreWidgetInstance = UScoreDisplayWidget::CreateScoreWidget(World);
        ScoreManager->bWidgetCreated = true;
    }
    
    // 점수 추가 실행
    ScoreManager->AddScore(BallType);
}

void UScoreManagerComponent::ResetComboStatic(UWorld* World)
{
    if (!World) return;
    
    AUE_FruitMountainGameMode* GameMode = Cast<AUE_FruitMountainGameMode>(UGameplayStatics::GetGameMode(World));
    if (!GameMode) return;
    
    UScoreManagerComponent* ScoreManager = GameMode->FindComponentByClass<UScoreManagerComponent>();
    if (ScoreManager)
    {
        ScoreManager->ResetCombo();
    }
}