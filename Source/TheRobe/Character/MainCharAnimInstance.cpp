// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharAnimInstance.h"
#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TheRobe/Weapon/Weapon.h"

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
	EquippedWeapon = MainCharacter->GetEquippedWeapon();
	bIsCrouched = MainCharacter->bIsCrouched;
	bAiming = MainCharacter->IsAiming();
	TurningInPlace = MainCharacter->GetTurningInPlace();
	
	// Offset Yaw for Strafing 
	FRotator AimRotation = MainCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MainCharacter->GetVelocity());
	FRotator DeltaRot  = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, dt, 6.f);
	YawOffset = DeltaRotation.Yaw;


	//
	CharacterRotationLastFrame = CharacterRotationCurrFrame;
	CharacterRotationCurrFrame = MainCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotationCurrFrame, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / dt;
	const float Interpolation = FMath::FInterpTo(Lean, Target, dt, 6.f);
	Lean = FMath::Clamp(Interpolation, -90.f, 90.f);

	AO_Yaw = MainCharacter->GetAO_Yaw();
	AO_Pitch = MainCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MainCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MainCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}
