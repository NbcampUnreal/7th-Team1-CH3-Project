#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawnRow.h"
#include "HJ_SpawnZombie.generated.h"

class UBoxComponent;
class UDataTable;

UCLASS()
class HJ_PROJECT_API AHJ_SpawnZombie : public AActor
{
	GENERATED_BODY()

public:
	AHJ_SpawnZombie();

	virtual void BeginPlay() override;

	// 웨이브 시작
	void StartWave(int32 WaveNumber);

	// 다음 웨이브
	void NextWave();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	UBoxComponent* SpawnBox;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	UDataTable* WaveDataTable;

private:

	int32 CurrentWave;

	FTimerHandle WaveTimer;

	FZombieSpawnRow* FindWave(int32 WaveNumber);

	FVector GetRandomPointInBox() const;
};
