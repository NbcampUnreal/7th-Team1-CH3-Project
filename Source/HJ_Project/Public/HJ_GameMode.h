#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HJ_GameMode.generated.h"

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

public:
	void OnZombieKilled();
	void HandleDefeat();
};
