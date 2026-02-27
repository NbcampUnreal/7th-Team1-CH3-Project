#include "HJ_GameMode.h"
#include "HJ_GameState.h"
#include "HJ_Player.h"
#include "HJ_PlayerController.h"
#include "HJ_SpawnZombie.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
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
    if (bIsDefeatHandled) return;
    bIsDefeatHandled = true;

    AHJ_GameState* GS = GetGameState<AHJ_GameState>();
    if (GS)
    {
        GS->SetBattleState(EBattleState::Defeat);
    }

    // 1) 게임 정지(완전 멈춤)
    UGameplayStatics::SetGamePaused(this, true);

    // PlayerController를 가져와 재시작 메뉴 UI를 띄움
    if (AHJ_PlayerController* PC = Cast<AHJ_PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        // 내부에서 커서를 보이게 하고 UIOnly 모드로 전환하므로 추가 입력 방지 코드가 불필요해집니다.
        PC->ShowMainMenu(true);
    }
    
    BP_OnDefeat();
}