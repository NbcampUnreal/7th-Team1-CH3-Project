#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawnRow.h"
#include "HJ_SpawnZombie.generated.h"

class UBoxComponent;
class UDataTable;
class AHordeManager;

UCLASS()
class HJ_PROJECT_API AHJ_SpawnZombie : public AActor
{
	GENERATED_BODY()

public:
	AHJ_SpawnZombie();

	virtual void BeginPlay() override;

	void SpawnWave(int32 WaveNumber);

protected:

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	UBoxComponent* SpawnBox;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	UDataTable* WaveDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Horde")
	TObjectPtr<AHordeManager> HordeManager = nullptr;

private:

	FZombieSpawnRow* FindWave(int32 WaveNumber);
	FVector GetRandomPointInBox() const;
};