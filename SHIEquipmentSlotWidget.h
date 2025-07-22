#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/SHIEquipmentComponent.h"
#include "Data/SHIItemData.h"
#include "SHIEquipmentSlotWidget.generated.h"

class ASHICharacter;
class UImage;
class UTextBlock;
class UButton;

UCLASS()
class STILLHEREISTANBUL_API USHIEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USHIEquipmentSlotWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

	// UI Elements
	UPROPERTY(meta = (BindWidget))
	UButton* SlotButton;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuantityText;

public:
	// Setup functions
	UFUNCTION(BlueprintCallable, Category = "SHI Equipment")
	void SetSlotType(ESHIEquipmentSlot InSlotType);

	UFUNCTION(BlueprintCallable, Category = "SHI Equipment")
	void SetOwnerCharacter(ASHICharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "SHI Equipment")
	void SetItem(USHIItemData* Item, int32 Quantity);

protected:
	// Properties
	UPROPERTY()
	ESHIEquipmentSlot SlotType;

	UPROPERTY()
	ASHICharacter* OwnerCharacter;

	UPROPERTY()
	USHIItemData* CurrentItem;

	UPROPERTY()
	int32 CurrentQuantity;

	// Button events
	UFUNCTION()
	void OnSlotClicked();
};