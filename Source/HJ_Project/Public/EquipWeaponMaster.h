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

	/* ================= Fire ================= */

	UFUNCTION(BlueprintCallable)
	virtual void Fire();

	virtual void StartFire();
	virtual void StopFire();

	/* ================= Reload ================= */

	UFUNCTION(BlueprintCallable)
	void Reload();

protected:
	void FinishReload();
	FVector GetMuzzleLocation() const;

	/* ================= Components ================= */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* Muzzle;

	/* ================= Weapon Stat ================= */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float TraceDistance = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stat")
	float CurrentDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float FireRate = 0.1f;

	/* ================= Ammo ================= */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MagazineSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MaxAmmo = 120;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentMagazineAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	float ReloadTime = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
	bool bIsReloading = false;

	/* ================= Debug ================= */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawCameraDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawMuzzleDebug = true;

	FTimerHandle FireTimerHandle;
	FTimerHandle ReloadTimerHandle;
};