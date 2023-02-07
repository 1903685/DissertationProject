// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"
#include "ProjectileActor.h"

void AProjectile::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority()) return;
	

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleSocket)
	{
		FTransform SockTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());

		//from muzzleflash socket to hit location 
		FVector ToTarget = HitTarget - SockTransform.GetLocation();
		FRotator TargetRot = ToTarget.Rotation();

		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectileActor>(
					ProjectileClass,
					SockTransform.GetLocation(),
					TargetRot,
					SpawnParameters
					);
			}
		}
	}
}
