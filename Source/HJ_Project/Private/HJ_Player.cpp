#include "HJ_Player.h"
#include "Camera/CameraComponent.h"
#include "EquipWeaponMaster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/DamageEvents.h"

AHJ_Player::AHJ_Player()
{
    PrimaryActorTick.bCanEverTick = true;

    /* ===== Camera ===== */

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = NormalArmLength;
    CameraBoom->SocketOffset = NormalSocketOffset;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.f;
    CameraBoom->CameraLagMaxDistance = 20.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom);
    FollowCamera->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = false;

    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->bOrientRotationToMovement = true;
    Move->RotationRate = FRotator(0.f, 720.f, 0.f);
    Move->MaxWalkSpeed = NormalWalkSpeed;

    CurrentHP = MaxHP;
    bHasWeapon = false;
}

void AHJ_Player::BeginPlay()
{
    Super::BeginPlay();
    FollowCamera->SetFieldOfView(NormalFOV);
}

void AHJ_Player::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!FollowCamera || !CameraBoom || bIsDead) return;

    /* ===== FOV ===== */

    float TargetFOV = bIsAiming ? AimFOV : NormalFOV;

    float NewFOV = FMath::FInterpTo(
        FollowCamera->FieldOfView,
        TargetFOV,
        DeltaTime,
        AimInterpSpeed
    );

    FollowCamera->SetFieldOfView(NewFOV);

    /* ===== Arm Length ===== */

    float TargetArm = bIsAiming ? AimArmLength : NormalArmLength;

    CameraBoom->TargetArmLength = FMath::FInterpTo(
        CameraBoom->TargetArmLength,
        TargetArm,
        DeltaTime,
        CameraInterpSpeed
    );

    /* ===== Socket Offset ===== */

    FVector TargetOffset = bIsAiming ? AimSocketOffset : NormalSocketOffset;

    CameraBoom->SocketOffset = FMath::VInterpTo(
        CameraBoom->SocketOffset,
        TargetOffset,
        DeltaTime,
        CameraInterpSpeed
    );

    /* ===== 견착 시 캐릭터 카메라 방향 정렬 ===== */

    if (bIsAiming && Controller)
    {
        FRotator ControlRot = Controller->GetControlRotation();
        FRotator TargetRot(0.f, ControlRot.Yaw, 0.f);

        FRotator NewRot = FMath::RInterpTo(
            GetActorRotation(),
            TargetRot,
            DeltaTime,
            15.f
        );

        SetActorRotation(NewRot);
    }
}

void AHJ_Player::EquipWeapon()
{
    if (bHasWeapon || !WeaponClass) return;

    CurrentWeapon = GetWorld()->SpawnActor<AEquipWeaponMaster>(WeaponClass);

    if (CurrentWeapon)
    {
        CurrentWeapon->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            TEXT("hand_rSocket")
        );

        CurrentWeapon->SetOwner(this);
        bHasWeapon = true;
    }
}

void AHJ_Player::StartFire()
{
    if (CurrentWeapon && bHasWeapon)
        CurrentWeapon->StartFire();
}

void AHJ_Player::StopFire()
{
    if (CurrentWeapon && bHasWeapon)
        CurrentWeapon->StopFire();
}

void AHJ_Player::SetAimMode(bool bAim)
{
    bIsAiming = bAim;

    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->bOrientRotationToMovement = !bIsAiming;
    Move->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : NormalWalkSpeed;
}

float AHJ_Player::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser)
{
    if (bIsDead) return 0.f;

    float AppliedDamage = Super::TakeDamage(
        DamageAmount,
        DamageEvent,
        EventInstigator,
        DamageCauser);

    CurrentHP = FMath::Clamp(CurrentHP - AppliedDamage, 0.f, MaxHP);

    if (CurrentHP <= 0.f)
    {
        bIsDead = true;
        GetCharacterMovement()->DisableMovement();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    return AppliedDamage;
}

void AHJ_Player::ReloadWeapon()
{
    if (CurrentWeapon && bHasWeapon)
    {
        CurrentWeapon->Reload();
    }
}