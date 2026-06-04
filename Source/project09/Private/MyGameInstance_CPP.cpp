// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGameInstance_CPP.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"


void UMyGameInstance_CPP::StartServer(const FString& MapName, int32 Port)
{
    // ServerTravel 的 ?listen 参数会自动处理 NetDriver 创建，无需手动 Listen
    FString TravelURL = MapName + FString::Printf(TEXT("?listen?Port=%d"), Port);
    GetWorld()->ServerTravel(TravelURL);
}

void UMyGameInstance_CPP::JoinServer(const FString& IP, int32 Port)
{
    // 构建完整地址字符串
    FString Address = IP + FString::Printf(TEXT(":%d"), Port);

    // 获取第一个 PlayerController 并执行客户端跳转
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->ClientTravel(Address, TRAVEL_Absolute);
    }
}

void UMyGameInstance_CPP::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    UE_LOG(LogTemp, Error, TEXT("网络错误: %s"), *ErrorString);
    // 这里可以弹出友好的错误提示 UI
}

void UMyGameInstance_CPP::Init()
{
    Super::Init();
    GEngine->NetworkFailureEvent.AddUObject(this, &UMyGameInstance_CPP::HandleNetworkFailure);
}