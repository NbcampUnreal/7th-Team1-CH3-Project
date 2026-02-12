// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h" 
#include "GameFramework/Actor.h" 
#include "AiEnemyController.generated.h"



UENUM(BlueprintType)
enum class EAIState : uint8
{
	MovingToGate,//관문으로 이동
	ChasingPlayer,//플레이어 추격
	AttackingPlayer, //플레이어 공격
	Stunned//총맞았을때 경직
};



UCLASS(Blueprintable)
class HJ_PROJECT_API AAiEnemyController : public AAIController
{
	GENERATED_BODY()
	
public:
	AAiEnemyController();
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	//추격 갱신
	UFUNCTION()
	void UpdateChase();

public:
	//ai의 상태 저장 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EAIState CurrentState = EAIState::MovingToGate;

	//추격 목표
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AActor* TargetPlayer;

	// 최종관문 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class AActor* FinalGate;


	TArray<AActor*> FoundGates;//나중에 최종관문 추가시에 삭제 가능

	//인식거리 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ai")
	float DetectionRange = 1000.0f;

	//공격 범위 설정값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AttackRange = 150.0f;

	//목표가 죽으면 정지 할수 있는 타이머 핸들
	FTimerHandle ChaseTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float chaseInterval = 0.2f;

};
