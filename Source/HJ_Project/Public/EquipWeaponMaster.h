#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquipWeaponMaster.generated.h"

UCLASS()
class HJ_PROJECT_API AEquipWeaponMaster : public AActor
{
    GENERATED_BODY()

public:
    AEquipWeaponMaster();

    UFUNCTION(BlueprintCallable)
    virtual void Fire();
};
