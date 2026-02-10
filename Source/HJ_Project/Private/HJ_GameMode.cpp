#include "HJ_GameMode.h"
#include "HJ_GameState.h"
#include "HJ_Player.h"
#include "TimerManager.h"

AHJ_GameMode::AHJ_GameMode()
{
	// 🔥 중요: GameState & Pawn 연결
	GameStateClass = AHJ_GameState::StaticClass();
	DefaultPawnClass = AHJ_Player::StaticClass();

	BaseZombieCount = 5;
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

	int32 NewWave = GS->CurrentWave + 1;
	GS->SetWave(NewWave);

	ZombiesAlive = BaseZombieCount * NewWave;
	GS->SetZombiesRemaining(ZombiesAlive);

	// 👉 여기서 SpawnManager 연결 예정
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
	StartPreparation();
}

void AHJ_GameMode::HandleDefeat()
{
	AHJ_GameState* GS = GetGameState<AHJ_GameState>();
	if (!GS) return;

	GS->SetBattleState(EBattleState::Defeat);
}
