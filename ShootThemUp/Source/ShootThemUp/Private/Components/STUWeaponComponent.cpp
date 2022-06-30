// Shoot Them Up Game, All Rights Reserved


#include "Components/STUWeaponComponent.h"
#include "Weapon/STUBaseWeapon.h"
#include "GameFramework/Character.h"
#include "Animations/STUEquipFinishedAnimNotify.h"
#include "Animations/STUReloadFinishedAnimNotify.h"
#include "Animations/AnimUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogWeaponComponent, All, All)

constexpr static int32 WeaponNum = 2;

USTUWeaponComponent::USTUWeaponComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}


void USTUWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	
	checkf(WeaponData.Num() == WeaponNum, TEXT ("Our character can hold only %i weapon items"), WeaponNum);//проверка на установленные оружия(2)

	CurrentWeaponIndex = 0;
	InitAnimations();
	SpawnWeapons();
	EquipWeapon(CurrentWeaponIndex);
	
}

void USTUWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) //уничтожаем оружия 
{
	CurrentWeapon = nullptr;
	for (auto Weapon : Weapons) //проходимся по массиву
	{
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); //отсоединяем актор от меша
		Weapon->Destroy();
	}
	Weapons.Empty();//очищаем массив Weapons
	Super::EndPlay(EndPlayReason);
}

void USTUWeaponComponent::SpawnWeapons()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());//делаем Cast на pawn, возвращает функцию GetOwner
	if (!Character || !GetWorld() ) return; //если не персонажа мы не спавним оружие 

	for (auto OneWeaponData : WeaponData) //Смена оружия
	{
		auto Weapon = GetWorld()->SpawnActor<ASTUBaseWeapon>(OneWeaponData.WeaponClass); //спавним оружие
		if (!Weapon) continue;		 //проверка что объект создался

		Weapon->OnClipEmpty.AddUObject(this, &USTUWeaponComponent::OnClipEmpty);//подписываемся на делегат OnClipEmpty
		Weapon->SetOwner(Character);//для стрельбы
		Weapons.Add(Weapon);

		AttachWeaponToSocket(Weapon, Character->GetMesh(), WeaponArmorySocketName);
	}
} 


void USTUWeaponComponent::AttachWeaponToSocket(ASTUBaseWeapon* Weapon,USceneComponent* SceneComponent,const FName& SocketName)
{
	if (!Weapon || !SceneComponent) return;
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
	Weapon->AttachToComponent(SceneComponent, AttachmentRules, SocketName);;  //функция присоединения
}

void USTUWeaponComponent::EquipWeapon(int32 WeaponIndex) //смена оружия
{
	if (WeaponIndex < 0 || WeaponIndex >= Weapons.Num())
	{
		UE_LOG(LogWeaponComponent, Warning, TEXT("Invalid Weapon index"));
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	if (CurrentWeapon)
	{
		CurrentWeapon->Zoom(false); //при смене оружия выключается прицеливание
		CurrentWeapon->StopFire(); //при смене оружия не можем стрелять
		AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponArmorySocketName);
	}

	CurrentWeapon = Weapons[WeaponIndex];//смена оружия
	//CurrentReloadAnimMontage = WeaponData[WeaponIndex].ReloadAnimMontage;//меняем анимациюперезарядки после смены оружия
	
	const auto CurrentsWeaponData = WeaponData.FindByPredicate([&](const FWeaponData& Data) //каждый раз будем искать в массиве WeaponData структуру
		{return Data.WeaponClass == CurrentWeapon->GetClass(); }); //которая соответсвует текущему оружию,после нахождение берём анимацию перезарядки
	CurrentReloadAnimMontage = CurrentsWeaponData ? CurrentsWeaponData->ReloadAnimMontage : nullptr;

	AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponEquipSocketName);
	EquipAnimInProgress = true;
	PlayAnimMontage(EquipAnimMontage); //при смене оружия проигрывается анимация 
}


void USTUWeaponComponent::StartFire()
{
	if (!CanFire()) return;
	CurrentWeapon->StartFire();
}

void USTUWeaponComponent::StopFire()
{
	if (!CurrentWeapon) return;
	CurrentWeapon->StopFire();
}


void USTUWeaponComponent::NextWeapon()
{
	if (!CanEquip()) return;
	CurrentWeaponIndex = (CurrentWeaponIndex + 1) % Weapons.Num(); //вычисление при смене оружия
	EquipWeapon(CurrentWeaponIndex);
}


