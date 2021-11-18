// Fill out your copyright notice in the Description page of Project Settings.


#include "TCPComponent.h"

#include "GeometryCollection/GeometryCollectionAlgo.h"

// Sets default values for this component's properties
UTCPComponent::UTCPComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ClientSocketName = FString(TEXT("ue4-tcp-client"));

	ClientSocket = nullptr;

	Parent = this->GetOwner();
	
	BufferMaxSize = 2 * 1024 * 1024;
	// ...
}


// Called when the game starts
void UTCPComponent::BeginPlay()
{
	Super::BeginPlay();

	//this->ConnectToServer(TEXT("127.0.0.1"), 53000);
	// ...
	
}


// Called every frame
void UTCPComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Rotate(rotation);
	// ...
}

void UTCPComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f,FColor::Red, TEXT("End play"));

	CloseSocket();
}

void UTCPComponent::ConnectToServer(const FString& InIP, const int32 InPort)
{
	RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	UE_LOG(LogTemp, Display, TEXT("Attempting to connect via TCP"));

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
				TArray<uint8> BufferCopy = ReceiveBuffer;
				OnReceivedBytes.Broadcast(ReceiveBuffer);
				OnReceivedStr.Broadcast(fBytesToString(ReceiveBuffer));
				HandleMsg(fBytesToString(ReceiveBuffer));
				//FRotator rot = new FRotator(0);
				//angle = ReceiveBuffer[ReceiveBuffer.Num() - 1];
				//if(ReceiveBuffer.Num() == 3) Rotate(ReceiveBuffer[2]);
				//UE_LOG(LogTemp, Error, TEXT("Received data: %s"), *(fBytesToString(ReceiveBuffer)));
				UE_LOG(LogTemp, Display, TEXT("Received data: %s"), *fBytesToString(ReceiveBuffer));
				
			}
			ClientSocket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan(1));
		}
	});
}

void UTCPComponent::Rotate(FRotator rot)
{
	FRotator CurretnRot = Parent->GetActorRotation();
	Parent->SetActorRotation(FMath::Lerp<FRotator, float>(CurretnRot, rot, 0.33f));
}

void UTCPComponent::CloseSocket()
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

bool UTCPComponent::Emit(const TArray<uint8>& Bytes)
{
	bool resp = false;
	if(ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		int32 BytesSent = 0;
		resp = ClientSocket->Send(Bytes.GetData(), Bytes.Num(), BytesSent);
	}
	return resp;
}

bool UTCPComponent::EmitStr(FString str)
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

FString UTCPComponent::fBytesToString(const TArray<uint8>& InArray)
{
	FString ResultString;
	FFileHelper::BufferToString(ResultString, InArray.GetData(), InArray.Num());
	bool next = true;
	return ResultString;
}

TArray<uint8> UTCPComponent::fStringToBytes(FString InString)
{
	TArray<uint8> ResultBytes;
	ResultBytes.Append((uint8*)TCHAR_TO_UTF8(*InString), InString.Len());
	return ResultBytes;
}

void UTCPComponent::HandleMsg(FString Message)
{
	TArray<FString> results;
	FString left;
	bool next = true;
	while(next)
	{
		FString bLeft;
		FString bRight;
		next = Message.Split(";", &left, &Message);
		if(left.Contains("forward") & next)
		{
			left.Split(":", &bLeft, &bRight);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, bRight);
		}
		if(left.Contains("brake") & next)
		{
			left.Split(":", &bLeft, &bRight);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, bRight);
		}
		if(left.Contains("rotationx") & next)
		{
			left.Split(":", &bLeft, &bRight);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, bRight);
			rotation += FRotator(0, 0, atoi(TCHAR_TO_ANSI(*bRight)));
		}
		if(left.Contains("rotationy") & next)
		{
			left.Split(":", &bLeft, &bRight);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, bRight);
			rotation += FRotator(atoi(TCHAR_TO_ANSI(*bRight)), 0, 0);
		}
		if(left.Contains("rotationz") & next)
		{
			left.Split(":", &bLeft, &bRight);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, bRight);
			rotation += FRotator(0, atoi(TCHAR_TO_ANSI(*bRight)), 0);
		}
	}
}


FDateTime UTCPComponent::GetDate()
{
	FDateTime Time;
	return Time.Now();
}