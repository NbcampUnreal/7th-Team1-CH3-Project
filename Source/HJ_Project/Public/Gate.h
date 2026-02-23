
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h" 
#include "Gate.generated.h"

UCLASS()
class HJ_PROJECT_API AGate : public AActor
{
	GENERATED_BODY()

public:
	AGate();

protected:
	virtual void BeginPlay() override;

	// 데미지 처리
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	//관문 메쉬 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	class UStaticMeshComponent* MeshComp;

	//체력 UI 컴포넌트 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	UWidgetComponent* HPWidgetComp;

	//현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate|Stats")
	float Health = 500.0f;

	//최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate|Stats")
	float MaxHealth = 500.0f;
};