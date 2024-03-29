// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "TheRobe/Character/MainCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "BulletShell.h"
#include "Engine/SkeletalMeshSocket.h"
#include "TheRobe/PlayerController/MainCharPlayerController.h"
#include "TheRobe/TheRobeComponent/Wall.h"

// Sets default values
AWeapon::AWeapon()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidget->SetupAttachment(RootComponent);

	wall = CreateDefaultSubobject<AWall>(FName("wall"));

}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();

		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}
	
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	//DOREPLIFETIME(AWeapon, Ammunition);
	DOREPLIFETIME_CONDITION(AWeapon, bUseLagCompensation, COND_OwnerOnly);
}


void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMainCharacter* MainCharacter = Cast<AMainCharacter>(OtherActor);

	if (MainCharacter && PickUpWidget)
	{
		MainCharacter->SetOverlappingWeapon(this);
	}

}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	AMainCharacter* MainCharacter = Cast<AMainCharacter>(OtherActor);

	if (MainCharacter && PickUpWidget)
	{
		MainCharacter->SetOverlappingWeapon(nullptr);
	}

}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	
	case EWeaponState::EWS_Equipped:
		ShowPickUpWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}



}
void AWeapon::SetHUDAmmunition()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMainCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter)
	{
		OwnerController = OwnerController == nullptr ? Cast<AMainCharPlayerController>(OwnerCharacter->Controller) : OwnerController;
		if (OwnerController)
		{
			OwnerController->SetHudWeaponAmmo(Ammunition);
		}
	}
}

void AWeapon::SpendRound()
{
   Ammunition = FMath::Clamp(Ammunition - 1, 0, MaxCapacity);
   SetHUDAmmunition();
   if (HasAuthority())
   {
	   UpdateAmmoClient(Ammunition);
   }
   else if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
   {
	   ++Sequence;
   }
}


void AWeapon::UpdateAmmoClient_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
    Ammunition = ServerAmmo;
	--Sequence;
	Ammunition -= Sequence;
	SetHUDAmmunition();
}

void AWeapon::AddAmmunition(int32 AmmoToAdd)
{
	Ammunition = FMath::Clamp(Ammunition + AmmoToAdd, 0, MaxCapacity);
	SetHUDAmmunition();
	AddAmmoClient(AmmoToAdd);
}

void AWeapon::AddAmmoClient_Implementation(int32 AmmoToAdd)
{
	if(HasAuthority()) return;
	Ammunition = FMath::Clamp(Ammunition + AmmoToAdd, 0, MaxCapacity);
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMainCharacter>(GetOwner()) : OwnerCharacter;
	SetHUDAmmunition();
}

//void AWeapon::OnRep_Ammunition()
//{
//	SetHUDAmmunition();
//}
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
	}
	else
	{
		SetHUDAmmunition();
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickUpWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
   
}

void AWeapon::ShowPickUpWidget(bool bShowWidget)
{

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(bShowWidget);
	}

}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnim)
	{
		WeaponMesh->PlayAnimation(FireAnim, false);
	}
	if (BulletShellClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SockTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ABulletShell>(
					BulletShellClass,
					SockTransform.GetLocation(),
					SockTransform.GetRotation().Rotator()
					);
			}

		}
	}
	
		SpendRound();
}

void AWeapon::DroppedWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}


bool AWeapon::IsEmpty()
{
	return Ammunition <= 0;
}