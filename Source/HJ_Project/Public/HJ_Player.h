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
    void StopFire();
    void SetAimMode(bool bAim);

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    /* ================= Camera ================= */

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    /* ================= Weapon ================= */

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
    AEquipWeaponMaster* CurrentWeapon;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AEquipWeaponMaster> WeaponClass;

    /* ================= Stat ================= */

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MaxHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float CurrentHP;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsAiming = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
    bool bIsDead = false;

    /* ================= Aim ================= */

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

    /* ===== 추가 : 카메라 위치 보간용 ===== */

    UPROPERTY(EditAnywhere, Category = "Camera")
    float NormalArmLength = 420.f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float AimArmLength = 180.f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    FVector NormalSocketOffset = FVector(0.f, 55.f, 70.f);

    UPROPERTY(EditAnywhere, Category = "Camera")
    FVector AimSocketOffset = FVector(0.f, 80.f, 80.f);

    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraInterpSpeed = 12.f;

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