// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "YeGameInstance.generated.h"

class UDatabaseSubsystem;

UCLASS()
class YE_API UYeGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void OnStart() override;

    UFUNCTION(BlueprintCallable, Category = "Database")
    bool ConnectToDatabase(const FString& ConnectionString);

    UFUNCTION(BlueprintCallable, Category = "Database")
    UDatabaseSubsystem* GetDatabaseSubsystem() const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Database")
    FString DatabaseConnectionString = TEXT("host=127.0.0.1 port=5432 dbname=mmorpg user=postgres password=0708");

    /** Widget class to show as login screen — set in BP_GameInstance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> LoginWidgetClass;

    /** Widget class to show as character select — set in BP_GameInstance */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> CharacterSelectWidgetClass;
};
