#include "HJ_GameMode.h"
#include "HJ_GameState.h"
#include "HJ_Player.h"
#include "HJ_PlayerController.h"
#include "HJ_SpawnZombie.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"

AHJ_GameMode::AHJ_GameMode()
{
    GameStateClass = AHJ_GameState::StaticClass();
    DefaultPawnClass = AHJ_Player::StaticClass();
    PlayerControllerClass = AHJ_PlayerController::StaticClass();

    TimeBetweenWaves = 5.f;
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

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(
        TimerHandle,
        this,
        &AHJ_GameMode::StartWave,
        TimeBetweenWaves,
        false
    );
}

void AHJ_GameMode::StartWave()
{
    AHJ_GameState* GS = GetGameState<AHJ_GameState>();
    if (!GS) return;

    GS->SetBattleState(EBattleState::InBattle);

    if (SpawnZombieActor)
    {
        SpawnZombieActor->StartWave(1); // 최초 웨이브 시작
    }
}

void AHJ_GameMode::OnZombieKilled()
{
}

void AHJ_GameMode::EndWave()
{
}

void AHJ_GameMode::HandleDefeat()
{
    AHJ_GameState* GS = GetGameState<AHJ_GameState>();
    if (!GS) return;

    GS->SetBattleState(EBattleState::Defeat);
}
