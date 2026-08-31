#include "RoomGameMode.h"

#include "Engine/World.h"
#include "RoomPlayerController.h"
#include "TimerManager.h"

ARoomGameMode::ARoomGameMode()
{
	GameStateClass = ARoomGameState::StaticClass();
	PlayerControllerClass = ARoomPlayerController::StaticClass();
}

void ARoomGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableNativeRoundRules)
	{
		StartRound();
	}
}

void ARoomGameMode::StartRound()
{
	if (!HasAuthority() || bIsTransitioning)
	{
		return;
	}

	UWorld* World = GetWorld();
	ARoomGameState* RoomState = GetRoomGameState();
	if (!World || !RoomState)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(RoundTimer);
	World->GetTimerManager().ClearTimer(EmptyRoomTimer);
	World->GetTimerManager().ClearTimer(RestartTimer);

	++RoundId;
	RoomState->SetRoundState(
		ERoomRoundPhase::Playing,
		RoundDurationSeconds,
		RoundId);

	World->GetTimerManager().SetTimer(
		RoundTimer,
		this,
		&ARoomGameMode::TickRound,
		1.0f,
		true);
}

void ARoomGameMode::EndRound()
{
	if (!HasAuthority() || bIsTransitioning)
	{
		return;
	}

	UWorld* World = GetWorld();
	ARoomGameState* RoomState = GetRoomGameState();
	if (!World || !RoomState)
	{
		return;
	}

	bIsTransitioning = true;
	World->GetTimerManager().ClearTimer(RoundTimer);
	World->GetTimerManager().ClearTimer(EmptyRoomTimer);
	RoomState->SetRoundState(ERoomRoundPhase::Ending, 0, RoundId);

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (ARoomPlayerController* Controller = Cast<ARoomPlayerController>(It->Get()))
		{
			Controller->ClientReturnToMain();
		}
	}

	// Dedicated Server 没有本地菜单，需要自己重新加载回合地图。
	// Listen Server 的房主会通过 ClientReturnToMain 离开，不能再同时 ServerTravel。
	if (GetNetMode() == NM_DedicatedServer)
	{
		World->GetTimerManager().SetTimer(
			RestartTimer,
			this,
			&ARoomGameMode::RestartRound,
			RestartDelaySeconds,
			false);
	}
}

void ARoomGameMode::StartEmptyRoomTimer()
{
	if (!HasAuthority() || bIsTransitioning || !bHasHadPlayers || HasPlayers())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EmptyRoomTimer);
		World->GetTimerManager().SetTimer(
			EmptyRoomTimer,
			this,
			&ARoomGameMode::ResetIfEmpty,
			EmptyRoomResetSeconds,
			false);
	}
}

void ARoomGameMode::CancelEmptyRoomTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EmptyRoomTimer);
	}
}

void ARoomGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	bHasHadPlayers = true;
	CancelEmptyRoomTimer();
}

void ARoomGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	UWorld* World = GetWorld();

	// 玩家正常离开时检查是否变成空房间。
	// 如果世界正在切换地图，就不要再启动计时器。
	if (World && !World->bIsTearingDown)
	{
		StartEmptyRoomTimer();
	}
}

ARoomGameState* ARoomGameMode::GetRoomGameState() const
{
	return GetGameState<ARoomGameState>();
}

void ARoomGameMode::TickRound()
{
	ARoomGameState* RoomState = GetRoomGameState();
	if (!RoomState || bIsTransitioning)
	{
		return;
	}

	const int32 NewSeconds = RoomState->GetRoundState().RemainingSeconds - 1;
	RoomState->SetRemainingSeconds(NewSeconds);

	if (NewSeconds <= 0)
	{
		EndRound();
	}
}

void ARoomGameMode::RestartRound()
{
	if (!HasAuthority())
	{
		return;
	}

	if (ARoomGameState* RoomState = GetRoomGameState())
	{
		RoomState->SetRoundState(ERoomRoundPhase::Restarting, 0, RoundId);
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(RoundMap);
	}
}

void ARoomGameMode::ResetIfEmpty()
{
	if (!HasAuthority() || bIsTransitioning || HasPlayers())
	{
		return;
	}

	bIsTransitioning = true;
	RestartRound();
}

bool ARoomGameMode::HasPlayers()
{
	return GetNumPlayers() > 0;
}
