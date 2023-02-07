

#include "BulletShell.h"

ABulletShell::ABulletShell()
{
	PrimaryActorTick.bCanEverTick = false;

	ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellMesh"));
	SetRootComponent(ShellMesh);
}

void ABulletShell::BeginPlay()
{
	Super::BeginPlay();
	
}
