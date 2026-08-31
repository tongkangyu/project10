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

	// Session 销毁如果没有及时回调，就在这个时间后直接返回主菜单。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	float SessionDestroyTimeoutSeconds = 5.0f;

	UFUNCTION(BlueprintPure, Category = "Round")
	bool IsReturningToMain() const { return bIsReturningToMain; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Round")
	void OnReturningToMain();

private:
	// 等待结束 UI 显示后，开始处理 Session。
	void BeginSessionCleanup();

	// OnlineSubsystem 完成销毁 Session 后会调用这个函数。
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// 离开前移除 Session 回调，避免回调访问已经离开的控制器。
	void ClearDestroySessionDelegate();

	// 无论 Session 销毁成功、失败还是超时，最终都从这里离开。
	void TravelToMain();

	FTimerHandle ReturnToMainTimer;
	FTimerHandle SessionDestroyTimeoutTimer;
	FDelegateHandle DestroySessionCompleteHandle;

	bool bIsReturningToMain = false;
	bool bHasStartedTravel = false;
};
