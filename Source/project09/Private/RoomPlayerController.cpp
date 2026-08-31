#include "RoomPlayerController.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
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
		BeginSessionCleanup();
		return;
	}

	GetWorldTimerManager().SetTimer(
		ReturnToMainTimer,
		this,
		&ARoomPlayerController::BeginSessionCleanup,
		ReturnToMainDelaySeconds,
		false);
}

void ARoomPlayerController::BeginSessionCleanup()
{
	// Unreal 默认把游戏房间命名为 GameSession。
	// 创建、加入和销毁房间时必须使用同一个名字。
	const FName GameSessionName(TEXT("GameSession"));

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		// 没有 OnlineSubsystem 时，不能销毁 Session，但仍然要让玩家离开。
		TravelToMain();
		return;
	}

	IOnlineSessionPtr Sessions = OnlineSubsystem->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		TravelToMain();
		return;
	}

	// 不是每个客户端都拥有本地 Session。没有 Session 时直接返回即可。
	if (Sessions->GetNamedSession(GameSessionName) == nullptr)
	{
		TravelToMain();
		return;
	}

	DestroySessionCompleteHandle =
		Sessions->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(
				this,
				&ARoomPlayerController::HandleDestroySessionComplete));

	// 如果 OnlineSubsystem 没有返回结果，超时也要保证玩家能够离开。
	if (SessionDestroyTimeoutSeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			SessionDestroyTimeoutTimer,
			this,
			&ARoomPlayerController::TravelToMain,
			SessionDestroyTimeoutSeconds,
			false);
	}

	if (!Sessions->DestroySession(GameSessionName))
	{
		// DestroySession 可能因为状态不允许而立即失败，此时走同一个退出入口。
		ClearDestroySessionDelegate();
		TravelToMain();
	}
}

void ARoomPlayerController::HandleDestroySessionComplete(
	FName SessionName,
	bool bWasSuccessful)
{
	GetWorldTimerManager().ClearTimer(SessionDestroyTimeoutTimer);
	ClearDestroySessionDelegate();

	// 销毁成功和失败都不能阻止玩家退出比赛。
	TravelToMain();
}

void ARoomPlayerController::ClearDestroySessionDelegate()
{
	if (!DestroySessionCompleteHandle.IsValid())
	{
		return;
	}

	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr Sessions = OnlineSubsystem->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(
				DestroySessionCompleteHandle);
		}
	}

	DestroySessionCompleteHandle.Reset();
}

void ARoomPlayerController::TravelToMain()
{
	if (bHasStartedTravel)
	{
		return;
	}

	bHasStartedTravel = true;
	GetWorldTimerManager().ClearTimer(ReturnToMainTimer);
	GetWorldTimerManager().ClearTimer(SessionDestroyTimeoutTimer);
	ClearDestroySessionDelegate();

	ClientTravel(MainMenuMap, TRAVEL_Absolute);
}
