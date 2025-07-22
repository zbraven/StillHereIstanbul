#include "SHIInventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/SHIInventoryWidget.h"
#include "Data/SHIItemData.h"
#include "Engine/Engine.h"

void USHIInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Bind button events
    if (SlotButton)
    {
        SlotButton->OnClicked.AddDynamic(this, &USHIInventorySlotWidget::OnSlotClicked);
        SlotButton->OnHovered.AddDynamic(this, &USHIInventorySlotWidget::OnSlotHovered);
        SlotButton->OnUnhovered.AddDynamic(this, &USHIInventorySlotWidget::OnSlotUnhovered);
    }
    
    // Initialize as empty slot
    SetSlotEmpty();
    
    UE_LOG(LogTemp, Log, TEXT("Inventory slot widget constructed"));
}

void USHIInventorySlotWidget::SetSlotIndex(int32 InSlotIndex)
{
    SlotIndex = InSlotIndex;
    UE_LOG(LogTemp, Log, TEXT("Slot index set to: %d"), SlotIndex);
}

void USHIInventorySlotWidget::SetOwnerInventory(USHIInventoryWidget* InOwnerInventory)
{
    OwnerInventory = InOwnerInventory;
    UE_LOG(LogTemp, VeryVerbose, TEXT("Slot %d owner inventory set"), SlotIndex);
}

void USHIInventorySlotWidget::UpdateSlotData(const FSHIInventorySlot& SlotData)
{
    CurrentSlotData = SlotData;
    UpdateSlotVisuals();
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Slot %d updated: %s"), 
           SlotIndex, 
           SlotData.IsEmpty() ? TEXT("Empty") : *SlotData.ItemData->ItemName.ToString());
}

void USHIInventorySlotWidget::UpdateSlotVisuals()
{
    if (CurrentSlotData.IsEmpty())
    {
        SetSlotEmpty();
    }
    else
    {
        SetSlotWithItem();
    }
}

void USHIInventorySlotWidget::SetSlotEmpty()
{
    // Set background to empty color
    if (SlotBackground)
    {
        SlotBackground->SetColorAndOpacity(EmptySlotColor);
    }
    
    // Hide item icon
    if (ItemIcon)
    {
        ItemIcon->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // Hide quantity text
    if (QuantityText)
    {
        QuantityText->SetVisibility(ESlateVisibility::Hidden);
        QuantityText->SetText(FText::GetEmpty());
    }
}

void USHIInventorySlotWidget::SetSlotWithItem()
{
    if (!CurrentSlotData.ItemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Slot %d has no item data"), SlotIndex);
        return;
    }
    
    // Set background to filled color
    if (SlotBackground)
    {
        SlotBackground->SetColorAndOpacity(FilledSlotColor);
    }
    
    // Show and set item icon
    if (ItemIcon)
    {
        if (CurrentSlotData.ItemData->ItemIcon)
        {
            // Use the item's icon texture
            ItemIcon->SetBrushFromTexture(CurrentSlotData.ItemData->ItemIcon);
            ItemIcon->SetColorAndOpacity(CurrentSlotData.ItemData->RarityColor);
        }
        else
        {
            // No icon available, show colored square based on rarity
            ItemIcon->SetColorAndOpacity(CurrentSlotData.ItemData->RarityColor);
        }
        ItemIcon->SetVisibility(ESlateVisibility::Visible);
    }
    
    // Show quantity if greater than 1
    if (QuantityText)
    {
        if (CurrentSlotData.Quantity > 1)
        {
            QuantityText->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentSlotData.Quantity)));
            QuantityText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            QuantityText->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void USHIInventorySlotWidget::SetSlotHighlighted(bool bHighlighted)
{
    if (SlotBackground)
    {
        if (bHighlighted)
        {
            SlotBackground->SetColorAndOpacity(HoverSlotColor);
        }
        else
        {
            // Restore original color based on slot state
            if (CurrentSlotData.IsEmpty())
            {
                SlotBackground->SetColorAndOpacity(EmptySlotColor);
            }
            else
            {
                SlotBackground->SetColorAndOpacity(FilledSlotColor);
            }
        }
    }
}

void USHIInventorySlotWidget::OnSlotClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Slot %d clicked"), SlotIndex);
    
    // Show slot info on screen with different colors for different states
    if (GEngine)
    {
        if (CurrentSlotData.IsEmpty())
        {
            FString SlotInfo = FString::Printf(TEXT("Slot %d: Boş"), SlotIndex);
            // Custom gray color for empty slots
            FColor GrayColor(128, 128, 128, 255);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, GrayColor, SlotInfo);
        }
        else
        {
            // Show detailed item information
            FString SlotInfo = FString::Printf(TEXT("Slot %d: %s x%d (%s)"), 
                                             SlotIndex,
                                             *CurrentSlotData.ItemData->ItemName.ToString(),
                                             CurrentSlotData.Quantity,
                                             *CurrentSlotData.ItemData->GetLocalizedTypeName().ToString());
            
            // Use different colors based on item type
            FColor DisplayColor = FColor::White;
            switch (CurrentSlotData.ItemData->ItemType)
            {
                case ESHIItemType::Silah:
                    DisplayColor = FColor(255, 100, 100, 255); // Light red for weapons
                    break;
                case ESHIItemType::Zirh:
                    DisplayColor = FColor(100, 100, 255, 255); // Light blue for armor
                    break;
                case ESHIItemType::Tuketim:
                    DisplayColor = FColor(100, 255, 100, 255); // Light green for consumables
                    break;
                case ESHIItemType::Materyal:
                    DisplayColor = FColor(255, 255, 100, 255); // Light yellow for materials
                    break;
                default:
                    DisplayColor = FColor::White;
                    break;
            }
            
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, DisplayColor, SlotInfo);
            
            // Also show item description if available
            if (!CurrentSlotData.ItemData->ItemDescription.IsEmpty())
            {
                FString Description = CurrentSlotData.ItemData->ItemDescription.ToString();
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, Description);
            }
        }
    }
}

void USHIInventorySlotWidget::OnSlotHovered()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("Slot %d hovered"), SlotIndex);
    SetSlotHighlighted(true);
    
    // Show quick item info on hover (without consuming screen message queue)
    if (!CurrentSlotData.IsEmpty() && GEngine)
    {
        FString QuickInfo = FString::Printf(TEXT("%s"), 
                                          *CurrentSlotData.ItemData->ItemName.ToString());
        // Use a very short duration for hover messages
        GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, QuickInfo);
    }
}

void USHIInventorySlotWidget::OnSlotUnhovered()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("Slot %d unhovered"), SlotIndex);
    SetSlotHighlighted(false);
}