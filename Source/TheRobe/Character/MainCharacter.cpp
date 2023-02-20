// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "TheRobe/Weapon/Weapon.h"
#include "TheRobe/TheRobeComponent/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MainCharAnimInstance.h"
#include "TheRobe/TheRobe.h"
#include "TheRobe/PlayerController/MainCharPlayerController.h"
#include "TheRobe/GameMode/TheRobeGameMode.h"
#include "TimerManager.h"
#include "TheRobe/PlayerState/TheRobePlayerState.h"
#include "TheRobe/Weapon/WeaponTypes.h"


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

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.0;

	DisolveTimeLine = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void AMainCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	DOREPLIFETIME_CONDITION(AMainCharacter, OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AMainCharacter, Health);

}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMainCharacter::ReceiveDamage);
	}
	
}


void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementRep += DeltaTime;
		if (TimeSinceLastMovementRep > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculatePitch_AO();
	}
	HideCameraWhenCharacterClose();
	PollInit();
}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMainCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUP", this, &AMainCharacter::LookUP);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMainCharacter::EquipButtonActivated);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMainCharacter::CrouchButtonActivated);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMainCharacter::AimButtonActivated);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMainCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMainCharacter::FireButtonActivated);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMainCharacter::FireButtonDeactivated);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMainCharacter::ReloadButtonActivated);


}

void AMainCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}

}

void AMainCharacter::PlayFireMontage(bool bIsAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireMontage)
	{
		AnimInstance->Montage_Play(FireMontage);
	   
		//section name
		FName SName;
		SName = bIsAiming ? FName("RifleAim") : FName("RiffleHip");
		AnimInstance->Montage_JumpToSection(SName);
	}
}

void AMainCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		//section name
		FName SName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SName = FName("Rifle");
			break;
		}
		
		AnimInstance->Montage_JumpToSection(SName);
	}
}

void AMainCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMainCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AMainCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementRep = 0.f;
}

void AMainCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->DroppedWeapon();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AMainCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void AMainCharacter::MulticastElim_Implementation()
{
	if (MainCharPlayerController)
	{
		MainCharPlayerController->SetHudWeaponAmmo(0);
	}
	bIsElimmed = true;
	PlayElimMontage();

	//start dessolve material effect 
	if (DisolveMaterialInstance)
	{
		DynDisolveMaterialInstance = UMaterialInstanceDynamic::Create(DisolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynDisolveMaterialInstance);
		DynDisolveMaterialInstance->SetScalarParameterValue(TEXT("Disolve"), 0.55f);
		DynDisolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}

	StartMaterial();

	//disable movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (MainCharPlayerController)
	{
		DisableInput(MainCharPlayerController);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AMainCharacter::ElimTimerFinished()
{
	ATheRobeGameMode* GameMode = GetWorld()->GetAuthGameMode<ATheRobeGameMode>();
	if (GameMode)
	{
		GameMode->RequestRespawn(this, Controller);
	}

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

void AMainCharacter::EquipButtonActivated()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else 
		{
			ServerEquipButtonActivated();
		
		}
	}
}

void AMainCharacter::CrouchButtonActivated()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else 
	{
		Crouch();
	}
	
}

void AMainCharacter::ReloadButtonActivated()
{
	if (Combat)
	{
		Combat->Reload();
	}
}

void AMainCharacter::AimButtonActivated()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AMainCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void AMainCharacter::AimOffset(float dt)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)  //standing still and not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRot = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRot);
		AO_Yaw = DeltaAimRot.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(dt);
	}
	if (Speed > 0.f || bIsInAir) //running or jumping
	{
		bRotateRootBone = false;
		StartingAimRot = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculatePitch_AO();
}
void AMainCharacter::CalculatePitch_AO()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from the range [270, 360] to [-90, 0]
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
void AMainCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning; 
		return;
	}

	ProxyRotLastFrame = ProxyRot;
	ProxyRot = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRot, ProxyRotLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnTreshhold)
	{
		if (ProxyYaw > TurnTreshhold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnTreshhold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}

		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}
void AMainCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else 
	{

		Super::Jump();
	
	}
}
void AMainCharacter::FireButtonActivated()
{
	if (Combat)
	{
		Combat->FireButtonActivated(true);
	}
}

void AMainCharacter::FireButtonDeactivated()
{
	if (Combat)
	{
		Combat->FireButtonActivated(false);
	}
}

void AMainCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHealth();
	PlayHitReactMontage();
	if (Health == 0.f)
	{
		ATheRobeGameMode* GameMode = GetWorld()->GetAuthGameMode<ATheRobeGameMode>();
		if (GameMode)
		{
			MainCharPlayerController = MainCharPlayerController == nullptr ? Cast<AMainCharPlayerController>(Controller) : MainCharPlayerController;
			AMainCharPlayerController* AttackerController = Cast<AMainCharPlayerController>(InstigatorController);
			GameMode->PlayerEliminated(this, MainCharPlayerController, AttackerController);
		}
	}	
}

void AMainCharacter::PollInit()
{
	if (PlayerState == nullptr)
	{
		PlayerState = GetPlayerState<ATheRobePlayerState>();
		if (PlayerState)
		{
			PlayerState->AddToScore(0.f);
			PlayerState->AddToDefeats(0);
		}
	}
}

void AMainCharacter::TurnInPlace(float dt)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, dt, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRot = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AMainCharacter::ServerEquipButtonActivated_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}
void AMainCharacter::HideCameraWhenCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((CameraFollow->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float AMainCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AMainCharacter::OnRep_Health()
{
	PlayHitReactMontage();
	UpdateHealth();
}

void AMainCharacter::UpdateHealth()
{
	MainCharPlayerController = MainCharPlayerController == nullptr ? Cast<AMainCharPlayerController>(Controller) : MainCharPlayerController;
	if (MainCharPlayerController)
	{
		MainCharPlayerController->SetHealth(Health, MaxHealth);
	}
}

void AMainCharacter::UpdateMaterial(float DisolveVal)
{
	if (DynDisolveMaterialInstance)
	{
		DynDisolveMaterialInstance->SetScalarParameterValue(TEXT("Disolve"), DisolveVal);
	}
}

void AMainCharacter::StartMaterial()
{
	DisolveTrack.BindDynamic(this, &AMainCharacter::UpdateMaterial);
	if (DisolveCurve && DisolveTimeLine)
	{
		DisolveTimeLine->AddInterpFloat(DisolveCurve, DisolveTrack);
		DisolveTimeLine->Play();
	}
}

void AMainCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{

	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}
	
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{

		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}

	}

}

bool AMainCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AMainCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* AMainCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AMainCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

void AMainCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}
	
	if (LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
}

ECombatState AMainCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}





