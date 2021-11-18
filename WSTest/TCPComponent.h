// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Async/Async.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "Networking.h"
#include "Misc/DateTime.h"

#include "Misc/FileHelper.h"
#include "IPAddress.h"
#include "HAL/ThreadSafeBool.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TCPComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNetEventS_Comp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetMsgEvent_Comp, const TArray<uint8>&, Bytes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetMsgEventStr_Comp, const FString&, Msg);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WSTEST_API UTCPComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTCPComponent();
	
	FSocket* ClientSocket;
	FThreadSafeBool bShouldReceiveData;
	TFuture<void> ClientConnectionFinishedFuture;
	TSharedPtr<FInternetAddr> RemoteAddress;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FNetMsgEvent_Comp OnReceivedBytes;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FNetMsgEventStr_Comp OnReceivedStr;

	FNetEventS_Comp OnConnected;
	FString fBytesToString(const TArray<uint8>& InArray);
	TArray<uint8> fStringToBytes(FString InString);
	FString ClientSocketName;
	int32 BufferMaxSize;
	bool bIsConnected;
	AActor* Parent;
	FRotator rotation;
	void HandleMsg(FString Message);
	
	UPROPERTY(BlueprintReadWrite)
	float angle;
	
	void ConnectToServer(const FString& InIP = TEXT("127.0.0.1"), const int32 InPort = 53000);
	void Rotate(FRotator rot);
	void CloseSocket();

	UFUNCTION(BlueprintCallable, Category="TCP")
	bool Emit(const TArray<uint8>& Bytes);

	UFUNCTION(BlueprintCallable, Category="TCP")
	bool EmitStr(FString str);

	UFUNCTION(BlueprintCallable, Category="TCP")
	FDateTime GetDate();
};
