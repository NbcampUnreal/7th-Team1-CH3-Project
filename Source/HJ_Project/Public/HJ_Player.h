#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HJ_Player.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AEquipWeaponMaster;

UCLASS()
class HJ_PROJECT_API AHJ_Player : public ACharacter
{
    GENERATED_BODY()

public:
    AHJ_Player();

    void StartFire();
    void SetAimMode(bool bAim);

    // ✅ FOV 보간을 위해 Tick 사용
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
    AEquipWeaponMaster* CurrentWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MaxHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float CurrentHP;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AEquipWeaponMaster> WeaponClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsAiming = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
    bool bIsDead = false;

    // ✅ Aim(견착) 카메라 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
    float NormalFOV = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
    float AimFOV = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
    float AimInterpSpeed = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
    float NormalWalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
    float AimWalkSpeed = 350.0f;

public:
    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        AActor* DamageCauser
    ) override;

    UFUNCTION(BlueprintCallable)
    bool IsDead() const { return bIsDead; }
};