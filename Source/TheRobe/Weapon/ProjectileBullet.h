// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileActor.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class THEROBE_API AProjectileBullet : public AProjectileActor
{
	GENERATED_BODY()
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
};
