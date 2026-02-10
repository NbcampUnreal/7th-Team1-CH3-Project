#include "HJ_GameState.h"

AHJ_GameState::AHJ_GameState()
{
	CurrentWave = 0;
	ZombiesRemaining = 0;
	BattleState = EBattleState::Preparation;
}

void AHJ_GameState::SetWave(int32 NewWave)
{
	CurrentWave = NewWave;
}

void AHJ_GameState::SetZombiesRemaining(int32 NewCount)
{
	ZombiesRemaining = NewCount;
}

void AHJ_GameState::SetBattleState(EBattleState NewState)
{
	BattleState = NewState;
}
