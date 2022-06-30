// Shoot Them Up Game, All Rights Reserved


#include "Pickups/STUAmmoPickup.h"
#include "Components/STUHealthComponent.h"
#include "Components/STUWeaponComponent.h"
#include "STUUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogAmmoPickup, All, All);

bool ASTUAmmoPickup::GivePickupTo(APawn* PlayerPawn)//true-взяли, false не удалось
{
	const auto HealthComponent = STUUtils::GetSTUPlayerComponent<USTUHealthComponent>(PlayerPawn);
	if (!HealthComponent || HealthComponent->IsDead()) return false;

	const auto WeaponhComponent = STUUtils::GetSTUPlayerComponent<USTUWeaponComponent>(PlayerPawn);
	if (!WeaponhComponent ) return false;

	/*UE_LOG(LogAmmoPickup, Display, TEXT("Ammo was taken"));*/
	return WeaponhComponent->TryToAddAmmo(WeaponType, ClipsAmount);
}
