// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"
#include "ProjectileActor.h"

void AProjectile::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleSocket && World)
	{
		FTransform SockTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());

		//from muzzleflash socket to hit location 
		FVector ToTarget = HitTarget - SockTransform.GetLocation();
		FRotator TargetRot = ToTarget.Rotation();

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.Instigator = InstigatorPawn;
		
		AProjectileActor* SpawnedProj = nullptr;
		if (bUseLagCompensation)
		{
			if (InstigatorPawn->HasAuthority()) //server
			{
				if (InstigatorPawn->IsLocallyControlled()) //server, host - use replicated projectile
				{
					SpawnedProj = World->SpawnActor<AProjectileActor>(
						ProjectileClass,
						SockTransform.GetLocation(),
						TargetRot,
						SpawnParameters);
					SpawnedProj->bUseLagCompensation = false;
					SpawnedProj->Damage = Damage;
				}
				else  //server, not locally controlled - spawn not replicated proj  
				{
					SpawnedProj = World->SpawnActor<AProjectileActor>(
						LCAProjectileClass,
						SockTransform.GetLocation(),
						TargetRot,
						SpawnParameters);
				     	SpawnedProj->bUseLagCompensation = true;
				
				}
			}
			else // client using LCA
			{
				if (InstigatorPawn->IsLocallyControlled()) //spawn non replicated proj, use LCA
				{
					SpawnedProj = World->SpawnActor<AProjectileActor>(
						LCAProjectileClass,
						SockTransform.GetLocation(),
						TargetRot,
						SpawnParameters);
					 SpawnedProj->bUseLagCompensation = true;
					 SpawnedProj->TraceStart = SockTransform.GetLocation();
					 SpawnedProj->InitVelocity = SpawnedProj->GetActorForwardVector() * SpawnedProj->InitSpeed;
					 SpawnedProj->Damage = Damage;
				}
				else // client not locally controlled - spawn non replicated Proj, not using LCA
				{
					SpawnedProj = World->SpawnActor<AProjectileActor>(
						LCAProjectileClass,
						SockTransform.GetLocation(),
						TargetRot,
						SpawnParameters);
					SpawnedProj->bUseLagCompensation = false;
				
				}
			}
		}
		else  // weapon not using LCA
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnedProj = World->SpawnActor<AProjectileActor>(
					ProjectileClass,
					SockTransform.GetLocation(),
					TargetRot,
					SpawnParameters);
				SpawnedProj->bUseLagCompensation = false;
				SpawnedProj->Damage = Damage;
			}
		}
	}
}
