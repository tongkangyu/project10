#include "RoomPlayerController.h"

#include "TimerManager.h"

void ARoomPlayerController::ClientReturnToMain_Implementation()
{
	if (bIsReturningToMain)
	{
		return;
	}

	bIsReturningToMain = true;
	OnReturningToMain();

	if (ReturnToMainDelaySeconds <= 0.0f)
	{
		TravelToMain();
		return;
	}

	GetWorldTimerManager().SetTimer(
		ReturnToMainTimer,
		this,
		&ARoomPlayerController::TravelToMain,
		ReturnToMainDelaySeconds,
		false);
}

void ARoomPlayerController::TravelToMain()
{
	ClientTravel(MainMenuMap, TRAVEL_Absolute);
}
