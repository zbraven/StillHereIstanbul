#include "SHIInventoryComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

USHIInventoryComponent::USHIInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void USHIInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeInventory();
    
    UE_LOG(LogTemp, Warning, TEXT("SHI Inventory initialized with %d slots"), InventorySize);
}

void USHIInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USHIInventoryComponent, InventorySlots);
}

void USHIInventoryComponent::OnRep_InventorySlots()
{
    // Broadcast inventory changes to UI
    for (int32 i = 0; i < InventorySlots.Num(); i++)
    {
        OnInventoryChanged.Broadcast(i, InventorySlots[i]);
    }
}

void USHIInventoryComponent::InitializeInventory()
{
    if (InventorySlots.Num() != InventorySize)
    {
        InventorySlots.SetNum(InventorySize);
        for (int32 i = 0; i < InventorySize; i++)
        {
            InventorySlots[i] = FSHIInventorySlot();
        }
    }
}

void USHIInventoryComponent::Server_AddItem_Implementation(USHIItemData* ItemData, int32 Quantity)
{
    if (!ItemData || Quantity <= 0)
    {
        return;
    }
    
    int32 RemainingQuantity = Quantity;
    
    // First, try to stack with existing items
    if (ItemData->IsStackable())
    {
        for (int32 i = 0; i < InventorySlots.Num(); i++)
        {
            if (InventorySlots[i].CanStackWith(ItemData))
            {
                int32 CanAdd = ItemData->MaxStackSize - InventorySlots[i].Quantity;
                int32 AddAmount = FMath::Min(CanAdd, RemainingQuantity);
                
                if (AddAmount > 0)
                {
                    InventorySlots[i].Quantity += AddAmount;
                    RemainingQuantity -= AddAmount;
                    BroadcastSlotChange(i);
                    
                    if (RemainingQuantity <= 0)
                        break;
                }
            }
        }
    }
    
    // Then add to empty slots
    while (RemainingQuantity > 0)
    {
        int32 EmptySlot = FindFirstEmptySlot();
        if (EmptySlot == -1)
        {
            UE_LOG(LogTemp, Warning, TEXT("Inventory full! Could not add %d of %s"), 
                   RemainingQuantity, *ItemData->ItemName.ToString());
            break;
        }
        
        int32 AddAmount = ItemData->IsStackable() ? 
            FMath::Min(RemainingQuantity, ItemData->MaxStackSize) : 1;
            
        InventorySlots[EmptySlot].ItemData = ItemData;
        InventorySlots[EmptySlot].Quantity = AddAmount;
        RemainingQuantity -= AddAmount;
        
        BroadcastSlotChange(EmptySlot);
        OnItemAdded.Broadcast(ItemData, AddAmount, EmptySlot);
        
        UE_LOG(LogTemp, Log, TEXT("Added %d of %s to slot %d"), 
               AddAmount, *ItemData->ItemName.ToString(), EmptySlot);
    }
}

void USHIInventoryComponent::Server_RemoveItem_Implementation(int32 SlotIndex, int32 Quantity)
{
    if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num())
        return;
        
    if (InventorySlots[SlotIndex].IsEmpty())
        return;
    
    int32 RemoveAmount = FMath::Min(Quantity, InventorySlots[SlotIndex].Quantity);
    InventorySlots[SlotIndex].Quantity -= RemoveAmount;
    
    if (InventorySlots[SlotIndex].Quantity <= 0)
    {
        InventorySlots[SlotIndex].ItemData = nullptr;
        InventorySlots[SlotIndex].Quantity = 0;
    }
    
    BroadcastSlotChange(SlotIndex);
    
    UE_LOG(LogTemp, Log, TEXT("Removed %d items from slot %d"), RemoveAmount, SlotIndex);
}

void USHIInventoryComponent::Server_MoveItem_Implementation(int32 FromSlot, int32 ToSlot)
{
    if (FromSlot < 0 || FromSlot >= InventorySlots.Num() || 
        ToSlot < 0 || ToSlot >= InventorySlots.Num() || 
        FromSlot == ToSlot)
        return;
    
    // Swap slots
    FSHIInventorySlot TempSlot = InventorySlots[FromSlot];
    InventorySlots[FromSlot] = InventorySlots[ToSlot];
    InventorySlots[ToSlot] = TempSlot;
    
    BroadcastSlotChange(FromSlot);
    BroadcastSlotChange(ToSlot);
    
    UE_LOG(LogTemp, Log, TEXT("Moved item from slot %d to slot %d"), FromSlot, ToSlot);
}

void USHIInventoryComponent::Server_UseItem_Implementation(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num())
        return;
        
    if (InventorySlots[SlotIndex].IsEmpty())
        return;
    
    USHIItemData* ItemData = InventorySlots[SlotIndex].ItemData;
    
    // For now, just remove consumable items when used
    if (ItemData->ItemType == ESHIItemType::Tuketim)
    {
        Server_RemoveItem(SlotIndex, 1);
        UE_LOG(LogTemp, Log, TEXT("Used consumable: %s"), *ItemData->ItemName.ToString());
    }
}

bool USHIInventoryComponent::CanAddItem(USHIItemData* ItemData, int32 Quantity) const
{
    if (!ItemData || Quantity <= 0)
        return false;
    
    int32 RemainingQuantity = Quantity;
    
    // Check existing stacks
    if (ItemData->IsStackable())
    {
        for (const auto& Slot : InventorySlots)
        {
            if (Slot.CanStackWith(ItemData))
            {
                int32 CanAdd = ItemData->MaxStackSize - Slot.Quantity;
                RemainingQuantity -= CanAdd;
                if (RemainingQuantity <= 0)
                    return true;
            }
        }
    }
    
    // Check empty slots
    int32 EmptySlots = 0;
    for (const auto& Slot : InventorySlots)
    {
        if (Slot.IsEmpty())
            EmptySlots++;
    }
    
    int32 SlotsNeeded = ItemData->IsStackable() ? 
        FMath::CeilToInt(float(RemainingQuantity) / ItemData->MaxStackSize) : RemainingQuantity;
    
    return EmptySlots >= SlotsNeeded;
}

int32 USHIInventoryComponent::GetItemCount(USHIItemData* ItemData) const
{
    int32 TotalCount = 0;
    for (const auto& Slot : InventorySlots)
    {
        if (Slot.ItemData == ItemData)
        {
            TotalCount += Slot.Quantity;
        }
    }
    return TotalCount;
}

FSHIInventorySlot USHIInventoryComponent::GetSlot(int32 SlotIndex) const
{
    if (SlotIndex >= 0 && SlotIndex < InventorySlots.Num())
    {
        return InventorySlots[SlotIndex];
    }
    return FSHIInventorySlot();
}

int32 USHIInventoryComponent::FindFirstEmptySlot() const
{
    for (int32 i = 0; i < InventorySlots.Num(); i++)
    {
        if (InventorySlots[i].IsEmpty())
        {
            return i;
        }
    }
    return -1;
}

int32 USHIInventoryComponent::FindItemSlot(USHIItemData* ItemData) const
{
    for (int32 i = 0; i < InventorySlots.Num(); i++)
    {
        if (InventorySlots[i].ItemData == ItemData)
        {
            return i;
        }
    }
    return -1;
}

void USHIInventoryComponent::BroadcastSlotChange(int32 SlotIndex)
{
    if (SlotIndex >= 0 && SlotIndex < InventorySlots.Num())
    {
        OnInventoryChanged.Broadcast(SlotIndex, InventorySlots[SlotIndex]);
    }
}