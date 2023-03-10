#include "LatencyCompensationComponent.h"

ULatencyCompensationComponent::ULatencyCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULatencyCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void ULatencyCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

