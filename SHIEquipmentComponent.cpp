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

    // Handle shield dependency logic
    if (SlotType == ESHIEquipmentSlot::Kalkan)
    {
        if (!HasSwordEquipped())
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot equip shield without sword equipped"));
            return;
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

    // Validate shield equipment after unequipping
    ValidateShieldEquipment();

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

bool USHIEquipmentComponent::HasSwordEquipped() const
{
    // Check both weapon slots for sword (Kılıç)
    for (ESHIEquipmentSlot WeaponSlot : {ESHIEquipmentSlot::Silah1, ESHIEquipmentSlot::Silah2})
    {
        const FSHIEquipmentSlot* Slot = EquipmentState.GetSlotByType(WeaponSlot);
        if (Slot && !Slot->IsEmpty() && Slot->ItemData)
        {
            // Check if item is a sword (not shield)
            if (Slot->ItemData->ItemType == ESHIItemType::Silah && 
                !Slot->ItemData->ItemName.ToString().Contains(TEXT("Kalkan")))
            {
                return true;
            }
        }
    }
    return false;
}

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

void USHIEquipmentComponent::ValidateShieldEquipment()
{
    // Auto-unequip shield if no sword equipped
    if (!HasSwordEquipped() && !IsSlotEmpty(ESHIEquipmentSlot::Kalkan))
    {
        UE_LOG(LogTemp, Warning, TEXT("Auto-unequipping shield - no sword equipped"));
        
        // Get shield item data before unequipping
        FSHIEquipmentSlot ShieldSlot = GetEquippedItem(ESHIEquipmentSlot::Kalkan);
        
        // Move shield back to inventory if possible
        if (ASHICharacter* Character = Cast<ASHICharacter>(GetOwner()))
        {
            if (USHIInventoryComponent* Inventory = Character->GetInventoryComponent())
            {
                if (!ShieldSlot.IsEmpty())
                {
                    // Try to add shield back to inventory
                    if (Inventory->CanAddItem(ShieldSlot.ItemData, ShieldSlot.Quantity))
                    {
                        Inventory->Server_AddItem(ShieldSlot.ItemData, ShieldSlot.Quantity);
                    }
                }
            }
        }
        
        // Unequip the shield
        Server_UnequipItem(ESHIEquipmentSlot::Kalkan);
    }
}

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