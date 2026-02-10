#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZombieSpawnRow.h"
#include "HJ_GameMode.generated.h"

class AHJ_SpawnZombie;

UCLASS()
class HJ_PROJECT_API AHJ_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHJ_GameMode();

protected:
	virtual void BeginPlay() override;

	// 웨이브 기본 설정
	UPROPERTY(EditAnywhere, Category = "Wave")
	int32 BaseZombieCount;

	UPROPERTY(EditAnywhere, Category = "Wave")
	float TimeBetweenWaves;

	int32 ZombiesAlive;

	void StartPreparation();
	void StartWave();
	void EndWave();

	// 🔥 타입 수정
	UPROPERTY(EditAnywhere, Category = "Spawn")
	AHJ_SpawnZombie* SpawnZombieActor;
	UPROPERTY(EditAnywhere, Category = "Wave")
	UDataTable* WaveDataTable;

	// 현재 행을 추적할 인덱스 (보통 웨이브 번호와 일치)
	int32 CurrentWaveIndex = 1;

public:
	void OnZombieKilled();
	void HandleDefeat();
};
