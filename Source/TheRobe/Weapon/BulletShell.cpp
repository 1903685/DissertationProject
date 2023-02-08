

#include "BulletShell.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ABulletShell::ABulletShell()
{
	PrimaryActorTick.bCanEverTick = false;

	ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellMesh"));
	SetRootComponent(ShellMesh);
	ShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	ShellMesh->SetSimulatePhysics(true);
	ShellMesh->SetEnableGravity(true);
	ShellMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
}

void ABulletShell::BeginPlay()
{
	Super::BeginPlay();

	ShellMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	ShellMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);

}

void ABulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (BulletShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BulletShellSound, GetActorLocation());
	}
	Destroy();

}
