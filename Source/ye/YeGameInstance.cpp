// Copyright Epic Games, Inc. All Rights Reserved.

#include "YeGameInstance.h"
#include "Database/DatabaseSubsystem.h"
#include "Blueprint/UserWidget.h"

void UYeGameInstance::Init()
{
    Super::Init();

    if (!DatabaseConnectionString.IsEmpty())
    {
        ConnectToDatabase(DatabaseConnectionString);
    }
}

void UYeGameInstance::OnStart()
{
    Super::OnStart();

    // Show login screen when game starts
    if (LoginWidgetClass)
    {
        UUserWidget* LoginWidget = CreateWidget<UUserWidget>(this, LoginWidgetClass);
        if (LoginWidget)
        {
            LoginWidget->AddToViewport(10);

            // Show mouse cursor for login screen
            if (APlayerController* PC = GetFirstLocalPlayerController())
            {
                PC->SetShowMouseCursor(true);
                PC->SetInputMode(FInputModeUIOnly{});
            }
        }
    }
}

bool UYeGameInstance::ConnectToDatabase(const FString& ConnectionString)
{
    UDatabaseSubsystem* DB = GetSubsystem<UDatabaseSubsystem>();
    if (!DB)
    {
        UE_LOG(LogTemp, Error, TEXT("ConnectToDatabase: DatabaseSubsystem not found"));
        return false;
    }
    return DB->Connect(ConnectionString);
}

UDatabaseSubsystem* UYeGameInstance::GetDatabaseSubsystem() const
{
    return GetSubsystem<UDatabaseSubsystem>();
}
