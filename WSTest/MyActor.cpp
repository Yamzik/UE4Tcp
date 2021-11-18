// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"

#include "SEditorViewportToolBarMenu.h"
#include "WebSockets/Public/IWebSocket.h"
#include "WebSockets/Public/WebSocketsModule.h"

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	#pragma region WebSocketStuff
	const FString ServerURL = TEXT("ws://127.0.0.1:53000/"); // Your server URL. You can use ws, wss or wss+insecure.
	const FString ServerProtocol = TEXT("ws");              // The WebServer protocol you want to use.
	const TMap<FString, FString> Headers;
	Socket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, ServerProtocol, Headers);
	//Socket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, ServerProtocol);
	
	// We bind all available events
	Socket->OnConnected().AddLambda([this]() -> void {
		// This code will run once connected.
		
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("Connected!"));
		Send();
	});
    
	Socket->OnConnectionError().AddLambda([](const FString & Error) -> void {
		// This code will run if the connection failed. Check Error to see what happened.
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Failed to connect: " + Error + "\n"));
	});
    
	Socket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) -> void {
		// This code will run when the connection to the server has been terminated.
		// Because of an error or a call to Socket->Close().
		int a = 2;
	});
    
	Socket->OnMessage().AddLambda([](const FString & Message) -> void {
		// This code will run when we receive a string message from the server.
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, Message);
	});
    
	Socket->OnRawMessage().AddLambda([](const void* Data, SIZE_T Size, SIZE_T BytesRemaining) -> void {
		// This code will run when we receive a raw (binary) message from the server.
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Got message"));
	});
    
	Socket->OnMessageSent().AddLambda([](const FString& MessageString) -> void {
		// This code is called after we sent a message to the server.
	});
}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	// And we finally connect to the server. 
	Socket->Connect();
	Send();
	Super::BeginPlay();
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyActor::Send()
{
	Socket->Send(TEXT("Hello there!"));
}

