// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "STUCoreTypes.h"
#include "STUWeaponComponent.generated.h"//должен быть последним


class ASTUBaseWeapon;


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTTHEMUP_API USTUWeaponComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USTUWeaponComponent();

    virtual void StartFire();
    void StopFire();
    virtual void NextWeapon();
    void Reload();

    bool GetCurrentWeaponUIData(FWeaponUIData& UIData) const;//возвращает UIData текущего оружия
    bool GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const;//

    bool TryToAddAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType, int32 ClipsAmount);
    bool NeedAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType);

    void Zoom(bool Enabled);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
        TArray<FWeaponData> WeaponData;//массив смены оружия

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
        FName WeaponEquipSocketName = "WeaponSocket";

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
        FName WeaponArmorySocketName = "ArmorySocket";

    UPROPERTY(EditDefaultsOnly, Category = "Animation")//метод для анимаций
        UAnimMontage* EquipAnimMontage;

    UPROPERTY()
        ASTUBaseWeapon* CurrentWeapon = nullptr;

    UPROPERTY()
        TArray<ASTUBaseWeapon*> Weapons;

    int32 CurrentWeaponIndex = 0; //индекс элемента массива

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; //уничтожаем оружия 

    bool CanFire() const; //true = можем стрелять
    bool CanEquip() const; //true = можем сменить оружие
    void EquipWeapon(int32 WeaponIndex); //прикрепление оружия к персонажу
private:
    UPROPERTY()
        UAnimMontage* CurrentReloadAnimMontage = nullptr;

    bool EquipAnimInProgress = false; // при смене орудия true
    bool ReloadAnimInProgress = false; // при перезарядки true

    void SpawnWeapons();

    void AttachWeaponToSocket(ASTUBaseWeapon* Weapon, USceneComponent* SceneComponent, const FName& SocketName);


    void PlayAnimMontage(UAnimMontage* Animation);
    void InitAnimations();
    void OnEauirFinished(USkeletalMeshComponent* MeshComponent);
    void OnReloadFinished(USkeletalMeshComponent* MeshComponent);

    bool CanReload() const;

    /*void OnEmptyClip(ASTUBaseWeapon* AmmoEmptyWeapon);*/
    void OnClipEmpty(ASTUBaseWeapon* AmmoEmptyWeapon);//НУЖНОЕ
    void ChangeClip();//вся логика перезарядки


    template <typename T> //Создаём шаблон
    T* FindNotifyByClass(UAnimSequenceBase* Animation)
    {
        if (!Animation) return nullptr;

        const auto NotifyEvents = Animation->Notifies;
        for (auto NotifyEvent : NotifyEvents) //анимация смены оружия
        {
            auto AnimNotify = Cast<T>(NotifyEvent.Notify);
            if (AnimNotify)
            {
                return AnimNotify;
            }
        }
        return nullptr;
    }
};
