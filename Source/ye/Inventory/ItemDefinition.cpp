// Copyright Epic Games, Inc. All Rights Reserved.

#include "ItemDefinition.h"

FPrimaryAssetId UItemDefinition::GetPrimaryAssetId() const
{
    // Тип регистрируется в DefaultGame.ini для Asset Manager
    return FPrimaryAssetId("Item", ItemID);
}
