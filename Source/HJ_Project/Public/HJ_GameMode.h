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

    UPROPERTY(EditAnywhere, Category = "Wave")
    UDataTable* WaveDataTable;

    UPROPERTY(EditAnywhere, Category = "Wave")
    int32 MaxWave = 3;

private:
    UPROPERTY()
    TArray<AHJ_SpawnZombie*> SpawnZombieActors;

    int32 CurrentWave = 0;
    int32 AliveZombieCount = 0;

    FTimerHandle WaveTimerHandle;
    bool bIsDefeatHandled = false;

    void StartPreparation();
    void StartWave();
    void EndWave();

public:
    void OnZombieKilled();
    void HandleDefeat();

    void RegisterSpawnedZombie();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_OnVictory();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_OnDefeat();
};