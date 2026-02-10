// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AiEnemyController.generated.h"


UCLASS(Blueprintable)
class HJ_PROJECT_API AAiEnemyController : public AAIController
{
	GENERATED_BODY()
	
public:
	AAiEnemyController();
	
protected:
	virtual void OnPossess(APawn* InPawn) override;

	//추격 갱신
	UFUNCTION()
	void UpdateChase();

public:
	//추격 목표
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AActor* TargetPlayer;

	//목표가 죽으면 정지 할수 있는 타이머 핸들
	FTimerHandle ChaseTimerHandle;

};
