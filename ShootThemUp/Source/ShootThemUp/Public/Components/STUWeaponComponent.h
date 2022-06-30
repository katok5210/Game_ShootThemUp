// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "STUCoreTypes.h"
#include "STUWeaponComponent.generated.h"//������ ���� ���������


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

    bool GetCurrentWeaponUIData(FWeaponUIData& UIData) const;//���������� UIData �������� ������
    bool GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const;//

    bool TryToAddAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType, int32 ClipsAmount);
    bool NeedAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType);

    void Zoom(bool Enabled);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
        TArray<FWeaponData> WeaponData;//������ ����� ������

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
        FName WeaponEquipSocketName = "WeaponSocket";

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
        FName WeaponArmorySocketName = "ArmorySocket";

    UPROPERTY(EditDefaultsOnly, Category = "Animation")//����� ��� ��������
        UAnimMontage* EquipAnimMontage;

    UPROPERTY()
        ASTUBaseWeapon* CurrentWeapon = nullptr;

    UPROPERTY()
        TArray<ASTUBaseWeapon*> Weapons;

    int32 CurrentWeaponIndex = 0; //������ �������� �������

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; //���������� ������ 

    bool CanFire() const; //true = ����� ��������
    bool CanEquip() const; //true = ����� ������� ������
    void EquipWeapon(int32 WeaponIndex); //������������ ������ � ���������
private:
    UPROPERTY()
        UAnimMontage* CurrentReloadAnimMontage = nullptr;

    bool EquipAnimInProgress = false; // ��� ����� ������ true
    bool ReloadAnimInProgress = false; // ��� ����������� true

    void SpawnWeapons();

    void AttachWeaponToSocket(ASTUBaseWeapon* Weapon, USceneComponent* SceneComponent, const FName& SocketName);


    void PlayAnimMontage(UAnimMontage* Animation);
    void InitAnimations();
    void OnEauirFinished(USkeletalMeshComponent* MeshComponent);
    void OnReloadFinished(USkeletalMeshComponent* MeshComponent);

    bool CanReload() const;

    /*void OnEmptyClip(ASTUBaseWeapon* AmmoEmptyWeapon);*/
    void OnClipEmpty(ASTUBaseWeapon* AmmoEmptyWeapon);//������
    void ChangeClip();//��� ������ �����������


    template <typename T> //������ ������
    T* FindNotifyByClass(UAnimSequenceBase* Animation)
    {
        if (!Animation) return nullptr;

        const auto NotifyEvents = Animation->Notifies;
        for (auto NotifyEvent : NotifyEvents) //�������� ����� ������
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
