// Fill out your copyright notice in the Description page of Project Settings.


#include "TCPActor.h"
#include "Camera/CameraComponent.h"

// Sets default values
ATCPActor::ATCPActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ClientSocketName = FString(TEXT("ue4-tcp-client"));

	BufferMaxSize = 2 * 1024 * 1024;
}

void SetRotation()
{
	
}

// Called when the game starts or when spawned
void ATCPActor::BeginPlay()
{
	Super::BeginPlay();

	this->ConnectToServer(TEXT("127.0.0.1"), 53000);
	EmitStr(TEXT("Hello there"));
	
}

// Called every frame
void ATCPActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATCPActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f,FColor::Red, TEXT("End play"));

	CloseSocket();
}

void ATCPActor::ConnectToServer(const FString& InIP, const int32 InPort)
{
	RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	UE_LOG(LogTemp, Error, TEXT("Attempting to connect via TCP"));

	bool bIsValid = false;
	RemoteAddress->SetIp(*InIP, bIsValid);
	RemoteAddress->SetPort(InPort);

	if(!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("TCP target is invalid"));
	}

	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, ClientSocketName, false);
	
	ClientSocket->SetSendBufferSize(BufferMaxSize, BufferMaxSize);
	ClientSocket->SetReceiveBufferSize(BufferMaxSize, BufferMaxSize);

	bIsConnected = ClientSocket->Connect(*RemoteAddress);

	if(bIsConnected)
	{
		OnConnected.Broadcast();
	}

	bShouldReceiveData = true;

	ClientConnectionFinishedFuture = Async(EAsyncExecution::Thread, [&]()
	{
		uint32 BufferSize = 0;
		TArray<uint8> ReceiveBuffer;
		FString ResultString;

		while(bShouldReceiveData)
		{
			if(ClientSocket->HasPendingData(BufferSize))
			{
				ReceiveBuffer.SetNumUninitialized(BufferSize);
				int32 Read = 0;
				ClientSocket->Recv(ReceiveBuffer.GetData(), ReceiveBuffer.Num(), Read);

				OnReceivedBytes.Broadcast(ReceiveBuffer);
				OnReceivedStr.Broadcast(fBytesToString(ReceiveBuffer));
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, fBytesToString(ReceiveBuffer));
				UE_LOG(LogTemp, Error, TEXT("Received data: %s"), *(fBytesToString(ReceiveBuffer)));
			}
			//ClientSocket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan(1));
		}
	});
}

void ATCPActor::CloseSocket()
{
	if(ClientSocket)
	{
		bShouldReceiveData = false;
		ClientConnectionFinishedFuture.Get();
		ClientSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
		ClientSocket = nullptr;
	}
}

bool ATCPActor::Emit(const TArray<uint8>& Bytes)
{
	bool resp = false;
	if(ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		int32 BytesSent = 0;
		resp = ClientSocket->Send(Bytes.GetData(), Bytes.Num(), BytesSent);
	}
	return resp;
}

bool ATCPActor::EmitStr(FString str)
{
	bool resp = false;
	if(ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		int32 BytesSent = 0;
		TArray<uint8> Bytes = fStringToBytes(str);
		resp = ClientSocket->Send(Bytes.GetData(), Bytes.Num(), BytesSent);
	}
	return resp;
}

FString ATCPActor::fBytesToString(const TArray<uint8>& InArray)
{
	FString ResultString;
	FFileHelper::BufferToString(ResultString, InArray.GetData(), InArray.Num());
	return ResultString;
}

TArray<uint8> ATCPActor::fStringToBytes(FString InString)
{
	TArray<uint8> ResultBytes;
	ResultBytes.Append((uint8*)TCHAR_TO_UTF8(*InString), InString.Len());
	return ResultBytes;
}

FDateTime ATCPActor::GetDate()
{
	FDateTime Time;
	return Time.Now();
}


