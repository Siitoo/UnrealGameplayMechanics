// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayMechanicsCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Interfaces/InteractionInterface.h"
#include "ActorComponents/DialogComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"

//////////////////////////////////////////////////////////////////////////
// AGameplayMechanicsCharacter

AGameplayMechanicsCharacter::AGameplayMechanicsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	BoxInteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxInteractionTriggerComponent"));
	BoxInteractionTrigger->SetupAttachment(GetCapsuleComponent());
	BoxInteractionTrigger->SetBoxExtent(FVector(50.f, 50.f, 90.f));
	BoxInteractionTrigger->SetRelativeLocation(FVector(85.f, 0.f, 0.f));
	BoxInteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGameplayMechanicsCharacter::OnOverlapBegin);
	BoxInteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &AGameplayMechanicsCharacter::OnOverlapEnd);

	bBlockInput = false;

	// Interaction
	bStartTriggerInteractions = false;
	SelectedInteractableActor = nullptr;
	NumInteractableObjects = 0;

	//Climbable wall detection
	MaxDistanceFromWall = 100.f;
	MaxHeightToJump = 300.f;

	//Climb controllers
	bCanJumpToClimb = false;
	bJumpToClimb = false;
	bClimbUp = false;
	bHangOff = false;

	MaxPlayerHealth = 5000.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AGameplayMechanicsCharacter::BeginPlay()
{
	Super::BeginPlay();
	ActualPlayerHealth = MaxPlayerHealth;
}

void AGameplayMechanicsCharacter::Tick(float DeltaTime)
{
	if (bStartTriggerInteractions && NumInteractableObjects > 1)
	{
		SelectCloseInteractableActor();
	}
	
	if (!bJumpToClimb)
	{
		WallDetection();
	}
}

//////////////////////////////////////////////////////////////////////////
// Interaction

void AGameplayMechanicsCharacter::SelectCloseInteractableActor()
{
	BoxInteractionTrigger->GetOverlappingActors(OverlappingActors);

	AActor* OverlappedActor = nullptr;
	float NewLength = 0.f;
	float SelectedActorLength = (SelectedInteractableActor->GetActorLocation() - BoxInteractionTrigger->GetComponentLocation()).Length();

	for (int Index = 0; Index < OverlappingActors.Num(); ++Index)
	{
		OverlappedActor = OverlappingActors[Index];
		NewLength = (OverlappedActor->GetActorLocation() - BoxInteractionTrigger->GetComponentLocation()).Length();

		if (NewLength < SelectedActorLength)
		{
			SelectedActorLength = NewLength;

			IInteractionInterface* InteractInterface = Cast<IInteractionInterface>(SelectedInteractableActor);
			InteractInterface->CancelInteraction();

			SelectedInteractableActor = OverlappedActor;
			InteractInterface = Cast<IInteractionInterface>(SelectedInteractableActor);
			InteractInterface->PrepareInteraction();
		}

	}
}

void AGameplayMechanicsCharacter::TriggerInteraction()
{
	if (bStartTriggerInteractions)
	{
		IInteractionInterface* InteractInterface = Cast<IInteractionInterface>(SelectedInteractableActor);

		if (InteractInterface == nullptr)
		{
			InteractInterface = Cast<IInteractionInterface>(SelectedInteractableActor->GetComponentByClass(UDialogComponent::StaticClass()));
		}

		InteractInterface->Interaction();
	}
}

void AGameplayMechanicsCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (NumInteractableObjects == 0)
	{
		SelectedInteractableActor = OtherActor;
		bStartTriggerInteractions = true;
		IInteractionInterface* InteractInterface = Cast<IInteractionInterface>(OtherActor);

		if (InteractInterface == nullptr)
		{
			InteractInterface = Cast<IInteractionInterface>(OtherActor->GetComponentByClass(UDialogComponent::StaticClass()));
		}

		InteractInterface->PrepareInteraction();
	}

	NumInteractableObjects++;
}

