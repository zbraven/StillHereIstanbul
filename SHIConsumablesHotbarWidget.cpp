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
    int32 ArrayIndex = SlotIndex - 3; // Convert key number to array index

    if (!IsValidSlotIndex(SlotIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid hotbar slot index: %d"), SlotIndex);
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
    int32 ArrayIndex = SlotIndex - 3;

    if (!IsValidSlotIndex(SlotIndex) || !OwnerCharacter)
    {
        return;
    }

    FConsumableSlotData& SlotData = HotbarSlots[ArrayIndex];

    if (SlotData.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Hotbar slot %d is empty"), SlotIndex);
        return;
    }

    // Check if item is consumable
    if (SlotData.ItemData->ItemType != ESHIItemType::Tuketim)
    {
        UE_LOG(LogTemp, Warning, TEXT("Item in slot %d is not consumable"), SlotIndex);
        return;
    }

    // Apply consumable effect
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

    // Show feedback
    if (GEngine)
    {
        FString UseText = FString::Printf(TEXT("Used: %s"), *SlotData.ItemData->ItemName.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, UseText);
    }

    UE_LOG(LogTemp, Log, TEXT("Used consumable in slot %d"), SlotIndex);
}

void USHIConsumablesHotbarWidget::ApplyConsumableEffect(USHIItemData* Item)
{
    if (!Item || !OwnerCharacter) 
        return;

    USHIStatsComponent* StatsComp = OwnerCharacter->GetStatsComponent();
    if (!StatsComp) 
        return;

    // Apply effects based on item name (simplified system)
    FString ItemName = Item->ItemName.ToString().ToLower();

    if (ItemName.Contains("potion") || ItemName.Contains("iksir") || ItemName.Contains("health"))
    {
        // Health potion effect
        float HealAmount = 50.0f; // Base heal amount
        
        // Apply healing (this would need a health system)
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
    }
}

void USHIConsumablesHotbarWidget::RefreshSlotDisplay(int32 SlotIndex)
{
    USHIConsumableSlotWidget* SlotWidget = GetSlotWidget(SlotIndex);
    if (!SlotWidget) 
        return;

    int32 ArrayIndex = SlotIndex - 3;
    const FConsumableSlotData& SlotData = HotbarSlots[ArrayIndex];

    SlotWidget->SetItem(SlotData.ItemData, SlotData.Quantity);
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