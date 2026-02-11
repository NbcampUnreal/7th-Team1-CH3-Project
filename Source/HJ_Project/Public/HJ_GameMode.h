#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZombieSpawnRow.h"
#include "HJ_GameMode.generated.h"

class AHJ_SpawnZombie;
class UDataTable;

UCLASS()
class HJ_PROJECT_API AHJ_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHJ_GameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Wave")
	float TimeBetweenWaves;

	void StartPreparation();
	void StartWave();
	void EndWave();

	UPROPERTY(EditAnywhere, Category = "Spawn")
	AHJ_SpawnZombie* SpawnZombieActor;

	UPROPERTY(EditAnywhere, Category = "Wave")
	UDataTable* WaveDataTable;

public:
	void OnZombieKilled();
	void HandleDefeat();
};
