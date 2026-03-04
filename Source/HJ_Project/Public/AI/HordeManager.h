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

	//리더 재선정용
	void NotifyZombieDied(AAiEnemyCharacter* Zombie);

	//리더 한명당 팔로워 8명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horde")
	int32 FollowersPerLeader = 8;

	//리더 최대치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horde")
	int32 MaxLeaders = 12;
	

protected:
	virtual void BeginPlay() override;

private:
	// 전체 좀비 목록
	UPROPERTY()
	TArray<TObjectPtr<AAiEnemyCharacter>> AllZombies;

	// 현재 리더
	UPROPERTY()
	TObjectPtr<AAiEnemyCharacter> CurrentLeader = nullptr;

	// 리더 재선정
	void AssignNewLeader();
};
