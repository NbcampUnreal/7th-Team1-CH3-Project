#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HJ_GameState.generated.h"

UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Preparation UMETA(DisplayName = "Preparation"),
	InBattle   UMETA(DisplayName = "In Battle"),
	Victory    UMETA(DisplayName = "Victory"),
	Defeat     UMETA(DisplayName = "Defeat")
};

UCLASS()
class HJ_PROJECT_API AHJ_GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AHJ_GameState();

	// 현재 웨이브
	UPROPERTY(BlueprintReadOnly, Category = "Battle")
	int32 CurrentWave;

	// 남은 좀비 수
	UPROPERTY(BlueprintReadOnly, Category = "Battle")
	int32 ZombiesRemaining;

	// 전투 상태
	UPROPERTY(BlueprintReadOnly, Category = "Battle")
	EBattleState BattleState;

	void SetWave(int32 NewWave);
	void SetZombiesRemaining(int32 NewCount);
	void SetBattleState(EBattleState NewState);
};