void AGameplayMechanicsCharacter::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IInteractionInterface* InteractInterface = Cast<IInteractionInterface>(OtherActor);

	if (InteractInterface == nullptr)
	{
		InteractInterface = Cast<IInteractionInterface>(OtherActor->GetComponentByClass(UDialogComponent::StaticClass()));
	}

	InteractInterface->CancelInteraction();

	NumInteractableObjects--;

	if (NumInteractableObjects == 0)
	{
		SelectedInteractableActor = nullptr;
		bStartTriggerInteractions = false;
	}
	else if (OtherActor == SelectedInteractableActor)
	{
		SelectCloseInteractableActor();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGameplayMechanicsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGameplayMechanicsCharacter::ProcessJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AGameplayMechanicsCharacter::TriggerInteraction);
	

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AGameplayMechanicsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AGameplayMechanicsCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AGameplayMechanicsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AGameplayMechanicsCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AGameplayMechanicsCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AGameplayMechanicsCharacter::TouchStopped);
}

void AGameplayMechanicsCharacter::ToggleBlockInput() 
{
	bBlockInput = !bBlockInput;
}

void AGameplayMechanicsCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AGameplayMechanicsCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AGameplayMechanicsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AGameplayMechanicsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AGameplayMechanicsCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr))
	{
		if ((Value != 0.0f) && !bBlockInput)
		{
			if (!bJumpToClimb)
			{
				// find out which way is forward
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				// get forward vector
				const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
				AddMovementInput(Direction, Value);
			}
			else
			{
				if (Value > 0.0f)
				{
					HangToClimbUp();
				}
				else
				{
					if (!bHangOff)
					{
						bHangOff = true;
					}	
				}

				ToggleBlockInput();
			}
		}
		
	}
}

void AGameplayMechanicsCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) && !bJumpToClimb && !bBlockInput)
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

//////////////////////////////////////////////////////////////////////////
// JUMP

void AGameplayMechanicsCharacter::WallDetection()
{
	UWorld* World = GetWorld();

	FVector LineStart = GetActorLocation();
	FVector LineEnd = GetActorLocation() + GetCapsuleComponent()->GetForwardVector() * MaxDistanceFromWall;

	FCollisionQueryParams CollisionParams;

	bCanJumpToClimb = World->LineTraceSingleByObjectType(OutHitForWallJump, LineStart, LineEnd, ECC_WorldStatic, CollisionParams);

	if (!bCanJumpToClimb)
	{
		bJumpToClimb = false;
	}
}

void AGameplayMechanicsCharacter::ProcessJump()
{
	UWorld* World = GetWorld();
	bool bLineTraceHit = false;

	if (bCanJumpToClimb && !bJumpToClimb)
	{
		WallLocation = OutHitForWallJump.Location;
		WallNormal = OutHitForWallJump.Normal;

		FVector LineStart =  OutHitForWallJump.ImpactPoint + FVector::UpVector * MaxHeightToJump;
		FVector LineEnd = OutHitForWallJump.ImpactPoint;
		TArray<AActor*> ActorsToIgnore;

		bLineTraceHit = UKismetSystemLibrary::SphereTraceSingle(World, LineStart, LineEnd, 10.f, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHitForWallJump, true);

		// To exclude wall that is larger than the sphere trace LineEnd
		if (OutHitForWallJump.ImpactNormal != FVector::UpVector)
		{
			bLineTraceHit = false;
		}
	}

	//offset for hitted object that are small
	float ZLocation = OutHitForWallJump.Location.Z - GetActorLocation().Z;

	if (bLineTraceHit && ZLocation > 60.f && !GetCharacterMovement()->IsFalling())
	{	
		if (!bJumpToClimb)
		{
			ToggleBlockInput();
			bJumpToClimb = true;
		}	
	}
	else
	{
		if (GetCharacterMovement()->IsJumpAllowed() && !bBlockInput)
		{
			ACharacter::Jump();
		}
		else
		{
			ACharacter::StopJumping();
		}
	}
}

