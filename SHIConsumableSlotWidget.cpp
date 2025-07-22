#include "SHIConsumableSlotWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Player/SHICharacter.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"

USHIConsumableSlotWidget::USHIConsumableSlotWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SlotIndex = 0;
    OwnerCharacter = nullptr;
    CurrentItem = nullptr;
    CurrentQuantity = 0;
}

void USHIConsumableSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SlotButton)
    {
        SlotButton->OnClicked.AddDynamic(this, &USHIConsumableSlotWidget::OnSlotClicked);
        SlotButton->OnHovered.AddDynamic(this, &USHIConsumableSlotWidget::OnSlotHovered);
        SlotButton->OnUnhovered.AddDynamic(this, &USHIConsumableSlotWidget::OnSlotUnhovered);
    }

    // Initialize as empty slot
    SetSlotEmpty();
}

void USHIConsumableSlotWidget::SetSlotIndex(int32 InSlotIndex)
{
    SlotIndex = InSlotIndex;
    
    // Update key display
    if (KeyText)
    {
        FString KeyString = FString::Printf(TEXT("%d"), SlotIndex);
        KeyText->SetText(FText::FromString(KeyString));
    }

    UE_LOG(LogTemp, Log, TEXT("Consumable slot initialized with key: %d"), SlotIndex);
}

void USHIConsumableSlotWidget::SetOwnerCharacter(ASHICharacter* Character)
{
    OwnerCharacter = Character;
}

void USHIConsumableSlotWidget::SetItem(USHIItemData* Item, int32 Quantity)
{
    CurrentItem = Item;
    CurrentQuantity = FMath::Max(0, Quantity);

    if (CurrentItem && CurrentQuantity > 0)
    {
        SetSlotFilled();
    }
    else
    {
        SetSlotEmpty();
    }

    UpdateVisualState();
}

void USHIConsumableSlotWidget::UseSlot()
{
    if (!CurrentItem || CurrentQuantity <= 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Consumable slot %d is empty, cannot use"), SlotIndex);
        return;
    }

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("No owner character for consumable slot %d"), SlotIndex);
        return;
    }

    // Store item name for feedback before it might be cleared
    FString ItemName = CurrentItem->ItemName.ToString();

    // Use the item through character
    OwnerCharacter->UseConsumableSlot(SlotIndex);

    // Show feedback with stored item name
    if (GEngine)
    {
        FString UseText = FString::Printf(TEXT("Used: %s"), *ItemName);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, UseText);
    }

    UE_LOG(LogTemp, Log, TEXT("Used consumable in slot %d: %s"), SlotIndex, *ItemName);
}

void USHIConsumableSlotWidget::OnSlotClicked()
{
    UseSlot();

    // Visual feedback
    if (GEngine)
    {
        FString ClickText = FString::Printf(TEXT("Hotbar Slot %d Clicked"), SlotIndex);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, ClickText);
    }
}

void USHIConsumableSlotWidget::OnSlotHovered()
{
    // Hover effect - brighten the slot
    if (SlotBackground)
    {
        SlotBackground->SetColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
    }

    // Show item tooltip if item exists
    if (CurrentItem && GEngine)
    {
        FString TooltipText = FString::Printf(TEXT("%s (x%d) - Key %d"), 
                                            *CurrentItem->ItemName.ToString(), 
                                            CurrentQuantity, 
                                            SlotIndex);
        GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Cyan, TooltipText);
    }
}

void USHIConsumableSlotWidget::OnSlotUnhovered()
{
    // Return to normal state
    UpdateVisualState();
}

void USHIConsumableSlotWidget::UpdateVisualState()
{
    if (!SlotBackground || !ItemIcon || !QuantityText) return;

    if (CurrentItem && CurrentQuantity > 0)
    {
        // Item present - show icon and quantity
        if (CurrentItem->ItemIcon)
        {
            ItemIcon->SetBrushFromTexture(CurrentItem->ItemIcon);
            ItemIcon->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            ItemIcon->SetVisibility(ESlateVisibility::Hidden);
        }

        // Show quantity if more than 1
        if (CurrentQuantity > 1)
        {
            QuantityText->SetText(FText::AsNumber(CurrentQuantity));
            QuantityText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            QuantityText->SetVisibility(ESlateVisibility::Hidden);
        }

        // Normal slot background
        SlotBackground->SetColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
    }
    else
    {
        // Empty slot
        ItemIcon->SetVisibility(ESlateVisibility::Hidden);
        QuantityText->SetVisibility(ESlateVisibility::Hidden);
        
        // Darker background for empty slot
        SlotBackground->SetColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f));
    }
}

void USHIConsumableSlotWidget::SetSlotEmpty()
{
    CurrentItem = nullptr;
    CurrentQuantity = 0;
}

void USHIConsumableSlotWidget::SetSlotFilled()
{
    // Visual indication that slot has an item
    // Implementation handled in UpdateVisualState()
}