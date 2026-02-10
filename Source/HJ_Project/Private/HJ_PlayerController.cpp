#include "HJ_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "HJ_Player.h"

AHJ_PlayerController::AHJ_PlayerController()
{
}

void AHJ_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void AHJ_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EI =
		Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 이동
		if (MoveAction)
		{
			EI->BindAction(MoveAction, ETriggerEvent::Triggered,
				this, &AHJ_PlayerController::Move);
		}

		// 시점 회전
		if (LookAction)
		{
			EI->BindAction(LookAction, ETriggerEvent::Triggered,
				this, &AHJ_PlayerController::Look);
		}

		// 점프
		if (JumpAction)
		{
			EI->BindAction(JumpAction, ETriggerEvent::Started,
				this, &AHJ_PlayerController::StartJump);

			EI->BindAction(JumpAction, ETriggerEvent::Completed,
				this, &AHJ_PlayerController::StopJump);
		}
	}
}

void AHJ_PlayerController::Move(const FInputActionValue& Value)
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	const FVector2D MoveInput = Value.Get<FVector2D>();

	// 🔥 컨트롤러 회전 가져오기
	const FRotator ControlRot = GetControlRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	// 🔥 카메라 기준 방향 계산
	const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	MyPawn->AddMovementInput(ForwardDir, MoveInput.Y);
	MyPawn->AddMovementInput(RightDir, MoveInput.X);
}

void AHJ_PlayerController::Look(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();

	AddYawInput(Input.X);
	AddPitchInput(Input.Y);
}

void AHJ_PlayerController::StartJump(const FInputActionValue& Value)
{
	AHJ_Player* HJPlayer = Cast<AHJ_Player>(GetPawn());
	if (HJPlayer)
	{
		HJPlayer->Jump();
	}
}

void AHJ_PlayerController::StopJump(const FInputActionValue& Value)
{
	AHJ_Player* HJPlayer = Cast<AHJ_Player>(GetPawn());
	if (HJPlayer)
	{
		HJPlayer->StopJumping();
	}
}
