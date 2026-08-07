// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance_CPP.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT09_API UMyGameInstance_CPP : public UGameInstance
{
	GENERATED_BODY()
	
public:
	//绑定网络失败
	virtual void Init() override;
	//创建服务器
	UFUNCTION(BlueprintCallable, Category = "Network")
	void StartServer(const FString& MapName,int32 port = 7777);

	//加入服务器
	UFUNCTION(BlueprintCallable, Category = "Network")
	void JoinServer(const FString& IP, int32 port = 7777);

	//连接失败时的回调
	UFUNCTION()
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

};
