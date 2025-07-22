#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "SHIItemData.generated.h"

// Istanbul-themed item types
UENUM(BlueprintType)
enum class ESHIItemType : uint8
{
    Silah       UMETA(DisplayName = "Silah"),         // Weapon
    Zirh        UMETA(DisplayName = "Zırh"),          // Armor  
    Aksesuar    UMETA(DisplayName = "Aksesuar"),      // Accessory
    Tuketim     UMETA(DisplayName = "Tüketim"),       // Consumable
    Materyal    UMETA(DisplayName = "Materyal"),      // Material
    Ozel        UMETA(DisplayName = "Özel")           // Special
};

// Equipment slots for Turkish MMO (UI Design Based)
UENUM(BlueprintType)
enum class ESHIEquipmentSlot : uint8
{
    None            UMETA(DisplayName = "Yok"),
    Kask            UMETA(DisplayName = "Kask"),          // Helmet
    GoguslukZirhi   UMETA(DisplayName = "Göğüslük"),     // Chest Armor
    Kalkan          UMETA(DisplayName = "Kalkan"),        // Shield
    Silah1          UMETA(DisplayName = "Silah-1"),       // Primary Weapon
    Silah2          UMETA(DisplayName = "Silah-2"),       // Secondary Weapon  
    Kolye           UMETA(DisplayName = "Kolye"),         // Necklace
    Yuzuk           UMETA(DisplayName = "Yüzük"),         // Ring
    Kupe            UMETA(DisplayName = "Küpe"),          // Earring
    
    Max UMETA(Hidden)
};

// Stat modifiers for equipment
USTRUCT(BlueprintType)
struct FSHIStatModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Bonus")
    FName StatName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Bonus")
    float BonusAmount = 0.0f;

    FSHIStatModifier()
    {
        StatName = FName("None");
        BonusAmount = 0.0f;
    }
};

// Equipment slot data structure (moved from component)
USTRUCT(BlueprintType)
struct FSHIEquipmentSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    class USHIItemData* ItemData = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    int32 Quantity = 0;

    // Helper functions
    bool IsEmpty() const { return ItemData == nullptr || Quantity <= 0; }
    void Clear() { ItemData = nullptr; Quantity = 0; }
    
    FSHIEquipmentSlot()
    {
        ItemData = nullptr;
        Quantity = 0;
    }
};

UCLASS(BlueprintType)
class STILLHEREISTANBUL_API USHIItemData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Basic Item Info
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
    FText ItemName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
    FText ItemDescription;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
    UTexture2D* ItemIcon;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
    ESHIItemType ItemType;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
    int32 MaxStackSize = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
    int32 ItemValue = 0;  // Gold value

    // Equipment Properties (Updated for new slots)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment", 
              meta = (EditCondition = "ItemType == ESHIItemType::Silah || ItemType == ESHIItemType::Zirh || ItemType == ESHIItemType::Aksesuar"))
    ESHIEquipmentSlot EquipmentSlot;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment", 
              meta = (EditCondition = "ItemType == ESHIItemType::Silah || ItemType == ESHIItemType::Zirh || ItemType == ESHIItemType::Aksesuar"))
    TArray<FSHIStatModifier> StatBonuses;

    // 3D Model for world representation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    UStaticMesh* WorldMesh;

    // Istanbul flavor - item rarity
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
    FLinearColor RarityColor = FLinearColor::White;

public:
    // Data Asset ID for networking
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("SHIItem", GetFName());
    }
    
    // Helper functions
    UFUNCTION(BlueprintPure, Category = "Item Info")
    bool IsEquippable() const
    {
        return ItemType == ESHIItemType::Silah || ItemType == ESHIItemType::Zirh || ItemType == ESHIItemType::Aksesuar;
    }
    
    UFUNCTION(BlueprintPure, Category = "Item Info")
    bool IsStackable() const
    {
        return MaxStackSize > 1;
    }
    
    UFUNCTION(BlueprintPure, Category = "Item Info")
    FText GetLocalizedTypeName() const
    {
        switch (ItemType)
        {
            case ESHIItemType::Silah: return FText::FromString(TEXT("Silah"));
            case ESHIItemType::Zirh: return FText::FromString(TEXT("Zırh"));
            case ESHIItemType::Aksesuar: return FText::FromString(TEXT("Aksesuar"));
            case ESHIItemType::Tuketim: return FText::FromString(TEXT("Tüketim"));
            case ESHIItemType::Materyal: return FText::FromString(TEXT("Materyal"));
            case ESHIItemType::Ozel: return FText::FromString(TEXT("Özel"));
            default: return FText::FromString(TEXT("Bilinmeyen"));
        }
    }

    // Equipment slot validation
    UFUNCTION(BlueprintPure, Category = "Equipment")
    bool CanEquipInSlot(ESHIEquipmentSlot SlotType) const
    {
        if (!IsEquippable())
        {
            return false;
        }

        // Basic validation based on item type and slot
        switch (SlotType)
        {
            case ESHIEquipmentSlot::Kask:
            case ESHIEquipmentSlot::GoguslukZirhi:
                return ItemType == ESHIItemType::Zirh;
                
            case ESHIEquipmentSlot::Silah1:
            case ESHIEquipmentSlot::Silah2:
                return ItemType == ESHIItemType::Silah;
                
            case ESHIEquipmentSlot::Kalkan:
                return ItemType == ESHIItemType::Silah && 
                       ItemName.ToString().Contains(TEXT("Kalkan"));
                       
            case ESHIEquipmentSlot::Kolye:
            case ESHIEquipmentSlot::Yuzuk:
            case ESHIEquipmentSlot::Kupe:
                return ItemType == ESHIItemType::Aksesuar;
                
            default:
                return false;
        }
    }
};