// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create CameraBoom and attach it to the mesh
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	//Create follow camera and attach it to the CameraBoom
	CameraFollow = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraFollow"));
	CameraFollow->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraFollow->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMainCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUP", this, &AMainCharacter::LookUP);
}

void AMainCharacter::MoveForward(float _val)
{
	if (Controller != nullptr && _val != 0.f) 
	{
	
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector _dir(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(2 * _dir, _val);
	}
}

void AMainCharacter::MoveRight(float _val)
{
	if (Controller != nullptr && _val != 0.f)
	{

		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector _dir(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(2 * _dir, _val);
	}
}

void AMainCharacter::Turn(float _val)
{
	AddControllerYawInput(_val);
}

void AMainCharacter::LookUP(float _val)
{
	AddControllerPitchInput(_val);
}

void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



