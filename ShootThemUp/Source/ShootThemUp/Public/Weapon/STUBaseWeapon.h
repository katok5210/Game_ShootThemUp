// Shoot Them Up Game, All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STUCoreTypes.h"
#include "STUBaseWeapon.generated.h"//должен быть последним

//DECLARE_MULTICAST_DELEGATE(FOnClipEmptySignature);//делагат оповещает, что закончились патроны

class USkeletalMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundCue;



UCLASS()
class SHOOTTHEMUP_API ASTUBaseWeapon : public AActor
{
	GENERATED_BODY()

public:
	ASTUBaseWeapon();

	FOnClipEmptySignature OnClipEmpty;

	virtual void StartFire();//делаем virtual(в дальнейшем будем переопределять) т.к. потом будут наследники
	virtual void StopFire();

	void ChangeClip(); //замена магазина
	bool CanReload() const; //определяет может ли оружие делать перезарядку

	FWeaponUIData GetUIData() const { return UIData; }
	FAmmoData GetAmmoData() const { return CurrentAmmo; } //получаем доступ к информации о патронах

	bool TryToAddAmmo(int32 ClipsAmount);
	bool IsAmmoEmpty() const; //пустой = true
	bool IsAmmoFull() const;

	virtual void Zoom(bool Enabled) {};

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")	//внешний вид оружия (skeletal mesh)
		USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
		FName MuzzleSocketName = "MuzzleSocket"; //отображение линий


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
		float TraceMaxDistance = 1500.0f; //стредяем на дистанцию = 15 м.

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
		FAmmoData DefaultAmmo {15,10,false}; //начальный арсенал оружия

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	FWeaponUIData UIData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
		USoundCue* FireSound;

	virtual void BeginPlay() override;

	virtual void MakeShot(); //вся логика выстрела
	virtual bool GetTraceData(FVector& TraceStart, FVector& TraceEnd) const;

	bool GetPlayerViewPoint(FVector& ViewLocation, FRotator& ViewRotation)const;
	FVector GetMuzzleWorldLocation() const;

	void MakeHit(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd);

	void DecreaseAmmo();//Вызывается после выстрела
	bool IsClipEmpty() const; //обойма пуста = true
	

	void LogAmmo();//информация в консоль

	UNiagaraComponent* SpawnMuzzleFX();

private:
	FAmmoData CurrentAmmo; //арсенал сейчас
};
