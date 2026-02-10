#include "HJ_GameMode.h"
#include "HJ_GameState.h"
#include "HJ_Player.h"
#include "HJ_PlayerController.h"
#include "HJ_SpawnZombie.h"
#include "TimerManager.h"

AHJ_GameMode::AHJ_GameMode()
{
    GameStateClass = AHJ_GameState::StaticClass();
    DefaultPawnClass = AHJ_Player::StaticClass();
    PlayerControllerClass = AHJ_PlayerController::StaticClass();

    BaseZombieCount = 30;
    TimeBetweenWaves = 5.f; 
    ZombiesAlive = 0;
}

void AHJ_GameMode::BeginPlay()
{
    Super::BeginPlay();
    StartPreparation();
}

void AHJ_GameMode::StartPreparation()
{
    AHJ_GameState* GS = GetGameState<AHJ_GameState>();
    if (!GS) return;

    GS->SetBattleState(EBattleState::Preparation);

    // 다음 웨이브 데이터를 미리 조회해서 대기 시간을 결정합니다.
    float NextWaitTime = TimeBetweenWaves;
    if (WaveDataTable)
    {
        FString NextRowName = FString::FromInt(GS->CurrentWave + 1);
        FZombieSpawnRow* Row = WaveDataTable->FindRow<FZombieSpawnRow>(FName(*NextRowName), TEXT(""));
        if (Row)
        {
            NextWaitTime = Row->PreparationTime; // 테이블에 설정된 시간 적용
        }
    }

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(
        TimerHandle,
        this,
        &AHJ_GameMode::StartWave,
        NextWaitTime,
        false
    );
}

void AHJ_GameMode::StartWave()
{
    AHJ_GameState* GS = GetGameState<AHJ_GameState>();
    if (!GS || !WaveDataTable) return;

    FString RowName = FString::FromInt(GS->CurrentWave + 1);
    FZombieSpawnRow* Row = WaveDataTable->FindRow<FZombieSpawnRow>(FName(*RowName), TEXT(""));

    if (Row)
    {
        GS->SetBattleState(EBattleState::InBattle);
        GS->SetWave(GS->CurrentWave + 1);

        ZombiesAlive = Row->ZombieCount;
        GS->SetZombiesRemaining(ZombiesAlive);

        if (SpawnZombieActor)
        {
            SpawnZombieActor->SpawnZombies(ZombiesAlive, Row->ZombieClass);
        }
    }
}

void AHJ_GameMode::OnZombieKilled()
{
    ZombiesAlive--;

    AHJ_GameState* GS = GetGameState<AHJ_GameState>();
    if (!GS) return;

    GS->SetZombiesRemaining(ZombiesAlive);

    if (ZombiesAlive <= 0)
    {
        EndWave();
    }
}

void AHJ_GameMode::EndWave()
{
    AHJ_GameState* GS = GetGameState<AHJ_GameState>();
    if (!GS || !WaveDataTable) return;

    // 다음 웨이브 데이터가 더 있는지 체크
    FString NextRowName = FString::FromInt(GS->CurrentWave + 1);
    FZombieSpawnRow* NextRow = WaveDataTable->FindRow<FZombieSpawnRow>(FName(*NextRowName), TEXT(""));

    if (NextRow)
    {
        // 다음 웨이브가 있다면 준비 단계로
        StartPreparation();
    }
    else
    {
        // 더 이상 웨이브 정보가 없다면 최종 승리!
        GS->SetBattleState(EBattleState::Victory);
        UE_LOG(LogTemp, Warning, TEXT("모든 웨이브 클리어! 승리하셨습니다!"));
    }
}

void AHJ_GameMode::HandleDefeat()
{
    AHJ_GameState* GS = GetGameState<AHJ_GameState>();
    if (!GS) return;

    GS->SetBattleState(EBattleState::Defeat);
}