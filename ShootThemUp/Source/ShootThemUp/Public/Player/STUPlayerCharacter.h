// Shoot Them Up Game, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Player/STUBaseCharacter.h"
#include "STUPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USphereComponent;

UCLASS()
class SHOOTTHEMUP_API ASTUPlayerCharacter : public ASTUBaseCharacter
{
	GENERATED_BODY()
	
public:
	ASTUPlayerCharacter(const FObjectInitializer& ObjInit); // заменяем класс(?)

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")		//ВРАЩЕНИЕ ВОКРУГ PAWN
		USpringArmComponent* SpringArmComponent;;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")		//КАМЕРА
		UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
		USphereComponent* CameraCollisionComponent;

	virtual void OnDeath()override;
	virtual void BeginPlay()override;

public:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

		virtual bool IsRunning() const override;


private:
	void MoveForward(float Amount); //объявляем переменные для перемещения
	void MoveRight(float Amount);

	void OnStartRunning();
	void OnStopRunning();
	bool WantsToRun = false; //зажата кнопка бега
	bool IsMovingForward = false; // бежит персонаж или нет

	UFUNCTION()
	void OnCameraCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, //
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	UFUNCTION()
	void OnCameraCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,//
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void CheckCameraOverlap();

};
