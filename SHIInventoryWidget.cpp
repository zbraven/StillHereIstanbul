#include "SHIInventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Player/SHICharacter.h"
#include "Components/SHIInventoryComponent.h"
#include "UI/SHIInventorySlotWidget.h"
#include "Engine/Engine.h"

void USHIInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Find owner character
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ASHICharacter* Character = Cast<ASHICharacter>(PC->GetPawn()))
        {
            SetOwnerCharacter(Character);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Inventory Widget constructed"));
}

void USHIInventoryWidget::SetOwnerCharacter(ASHICharacter* Character)
{
    OwnerCharacter = Character;
    
    if (OwnerCharacter)
    {
        InventoryComponent = OwnerCharacter->GetInventoryComponent();
        
        if (InventoryComponent)
        {
            // Bind to inventory change events
            InventoryComponent->OnInventoryChanged.AddDynamic(this, &USHIInventoryWidget::OnInventoryChanged);
            
            // Initialize the grid
            InitializeInventoryGrid();
            
            UE_LOG(LogTemp, Log, TEXT("Inventory widget bound to character inventory"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Character has no inventory component"));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Inventory widget owner set: %s"), 
           Character ? TEXT("Valid") : TEXT("Null"));
}

void USHIInventoryWidget::InitializeInventoryGrid()
{
    if (!InventoryGrid)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryGrid widget not bound"));
        return;
    }
    
    if (!SlotWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SlotWidgetClass not set"));
        return;
    }
    
    CreateSlotWidgets();
    RefreshInventoryDisplay();
    
    UE_LOG(LogTemp, Log, TEXT("Inventory grid initialized with %d slots"), TotalSlots);
}

void USHIInventoryWidget::CreateSlotWidgets()
{
    if (!InventoryGrid || !SlotWidgetClass)
        return;
    
    // Clear existing widgets
    InventoryGrid->ClearChildren();
    SlotWidgets.Empty();
    
    // Create slot widgets
    for (int32 i = 0; i < TotalSlots; i++)
    {
        USHIInventorySlotWidget* SlotWidget = CreateWidget<USHIInventorySlotWidget>(this, SlotWidgetClass);
        if (SlotWidget)
        {
            SlotWidget->SetSlotIndex(i);
            SlotWidget->SetOwnerInventory(this);
            
            // Calculate grid position
            int32 Row = i / GridColumns;
            int32 Column = i % GridColumns;
            
            // Add to grid
            InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Column);
            SlotWidgets.Add(SlotWidget);
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("Created slot widget %d at position (%d, %d)"), 
                   i, Row, Column);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create slot widget %d"), i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created %d inventory slot widgets"), SlotWidgets.Num());
}

void USHIInventoryWidget::RefreshInventoryDisplay()
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("No inventory component to refresh from"));
        return;
    }
    
    // Update all slot widgets
    for (int32 i = 0; i < SlotWidgets.Num() && i < TotalSlots; i++)
    {
        FSHIInventorySlot SlotData = InventoryComponent->GetSlot(i);
        UpdateSlotWidget(i, SlotData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Refreshed %d inventory slots"), SlotWidgets.Num());
}

void USHIInventoryWidget::UpdateSlotWidget(int32 SlotIndex, const FSHIInventorySlot& SlotData)
{
    if (SlotIndex >= 0 && SlotIndex < SlotWidgets.Num())
    {
        USHIInventorySlotWidget* SlotWidget = SlotWidgets[SlotIndex];
        if (SlotWidget)
        {
            SlotWidget->UpdateSlotData(SlotData);
            
            UE_LOG(LogTemp, VeryVerbose, TEXT("Updated slot widget %d: %s"), 
                   SlotIndex, 
                   SlotData.IsEmpty() ? TEXT("Empty") : *SlotData.ItemData->ItemName.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Slot widget %d is null"), SlotIndex);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid slot index: %d"), SlotIndex);
    }
}

void USHIInventoryWidget::OnInventoryChanged(int32 SlotIndex, const FSHIInventorySlot& NewSlot)
{
    // Update specific slot
    UpdateSlotWidget(SlotIndex, NewSlot);
    
    UE_LOG(LogTemp, Log, TEXT("Inventory slot %d updated in UI: %s"), 
           SlotIndex,
           NewSlot.IsEmpty() ? TEXT("Empty") : *NewSlot.ItemData->ItemName.ToString());
    
    // Show inventory status update on screen
    if (GEngine)
    {
        int32 UsedSlots = GetUsedSlots();
        FString StatusText = FString::Printf(TEXT("Envanter: %d/%d"), UsedSlots, TotalSlots);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, StatusText);
    }
}

int32 USHIInventoryWidget::GetUsedSlots() const
{
    if (!InventoryComponent)
        return 0;
    
    int32 UsedCount = 0;
    for (int32 i = 0; i < TotalSlots; i++)
    {
        if (!InventoryComponent->GetSlot(i).IsEmpty())
        {
            UsedCount++;
        }
    }
    
    return UsedCount;
}

USHIInventorySlotWidget* USHIInventoryWidget::GetSlotWidget(int32 SlotIndex) const
{
    if (SlotIndex >= 0 && SlotIndex < SlotWidgets.Num())
    {
        return SlotWidgets[SlotIndex];
    }
    
    return nullptr;
}