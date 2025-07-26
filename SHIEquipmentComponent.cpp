#include "SHIEquipmentComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Components/SHIInventoryComponent.h"
#include "Player/SHICharacter.h"

USHIEquipmentComponent::USHIEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    // Initialize equipment state
    EquipmentState = FSHIEquipmentState();
}

void USHIEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("SHI Equipment Component initialized"));
    UE_LOG(LogTemp, Log, TEXT("Equipment slots initialized: 8 slots ready"));
}

void USHIEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USHIEquipmentComponent, EquipmentState);
}

void USHIEquipmentComponent::Server_EquipItem_Implementation(ESHIEquipmentSlot SlotType, USHIItemData* ItemData, int32 Quantity)
{
    if (!ItemData || SlotType == ESHIEquipmentSlot::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid item or slot type for equip"));
        return;
    }

    // Get slot reference
    FSHIEquipmentSlot* TargetSlot = EquipmentState.GetSlotByType(SlotType);
    if (!TargetSlot)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid equipment slot type: %d"), (int32)SlotType);
        return;
    }

    // Validate item can be equipped in this slot
    if (!IsValidEquipmentSlot(SlotType, ItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Item %s cannot be equipped in slot %d"), 
               *ItemData->ItemName.ToString(), (int32)SlotType);
        return;
    }

    // Store old item for event broadcasting
    USHIItemData* OldItem = TargetSlot->ItemData;

    // ENHANCED: Shield dependency validation
    if (SlotType == ESHIEquipmentSlot::Kalkan)
    {
        if (!ValidateShieldDependency(ItemData))
        {
            UE_LOG(LogTemp, Warning, TEXT("Shield dependency validation failed"));
            return; // Block shield equipping
        }
    }

    // Equip the new item
    TargetSlot->ItemData = ItemData;
    TargetSlot->Quantity = Quantity;

    // If equipping a weapon and no active weapon set, make this active
    if (IsWeaponSlot(SlotType) && !GetActiveWeapon())
    {
        EquipmentState.ActiveWeaponSlot = SlotType;
        BroadcastActiveWeaponChange();
    }

    BroadcastEquipmentChange(SlotType, ItemData, OldItem);

    UE_LOG(LogTemp, Log, TEXT("Equipped %s in slot %d"), 
           *ItemData->ItemName.ToString(), (int32)SlotType);
}

void USHIEquipmentComponent::Server_UnequipItem_Implementation(ESHIEquipmentSlot SlotType)
{
    if (SlotType == ESHIEquipmentSlot::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot unequip from invalid slot"));
        return;
    }

    FSHIEquipmentSlot* TargetSlot = EquipmentState.GetSlotByType(SlotType);
    if (!TargetSlot || TargetSlot->IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot unequip from empty slot %d"), (int32)SlotType);
        return;
    }

    USHIItemData* OldItem = TargetSlot->ItemData;
    
    // Store sword info before unequipping
    bool WasSwordUnequipped = false;
    if (SlotType == ESHIEquipmentSlot::Silah1 || SlotType == ESHIEquipmentSlot::Silah2)
    {
        if (OldItem && OldItem->ItemType == ESHIItemType::Silah)
        {
            FString ItemName = OldItem->ItemName.ToString();
            if (ItemName.Contains(TEXT("Kılıç")) || ItemName.Contains(TEXT("Sword")))
            {
                WasSwordUnequipped = true;
                UE_LOG(LogTemp, Log, TEXT("Sword being unequipped: %s"), *ItemName);
            }
        }
    }

    // Clear the slot
    TargetSlot->Clear();

    // Handle weapon unequipping
    if (SlotType == EquipmentState.ActiveWeaponSlot)
    {
        // Switch to other weapon if available
        ESHIEquipmentSlot OtherWeaponSlot = (SlotType == ESHIEquipmentSlot::Silah1) ? 
                                           ESHIEquipmentSlot::Silah2 : ESHIEquipmentSlot::Silah1;
        
        if (!IsSlotEmpty(OtherWeaponSlot))
        {
            EquipmentState.ActiveWeaponSlot = OtherWeaponSlot;
        }
        else
        {
            EquipmentState.ActiveWeaponSlot = ESHIEquipmentSlot::Silah1; // Default to slot 1
        }
        BroadcastActiveWeaponChange();
    }

    // ENHANCED: Check if we need to auto-unequip shield after sword removal
    if (WasSwordUnequipped)
    {
        // Small delay to ensure unequip completed first
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
            {
                ValidateShieldEquipment();
            });
        }
    }
    else
    {
        // Regular validation for other items
        ValidateShieldEquipment();
    }

    BroadcastEquipmentChange(SlotType, nullptr, OldItem);

    UE_LOG(LogTemp, Log, TEXT("Unequipped item from slot %d"), (int32)SlotType);
}

