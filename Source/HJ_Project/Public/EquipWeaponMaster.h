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
	// 루트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	// 총구(화살표 컴포넌트) - BP에서 위치를 총구로 옮기면 정확해짐
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* Muzzle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float TraceDistance = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawCameraDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawMuzzleDebug = true;

protected:
	FVector GetMuzzleLocation() const;
};