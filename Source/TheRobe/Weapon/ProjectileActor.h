
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileActor.generated.h"

UCLASS()
class THEROBE_API AProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AProjectileActor();
	virtual void Tick(float DeltaTime) override;

protected:
	
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

public:	

	

};