void USHIEquipmentComponent::Server_SetActiveWeapon_Implementation(ESHIEquipmentSlot WeaponSlot)
{
    if (!IsWeaponSlot(WeaponSlot))
    {
        UE_LOG(LogTemp, Warning, TEXT("Slot %d is not a weapon slot"), (int32)WeaponSlot);
        return;
    }

    if (IsSlotEmpty(WeaponSlot))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot set active weapon - slot %d is empty"), (int32)WeaponSlot);
        return;
    }

    ESHIEquipmentSlot OldActiveWeapon = EquipmentState.ActiveWeaponSlot;
    EquipmentState.ActiveWeaponSlot = WeaponSlot;

    if (OldActiveWeapon != WeaponSlot)
    {
        BroadcastActiveWeaponChange();
        UE_LOG(LogTemp, Log, TEXT("Active weapon switched from slot %d to %d"), 
               (int32)OldActiveWeapon, (int32)WeaponSlot);
    }
}

FSHIEquipmentSlot USHIEquipmentComponent::GetEquippedItem(ESHIEquipmentSlot SlotType) const
{
    const FSHIEquipmentSlot* TargetSlot = EquipmentState.GetSlotByType(SlotType);
    if (TargetSlot)
    {
        return *TargetSlot;
    }
    return FSHIEquipmentSlot(); // Return empty slot
}

bool USHIEquipmentComponent::IsSlotEmpty(ESHIEquipmentSlot SlotType) const
{
    const FSHIEquipmentSlot* TargetSlot = EquipmentState.GetSlotByType(SlotType);
    if (TargetSlot)
    {
        return TargetSlot->IsEmpty();
    }
    return true; // Slot doesn't exist = empty
}

USHIItemData* USHIEquipmentComponent::GetActiveWeapon() const
{
    const FSHIEquipmentSlot* ActiveSlot = EquipmentState.GetSlotByType(EquipmentState.ActiveWeaponSlot);
    if (ActiveSlot && !ActiveSlot->IsEmpty())
    {
        return ActiveSlot->ItemData;
    }
    return nullptr;
}

bool USHIEquipmentComponent::CanEquipItem(USHIItemData* ItemData, ESHIEquipmentSlot SlotType) const
{
    if (!ItemData || SlotType == ESHIEquipmentSlot::None)
    {
        return false;
    }

    return IsValidEquipmentSlot(SlotType, ItemData);
}

TArray<FSHIStatModifier> USHIEquipmentComponent::GetAllEquipmentStatBonuses() const
{
    TArray<FSHIStatModifier> AllBonuses;

    // Check all equipment slots
    TArray<ESHIEquipmentSlot> AllSlots = GetAllEquipmentSlots();
    
    for (ESHIEquipmentSlot SlotType : AllSlots)
    {
        const FSHIEquipmentSlot* Slot = EquipmentState.GetSlotByType(SlotType);
        if (Slot && !Slot->IsEmpty() && Slot->ItemData)
        {
            AllBonuses.Append(Slot->ItemData->StatBonuses);
        }
    }

    return AllBonuses;
}

bool USHIEquipmentComponent::IsShieldSlotAvailable() const
{
    return HasSwordEquipped();
}

// Old HasSwordEquipped removed - using enhanced HasValidSwordEquipped version below

bool USHIEquipmentComponent::IsValidEquipmentSlot(ESHIEquipmentSlot SlotType, USHIItemData* ItemData) const
{
    if (!ItemData)
    {
        return false;
    }

    // Use ItemData's validation function
    return ItemData->CanEquipInSlot(SlotType);
}

TArray<ESHIEquipmentSlot> USHIEquipmentComponent::GetAllEquipmentSlots() const
{
    return {
        ESHIEquipmentSlot::Kask,
        ESHIEquipmentSlot::GoguslukZirhi,
        ESHIEquipmentSlot::Kalkan,
        ESHIEquipmentSlot::Silah1,
        ESHIEquipmentSlot::Silah2,
        ESHIEquipmentSlot::Kolye,
        ESHIEquipmentSlot::Yuzuk,
        ESHIEquipmentSlot::Kupe
    };
}

