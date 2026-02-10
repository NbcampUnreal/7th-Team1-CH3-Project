#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HJ_Player.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class HJ_PROJECT_API AHJ_Player : public ACharacter
{
	GENERATED_BODY()

public:
	AHJ_Player();

protected:
	virtual void BeginPlay() override;

	// 카메라 붐
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;
};
