#include "SHIConsumablesHotbarWidget.h"
#include "Player/SHICharacter.h"
#include "Components/SHIStatsComponent.h"
#include "Components/SHIInventoryComponent.h"
#include "SHIConsumableSlotWidget.h"
#include "Engine/Engine.h"

USHIConsumablesHotbarWidget::USHIConsumablesHotbarWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    OwnerCharacter = nullptr;
    // Initialize 4 hotbar slots (for keys 3,4,5,6)
    HotbarSlots.SetNum(4);
    for (int32 i = 0; i < 4; i++)
    {
        HotbarSlots[i] = FConsumableSlotData();
    }
}

void USHIConsumablesHotbarWidget::NativeConstruct()
{
    Super::NativeConstruct();
    UE_LOG(LogTemp, Log, TEXT("Consumables Hotbar Widget constructed"));
}

void USHIConsumablesHotbarWidget::SetOwnerCharacter(ASHICharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetOwnerCharacter: Invalid character"));
        return;
    }

    OwnerCharacter = Character;

    // Initialize slot widgets
    TArray<USHIConsumableSlotWidget*> SlotWidgets = {Slot3Widget, Slot4Widget, Slot5Widget, Slot6Widget};
    for (int32 i = 0; i < SlotWidgets.Num(); i++)
    {
        if (SlotWidgets[i])
        {
            SlotWidgets[i]->SetSlotIndex(i + 3); // Keys 3,4,5,6
            SlotWidgets[i]->SetOwnerCharacter(OwnerCharacter);
        }
    }

    // Initial display refresh
    RefreshAllSlots();
    UE_LOG(LogTemp, Log, TEXT("Consumables hotbar initialized for character"));
}

void USHIConsumablesHotbarWidget::SetSlotItem(int32 SlotIndex, USHIItemData* Item, int32 Quantity)
{
    UE_LOG(LogTemp, Log, TEXT("=== SetSlotItem Called: Slot %d ==="), SlotIndex);

    int32 ArrayIndex = SlotIndex - 3; // Convert key number to array index

    if (!IsValidSlotIndex(SlotIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("SetSlotItem: Invalid hotbar slot index: %d"), SlotIndex);
        return;
    }

    if (ArrayIndex < 0 || ArrayIndex >= HotbarSlots.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("SetSlotItem: ArrayIndex out of bounds: %d (max: %d)"), ArrayIndex, HotbarSlots.Num());
        return;
    }

    // Update hotbar data
    HotbarSlots[ArrayIndex].ItemData = Item;
    HotbarSlots[ArrayIndex].Quantity = FMath::Max(0, Quantity);

    // Update visual display
    RefreshSlotDisplay(SlotIndex);

    UE_LOG(LogTemp, Log, TEXT("Hotbar slot %d set to: %s x%d"),
        SlotIndex, Item ? *Item->ItemName.ToString() : TEXT("Empty"), Quantity);
}

void USHIConsumablesHotbarWidget::UseSlot(int32 SlotIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("=== Hotbar UseSlot Called: %d ==="), SlotIndex);

    int32 ArrayIndex = SlotIndex - 3;
    
    if (!IsValidSlotIndex(SlotIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid slot index: %d"), SlotIndex);
        return;
    }

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("OwnerCharacter is NULL"));
        return;
    }

    if (!IsValid(OwnerCharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("OwnerCharacter is not valid"));
        return;
    }

    if (ArrayIndex < 0 || ArrayIndex >= HotbarSlots.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("ArrayIndex out of bounds: %d (max: %d)"), ArrayIndex, HotbarSlots.Num());
        return;
    }

    FConsumableSlotData& SlotData = HotbarSlots[ArrayIndex];
    
    if (SlotData.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Hotbar slot %d is empty"), SlotIndex);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Empty Slot!"));
        }
        return;
    }

    if (!SlotData.ItemData)
    {
        UE_LOG(LogTemp, Error, TEXT("SlotData.ItemData is NULL"));
        return;
    }

    if (!IsValid(SlotData.ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("SlotData.ItemData is not valid"));
        return;
    }

    // Check if item is consumable
    if (SlotData.ItemData->ItemType != ESHIItemType::Tuketim)
    {
        UE_LOG(LogTemp, Warning, TEXT("Item in slot %d is not consumable"), SlotIndex);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("About to apply consumable effect"));

    // Store item name for feedback before it might be cleared
    FString ItemName = SlotData.ItemData->ItemName.ToString();

    // Apply consumable effect with safety
    ApplyConsumableEffect(SlotData.ItemData);

    // Decrease quantity
    SlotData.Quantity--;
    
    if (SlotData.Quantity <= 0)
    {
        SlotData.ItemData = nullptr;
        SlotData.Quantity = 0;
    }

    // Update display
    RefreshSlotDisplay(SlotIndex);

    // Show feedback with stored item name
    if (GEngine)
    {
        FString UseText = FString::Printf(TEXT("Used: %s"), *ItemName);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, UseText);
    }

    UE_LOG(LogTemp, Warning, TEXT("=== Hotbar UseSlot Completed Successfully ==="));
}

