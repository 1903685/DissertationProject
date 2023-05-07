// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Projectile.generated.h"

/**
 * 
 */
UCLASS()
class THEROBE_API AProjectile : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget);


private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectileActor> ProjectileClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectileActor> LCAProjectileClass;

};
