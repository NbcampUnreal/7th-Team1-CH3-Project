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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    bool bHasWeapon = false;

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

    //총기 반동
    FRotator RecoilTarget = FRotator::ZeroRotator;//반동 누적
    FRotator RecoilCurrent = FRotator::ZeroRotator;//화면에 적용중인 반동
    FRotator RecoilPrev = FRotator::ZeroRotator;//프레임 값

    //반동 튀는 속도값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float RecoilApplySpeed = 55.0f;

    //반동 복구
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float RecoilReturnSpeed = 45.0f;

    //힙파이어 반동 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float HipPitchMin = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float HipPitchMax = 0.30f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float HipYawMin = -0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float HipYawMax = 0.02f;

    //정조준 반동범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float ADSPitchMin = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float ADSPitchMax = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float ADSYawMin = -0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float ADSYawMax = 0.01f;

    //연사시에 화면 올라가는거 제한
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float MaxRecoilPitch = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
    float MaxRecoilYaw = 0.6f;


public:
    //무기 발사시에 반동 호출
    void AddRecoilImpulse();

    //틱에서 매 프레임 호출
    void TickRecoil(float DeltaSeconds);


    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        AActor* DamageCauser
    ) override;

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