// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharAnimInstance.h"
#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMainCharAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
}

void UMainCharAnimInstance::NativeUpdateAnimation(float dt)
{
	Super::NativeUpdateAnimation(dt);

	//Take care of accessing the main character
	if (MainCharacter == nullptr)
	{
		MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
	}
	if (MainCharacter == nullptr) return;

	FVector Velocity = MainCharacter->GetVelocity();
	Velocity.Z = 0.f;
	fSpeed = Velocity.Size();

	
	bIsInAir = MainCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = MainCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	bWeaponEquipped = MainCharacter->IsWeaponEquipped();
	
}
