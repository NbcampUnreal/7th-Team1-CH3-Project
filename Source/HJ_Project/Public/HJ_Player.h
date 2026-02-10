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
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 카메라 붐
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	// 카메라
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* FollowCamera;

	// 이동 함수
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
};
