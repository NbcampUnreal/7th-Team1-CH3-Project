#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HJ_SpawnZombie.generated.h"

class UBoxComponent;

UCLASS()
class HJ_PROJECT_API AHJ_SpawnZombie : public AActor
{
	GENERATED_BODY()

public:
	AHJ_SpawnZombie();

	// GameMode에서 호출
	void SpawnZombies(int32 Count);

	UFUNCTION(BlueprintCallable)
	void SpawnItem(TSubclassOf<AActor> ItemClass);
	void SpawnZombies(int32 Count, TSubclassOf<AActor> SelectedZombieClass = nullptr);

protected:
	virtual void BeginPlay() override;

	// 스폰 영역
	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	UBoxComponent* SpawnBox;

	// 스폰할 좀비 클래스
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AActor> ZombieClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	UDataTable* WaveDataTable;

private:
	FVector GetRandomPointInBox() const;
};
