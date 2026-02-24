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
    // ✅ FOV 보간을 위해 Tick 켬
    PrimaryActorTick.bCanEverTick = true;

    // 카메라 컴포넌트
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);

    // 카메라 위치 조정
    CameraBoom->TargetArmLength = 420.f;
    CameraBoom->SocketOffset = FVector(0.f, 55.f, 70.f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.f;

    // 카메라 회전
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.f;

    // FollowCamera 설정
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom);
    FollowCamera->bUsePawnControlRotation = false;

    // 캐릭터 회전 설정
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

    MaxHP = 100.0f;
    CurrentHP = MaxHP;
}

void AHJ_Player::BeginPlay()
{
    Super::BeginPlay();

    // ✅ 시작 FOV를 NormalFOV로 확정
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
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = NormalWalkSpeed;
    }
}

void AHJ_Player::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!FollowCamera) return;

    // 죽었으면 굳이 보간 안 해도 됨(선택)
    if (bIsDead) return;

    const float TargetFOV = bIsAiming ? AimFOV : NormalFOV;
    const float NewFOV = FMath::FInterpTo(
        FollowCamera->FieldOfView,
        TargetFOV,
        DeltaTime,
        AimInterpSpeed
    );

    FollowCamera->SetFieldOfView(NewFOV);
}

void AHJ_Player::SetAimMode(bool bAim)
{
    bIsAiming = bAim;
    UE_LOG(LogTemp, Warning, TEXT("AIM: %s"), bIsAiming ? TEXT("ON") : TEXT("OFF"));

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : NormalWalkSpeed;
    }
}

float AHJ_Player::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 안전장치: Super가 0을 리턴하는 케이스 대비
    const float AppliedDamage = (ActualDamage > 0.0f) ? ActualDamage : DamageAmount;

    if (AppliedDamage > 0.0f)
    {
        CurrentHP = FMath::Clamp(CurrentHP - AppliedDamage, 0.0f, MaxHP);

        UE_LOG(LogTemp, Warning, TEXT("HP: %f"), CurrentHP);

        if (CurrentHP <= 0.0f)
        {
            bIsDead = true;
            UE_LOG(LogTemp, Warning, TEXT("YOU DIE YANG!"));

            // (선택) 플레이어 움직임/충돌 끄기 (죽은 뒤에 더 이상 이상동작 방지)
            if (UCharacterMovementComponent* Move = GetCharacterMovement())
            {
                Move->DisableMovement();
            }
            if (UCapsuleComponent* Cap = GetCapsuleComponent())
            {
                Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }

            // GameMode에 패배 처리 요청 (게임 전체 종료는 GameMode가 담당)
            if (UWorld* World = GetWorld())
            {
                if (AGameModeBase* GMBase = World->GetAuthGameMode())
                {
                    if (AHJ_GameMode* GM = Cast<AHJ_GameMode>(GMBase))
                    {
                        GM->HandleDefeat();
                    }
                }
            }
        }
    }

    return AppliedDamage;
}

void AHJ_Player::StartFire()
{
    UE_LOG(LogTemp, Warning, TEXT("Fire Pressed"));

    if (CurrentWeapon)
    {
        CurrentWeapon->StartFire();  
    }
}

void AHJ_Player::StopFire()
{
    UE_LOG(LogTemp, Warning, TEXT("Fire Released"));

    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();    
    }
}