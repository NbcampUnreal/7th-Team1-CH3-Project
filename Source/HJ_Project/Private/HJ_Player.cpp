#include "HJ_Player.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AHJ_Player::AHJ_Player()
{
	PrimaryActorTick.bCanEverTick = false;

	// 카메라 붐
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	// 카메라
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	// TPS 스타일 회전 설정
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AHJ_Player::BeginPlay()
{
	Super::BeginPlay();
}

void AHJ_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHJ_Player::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHJ_Player::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AHJ_Player::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AHJ_Player::LookUp);
}

void AHJ_Player::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);
}

void AHJ_Player::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void AHJ_Player::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AHJ_Player::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}
