#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HJ_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;              // ✅ 추가
struct FInputActionValue;

UCLASS()
class HJ_PROJECT_API AHJ_PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHJ_PlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // IMC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputMappingContext* InputMappingContext;

    // Input Actions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UInputAction* FireAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputAction* AimAction;

    // ✅ Crosshair UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> CrosshairWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<UUserWidget> HudWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<UUserWidget> HudWidgetInstance;

    UPROPERTY()
    UUserWidget* CrosshairWidget;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    UUserWidget* GetHUDWidget() const;


    // 입력 처리 함수
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartJump(const FInputActionValue& Value);
    void StopJump(const FInputActionValue& Value);
    void StartFire();
    void StopFire();
    void StartAim(const FInputActionValue& Value);
    void StopAim(const FInputActionValue& Value);
};