void USHIEquipmentComponent::OnRep_EquipmentState()
{
    UE_LOG(LogTemp, Log, TEXT("Equipment state replicated"));
    
    // Broadcast all equipment changes for UI updates
    TArray<ESHIEquipmentSlot> AllSlots = GetAllEquipmentSlots();
    
    for (ESHIEquipmentSlot SlotType : AllSlots)
    {
        const FSHIEquipmentSlot* Slot = EquipmentState.GetSlotByType(SlotType);
        if (Slot)
        {
            OnEquipmentChanged.Broadcast(SlotType, Slot->ItemData, nullptr);
        }
    }
    
    OnActiveWeaponChanged.Broadcast(EquipmentState.ActiveWeaponSlot);
}

// Old ValidateShieldEquipment removed - using enhanced version below

void USHIEquipmentComponent::BroadcastEquipmentChange(ESHIEquipmentSlot SlotType, USHIItemData* NewItem, USHIItemData* OldItem)
{
    OnEquipmentChanged.Broadcast(SlotType, NewItem, OldItem);
}

void USHIEquipmentComponent::BroadcastActiveWeaponChange()
{
    OnActiveWeaponChanged.Broadcast(EquipmentState.ActiveWeaponSlot);
}

bool USHIEquipmentComponent::IsWeaponSlot(ESHIEquipmentSlot SlotType) const
{
    return SlotType == ESHIEquipmentSlot::Silah1 || 
           SlotType == ESHIEquipmentSlot::Silah2 || 
           SlotType == ESHIEquipmentSlot::Kalkan;
}

bool USHIEquipmentComponent::IsArmorSlot(ESHIEquipmentSlot SlotType) const
{
    return SlotType == ESHIEquipmentSlot::Kask || 
           SlotType == ESHIEquipmentSlot::GoguslukZirhi;
}

bool USHIEquipmentComponent::IsAccessorySlot(ESHIEquipmentSlot SlotType) const
{
    return SlotType == ESHIEquipmentSlot::Kolye || 
           SlotType == ESHIEquipmentSlot::Yuzuk || 
           SlotType == ESHIEquipmentSlot::Kupe;
}

void USHIEquipmentComponent::DebugPrintEquipment() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== EQUIPMENT DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Weapon Slot: %d"), (int32)EquipmentState.ActiveWeaponSlot);
    
    TArray<ESHIEquipmentSlot> AllSlots = GetAllEquipmentSlots();
    
    for (ESHIEquipmentSlot SlotType : AllSlots)
    {
        const FSHIEquipmentSlot* Slot = EquipmentState.GetSlotByType(SlotType);
        if (Slot && !Slot->IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Slot %d: %s x%d"), 
                   (int32)SlotType, 
                   *Slot->ItemData->ItemName.ToString(), 
                   Slot->Quantity);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Slot %d: Empty"), (int32)SlotType);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("=== END EQUIPMENT DEBUG ==="));
}

int32 USHIEquipmentComponent::GetEquippedItemCount() const
{
    int32 Count = 0;
    TArray<ESHIEquipmentSlot> AllSlots = GetAllEquipmentSlots();
    
    for (ESHIEquipmentSlot SlotType : AllSlots)
    {
        if (!IsSlotEmpty(SlotType))
        {
            Count++;
        }
    }
    return Count;
}

// Enhanced Shield Logic Implementation
bool USHIEquipmentComponent::CanEquipShield() const
{
    // Check if we have a valid sword equipped
    return HasValidSwordEquipped();
}

