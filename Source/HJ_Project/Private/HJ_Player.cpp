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
    CameraBoom->TargetArmLength = 420.f;
    CameraBoom->SocketOffset = FVector(0.f, 55.f, 70.f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.f;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom);
    FollowCamera->bUsePawnControlRotation = false;

    /* ================= Rotation ================= */

    // 🔴 기본은 이동 방향 기준 회전
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

    // 🔥 초기 회전 동기화 (비틀림 해결 핵심)
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
            CurrentWeapon->SetInstigator(this);
        }
    }
}

void AHJ_Player::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickRecoil(DeltaTime);//반동 매 프레임 적용 및 복구

    if (!FollowCamera || bIsDead) return;

    /* ===== FOV 보간 ===== */

    const float TargetFOV = bIsAiming ? AimFOV : NormalFOV;

    const float NewFOV = FMath::FInterpTo(
        FollowCamera->FieldOfView,
        TargetFOV,
        DeltaTime,
        AimInterpSpeed
    );

    FollowCamera->SetFieldOfView(NewFOV);

    /* ===== 견착 시 카메라 Yaw 고정 ===== */

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

//무기 발사 했을 때 호출되는 함수
void AHJ_Player::AddRecoilImpulse()
{
    UE_LOG(LogTemp, Warning, TEXT("[Recoil] AddRecoilImpulse CALLED. Locally=%d Dead=%d Controller=%s"),
        IsLocallyControlled() ? 1 : 0,
        bIsDead ? 1 : 0,
        Controller ? TEXT("YES") : TEXT("NO"));//로그

    // 내 화면에서만 반동 적용
    if (!IsLocallyControlled()) return;
    if (bIsDead) return;

    // 조준 중이면 더 약하게, 아니면 조금 더 세게
    const float PitchMin = bIsAiming ? ADSPitchMin : HipPitchMin;
    const float PitchMax = bIsAiming ? ADSPitchMax : HipPitchMax;
    const float YawMin = bIsAiming ? ADSYawMin : HipYawMin;
    const float YawMax = bIsAiming ? ADSYawMax : HipYawMax;

    // 랜덤으로 약간의 흔들림
    const float PitchKick = FMath::FRandRange(PitchMin, PitchMax); // 위로
    const float YawKick = FMath::FRandRange(YawMin, YawMax);   // 좌우

    // 목표 반동에 누적
    RecoilTarget.Pitch += PitchKick;
    RecoilTarget.Yaw += YawKick;

    // 연사해도 하늘로 안 가게 누적 제한
    RecoilTarget.Pitch = FMath::Clamp(RecoilTarget.Pitch, 0.f, MaxRecoilPitch);
    RecoilTarget.Yaw = FMath::Clamp(RecoilTarget.Yaw, -MaxRecoilYaw, MaxRecoilYaw);
}


// 매 프레임 호출해서 반동을 부드럽게 적용하고, 다시 0으로 복구시킴
void AHJ_Player::TickRecoil(float DeltaSeconds)
{
    static float Acc = 0.f;
    Acc += DeltaSeconds;
    if (Acc > 1.0f)
    {
        Acc = 0.f;
        UE_LOG(LogTemp, Warning, TEXT("[Recoil] TickRecoil running. Locally=%d Controller=%s"),
            IsLocallyControlled() ? 1 : 0,
            Controller ? TEXT("YES") : TEXT("NO"));
    }//로그

    if (!IsLocallyControlled()) return;
    if (bIsDead) return;
    if (!Controller) return;

    //현재값이 목표값을 따라가도록함
    RecoilCurrent = FMath::RInterpTo(RecoilCurrent, RecoilTarget, DeltaSeconds, RecoilApplySpeed);

    //목표값은 0으로 돌아가도록(조준 복구)
    RecoilTarget = FMath::RInterpTo(RecoilTarget, FRotator::ZeroRotator, DeltaSeconds, RecoilReturnSpeed);

    //이번 프레임 변화량만 컨트롤러 입력으로 적용
    const FRotator Delta = RecoilCurrent - RecoilPrev;

    AddControllerPitchInput(Delta.Pitch);
    AddControllerYawInput(Delta.Yaw);

    RecoilPrev = RecoilCurrent;
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

    const float AppliedDamage =
        (ActualDamage > 0.f) ? ActualDamage : DamageAmount;

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