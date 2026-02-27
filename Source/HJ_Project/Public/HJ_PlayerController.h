#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HJ_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
struct FInputActionValue;

UCLASS()
class HJ_PROJECT_API AHJ_PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHJ_PlayerController();

protected:
    virtual void SetupInputComponent() override;

    /* ================= Input ================= */

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputMappingContext* InputMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* FireAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* AimAction;

    /* 🔥 Reload 추가 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* ReloadAction;

    /* ================= UI ================= */

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> CrosshairWidgetClass;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> HudWidgetClass;

    UPROPERTY()
    UUserWidget* HudWidgetInstance;


    UPROPERTY()
    UUserWidget* CrosshairWidget;


    /* ================= Input Functions ================= */




    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartJump(const FInputActionValue& Value);
    void StopJump(const FInputActionValue& Value);
    void StartFire();
    void StopFire();
    void StartAim(const FInputActionValue& Value);
    void StopAim(const FInputActionValue& Value);
    void Reload(const FInputActionValue& Value);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<UUserWidget> HUDWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    UUserWidget* HUDWidgetInstance;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
    UUserWidget* MainMenuWidgetInstance;

    UFUNCTION(BlueprintPure, Category = "HUD")
    UUserWidget* GetHUDWidget() const;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowGameHUD();
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowMainMenu(bool bIsRestart);
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void StartGame();

    virtual void BeginPlay() override;
};