// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"        
#include "AIController.h" 
#include "TimerManager.h" 
#include "AiEnemyController.generated.h"

class AAiEnemyCharacter;       
class AGate;


// 좀비 AI 상태(간단하게)
UENUM(BlueprintType)
enum class EAIState : uint8
{
	MovingToGate,      // Gate로 이동 중
	ChasingPlayer,     // 플레이어 추격
	AttackingGate,     // Gate 공격
	AttackingPlayer,   // 플레이어 공격
	Stunned            // 스턴
};

UCLASS()
class HJ_PROJECT_API AAiEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AAiEnemyController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	//Tick 대신 일정 간격으로만 AI 판단
	void UpdateAI();

public:
	// 현재 상태 
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI")
	EAIState CurrentState = EAIState::MovingToGate;

	// AI 판단 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perf")
	float UpdateInterval = 0.25f;

	// 플레이어 인식 거리 (리더만 사용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sense")
	float DetectionRange = 1000.0f;

	// 공격 사거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat")
	float AttackRange = 150.f;

	// Gate 근처에서 공격 시도할 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat")
	float GateAttackEnterRange = 250.f;

private:
	// 내 좀비 캐릭터
	UPROPERTY()
	TObjectPtr<AAiEnemyCharacter> MyZombie = nullptr;

	// 플레이어 공격/추격
	UPROPERTY()
	TObjectPtr<AActor> TargetPlayer = nullptr;

	//게이트 기본 목표
	UPROPERTY()
	TObjectPtr<AGate> TargetGate = nullptr;

	//타이머 핸들
	FTimerHandle UpdateTimerHandle;

	// 공격 타이밍 누적 
	float AttackAccTime = 0.0f;

private:
	
	void FindTargets();
	void MoveToGateIfNeeded(EAIState PrevState);
	void MoveToPlayerIfNeeded(EAIState PrevState);
	void TryEnterGateAttack();
	void TickAttack(); // 공격 누적 및 ApplyDamage
};