#include "ScoreManagerComponent.h"
#include "Framework/UE_FruitMountainGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/UI/ScoreDisplayWidget.h"
#include "Interface/UI/TotalScoreWidget.h"
#include "Interface/UI/ScoreWidgetAnimator.h"
#include "ComboSystem.h"

UScoreManagerComponent::UScoreManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // 기본값 초기화
    CurrentScore = 0;
    TotalScore = 0;
    
    // 위젯 초기화
    ScoreWidgetInstance = nullptr;
    TotalScoreWidgetInstance = nullptr;
    bWidgetCreated = false;
    
    // 콤보 시스템은 BeginPlay에서 초기화
    ComboSystem = nullptr;
}

void UScoreManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 게임 시작 시 위젯 확인
    if (!ScoreWidgetInstance && GetWorld())
    {
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
    }
    
    // 총점 위젯 확인
    if (!TotalScoreWidgetInstance && GetWorld())
    {
        // 이미 생성된 정적 인스턴스가 있는지 확인
        TotalScoreWidgetInstance = UTotalScoreWidget::Instance;
        
        // 없으면 새로 생성 시도
        if (!TotalScoreWidgetInstance)
        {
            TotalScoreWidgetInstance = UTotalScoreWidget::CreateTotalScoreWidget(GetWorld());
            if (TotalScoreWidgetInstance)
            {
                UE_LOG(LogTemp, Display, TEXT("BeginPlay: 총점 위젯 초기 생성 완료"));
            }
        }
    }
    
    // 콤보 시스템 초기화
    InitializeComboSystem();
}

void UScoreManagerComponent::InitializeComboSystem()
{
    // 콤보 시스템 생성 - RF_Transactional 플래그 추가로 객체 생존 보장
    ComboSystem = NewObject<UComboSystem>(this, UComboSystem::StaticClass(), NAME_None, RF_Transactional);
    if (!ComboSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("콤보 시스템 생성 실패"));
        return;
    }
    
    // 콤보 시스템 초기화
    ComboSystem->Initialize(this, ScoreWidgetInstance);
    ComboSystem->SetComboTimeLimit(ComboTimeLimit);
    
    // 이벤트 연결 전 디버깅 로그
    UE_LOG(LogTemp, Display, TEXT("콤보 시스템 이벤트 등록 시작"));
    
    // 콤보 시스템 이벤트 등록 - 바인딩 전 객체 유효성 재확인
    if (IsValid(ComboSystem))
    {
        ComboSystem->OnComboScoreFinalized.AddDynamic(this, &UScoreManagerComponent::OnComboScoreFinalized);
        ComboSystem->OnComboUpdated.AddDynamic(this, &UScoreManagerComponent::OnComboUpdated);
        
        // 바인딩 성공 여부 확인
        bool bScoreFinalized = ComboSystem->OnComboScoreFinalized.IsBound();
        bool bComboUpdated = ComboSystem->OnComboUpdated.IsBound();
        
        UE_LOG(LogTemp, Display, TEXT("콤보 이벤트 바인딩 결과: OnComboScoreFinalized=%s, OnComboUpdated=%s"), 
              bScoreFinalized ? TEXT("성공") : TEXT("실패"),
              bComboUpdated ? TEXT("성공") : TEXT("실패"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("이벤트 등록 실패: 콤보 시스템이 유효하지 않음"));
    }
}

void UScoreManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // 콤보 시스템 틱 업데이트
    if (ComboSystem)
    {
        ComboSystem->Tick(DeltaTime);
    }
}

void UScoreManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // ComboSystem 정리 - RemoveFromRoot 대신 안전한 정리 함수 호출
    if (ComboSystem && IsValid(ComboSystem))
    {
        ComboSystem->SafeCleanup();
        ComboSystem = nullptr;
        UE_LOG(LogTemp, Display, TEXT("EndPlay: ComboSystem 안전하게 정리됨"));
    }
}

int32 UScoreManagerComponent::AddScore(int32 BallType)
{
    if (!ComboSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboSystem이 초기화되지 않았습니다."));
        return 0;
    }
    
    // 콤보 시스템에 점수 추가 요청
    int32 FinalScore = ComboSystem->CalculateFinalScore(BallType);
    
    // 현재 누적 점수 업데이트
    CurrentScore += FinalScore;
    
    return FinalScore;
}

void UScoreManagerComponent::OnComboScoreFinalized(int32 FinalComboScore)
{
    // 콤보 점수를 총점에 반영
    if (FinalComboScore > 0 && TotalScoreWidgetInstance)
    {
        // 기존 총점에 콤보 점수 더하기
        int32 NewTotalScore = TotalScore + FinalComboScore;
        TotalScore = NewTotalScore; // 내부 변수도 업데이트
        
        // 애니메이션과 함께 총점 업데이트
        TotalScoreWidgetInstance->AnimateScoreIncrease(NewTotalScore);
    }
}

void UScoreManagerComponent::OnComboUpdated(int32 ComboCount, float ComboMultiplier)
{
    // 콤보 점수는 별도로 계산하거나 필요 없는 경우 0으로 설정
    int32 ComboScore = 0; // 필요에 따라 계산 로직 구현
    
    // 점수 추가 이벤트 발생 (UI나 다른 시스템에서 활용할 수 있음)
    OnScoreAdded.Broadcast(ComboScore, ComboCount, ComboMultiplier);
}

// 정적 헬퍼 함수 구현
void UScoreManagerComponent::AddScoreStatic(UWorld* World, int32 BallType)
{
    if (!World) 
    {
        return;
    }
    
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
        ScoreManager->TotalScoreWidgetInstance = UTotalScoreWidget::CreateTotalScoreWidget(World);
        ScoreManager->bWidgetCreated = true;
        
        // 콤보 시스템 초기화
        ScoreManager->InitializeComboSystem();
    }
    
    // 점수 추가 실행
    ScoreManager->AddScore(BallType);
}

void UScoreManagerComponent::ResetComboStatic(UWorld* World)
{
    if (!World) 
    {
        return;
    }
    
    AUE_FruitMountainGameMode* GameMode = Cast<AUE_FruitMountainGameMode>(UGameplayStatics::GetGameMode(World));
    if (!GameMode) 
    {
        return;
    }
    
    UScoreManagerComponent* ScoreManager = GameMode->FindComponentByClass<UScoreManagerComponent>();
    if (ScoreManager && ScoreManager->ComboSystem)
    {
        ScoreManager->ComboSystem->ResetCombo();
    }
}

void UScoreManagerComponent::AddToTotalScore(int32 ScoreToAdd)
{
    // 총점 내부 업데이트
    TotalScore += ScoreToAdd;
    
    // TotalScoreWidget을 통해 총점 UI 업데이트
    if (!TotalScoreWidgetInstance)
    {
        TotalScoreWidgetInstance = UTotalScoreWidget::Instance;
    }
    
    if (TotalScoreWidgetInstance)
    {
        TotalScoreWidgetInstance->UpdateTotalScore(TotalScore);
    }
}