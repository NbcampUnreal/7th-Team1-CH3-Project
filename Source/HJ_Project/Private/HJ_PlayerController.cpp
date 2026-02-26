#include "HJ_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "HJ_Player.h"
#include "Blueprint/UserWidget.h"           
#include "Components/Widget.h"              

AHJ_PlayerController::AHJ_PlayerController()
{
    CrosshairWidget = nullptr;
    HudWidgetInstance = nullptr;
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

    if (CrosshairWidgetClass)
    {
        CrosshairWidget = CreateWidget<UUserWidget>(this, CrosshairWidgetClass);
        if (CrosshairWidget)
        {
            CrosshairWidget->AddToViewport();
            CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

 /*   if (HudWidgetClass)
    {
        HudWidgetInstance = CreateWidget<UUserWidget>(this, HudWidgetClass);
        if (HudWidgetInstance)
        {
            HudWidgetInstance->AddToViewport();
        }
    }*/
}

void AHJ_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(InputComponent))
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

        // 발사
        if (FireAction)
        {
            EI->BindAction(FireAction, ETriggerEvent::Started,
                this, &AHJ_PlayerController::StartFire);

            EI->BindAction(FireAction, ETriggerEvent::Completed,
                this, &AHJ_PlayerController::StopFire);
        }

        // 견착
        if (AimAction)
        {
            EI->BindAction(AimAction, ETriggerEvent::Started,
                this, &AHJ_PlayerController::StartAim);

            EI->BindAction(AimAction, ETriggerEvent::Completed,
                this, &AHJ_PlayerController::StopAim);
        }
    }
}

void AHJ_PlayerController::Move(const FInputActionValue& Value)
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    const FVector2D MoveInput = Value.Get<FVector2D>();

    const FRotator ControlRot = GetControlRotation();
    const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

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

void AHJ_PlayerController::StartFire()
{
    AHJ_Player* MyPlayer = Cast<AHJ_Player>(GetPawn());
    if (!MyPlayer) return;

    MyPlayer->StartFire();
}

void AHJ_PlayerController::StopFire()
{
    AHJ_Player* MyPlayer = Cast<AHJ_Player>(GetPawn());
    if (!MyPlayer) return;
    MyPlayer->StopFire();
}

void AHJ_PlayerController::StartAim(const FInputActionValue& Value)
{
    AHJ_Player* MyPlayer = Cast<AHJ_Player>(GetPawn());
    if (!MyPlayer) return;

    MyPlayer->SetAimMode(true);

    if (CrosshairWidget)
    {
        CrosshairWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}

void AHJ_PlayerController::StopAim(const FInputActionValue& Value)
{
    AHJ_Player* MyPlayer = Cast<AHJ_Player>(GetPawn());
    if (!MyPlayer) return;

    MyPlayer->SetAimMode(false);

    if (CrosshairWidget)
    {
        CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}