#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RoomGameState.h"
#include "RoomGameMode.generated.h"

UCLASS()
class PROJECT09_API ARoomGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARoomGameMode();
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	int32 RoundDurationSeconds = 300;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	float EmptyRoomResetSeconds = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	float RestartDelaySeconds = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	FString RoundMap = TEXT("/Game/Maps/csgo");

	// Turn this on only after the old Blueprint lifecycle nodes are disconnected.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	bool bEnableNativeRoundRules = false;

	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartRound();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void EndRound();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartEmptyRoomTimer();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void CancelEmptyRoomTimer();

	UFUNCTION(BlueprintPure, Category = "Round")
	bool IsTransitioning() const { return bIsTransitioning; }

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	ARoomGameState* GetRoomGameState() const;

private:
	void TickRound();
	void RestartRound();
	void ResetIfEmpty();
	bool HasPlayers();

	FTimerHandle RoundTimer;
	FTimerHandle EmptyRoomTimer;
	FTimerHandle RestartTimer;
	bool bIsTransitioning = false;
	bool bHasHadPlayers = false;
	int32 RoundId = 0;
};