bool USHIEquipmentComponent::HasValidSwordEquipped() const
{
    // Check both weapon slots for a valid sword (Kılıç)
    for (ESHIEquipmentSlot WeaponSlot : {ESHIEquipmentSlot::Silah1, ESHIEquipmentSlot::Silah2})
    {
        const FSHIEquipmentSlot* Slot = EquipmentState.GetSlotByType(WeaponSlot);
        if (Slot && !Slot->IsEmpty() && Slot->ItemData)
        {
            // Check if item is a sword (Kılıç) - enhanced detection
            if (Slot->ItemData->ItemType == ESHIItemType::Silah)
            {
                FString ItemName = Slot->ItemData->ItemName.ToString();
                // Enhanced sword detection - multiple variations + encoding fix
                if (ItemName.Contains(TEXT("Kılıç")) || ItemName.Contains(TEXT("Sword")) ||
                    ItemName.Contains(TEXT("Kilič")) ||  // Alternative spelling
                    ItemName.Contains(TEXT("Saber")) ||
                    ItemName.Contains(TEXT("Sabre")) ||
                    ItemName.Contains(TEXT("Test K")) ||  // Encoding-safe detection
                    ItemName.Contains(TEXT("Kilic")))     // ASCII version
                {
                    // Make sure it's NOT a shield being misidentified
                    if (!ItemName.Contains(TEXT("Kalkan")) && !ItemName.Contains(TEXT("Shield")))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("✅ Valid sword found: %s in slot %d"), *ItemName, (int32)WeaponSlot);
                        return true;
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("❌ No valid sword equipped - checked Silah1 & Silah2"));
    return false;
}

USHIItemData* USHIEquipmentComponent::GetEquippedSword() const
{
    // Return the first valid sword found
    for (ESHIEquipmentSlot WeaponSlot : {ESHIEquipmentSlot::Silah1, ESHIEquipmentSlot::Silah2})
    {
        const FSHIEquipmentSlot* Slot = EquipmentState.GetSlotByType(WeaponSlot);
        if (Slot && !Slot->IsEmpty() && Slot->ItemData)
        {
            if (Slot->ItemData->ItemType == ESHIItemType::Silah)
            {
                FString ItemName = Slot->ItemData->ItemName.ToString();
                if (ItemName.Contains(TEXT("Kılıç")) || ItemName.Contains(TEXT("Sword")) ||
                    ItemName.Contains(TEXT("Test K")) || ItemName.Contains(TEXT("Kilic")))
                {
                    if (!ItemName.Contains(TEXT("Kalkan")) && !ItemName.Contains(TEXT("Shield")))
                    {
                        return Slot->ItemData;
                    }
                }
            }
        }
    }
    return nullptr;
}

void USHIEquipmentComponent::ValidateShieldEquipment()
{
    // Check if shield is equipped but no sword available
    const FSHIEquipmentSlot* ShieldSlot = EquipmentState.GetSlotByType(ESHIEquipmentSlot::Kalkan);
    if (ShieldSlot && !ShieldSlot->IsEmpty())
    {
        if (!HasValidSwordEquipped())
        {
            UE_LOG(LogTemp, Warning, TEXT("Shield equipped but no sword found - auto-unequipping shield"));
            // Auto-unequip shield
            AutoUnequipShieldIfNeeded();
        }
    }
}

bool USHIEquipmentComponent::AutoUnequipShieldIfNeeded()
{
    const FSHIEquipmentSlot* ShieldSlot = EquipmentState.GetSlotByType(ESHIEquipmentSlot::Kalkan);
    if (ShieldSlot && !ShieldSlot->IsEmpty() && !HasValidSwordEquipped())
    {
        // Get shield item data before unequipping
        USHIItemData* ShieldItem = ShieldSlot->ItemData;
        int32 ShieldQuantity = ShieldSlot->Quantity;
        
        UE_LOG(LogTemp, Log, TEXT("Auto-unequipping shield: %s (no sword equipped)"), 
               *ShieldItem->ItemName.ToString());
        
        // Unequip shield
        Server_UnequipItem(ESHIEquipmentSlot::Kalkan);
        
        // Show user feedback
        if (GEngine)
        {
            FString ShieldMessage = FString::Printf(TEXT("🛡️ %s otomatik çıkarıldı - Kılıç gerekli!"), 
                                                   *ShieldItem->ItemName.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Orange, ShieldMessage);
        }
        
        return true;
    }
    return false;
}

bool USHIEquipmentComponent::ValidateShieldDependency(USHIItemData* ShieldItem) const
{
    if (!ShieldItem)
    {
        return false;
    }
    
    // Check if this is a shield item
    FString ItemName = ShieldItem->ItemName.ToString();
    bool IsShield = ItemName.Contains(TEXT("Kalkan")) || ItemName.Contains(TEXT("Shield")) ||
                    ItemName.Contains(TEXT("Buckler"));
    
    if (IsShield)
    {
        // Shield requires sword
        if (!HasValidSwordEquipped())
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot equip shield %s - no sword equipped"), *ItemName);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                    TEXT("🛡️ Kalkan takabilmek için önce Kılıç takmalısın!"));
            }
            return false;
        }
    }
    
    return true;
}

// Legacy function (kept for compatibility)
bool USHIEquipmentComponent::HasSwordEquipped() const
{
    return HasValidSwordEquipped();
}