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

	UFUNCTION(BlueprintCallable)
	virtual void Fire();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* Muzzle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float TraceDistance = 10000.f;

	// ✅ DT_Weapon(블루프린트)에서 읽어온 데미지를 BP가 이 변수에 넣어줌
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stat")
	float CurrentDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawCameraDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawMuzzleDebug = true;

protected:
	FVector GetMuzzleLocation() const;
};