void USTUWeaponComponent::PlayAnimMontage(UAnimMontage* Animation)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	Character->PlayAnimMontage(Animation);
}


void USTUWeaponComponent::InitAnimations() 
{
	auto EquipFinishedNotify = AnimUtils::FindNotifyByClass<USTUEquipFinishedAnimNotify>(EquipAnimMontage);
	if (EquipFinishedNotify)
	{
		EquipFinishedNotify->OnNotified.AddUObject(this, &USTUWeaponComponent::OnEauirFinished);
	}else
	{
		UE_LOG(LogWeaponComponent,Error, TEXT("Equip anim notify is forgotten to set"));//если не найдена анимация
		checkNoEntry();
	}

	for (auto OneWeaponData:WeaponData)//для анимации перезарядки пройдёмся по массиву
	{
		auto ReloadFinishedNotify = AnimUtils::FindNotifyByClass<USTUReloadFinishedAnimNotify>(OneWeaponData.ReloadAnimMontage);
		if (!ReloadFinishedNotify)
		{
			UE_LOG(LogWeaponComponent, Error, TEXT("Reload anim notify is forgotten to set"));//если не найдена анимация
			checkNoEntry();
		}
		ReloadFinishedNotify->OnNotified.AddUObject(this, &USTUWeaponComponent::OnReloadFinished);

	}
}
void USTUWeaponComponent::OnEauirFinished(USkeletalMeshComponent* MeshComp)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || MeshComp != Character->GetMesh()) return;

	EquipAnimInProgress = false;
}

void USTUWeaponComponent::OnReloadFinished(USkeletalMeshComponent* MeshComp)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || MeshComp != Character->GetMesh()) return;

	ReloadAnimInProgress = false;
}

bool USTUWeaponComponent::CanFire() const //true = можем стрелять
{
	return CurrentWeapon && !EquipAnimInProgress && !ReloadAnimInProgress;
}

bool USTUWeaponComponent::CanEquip() const //true = можем сменить оружие
{
	return !EquipAnimInProgress && !ReloadAnimInProgress;
}

bool USTUWeaponComponent::CanReload() const //true = можем стрелять
{
	return CurrentWeapon//
		&& !EquipAnimInProgress //
		&& !ReloadAnimInProgress//
		&& CurrentWeapon->CanReload();
}			

void USTUWeaponComponent::Reload()
{
	ChangeClip();
}

void USTUWeaponComponent::OnClipEmpty(ASTUBaseWeapon* AmmoEmptyWeapon)
{
	if (!AmmoEmptyWeapon) return;
	if (CurrentWeapon == AmmoEmptyWeapon)//если при смене оружие нужно перезарядить, пеезарядка автоматическая
	{
		ChangeClip();
	}
	else
	{
		for (const auto Weapon : Weapons)
		{
			if (Weapon == AmmoEmptyWeapon)
			{
				Weapon->ChangeClip();
			}
		}
	}
}


void USTUWeaponComponent::ChangeClip() //вся логика перезарядки
{
	if (!CanReload()) return;
	CurrentWeapon->StopFire();//при начале перезарядки останавливаем стрельбу
	CurrentWeapon->ChangeClip();
	ReloadAnimInProgress = true;
	PlayAnimMontage(CurrentReloadAnimMontage);//проигрывание анимации
}

bool USTUWeaponComponent::GetCurrentWeaponUIData(FWeaponUIData& UIData) const
{
	if (CurrentWeapon)
	{
		UIData = CurrentWeapon->GetUIData();
		return true;
	}
	return false;
}


bool USTUWeaponComponent::GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const
{
	if (CurrentWeapon)
	{
		AmmoData = CurrentWeapon->GetAmmoData();
		return true;
	}
	return false;
}

bool USTUWeaponComponent::TryToAddAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType, int32 ClipsAmount)
{
	for (const auto Weapon : Weapons)//проходимся по всему массиву
	{
		if (Weapon && Weapon->IsA(WeaponType))
		{
			return Weapon->TryToAddAmmo(ClipsAmount);
		}
	}
	return false;
}

bool USTUWeaponComponent::NeedAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType)
{
	for (const auto Weapon : Weapons)//проходимся по всему массиву
	{
		if (Weapon && Weapon->IsA(WeaponType))
		{
			return !Weapon->IsAmmoFull();
		}
	}
	return false;
}

void USTUWeaponComponent::Zoom(bool Enabled) //вызывает зум текущего оружия
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Zoom(Enabled);
	}
}