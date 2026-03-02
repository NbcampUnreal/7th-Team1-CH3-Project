#include "HJ_Player.h"
#include "HJ_GameMode.h"
#include "Camera/CameraComponent.h"
#include "EquipWeaponMaster.h"
#include "Engine/DamageEvents.h"
#include "HJ_PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
    if (bIsReloading) return;  

    if (CurrentWeapon && bHasWeapon)
        CurrentWeapon->StartFire();

    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        if (FireMontage)
        {
            Anim->Montage_Play(FireMontage);
        }
    }
}


void AHJ_Player::StopFire()
{
    if (CurrentWeapon && bHasWeapon)
        CurrentWeapon->StopFire();
}

void AHJ_Player::SetAimMode(bool bAim)
{
    if (!bHasWeapon)
    {
        bIsAiming = false;
        return;
    }

    bIsAiming = bAim;

    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->bOrientRotationToMovement = !bIsAiming;
    Move->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : NormalWalkSpeed;
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

        if (AHJ_GameMode* GM = Cast<AHJ_GameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GM->HandleDefeat();
        }
    }

    return AppliedDamage;
}

void AHJ_Player::ReloadWeapon()
{
    if (!CurrentWeapon || !bHasWeapon) return;
    if (bIsReloading) return;

    bIsReloading = true;

    UE_LOG(LogTemp, Warning, TEXT("Reload Start"));


    SetAimMode(false);
    GetCharacterMovement()->DisableMovement();

    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        if (ReloadMontage)
        {
            Anim->Montage_Play(ReloadMontage);
        }
    }

    float ReloadTime = 0.f;

    if (ReloadMontage)
    {
        ReloadTime = ReloadMontage->GetPlayLength();
    }

    GetWorldTimerManager().SetTimer(
        ReloadTimerHandle,
        this,
        &AHJ_Player::FinishReload,
        ReloadTime,
        false
    );
}

void AHJ_Player::FinishReload()
{
    UE_LOG(LogTemp, Warning, TEXT("Reload Finish"));

    if (CurrentWeapon)
    {
        CurrentWeapon->Reload(); 
    }

    bIsReloading = false;

    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

    UpDateAmmoHUD();
}

void AHJ_Player::UpDateAmmoHUD()
{
    // 무기를 들고 있지 않다면 띄우지 않음
    if (!CurrentWeapon) return;

    AHJ_PlayerController* PC = Cast<AHJ_PlayerController>(GetController());
    if (!PC) return;

    UUserWidget* HUDWidget = PC->GetHUDWidget();
    if (!HUDWidget) return;

    // 블루프린트에서 "AmmoText"라는 이름의 TextBlock을 찾습니다.
    UTextBlock* AmmoText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("AmmoText")));
    if (AmmoText)
    {
        // "탄약 : 30 / 45" 형태로 출력
        FString AmmoStr = FString::Printf(TEXT("%d / %d"), CurrentWeapon->CurrentAmmoInMag, CurrentWeapon->MaxAmmoInMag);
        AmmoText->SetText(FText::FromString(AmmoStr));
    }
}
