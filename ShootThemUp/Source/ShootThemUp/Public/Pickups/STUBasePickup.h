// Shoot Them Up Game, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STUBasePickup.generated.h"

class USphereComponent;
class USoundCue;

UCLASS()
class SHOOTTHEMUP_API ASTUBasePickup : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ASTUBasePickup();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	USphereComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
		float RespawnTime = 5.0f; //время респауна
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
		USoundCue* PickupTakenSound;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	//	bool CouldBeTakenTest = true;

	virtual void BeginPlay() override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

public:	
	
	virtual void Tick(float DeltaTime) override;
	bool CouldBeTaken() const;

private:
	float RotationYaw = 0.0f; //Вращение объектов
	FTimerHandle RespawnTimerHandle; //Таймер респауна

	virtual bool GivePickupTo(APawn* PlayerPawn);//true-взяли, false не удалось

	void PickupWasTaken();//вызывается когда мы взяли pickup
	void Respawn();
	void GenerationRotationYaw();//вычисляет угол
};
