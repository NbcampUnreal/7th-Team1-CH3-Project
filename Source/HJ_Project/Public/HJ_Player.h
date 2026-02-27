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

    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        AActor* DamageCauser
    ) override;

    /* ===== Weapon ===== */

    UFUNCTION(BlueprintCallable)
    void EquipWeapon();

    UFUNCTION(BlueprintCallable)
    void StartFire();

    UFUNCTION(BlueprintCallable)
    void StopFire();

    UFUNCTION(BlueprintCallable)
    void SetAimMode(bool bAim);

    UFUNCTION(BlueprintPure)
    bool HasWeapon() const { return bHasWeapon; }

    UFUNCTION(BlueprintCallable)
    void ReloadWeapon();

protected:
    virtual void BeginPlay() override;

    /* ================= Camera ================= */

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    /* ================= Weapon ================= */

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon")
    AEquipWeaponMaster* CurrentWeapon;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AEquipWeaponMaster> WeaponClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bHasWeapon = false;

    /* ================= Stat ================= */

    UPROPERTY(EditAnywhere, Category = "Stat")
    float MaxHP = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "Stat")
    float CurrentHP;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsAiming = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsDead = false;

    /* ================= Aim ================= */

    UPROPERTY(EditAnywhere, Category = "Aim")
    float NormalFOV = 90.f;

    UPROPERTY(EditAnywhere, Category = "Aim")
    float AimFOV = 65.f;

    UPROPERTY(EditAnywhere, Category = "Aim")
    float AimInterpSpeed = 12.f;

    UPROPERTY(EditAnywhere, Category = "Aim")
    float NormalWalkSpeed = 600.f;

    UPROPERTY(EditAnywhere, Category = "Aim")
    float AimWalkSpeed = 350.f;

    /* ================= Camera Position ================= */

    UPROPERTY(EditAnywhere, Category = "Camera")
    float NormalArmLength = 420.f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float AimArmLength = 160.f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    FVector NormalSocketOffset = FVector(0.f, 55.f, 70.f);

    UPROPERTY(EditAnywhere, Category = "Camera")
    FVector AimSocketOffset = FVector(0.f, 75.f, 72.f);

    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraInterpSpeed = 15.f;
};