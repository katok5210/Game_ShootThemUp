// Shoot Them Up Game, All Rights Reserved


#include "Weapon/Components/STUWeaponFXComponent.h"
#include "NiagaraFunctionLibrary.h" //Система спецэффектов
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "Sound/SoundCue.h"

USTUWeaponFXComponent::USTUWeaponFXComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USTUWeaponFXComponent::PlayImpactFX(const FHitResult& Hit)
{
    auto ImpactData = DefaultImpactData;

    if (Hit.PhysMaterial.IsValid())
    {
        const auto PhysMat = Hit.PhysMaterial.Get();
        if (ImpactDataMap.Contains(PhysMat))
        {
            ImpactData = ImpactDataMap[PhysMat];
        }
    }

    //спавн niagara
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),//
        ImpactData.NiagaraEffect,//
        Hit.ImpactPoint,//
        Hit.ImpactNormal.Rotation());

    //decal
    auto DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(),
        ImpactData.DecalData.Material,//матермал декаля
        ImpactData.DecalData.Size,//размер декаля
        Hit.ImpactPoint,//локация в мире декаля
        Hit.ImpactNormal.Rotation());//вращение декаля
    if (DecalComponent)
    {
        DecalComponent->SetFadeOut(ImpactData.DecalData.LifeTime,ImpactData.DecalData.FadeOutTime);
    }        //LifeTime - через какое времязапустится анимация  FadeOutTime - время исчезания анимации

    //sound
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactData.Sound, Hit.ImpactPoint); // звук при попадании
}