void USHIConsumablesHotbarWidget::ApplyConsumableEffect(USHIItemData* Item)
{
    UE_LOG(LogTemp, Log, TEXT("=== ApplyConsumableEffect Called ==="));

    if (!Item) 
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyConsumableEffect: Item is NULL"));
        return;
    }

    if (!IsValid(Item))
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyConsumableEffect: Item is not valid"));
        return;
    }

    if (!OwnerCharacter) 
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyConsumableEffect: OwnerCharacter is NULL"));
        return;
    }

    if (!IsValid(OwnerCharacter))
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyConsumableEffect: OwnerCharacter is not valid"));
        return;
    }

    USHIStatsComponent* StatsComp = OwnerCharacter->GetStatsComponent();
    if (!StatsComp) 
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyConsumableEffect: No StatsComponent found"));
        // Don't return here - we can still show effects without stats
    }

    // Apply effects based on item name (simplified system)
    FString ItemName = Item->ItemName.ToString().ToLower();
    UE_LOG(LogTemp, Log, TEXT("Applying effect for item: %s"), *ItemName);

    if (ItemName.Contains("potion") || ItemName.Contains("iksir") || ItemName.Contains("health"))
    {
        // Health potion effect
        float HealAmount = 50.0f; // Base heal amount
        
        UE_LOG(LogTemp, Warning, TEXT("Applied health potion: +%.0f health"), HealAmount);
        
        if (GEngine)
        {
            FString HealText = FString::Printf(TEXT("+%.0f Sağlık"), HealAmount);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, HealText);
        }
    }
    else if (ItemName.Contains("mana"))
    {
        // Mana potion effect
        float ManaAmount = 30.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Applied mana potion: +%.0f mana"), ManaAmount);
        
        if (GEngine)
        {
            FString ManaText = FString::Printf(TEXT("+%.0f Mana"), ManaAmount);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, ManaText);
        }
    }
    else
    {
        // Generic consumable effect
        UE_LOG(LogTemp, Log, TEXT("Applied generic consumable effect: %s"), *Item->ItemName.ToString());
        
        if (GEngine)
        {
            FString GenericText = FString::Printf(TEXT("Used: %s"), *Item->ItemName.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, GenericText);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("=== ApplyConsumableEffect Completed ==="));
}

void USHIConsumablesHotbarWidget::RefreshSlotDisplay(int32 SlotIndex)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("RefreshSlotDisplay called for slot %d"), SlotIndex);

    USHIConsumableSlotWidget* SlotWidget = GetSlotWidget(SlotIndex);
    if (!SlotWidget) 
    {
        UE_LOG(LogTemp, Warning, TEXT("RefreshSlotDisplay: No slot widget found for slot %d"), SlotIndex);
        return;
    }

    if (!IsValid(SlotWidget))
    {
        UE_LOG(LogTemp, Warning, TEXT("RefreshSlotDisplay: Slot widget is not valid for slot %d"), SlotIndex);
        return;
    }

    int32 ArrayIndex = SlotIndex - 3;
    if (ArrayIndex < 0 || ArrayIndex >= HotbarSlots.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("RefreshSlotDisplay: ArrayIndex out of bounds: %d"), ArrayIndex);
        return;
    }

    const FConsumableSlotData& SlotData = HotbarSlots[ArrayIndex];

    SlotWidget->SetItem(SlotData.ItemData, SlotData.Quantity);
    UE_LOG(LogTemp, VeryVerbose, TEXT("RefreshSlotDisplay completed for slot %d"), SlotIndex);
}

void USHIConsumablesHotbarWidget::RefreshAllSlots()
{
    for (int32 i = 3; i <= 6; i++)
    {
        RefreshSlotDisplay(i);
    }
}

USHIConsumableSlotWidget* USHIConsumablesHotbarWidget::GetSlotWidget(int32 SlotIndex)
{
    switch (SlotIndex)
    {
    case 3: return Slot3Widget;
    case 4: return Slot4Widget;
    case 5: return Slot5Widget;
    case 6: return Slot6Widget;
    default: return nullptr;
    }
}

bool USHIConsumablesHotbarWidget::IsValidSlotIndex(int32 SlotIndex) const
{
    return SlotIndex >= 3 && SlotIndex <= 6;
}