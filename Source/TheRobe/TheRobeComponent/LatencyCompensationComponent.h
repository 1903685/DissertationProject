// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LatencyCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInfo
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;

	UPROPERTY()
		FRotator Rotation;

	UPROPERTY()
		FVector BoxExtent;
};


USTRUCT(BlueprintType)
struct FramePackage 
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInfo> HitBoxInfo;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEROBE_API ULatencyCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULatencyCompensationComponent();
	friend class AMainCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void FramePackageStory(const FramePackage& Pack, const FColor& Color);
protected:
	
	virtual void BeginPlay() override;
	void FramePackageSaver(FramePackage& Pack);
private:

	UPROPERTY()
	AMainCharacter* Character;

	UPROPERTY()
	class AMainCharPlayerController* Controller;

	TDoubleLinkedList<FramePackage> HistoryFrame;

	UPROPERTY(EditAnywhere)
	float MaxHistoryTime = 4.f;

public:
	
};
