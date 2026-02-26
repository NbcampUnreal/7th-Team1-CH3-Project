// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HordeManager.generated.h"


class AAiEnemyCharacter;

UCLASS()
class HJ_PROJECT_API AHordeManager : public AActor
{
	GENERATED_BODY()

public:
	AHordeManager();

	// 스폰된 좀비 등록
	void RegisterZombie(AAiEnemyCharacter* Zombie);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<TObjectPtr<AAiEnemyCharacter>> AllZombies;

	UPROPERTY()
	TObjectPtr<AAiEnemyCharacter> CurrentLeader = nullptr;

	void AssignNewLeader();
};
