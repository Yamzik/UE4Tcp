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
#include "TCPActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNetEventS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetMsgEvent, const TArray<uint8>&, Bytes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetMsgEventStr, const FString&, Msg);

UCLASS()
class WSTEST_API ATCPActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATCPActor();

	FSocket* ClientSocket;
	FThreadSafeBool bShouldReceiveData;
	TFuture<void> ClientConnectionFinishedFuture;
	TSharedPtr<FInternetAddr> RemoteAddress;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FNetMsgEvent OnReceivedBytes;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FNetMsgEventStr OnReceivedStr;

	FNetEventS OnConnected;
	FString fBytesToString(const TArray<uint8>& InArray);
	TArray<uint8> fStringToBytes(FString InString);
	FString ClientSocketName;
	int32 BufferMaxSize;
	bool bIsConnected;

	void ConnectToServer(const FString& InIP = TEXT("127.0.0.1"), const int32 InPort = 53000);

	void CloseSocket();

	UFUNCTION(BlueprintCallable, Category="TCP")
	bool Emit(const TArray<uint8>& Bytes);

	UFUNCTION(BlueprintCallable, Category="TCP")
	bool EmitStr(FString str);

	UFUNCTION(BlueprintCallable, Category="TCP")
	FDateTime GetDate();



	
};
