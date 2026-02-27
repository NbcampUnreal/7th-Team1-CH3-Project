#include "HJ_Player.h"
#include "Camera/CameraComponent.h"
#include "EquipWeaponMaster.h"
#include "HJ_GameMode.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/DamageEvents.h"

AHJ_Player::AHJ_Player()
{
    PrimaryActorTick.bCanEverTick = true;

    /* ================= Camera ================= */

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = NormalArmLength;
    CameraBoom->SocketOffset = NormalSocketOffset;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.f;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom);
    FollowCamera->bUsePawnControlRotation = false;

    /* ================= Rotation ================= */

    bUseControllerRotationYaw = false;

    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->bOrientRotationToMovement = true;
    Move->RotationRate = FRotator(0.f, 720.f, 0.f);
    Move->MaxWalkSpeed = NormalWalkSpeed;

    MaxHP = 100.f;
    CurrentHP = MaxHP;
}

void AHJ_Player::BeginPlay()
{
    Super::BeginPlay();

    if (Controller)
    {
        FRotator ControlRot = Controller->GetControlRotation();
        SetActorRotation(FRotator(0.f, ControlRot.Yaw, 0.f));
    }

    if (FollowCamera)
    {
        FollowCamera->SetFieldOfView(NormalFOV);
    }

    if (WeaponClass)
    {
        CurrentWeapon = GetWorld()->SpawnActor<AEquipWeaponMaster>(WeaponClass);
        if (CurrentWeapon)
        {
            CurrentWeapon->AttachToComponent(
                GetMesh(),
                FAttachmentTransformRules::SnapToTargetIncludingScale,
                TEXT("hand_rSocket")
            );
            CurrentWeapon->SetOwner(this);
        }
    }
}

void AHJ_Player::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!FollowCamera || !CameraBoom || bIsDead) return;

    /* ===== FOV 보간 ===== */

    const float TargetFOV = bIsAiming ? AimFOV : NormalFOV;

    float NewFOV = FMath::FInterpTo(
        FollowCamera->FieldOfView,
        TargetFOV,
        DeltaTime,
        AimInterpSpeed
    );

    FollowCamera->SetFieldOfView(NewFOV);

    /* ===== 카메라 위치 보간 ===== */

    float TargetArm = bIsAiming ? AimArmLength : NormalArmLength;

    CameraBoom->TargetArmLength = FMath::FInterpTo(
        CameraBoom->TargetArmLength,
        TargetArm,
        DeltaTime,
        CameraInterpSpeed
    );

    FVector TargetOffset = bIsAiming ? AimSocketOffset : NormalSocketOffset;

    CameraBoom->SocketOffset = FMath::VInterpTo(
        CameraBoom->SocketOffset,
        TargetOffset,
        DeltaTime,
        CameraInterpSpeed
    );

    /* ===== 조준 시 캐릭터 회전 고정 ===== */

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

void AHJ_Player::SetAimMode(bool bAim)
{
    bIsAiming = bAim;

    UCharacterMovementComponent* Move = GetCharacterMovement();

    if (bIsAiming)
    {
        Move->bOrientRotationToMovement = false;
        Move->MaxWalkSpeed = AimWalkSpeed;
    }
    else
    {
        Move->bOrientRotationToMovement = true;
        Move->MaxWalkSpeed = NormalWalkSpeed;
    }
}

void AHJ_Player::StartFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StartFire();
    }
}

void AHJ_Player::StopFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }
}

float AHJ_Player::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser)
{
    if (bIsDead) return 0.f;

    float ActualDamage = Super::TakeDamage(
        DamageAmount,
        DamageEvent,
        EventInstigator,
        DamageCauser);

    float AppliedDamage = (ActualDamage > 0.f) ? ActualDamage : DamageAmount;

    CurrentHP = FMath::Clamp(CurrentHP - AppliedDamage, 0.f, MaxHP);

    if (CurrentHP <= 0.f)
    {
        bIsDead = true;

        GetCharacterMovement()->DisableMovement();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        if (UWorld* World = GetWorld())
        {
            if (AHJ_GameMode* GM =
                Cast<AHJ_GameMode>(World->GetAuthGameMode()))
            {
                GM->HandleDefeat();
            }
        }
    }

    return AppliedDamage;
}