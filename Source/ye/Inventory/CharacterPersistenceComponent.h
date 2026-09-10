// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Database/CharacterDB.h"
#include "CharacterPersistenceComponent.generated.h"

class UInventoryComponent;
class UDatabaseSubsystem;

/**
 * Компонент персонажа — управляет загрузкой и сохранением данных из/в БД.
 * Добавляется на BP_MyCharacter рядом с UInventoryComponent.
 * Только сервер: всё выполняется на HasAuthority.
 */
UCLASS(ClassGroup=(Game), meta=(BlueprintSpawnableComponent))
class YE_API UCharacterPersistenceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterPersistenceComponent();

    //~ Begin UActorComponent interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End UActorComponent interface

    /**
     * Инициализировать компонент данными персонажа из БД.
     * Вызывать после спавна персонажа.
     */
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void InitFromCharacterData(const FCharacterData& Data);

    /**
     * Принудительно сохранить всё сейчас (позиция + здоровье + золото).
     */
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void SaveNow();

    /**
     * Интервал автосохранения позиции в секундах (0 = отключено)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Persistence")
    float AutoSaveInterval = 30.0f;

    /** Получить ID персонажа в БД */
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    int64 GetCharacterDBID() const { return CharacterData.ID; }

private:
    FCharacterData CharacterData;

    FTimerHandle AutoSaveTimer;

    void LoadInventoryFromDB();
    void StartAutoSave();
    void OnAutoSaveTick();

    UFUNCTION()
    void OnItemAdded(UItemDefinition* ItemDef, int32 Quantity);

    UFUNCTION()
    void OnItemRemoved(UItemDefinition* ItemDef, int32 Quantity);

    void SaveInventorySlotToDB(UItemDefinition* ItemDef);

    UDatabaseSubsystem* GetDB() const;
    UInventoryComponent* GetInventoryComponent() const;
};
