// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Wall.generated.h"

UCLASS()
class THEROBE_API AWall : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AWall();
	virtual void Tick(float DeltaTime) override;

	
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnOverlapBegin(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

	UFUNCTION()
		virtual void OnEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);

	UPROPERTY(VisibleAnywhere, Category = "Wall Properties")
		UStaticMeshComponent* WallMesh;

	//box component is used for collison 
	UPROPERTY(VisibleAnywhere, Category = "Wall Properties")
		class UBoxComponent* AreaBox;

	UPROPERTY(VisibleAnywhere, Category = "Wall Properties")
	int insideTheBox = 0;

	UPROPERTY()
	class ULatencyCompensationComponent* LCC;

private:


	UPROPERTY()
	class AWeapon* Weapon;

public:	
	
};
