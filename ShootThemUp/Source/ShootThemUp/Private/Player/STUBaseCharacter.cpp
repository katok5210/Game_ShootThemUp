// Shoot Them Up Game, All Rights Reserved


#include "Player/STUBaseCharacter.h"
#include "Components/STUCharacterMovementComponent.h"
#include "Components/STUHealthComponent.h"
#include "Components/STUWeaponComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


DEFINE_LOG_CATEGORY_STATIC(LogBaseCharacter, All, All);


ASTUBaseCharacter::ASTUBaseCharacter(const FObjectInitializer& ObjInit)
	: Super(ObjInit.SetDefaultSubobjectClass<USTUCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<USTUHealthComponent>("HealthComponent");
	WeaponComponent = CreateDefaultSubobject<USTUWeaponComponent>("WeaponComponent");
}


void ASTUBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	check(HealthComponent); //проверка
	check(GetCharacterMovement());
	check(GetCapsuleComponent());
	check(GetMesh());

	OnHealthChanged(HealthComponent->GetHealth(), 0.0f);
	HealthComponent->OnDeath.AddUObject(this, &ASTUBaseCharacter::OnDeath);
	HealthComponent->OnHealthChanged.AddUObject(this, &ASTUBaseCharacter::OnHealthChanged); //отоброжение жизней

	LandedDelegate.AddDynamic(this, &ASTUBaseCharacter::OnGroundLanded);	//урон при падении (контакт персонажа с поверхностью)
	

}

void ASTUBaseCharacter::OnHealthChanged(float Health, float HealthDelta)
{

}

void ASTUBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ASTUBaseCharacter::IsRunning() const //проверка для бега (*вектор скорости не равен 0)
{
	return false;
}

float ASTUBaseCharacter::GetMovementDirection() const //Математика (вычисляем вектор)
{
	if (GetVelocity().IsZero()) return 0.0f; //оптимизация (если стоит не проверяем)
	const auto VelocityNormal = GetVelocity().GetSafeNormal(); //высчитываем нормаль нешего вектора скорости
	const auto AngleBetween = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), VelocityNormal));//высчитываем скалярное произведение,получаем угол между векторами 
	const auto CrossProduct = FVector::CrossProduct(GetActorForwardVector(), VelocityNormal);//высчитываем ортогональный вектор 
	const auto Degrees = FMath::RadiansToDegrees(AngleBetween);
	return CrossProduct.IsZero() ? Degrees : Degrees * FMath::Sign(CrossProduct.Z);// переводим в градусы и умножаем на знак координат Z 
}

void ASTUBaseCharacter::OnDeath() // вызывается когда УМЕРЛИ 
{
	UE_LOG(LogBaseCharacter, Display, TEXT("DEAD: %s"), *GetName());
	//PlayAnimMontage(DeathAnimMontage); //для проигрывания анимации смерти

	GetCharacterMovement()->DisableMovement();//Отнимаем управление после смерти 
	SetLifeSpan(5.0f); //уничтожение персонажа

	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); //игнорировать коллизию после смерти
	WeaponComponent->StopFire(); //при смене оружия не можем стрелять(или смерти)
	WeaponComponent->Zoom(false); //при смерти выключаем прицеливание 

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //Добавление физики смерти
	GetMesh()->SetSimulatePhysics(true);

	UGameplayStatics::PlaySoundAtLocation(GetWorld(),DeathSound,GetActorLocation());//звук смерти 
}

void ASTUBaseCharacter::OnGroundLanded(const FHitResult& Hit) // урон при падении
{
	const auto FallVelocityZ = -GetVelocity().Z; 
	/*UE_LOG(LogBaseCharacter, Display, TEXT("On landed %f"), FallVelocityZ);*/ //Вычисляем падение!

	if (FallVelocityZ < LandedDamageVelosity.X) return; // проверка если не допускает урн

	const auto FinalDamage = FMath::GetMappedRangeValueClamped(LandedDamageVelosity, LandedDamage, FallVelocityZ);
	TakeDamage(FinalDamage, FDamageEvent{}, nullptr, nullptr);

	UE_LOG(LogBaseCharacter, Display, TEXT("DAMAGE %f"), FinalDamage);
}

void ASTUBaseCharacter::SetPlayerColor(const FLinearColor& Color) //устанавливает параметр материалу
{
	const auto MaterialInst = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
	if (!MaterialInst) return;

	MaterialInst->SetVectorParameterValue(MaterialColorName, Color);
}

void ASTUBaseCharacter::TurnOff()
{
	WeaponComponent->StopFire();
	WeaponComponent->Zoom(false);
	Super::TurnOff();
}


void ASTUBaseCharacter::Reset()
{
	WeaponComponent->StopFire();
	WeaponComponent->Zoom(false);
	Super::Reset();
}