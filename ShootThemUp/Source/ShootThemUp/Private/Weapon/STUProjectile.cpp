// Shoot Them Up Game, All Rights Reserved


#include "Weapon/STUProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h" //для выстрела из Launcher
#include "DrawDebugHelpers.h"//для рисования сферы
#include "Kismet/GameplayStatics.h"//для нанесения ущерба
#include "Weapon/Components/STUWeaponFXComponent.h"//для эфеектов


ASTUProjectile::ASTUProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>("SphereComponents");
	CollisionComponent->InitSphereRadius(5.0f); //устанавливаем радиус коллизии
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//оповещение о столкновении
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//блок с другими коллизиями 
	CollisionComponent->bReturnMaterialOnMove = true;

	SetRootComponent(CollisionComponent);

	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent"); //BP
	//настройка через код
	MovementComponent->InitialSpeed = 2000.0f; //скорость выстрела
	MovementComponent->ProjectileGravityScale = 0.0f; //гравитация

	WeaponFXComponent = CreateDefaultSubobject<USTUWeaponFXComponent>("WeaponFXComponent");//эффект
}

void ASTUProjectile::BeginPlay()
{
	Super::BeginPlay();

	check(MovementComponent);
	check(CollisionComponent);
	check(WeaponFXComponent);

	MovementComponent->Velocity = ShotDirection * MovementComponent->InitialSpeed; //выстрел (в BP Gravity Scale = 0 )
	CollisionComponent->IgnoreActorWhenMoving(GetOwner(),true); //функция для игнорирования акторов
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASTUProjectile::OnProjectileHit); //вызывается при столкновении
	SetLifeSpan(LifeSeconds); //удаление актора через 5 секунд

}

void ASTUProjectile::OnProjectileHit( //столконовение
	UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!GetWorld()) return;
	MovementComponent->StopMovementImmediately(); //останавливаем пулю

	//make Damage
	UGameplayStatics::ApplyRadialDamage(GetWorld(),  //
		DamageAmount,                                //
		GetActorLocation(),                          //
		DamageRadius,                                //
		UDamageType::StaticClass(),                  // 
		{GetOwner()},                                          //урон по себе
		this,                                        //Актор который вызвал ущерб
		GetController(),                                     //подсчёт кто кого подстрелил
		DoFullDamage);                               //

	//DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 24, FColor::Red, false, 5.0f);//рисуем сферу взрыва
	WeaponFXComponent->PlayImpactFX(Hit);
	Destroy();
}

AController* ASTUProjectile::GetController() const
{
	const auto Pawn = Cast<APawn>(GetOwner());
	return Pawn ? Pawn->GetController() : nullptr;
}