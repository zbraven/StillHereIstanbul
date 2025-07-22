#include "SHIEquipmentSlotWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Player/SHICharacter.h"
#include "Engine/Engine.h"

USHIEquipmentSlotWidget::USHIEquipmentSlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SlotType = ESHIEquipmentSlot::None;
	OwnerCharacter = nullptr;
	CurrentItem = nullptr;
	CurrentQuantity = 0;
}

void USHIEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotButton)
	{
		SlotButton->OnClicked.AddDynamic(this, &USHIEquipmentSlotWidget::OnSlotClicked);
	}
}

void USHIEquipmentSlotWidget::SetSlotType(ESHIEquipmentSlot InSlotType)
{
	SlotType = InSlotType;
}

void USHIEquipmentSlotWidget::SetOwnerCharacter(ASHICharacter* Character)
{
	OwnerCharacter = Character;
}

void USHIEquipmentSlotWidget::SetItem(USHIItemData* Item, int32 Quantity)
{
	CurrentItem = Item;
	CurrentQuantity = Quantity;

	// Update UI elements
	if (ItemIcon)
	{
		if (Item && Item->ItemIcon)
		{
			ItemIcon->SetBrushFromTexture(Item->ItemIcon);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (QuantityText)
	{
		if (Item && Quantity > 1)
		{
			QuantityText->SetText(FText::AsNumber(Quantity));
			QuantityText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			QuantityText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void USHIEquipmentSlotWidget::OnSlotClicked()
{
	if (GEngine)
	{
		FString SlotName = FString::Printf(TEXT("Equipment Slot %d clicked"), (int32)SlotType);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, SlotName);
	}

	UE_LOG(LogTemp, Log, TEXT("Equipment slot clicked: %d"), (int32)SlotType);
}