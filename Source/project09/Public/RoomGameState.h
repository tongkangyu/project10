#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RoomGameState.generated.h"

UENUM(BlueprintType)
enum class ERoomRoundPhase : uint8
{
	Waiting,
	Playing,
	Ending,
	Restarting
};

USTRUCT(BlueprintType)
struct FRoomRoundState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Round")
	ERoomRoundPhase Phase = ERoomRoundPhase::Waiting;

	UPROPERTY(BlueprintReadOnly, Category = "Round")
	int32 RemainingSeconds = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Round")
	int32 RoundId = 0;
};

UCLASS()
class PROJECT09_API ARoomGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ARoomGameState();

	UPROPERTY(ReplicatedUsing = OnRep_RoundState, BlueprintReadOnly, Category = "Round")
	FRoomRoundState RoundState;

	void SetRoundState(ERoomRoundPhase NewPhase, int32 NewRemainingSeconds, int32 NewRoundId);
	void SetRemainingSeconds(int32 NewRemainingSeconds);
	const FRoomRoundState& GetRoundState() const { return RoundState; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Round")
	void OnRoundStateChanged(const FRoomRoundState& NewState);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_RoundState();

private:
	void NotifyRoundStateChanged();
};
