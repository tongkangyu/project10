#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RoomPlayerController.generated.h"

UCLASS()
class PROJECT09_API ARoomPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Reliable)
	void ClientReturnToMain();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	FString MainMenuMap = TEXT("/Game/Season7/Main");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	float ReturnToMainDelaySeconds = 3.0f;

	UFUNCTION(BlueprintPure, Category = "Round")
	bool IsReturningToMain() const { return bIsReturningToMain; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Round")
	void OnReturningToMain();

private:
	void TravelToMain();

	FTimerHandle ReturnToMainTimer;
	bool bIsReturningToMain = false;
};
