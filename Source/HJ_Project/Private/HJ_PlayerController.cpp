#include "HJ_PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"  
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "InputActionValue.h"
#include "HJ_GameState.h"
#include "HJ_Player.h"
#include "Components/CanvasPanelSlot.h"

AHJ_PlayerController::AHJ_PlayerController() : 
    HUDWidgetClass(nullptr),
    CrosshairWidget(nullptr),
    HUDWidgetInstance(nullptr),
    MainMenuWidgetClass(nullptr),
    MainMenuWidgetInstance(nullptr)
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

    if (CrosshairWidgetClass)
    {
        CrosshairWidget = CreateWidget<UUserWidget>(this, CrosshairWidgetClass);
        if (CrosshairWidget)
        {
            CrosshairWidget->AddToViewport();
            CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (HUDWidgetClass)
    {
        HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
        if (HUDWidgetInstance)
        {
            HUDWidgetInstance->AddToViewport();

            // ✅ 생성 직후 GameState에 있는 초기값을 UI에 반영
            if (AHJ_GameState* GS = GetWorld()->GetGameState<AHJ_GameState>())
            {
                GS->UpDateHUD();
            }
        }
    }

    FString CurrentMapName = GetWorld()->GetMapName();
    if (CurrentMapName.Contains("L_MainMenu"))
    {
        ShowMainMenu(false);
    }
    else
    {
        ShowGameHUD();
    }
}

UUserWidget* AHJ_PlayerController::GetHUDWidget() const
{
    return HUDWidgetInstance;
}

void AHJ_PlayerController::SetupInputComponent()
{
    APlayerController::SetupInputComponent();

    if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (MoveAction)
            EI->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHJ_PlayerController::Move);

        if (LookAction)
            EI->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHJ_PlayerController::Look);

        if (JumpAction)
        {
            EI->BindAction(JumpAction, ETriggerEvent::Started, this, &AHJ_PlayerController::StartJump);
            EI->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHJ_PlayerController::StopJump);
        }

        if (FireAction)
        {
            EI->BindAction(FireAction, ETriggerEvent::Started, this, &AHJ_PlayerController::StartFire);
            EI->BindAction(FireAction, ETriggerEvent::Completed, this, &AHJ_PlayerController::StopFire);
        }

        if (AimAction)
        {
            EI->BindAction(AimAction, ETriggerEvent::Started, this, &AHJ_PlayerController::StartAim);
            EI->BindAction(AimAction, ETriggerEvent::Completed, this, &AHJ_PlayerController::StopAim);
        }

        /* 🔥 Reload 바인딩 */
        if (ReloadAction)
        {
            EI->BindAction(ReloadAction, ETriggerEvent::Started,
                this, &AHJ_PlayerController::Reload);
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

void AHJ_PlayerController::Reload(const FInputActionValue& Value)
{
    AHJ_Player* MyPlayer = Cast<AHJ_Player>(GetPawn());
    if (!MyPlayer) return;

    MyPlayer->ReloadWeapon();   // 🔥 Player → Weapon → Reload()
}

void AHJ_PlayerController::ShowMainMenu(bool bIsRestart)
{
    if (HUDWidgetInstance)
    {
        HUDWidgetInstance->RemoveFromParent();
        HUDWidgetInstance = nullptr;
    }

    if (MainMenuWidgetInstance)
    {
        MainMenuWidgetInstance->RemoveFromParent();
        MainMenuWidgetInstance = nullptr;
    }

    if (MainMenuWidgetClass)
    {
        MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
        if (MainMenuWidgetInstance)
        {
            MainMenuWidgetInstance->AddToViewport();

            bShowMouseCursor = true;
            SetInputMode(FInputModeUIOnly());
        }

        AHJ_GameState* GS = GetWorld() ? GetWorld()->GetGameState<AHJ_GameState>() : nullptr;

        // "TitleText"라는 이름의 텍스트 블록을 찾아서 상태에 따라 글자를 바꿉니다.
        if (UTextBlock* TitleText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("GameTitle"))))
        {
            if (bIsRestart) // 죽거나 웨이브를 모두 클리어해서 메뉴가 떴을 때
            {
                if (GS && GS->BattleState == EBattleState::Victory)
                {
                    TitleText->SetText(FText::FromString(TEXT("승전!")));
                }
                else
                {
                    TitleText->SetText(FText::FromString(TEXT("함락되었습니다")));
                }
            }
            else // 처음에 게임을 켰을 때
            {
                TitleText->SetText(FText::FromString(TEXT("Hell 조선"))); // 원하는 게임 제목
            }
        }

        if (UTextBlock* ButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
        {
            if (bIsRestart)
            {
                ButtonText->SetText(FText::FromString(TEXT("재시작")));
            }
            else
            {
                ButtonText->SetText(FText::FromString(TEXT("시작")));
            }
        }

        if (UWidget* ButtonWidget = MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButton")))
        {
            if (UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(ButtonWidget->Slot))
            {
                if (bIsRestart)
                {
                    ButtonSlot->SetPosition(FVector2D(961.f, 732.f));
                }
                else
                {
                    ButtonSlot->SetPosition(FVector2D(285.f, 724.f));
                }
            }
        }

        if (UWidget* ButtonWidget = MainMenuWidgetInstance->GetWidgetFromName(TEXT("EndButton")))
        {
            if (UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(ButtonWidget->Slot))
            {
                if (bIsRestart)
                {
                    ButtonSlot->SetPosition(FVector2D(961.f, 912.f));
                }
                else
                {
                    ButtonSlot->SetPosition(FVector2D(285.f, 887.f));
                }
            }
        }

        // 2. 다른 TextBlock(예: 게임 제목 텍스트)의 위치 변경
        // 블루프린트 위젯에서 해당 TextBlock의 이름을 "TitleText"라고 지었다고 가정합니다.
        if (UWidget* TitleTextWidget = MainMenuWidgetInstance->GetWidgetFromName(TEXT("GameTitle")))
        {
            if (UCanvasPanelSlot* TitleSlot = Cast<UCanvasPanelSlot>(TitleTextWidget->Slot))
            {
                if (bIsRestart)
                {
                    TitleSlot->SetPosition(FVector2D(961.f, 172.f));
                }
                else
                {
                    TitleSlot->SetPosition(FVector2D(290.f, 152.f));
                }
            }
        }
    }
}



void AHJ_PlayerController::ShowGameHUD()
{
    if (HUDWidgetInstance)
    {
        HUDWidgetInstance->RemoveFromParent();
        HUDWidgetInstance = nullptr;
    }

    if (MainMenuWidgetInstance)
    {
        MainMenuWidgetInstance->RemoveFromParent();
        MainMenuWidgetInstance = nullptr;
    }

    if (HUDWidgetClass)
    {
        HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
        if (HUDWidgetInstance)
        {
            HUDWidgetInstance->AddToViewport();

            bShowMouseCursor = false;
            SetInputMode(FInputModeGameOnly());

            if (AHJ_GameState* GS = GetWorld()->GetGameState<AHJ_GameState>())
            {
                GS->UpDateHUD();
            }
        }
        
    }
}

void AHJ_PlayerController::StartGame()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName("L_Heungnyemun"));
}