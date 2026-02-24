#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquipWeaponMaster.generated.h"

class USceneComponent;
class UArrowComponent;

UCLASS()
class HJ_PROJECT_API AEquipWeaponMaster : public AActor
{
	GENERATED_BODY()

public:
	AEquipWeaponMaster();

	// ===== 연사 제어 =====
	void StartFire();
	void StopFire();

	UFUNCTION(BlueprintCallable)
	virtual void Fire();

protected:
	virtual void BeginPlay() override;

	// ===== 연사용 =====
	FTimerHandle FireTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float FireRate = 0.1f; // 초당 10발

	bool bIsFiring = false;

	// ===== 기존 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* Muzzle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float TraceDistance = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stat")
	float CurrentDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawCameraDebug = false;

public:
	FVector GetMuzzleLocation() const;
};