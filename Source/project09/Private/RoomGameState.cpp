#include "RoomGameState.h"

#include "Net/UnrealNetwork.h"

ARoomGameState::ARoomGameState()
{
	bReplicates = true;
}

void ARoomGameState::SetRoundState(
	ERoomRoundPhase NewPhase,
	int32 NewRemainingSeconds,
	int32 NewRoundId)
{
	RoundState.Phase = NewPhase;
	RoundState.RemainingSeconds = FMath::Max(0, NewRemainingSeconds);
	RoundState.RoundId = NewRoundId;
	NotifyRoundStateChanged();
}

void ARoomGameState::SetRemainingSeconds(int32 NewRemainingSeconds)
{
	RoundState.RemainingSeconds = FMath::Max(0, NewRemainingSeconds);
	NotifyRoundStateChanged();
}

void ARoomGameState::OnRep_RoundState()
{
	NotifyRoundStateChanged();
}

void ARoomGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARoomGameState, RoundState);
}

void ARoomGameState::NotifyRoundStateChanged()
{
	OnRoundStateChanged(RoundState);
}
