#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "SHIItemData.generated.h"

// Turkish-themed weapon ability types (moved from SHIAbilityComponent to avoid circular dependency)
UENUM(BlueprintType)
enum class ESHIWeaponAbilityType : uint8
{
    None,
    // Kılıç abilities
    KilicSlash,      // Hızlı doğrama
    KilicThrust,     // Delici saldırı
    KilicGuard,      // Savunma duruşu
    
    // Balta abilities  
    BaltaChop,       // Güçlü doğrama
    BaltaThrow,      // Balta fırlatma
    BaltaWhirlwind,  // Dönen saldırı
    
    // Meç abilities
    MecLunge,        // Ani hamle
    MecParry,        // Karşı saldırı
    MecRiposte,      // Geri saldırı
    
    // Ateş Asası abilities
    AtesFireball,    // Ateş topu
    AtesBurn,        // Yakıcı alan
    AtesIgnite       // Tutuşturma
};

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
    GoguslukZirhi   UMETA(DisplayName = "Göğüslük"),      // Chest Armor
    Eldiven         UMETA(DisplayName = "Eldiven"),       // Gloves
    Pantolon        UMETA(DisplayName = "Pantolon"),      // Pants
    Ayakkabi        UMETA(DisplayName = "Ayakkabı"),      // Boots
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

// Weapon Ability data structure for data-driven abilities
USTRUCT(BlueprintType)
struct STILLHEREISTANBUL_API FSHIWeaponAbility
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Ability")
    ESHIWeaponAbilityType AbilityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Ability")
    FText AbilityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Ability")
    FText AbilityDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Ability")
    float CooldownTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Ability")
    float BaseDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Ability")
    float Range = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Ability")
    TSoftObjectPtr<UTexture2D> AbilityIcon;

    FSHIWeaponAbility()
    {
        AbilityType = ESHIWeaponAbilityType::None;
        AbilityName = FText::FromString("No Ability");
        AbilityDescription = FText::FromString("No description");
        CooldownTime = 5.0f;
        BaseDamage = 50.0f;
        Range = 300.0f;
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

    // Weapon Abilities (only visible for weapons)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Abilities", 
              meta = (EditCondition = "ItemType == ESHIItemType::Silah", EditConditionHides))
    TArray<FSHIWeaponAbility> WeaponAbilities;

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
        case ESHIEquipmentSlot::Eldiven:      
        case ESHIEquipmentSlot::Pantolon:     
        case ESHIEquipmentSlot::Ayakkabi:     
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

    // Weapon Abilities Helper Functions
    UFUNCTION(BlueprintPure, Category = "Weapon Abilities")
    TArray<FSHIWeaponAbility> GetWeaponAbilities() const { return WeaponAbilities; }

    UFUNCTION(BlueprintPure, Category = "Weapon Abilities")
    bool HasAbilities() const { return WeaponAbilities.Num() > 0; }

    UFUNCTION(BlueprintPure, Category = "Weapon Abilities")
    int32 GetAbilityCount() const { return WeaponAbilities.Num(); }

    UFUNCTION(BlueprintPure, Category = "Weapon Abilities")
    FSHIWeaponAbility GetAbilityByIndex(int32 Index) const 
    { 
        if (WeaponAbilities.IsValidIndex(Index))
            return WeaponAbilities[Index];
        return FSHIWeaponAbility();
    }
};