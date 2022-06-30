// Shoot Them Up Game, All Rights Reserved.

#include "Weapon/STUBaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h" //для рисовки линий
#include  "GameFramework/Character.h"//получаем доступ к камере и контролю
#include  "GameFramework/Controller.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBaseWeapon, All, All);

ASTUBaseWeapon::ASTUBaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh"); //создание skeletal mesh в BP
	SetRootComponent(WeaponMesh);
}

void ASTUBaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	check(WeaponMesh);
	checkf(DefaultAmmo.Bullets > 0,TEXT("Bullets count couldn't be less or equal zero"));//f пользовательское сообщение 
	checkf(DefaultAmmo.Clips > 0, TEXT("Clips count couldn't be less or equal zero"));
	CurrentAmmo = DefaultAmmo;
}


void ASTUBaseWeapon::StartFire()
{

}

void ASTUBaseWeapon::StopFire()
{
	//GetWorldTimerManager().ClearTimer(ShotTimerHandle);
}


void ASTUBaseWeapon::MakeShot()
{

}


//Метод получения viewpoint игрока//
bool ASTUBaseWeapon::GetPlayerViewPoint(FVector& ViewLocation, FRotator& ViewRotation)const 
{
	const auto STUCharacter = Cast<ACharacter>(GetOwner());
	if (!STUCharacter) return false;

	if (STUCharacter->IsPlayerControlled()) //если управляется игроком
	{
		const auto Controller = STUCharacter->GetController<APlayerController>();
		if (!Controller) return false;
		Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);//функция возвращает позицию камеры и ориентации
	}
	else //если ии
	{
		ViewLocation = GetMuzzleWorldLocation();
		ViewRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);
	}

	return true;
}

FVector ASTUBaseWeapon::GetMuzzleWorldLocation() const
{
	return WeaponMesh->GetSocketLocation(MuzzleSocketName);
}

//Функция получения trace start и trace end
bool ASTUBaseWeapon::GetTraceData(FVector& TraceStart,FVector& TraceEnd) const
{
	FVector ViewLocation;
	FRotator ViewRotation;
	if (!GetPlayerViewPoint(ViewLocation, ViewRotation))return false; //функция возвращает позицию камеры и ориентации


	TraceStart = ViewLocation; //точка начаоа выстрела
	
	const FVector ShootDirection = ViewRotation.Vector();  //направленеие выстрела 
	TraceEnd = TraceStart + ShootDirection * TraceMaxDistance;	//конечная точка выстрела
	return true;
}

//Функция создающая трасеры
void ASTUBaseWeapon::MakeHit(FHitResult& HitResult,const FVector& TraceStart,const FVector& TraceEnd)
{
	if (!GetWorld()) return;

	//находим пересечение с нашем отрезком
	FCollisionQueryParams CollisionParams; // не попадать в себя
	CollisionParams.AddIgnoredActor(GetOwner());//трасер будет игнорировать нас
	CollisionParams.bReturnPhysicalMaterial = true;


	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParams);  //возвращает первое пересечение()

}



void ASTUBaseWeapon::DecreaseAmmo() 
{
	if (CurrentAmmo.Bullets == 0)
	{
		UE_LOG(LogBaseWeapon, Warning, TEXT("Clip is empty"));
		return;
	}
	CurrentAmmo.Bullets--;


	if (IsClipEmpty() && !IsAmmoEmpty())//нужна ли перезарядка
	{
		StopFire();//фикс бага
		OnClipEmpty.Broadcast(this);
	}
}//Вызывается после выстрела

bool ASTUBaseWeapon::IsAmmoEmpty() const 
{
	return !CurrentAmmo.Infinite && CurrentAmmo.Clips == 0 && IsClipEmpty();
} //пустой = true

bool ASTUBaseWeapon::IsClipEmpty() const
{
	return CurrentAmmo.Bullets == 0;
} //обойма пуста = true

void ASTUBaseWeapon::ChangeClip() 
{
	if (!CurrentAmmo.Infinite)
	{
		if (CurrentAmmo.Clips == 0)
		{
			UE_LOG(LogBaseWeapon,Warning,TEXT ("No more clips"));
			return;
		}
		CurrentAmmo.Clips--;
	}
	CurrentAmmo.Bullets = DefaultAmmo.Bullets;
	//UE_LOG(LogBaseWeapon, Display, TEXT("-------- Change Clip --------"));
} //замена магазина



bool ASTUBaseWeapon::CanReload() const
{
	return CurrentAmmo.Bullets < DefaultAmmo.Bullets && CurrentAmmo.Clips > 0;
}


void ASTUBaseWeapon::LogAmmo() 
{
	FString AmmoInfo = "Ammo: " + FString::FromInt(CurrentAmmo.Bullets) + "/";
	AmmoInfo += CurrentAmmo.Infinite ? "Infinite" : FString::FromInt(CurrentAmmo.Clips);
	UE_LOG(LogBaseWeapon,Display,TEXT ("%s"),*AmmoInfo);

}//информация в консоль

bool ASTUBaseWeapon::IsAmmoFull() const
{
	return CurrentAmmo.Clips == DefaultAmmo.Clips &&  //
		CurrentAmmo.Bullets == DefaultAmmo.Bullets;
}


bool ASTUBaseWeapon::TryToAddAmmo(int32 ClipsAmount)//БОЕПРИПАСЫ
{
	if (CurrentAmmo.Infinite || IsAmmoFull() || ClipsAmount <= 0) return false;//не добавляем патроны

	if (IsAmmoEmpty())//кол-во магазинов и обоймы = 0
	{
		/*UE_LOG(LogBaseWeapon, Display, TEXT("Ammo was empty !"));*/
		CurrentAmmo.Clips = FMath::Clamp(ClipsAmount,0,DefaultAmmo.Clips + 1);//магазин на 1 больше,он уходит в обойму
		OnClipEmpty.Broadcast(this);
	}
	else if (CurrentAmmo.Clips < DefaultAmmo.Clips)//кол-во магазинов < максимального
	{
		const auto NextClipsAmount = CurrentAmmo.Clips + ClipsAmount;
		if (DefaultAmmo.Clips - NextClipsAmount >= 0)
		{
			CurrentAmmo.Clips = NextClipsAmount;
			/*UE_LOG(LogBaseWeapon, Display, TEXT("Clips were added !!"));*/
		}
		else
		{
			CurrentAmmo.Clips = DefaultAmmo.Clips;
			CurrentAmmo.Bullets = DefaultAmmo.Bullets;
			/*UE_LOG(LogBaseWeapon, Display, TEXT("Ammo is full now !!!"));*/
		}
	}
	else//магазины полные, обойба не полная
	{
		CurrentAmmo.Bullets = DefaultAmmo.Bullets; //заполняем обойму
		/*UE_LOG(LogBaseWeapon, Display, TEXT("Bullets were added !!!!"));*/
	}

	return true;
}


UNiagaraComponent* ASTUBaseWeapon::SpawnMuzzleFX()//Эффект стрельбы
{
	return UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFX,  //
		WeaponMesh,                                                //
		MuzzleSocketName,                                          //
		FVector::ZeroVector,                                       //
		FRotator::ZeroRotator,                                     //
		EAttachLocation::SnapToTarget, true);
}
