// Shoot Them Up Game, All Rights Reserved


#include "UI/STUGameHUD.h"
#include "Engine/Canvas.h"//для нахождения центра экрана
#include "UI/STUBaseWidget.h"
#include "STUGameModeBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogSTUGameHUD, All, All);

void ASTUGameHUD::DrawHUD() 
{
	Super::DrawHUD();
	//DrawCrossHair();

}

void ASTUGameHUD::BeginPlay() 
{
	Super::BeginPlay();

	GameWidgets.Add(ESTUMatchState::InProgress, CreateWidget<USTUBaseWidget>(GetWorld(), PlayerHUDWidgetClass));
	GameWidgets.Add(ESTUMatchState::Pause, CreateWidget<USTUBaseWidget>(GetWorld(), PauseWidgetClass));
	GameWidgets.Add(ESTUMatchState::GameOver, CreateWidget<USTUBaseWidget>(GetWorld(), GameOverWidgetClass));

	for (auto GameWidgetPair : GameWidgets)
	{
		const auto GameWidget = GameWidgetPair.Value;
		if (!GameWidget) continue;

		GameWidget->AddToViewport();
		GameWidget->SetVisibility(ESlateVisibility::Hidden);//скрыаем HUD
	}

	if (GetWorld())
	{
		const auto GameMode = Cast<ASTUGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->OnMatchStateChanged.AddUObject(this, &ASTUGameHUD::OnMatchStateChanged);
		}
	}
}

void ASTUGameHUD::OnMatchStateChanged(ESTUMatchState State)
{
	if (CurrentWidget)//CurrentWidget - текущий видимый виджет
	{
		CurrentWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (GameWidgets.Contains(State))
	{
		CurrentWidget = GameWidgets[State];
	}

	if (CurrentWidget)
	{
		CurrentWidget->SetVisibility(ESlateVisibility::Visible);
		CurrentWidget->Show();
	}


	UE_LOG(LogSTUGameHUD, Display, TEXT("Match state changed: %s"), *UEnum::GetValueAsString(State));
}


void ASTUGameHUD::DrawCrossHair()
{
	//int32 SizeX = Canvas->SizeX; 
	//int32 SizeY = Canvas->SizeY;
	const TInterval<float> Center(Canvas->SizeX * 0.5f,Canvas->SizeY * 0.5f);	//вычисляем центр экрана(*представляем собой отрезок,хранит только 2 элемента)
	
	//вызов линии
	const float HalfLineSize = 10.0f;//длина
	const float LineThickness = 2.0f;//толщина
	const FLinearColor LineColor = FLinearColor::Green;//цвет

	DrawLine(Center.Min - HalfLineSize, Center.Max,Center.Min + HalfLineSize,Center.Max,LineColor,LineThickness);//горизонтальная
	DrawLine(Center.Min, Center.Max - HalfLineSize, Center.Min, Center.Max + HalfLineSize, LineColor, LineThickness);//вертикальная
}
