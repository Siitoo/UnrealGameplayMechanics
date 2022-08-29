// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayMechanicsCharacter.generated.h"

UCLASS(config=Game)
class AGameplayMechanicsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = InteractionTrigger, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxInteractionTrigger;

public:
	AGameplayMechanicsCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jump Detection")
	float MaxDistanceFromWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jump Detection")
	float MaxHeightToJump;

protected:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ProcessJump();

	void HangToClimbUp();

	UFUNCTION(BlueprintCallable)
	void HangOff();

	UFUNCTION(BlueprintCallable)
	void StartJumpToClimb();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

private:

	UFUNCTION(meta = (AllowPrivateAccess = "true"))
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(meta = (AllowPrivateAccess = "true"))
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TriggerInteraction();

	void SelectCloseInteractableActor();

	void WallDetection();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	void ResetClimb();

	UFUNCTION(BlueprintCallable)
	void ToggleBlockInput();

private:

	bool bStartTriggerInteractions;
	TArray<AActor*> OverlappingActors;

	AActor* SelectedInteractableActor;
	int NumInteractableObjects;
	bool bCanJumpToClimb;
	FHitResult OutHitForWallJump;

	bool bBlockInput;

	FVector WallLocation;
	FVector WallNormal;

public:
	UPROPERTY(BlueprintReadOnly)
	bool bJumpToClimb;

	UPROPERTY(BlueprintReadOnly)
	bool bClimbUp;

	UPROPERTY(BlueprintReadOnly)
	bool bHangOff;
};

