// Shoot Them Up Game, All Rights Reserved


#include "Pickups/STUBasePickup.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

DEFINE_LOG_CATEGORY_STATIC(LogBasePickup, All, All);


ASTUBasePickup::ASTUBasePickup()
{

	PrimaryActorTick.bCanEverTick = true;

	//СОЗДАНИЯ СФЕРЫ
	CollisionComponent = CreateDefaultSubobject<USphereComponent>("SphereComponents");
	CollisionComponent->InitSphereRadius(50.0f); //устанавливаем радиус коллизии
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//оповещение о столкновении
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);//блок с другими коллизиями 
	SetRootComponent(CollisionComponent);

}


void ASTUBasePickup::BeginPlay()
{
	Super::BeginPlay();
	
	check(CollisionComponent);

	GenerationRotationYaw();
}

void ASTUBasePickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorLocalRotation(FRotator(0.0f, RotationYaw, 0.0f));
}

void ASTUBasePickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	const auto Pawn = Cast <APawn>(OtherActor);
	if (GivePickupTo(Pawn))//если удалось забрать
	{
		PickupWasTaken();//делаем невидимым
	}
	//UE_LOG(LogBasePickup, Display, TEXT("Pickup was taken"));//при пересечении выводится сообщение
}


bool ASTUBasePickup::GivePickupTo(APawn* PlayerPawn)
{
	return false;
}

void ASTUBasePickup::PickupWasTaken()//вызывается когда мы взяли pickup
{
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//отключаем колизию
	if (GetRootComponent())
	{
		GetRootComponent()->SetVisibility(false, true);//делаем невидимым
	}

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ASTUBasePickup::Respawn, RespawnTime);

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupTakenSound, GetActorLocation()); //звук при поднятии
}


void ASTUBasePickup::Respawn()
{
	GenerationRotationYaw();
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);//включаем колизию
	if (GetRootComponent())
	{
		GetRootComponent()->SetVisibility(true, true);//делаем невидимым
	}

}

void ASTUBasePickup::GenerationRotationYaw() //вычисляет угол
{
	const auto Direction = FMath::RandBool() ? 1.0f : -1.0f;
	RotationYaw = FMath::RandRange(1.0f, 2.0f) * Direction;
}

bool ASTUBasePickup::CouldBeTaken() const
{
	return !GetWorldTimerManager().IsTimerActive(RespawnTimerHandle);
	//return CouldBeTakenTest;
}