void AGameplayMechanicsCharacter::StartJumpToClimb()
{
	float ZLocation = OutHitForWallJump.Location.Z - 80.f;
	FVector WallNormalWithOffset = WallNormal * 30.f;
	FVector WallNormalInvert = WallNormal * -1.0f;

	FVector PositionVector = FVector(WallLocation.X + WallNormalWithOffset.X, WallLocation.Y + WallNormalWithOffset.Y, ZLocation);
	FRotator Rotation = UKismetMathLibrary::MakeRotFromX(WallNormalInvert);

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	//By Default the Mesh position is 0,0,-90
	FVector MeshRelativeLocation = GetMesh()->GetRelativeLocation();

	//By Default the Mesh rotation is 270 on Z
	FRotator OriginalMeshRelativeRotator = FRotator(0.f, 270.f, 0.f);

	//ofsset to mesh for clipping and distance
	MeshRelativeLocation.X = -28.f;
	MeshRelativeLocation.Z -= 65.f;

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;

	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), PositionVector, Rotation, true, true, 0.2f, true, EMoveComponentAction::Move, LatentInfo);

	LatentInfo.UUID = 1;

	UKismetSystemLibrary::MoveComponentTo(GetMesh(), MeshRelativeLocation, OriginalMeshRelativeRotator, true, true, 0.3f, true, EMoveComponentAction::Move, LatentInfo);
}

void AGameplayMechanicsCharacter::HangToClimbUp()
{
	//FVector OriginalMeshRelativeLocation = FVector(0.f, 0.f, -230.f);;
	//FRotator OriginalMeshRelativeRotator = FRotator(0.f, 270.f, 0.f);

	FVector NewCapsuleLocation = OutHitForWallJump.ImpactPoint;
	NewCapsuleLocation.Z += 96.f; // Capsule Height

	bClimbUp = true;

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;

	//UKismetSystemLibrary::MoveComponentTo(GetMesh(), OriginalMeshRelativeLocation, OriginalMeshRelativeRotator, true, true, 1.0f, true, EMoveComponentAction::Move, LatentInfo);
	
	LatentInfo.ExecutionFunction = "ResetClimb";
	LatentInfo.UUID = 1;
	LatentInfo.Linkage = 0;

	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), NewCapsuleLocation, GetActorRotation(), true, true, 1.0f, true, EMoveComponentAction::Move, LatentInfo);
}

void AGameplayMechanicsCharacter::HangOff()
{
	ResetClimb();
}

void AGameplayMechanicsCharacter::ResetClimb()
{
	FVector OriginalMeshRelativeLocation = FVector(0.f, 0.f, -90.f);
	FRotator OriginalMeshRelativeRotator = FRotator(0.f, 270.f, 0.f);

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(GetMesh(), OriginalMeshRelativeLocation, OriginalMeshRelativeRotator, true, true, 0.1f, true, EMoveComponentAction::Move, LatentInfo);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	
	ToggleBlockInput();

	bCanJumpToClimb = false;
	bJumpToClimb = false;
	bClimbUp = false;
	bHangOff = false;
}

//////////////////////////////////////////////////////////////////////////
// Dialog

void AGameplayMechanicsCharacter::StartDialog(UUserWidget* Widget)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	PlayerController->SetShowMouseCursor(true);

	InputComponent->ClearActionBindings();
	InputComponent->AxisBindings.Reset(0);
	InputComponent->BindAction("Interact", IE_Pressed, this, &AGameplayMechanicsCharacter::TriggerInteraction);
	
	UserWidget = Widget;
	UserWidget->SetOwningPlayer(UGameplayStatics::GetPlayerController(GetWorld(),0));
	UserWidget->AddToViewport();
	
}

void AGameplayMechanicsCharacter::StopDialog(UUserWidget* Widget)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	PlayerController->SetShowMouseCursor(false);
	InputComponent->AxisBindings.Reset(0);
	PlayerController->SetInputMode(FInputModeGameOnly());

	SetupPlayerInputComponent(InputComponent);

	UserWidget->RemoveFromViewport();

	UserWidget = nullptr;
}

// Health and Mana progress bar

float AGameplayMechanicsCharacter::GetCurrentHealth()
{
	return ActualPlayerHealth;
}

float AGameplayMechanicsCharacter::GetHealthAsRatio()
{
	return ActualPlayerHealth/MaxPlayerHealth;
}

float AGameplayMechanicsCharacter::GetCurrentMana()
{
	return 0.0f;
}

float AGameplayMechanicsCharacter::GetManaAsRatio()
{
	return 0.0f;
}
