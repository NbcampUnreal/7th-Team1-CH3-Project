#include "HJ_GameMode.h"
#include "HJ_GameState.h"
#include "HJ_Player.h"
#include "HJ_PlayerController.h"
#include "HJ_SpawnZombie.h"
#include "Kismet/GameplayStatics.h"
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

	// SpawnZombie 자동 수집
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		AHJ_SpawnZombie::StaticClass(),
		FoundActors
	);

	SpawnZombieActors.Empty();

	for (AActor* Actor : FoundActors)
	{
		if (AHJ_SpawnZombie* Spawner = Cast<AHJ_SpawnZombie>(Actor))
		{
			SpawnZombieActors.Add(Spawner);
		}
	}

	StartPreparation();
}

void AHJ_GameMode::StartPreparation()
{
	AHJ_GameState* GS = GetGameState<AHJ_GameState>();
	if (!GS) return;

	GS->SetBattleState(EBattleState::Preparation);

	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&AHJ_GameMode::StartWave,
		TimeBetweenWaves,
		false
	);
}

void AHJ_GameMode::StartWave()
{
	AHJ_GameState* GS = GetGameState<AHJ_GameState>();
	if (!GS || !WaveDataTable) return;

	GS->SetBattleState(EBattleState::InBattle);

	CurrentWave++;
	FZombieSpawnRow* Row =
		WaveDataTable->FindRow<FZombieSpawnRow>(
			FName(*FString::FromInt(CurrentWave)),
			TEXT("WaveLookup")
		);

	if (!Row)
	{
		return;
	}
	AliveZombieCount = Row->SpawnCount * SpawnZombieActors.Num();

	for (AHJ_SpawnZombie* Spawner : SpawnZombieActors)
	{
		if (Spawner)
		{
			Spawner->SpawnWave(CurrentWave);
		}
	}
}

void AHJ_GameMode::OnZombieKilled()
{
	AliveZombieCount--;

	UE_LOG(LogTemp, Warning, TEXT("Remaining: %d"), AliveZombieCount);

	if (AliveZombieCount <= 0)
	{
		EndWave();
	}
}

void AHJ_GameMode::EndWave()
{
	if (CurrentWave >= MaxWave)
	{
		AHJ_GameState* GS = GetGameState<AHJ_GameState>();
		if (GS)
		{
			GS->SetBattleState(EBattleState::Victory);
		}

		if (AHJ_PlayerController* PC = Cast<AHJ_PlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			PC->ShowMainMenu(true);
		}
		UGameplayStatics::SetGamePaused(this, true);

		BP_OnVictory();
		return;
	}

	StartPreparation();
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

    UGameplayStatics::SetGamePaused(this, true);

    if (AHJ_PlayerController* PC = Cast<AHJ_PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        PC->ShowMainMenu(true);
    }
    
    BP_OnDefeat();
}