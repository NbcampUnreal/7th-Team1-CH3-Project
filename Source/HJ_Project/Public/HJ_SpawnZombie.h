#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HJ_SpawnZombie.generated.h"

class UBoxComponent;
class UDataTable; // 데이터 테이블 전방 선언

UCLASS()
class HJ_PROJECT_API AHJ_SpawnZombie : public AActor
{
	GENERATED_BODY()

public:
	AHJ_SpawnZombie();

	// Count와 Class를 모두 받는 함수 하나로 통합하는 것이 깔끔합니다.
	void SpawnZombies(int32 Count, TSubclassOf<AActor> SelectedZombieClass = nullptr);

	UFUNCTION(BlueprintCallable)
	void SpawnItem(TSubclassOf<AActor> ItemClass);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	UBoxComponent* SpawnBox;

	// 기본 좀비 클래스 (데이터 테이블에 값이 없을 때 대비)
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AActor> ZombieClass;

	// 에디터에서 데이터 테이블을 할당할 변수
	UPROPERTY(EditAnywhere, Category = "Spawn")
	UDataTable* WaveDataTable;

private:
	FVector GetRandomPointInBox() const;
};