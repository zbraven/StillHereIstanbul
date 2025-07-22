#include "UI/SHIEquipmentPanelWidget.h"
#include "Player/SHICharacter.h"
#include "Components/SHIEquipmentComponent.h"
#include "Components/SHIStatsComponent.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"

USHIEquipmentPanelWidget::USHIEquipmentPanelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    OwnerCharacter = nullptr;
    EquipmentComponent = nullptr;
    StatsComponent = nullptr;
}

void USHIEquipmentPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UE_LOG(LogTemp, Log, TEXT("Equipment Panel Widget constructed"));
    
    // Initialize slots if we have them
    InitializeSlots();
}

void USHIEquipmentPanelWidget::SetOwnerCharacter(ASHICharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("Equipment Panel: Invalid character provided"));
        return;
    }

    OwnerCharacter = Character;
    EquipmentComponent = Character->GetEquipmentComponent();
    StatsComponent = Character->GetStatsComponent();

    if (!EquipmentComponent || !StatsComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Equipment Panel: Missing required components"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Equipment Panel: Owner character set successfully"));

    // Initialize equipment slots (11 slots)
    TMap<ESHIEquipmentSlot, USHIEquipmentSlotWidget*> SlotWidgets = {
        {ESHIEquipmentSlot::Kask, KaskSlot},
        {ESHIEquipmentSlot::GoguslukZirhi, GoguslukZirhiSlot},
        {ESHIEquipmentSlot::Kalkan, KalkanSlot},
        {ESHIEquipmentSlot::Silah1, Silah1Slot},
        {ESHIEquipmentSlot::Silah2, Silah2Slot},
        {ESHIEquipmentSlot::Kolye, KolyeSlot},
        {ESHIEquipmentSlot::Yuzuk, YuzukSlot},
        {ESHIEquipmentSlot::Kupe, KupeSlot},
        {ESHIEquipmentSlot::Eldiven, EldivenSlot},      // ⬅️ NEW
        {ESHIEquipmentSlot::Pantolon, PantolonSlot},    // ⬅️ NEW
        {ESHIEquipmentSlot::Ayakkabi, AyakkabiSlot}     // ⬅️ NEW
    };

    // Initialize each slot widget - simplified approach
    for (const auto& SlotPair : SlotWidgets)
    {
        ESHIEquipmentSlot SlotType = SlotPair.Key;
        USHIEquipmentSlotWidget* SlotWidget = SlotPair.Value;

        if (SlotWidget)
        {
            // Just log that we found the slot widget - actual setup will happen in RefreshEquipmentDisplay
            UE_LOG(LogTemp, VeryVerbose, TEXT("Found equipment slot widget for type: %d"), (int32)SlotType);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Equipment Panel: Slot widget for type %d is null"), (int32)SlotType);
        }
    }

    // Bind equipment events
    BindEquipmentEvents();

    // Initial display refresh
    RefreshEquipmentDisplay();
    RefreshStatsDisplay();
}

void USHIEquipmentPanelWidget::BindEquipmentEvents()
{
    if (EquipmentComponent)
    {
        // Bind equipment change events
        EquipmentComponent->OnEquipmentChanged.AddDynamic(this, &USHIEquipmentPanelWidget::OnEquipmentChanged);
        UE_LOG(LogTemp, Log, TEXT("Equipment Panel: Equipment events bound"));
    }

    if (StatsComponent)
    {
        // Bind stats recalculation events
        StatsComponent->OnStatsRecalculated.AddDynamic(this, &USHIEquipmentPanelWidget::OnStatsRecalculated);
        UE_LOG(LogTemp, Log, TEXT("Equipment Panel: Stats events bound"));
    }
}

void USHIEquipmentPanelWidget::InitializeSlots()
{
    // Validate that all slot widgets are bound
    TArray<USHIEquipmentSlotWidget**> SlotWidgets = {
        &KaskSlot, &GoguslukZirhiSlot, &KalkanSlot,
        &Silah1Slot, &Silah2Slot, &KolyeSlot,
        &YuzukSlot, &KupeSlot, &EldivenSlot,
        &PantolonSlot, &AyakkabiSlot
    };

    for (int32 i = 0; i < SlotWidgets.Num(); i++)
    {
        if (*SlotWidgets[i] == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("Equipment Panel: Slot widget %d is not bound"), i);
        }
    }
}

void USHIEquipmentPanelWidget::RefreshEquipmentDisplay()
{
    if (!EquipmentComponent) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment Panel: Cannot refresh - no equipment component"));
        return;
    }

    // Update all equipment slots (11 slots)
    TArray<ESHIEquipmentSlot> AllSlots = {
        ESHIEquipmentSlot::Kask,
        ESHIEquipmentSlot::GoguslukZirhi,
        ESHIEquipmentSlot::Kalkan,
        ESHIEquipmentSlot::Silah1,
        ESHIEquipmentSlot::Silah2,
        ESHIEquipmentSlot::Kolye,
        ESHIEquipmentSlot::Yuzuk,
        ESHIEquipmentSlot::Kupe,
        ESHIEquipmentSlot::Eldiven,     // ⬅️ NEW
        ESHIEquipmentSlot::Pantolon,    // ⬅️ NEW
        ESHIEquipmentSlot::Ayakkabi     // ⬅️ NEW
    };

    for (ESHIEquipmentSlot SlotType : AllSlots)
    {
        USHIEquipmentSlotWidget* SlotWidget = GetSlotWidget(SlotType);
        if (SlotWidget)
        {
            FSHIEquipmentSlot EquippedItem = EquipmentComponent->GetEquippedItem(SlotType);
            SlotWidget->SetItem(EquippedItem.ItemData, EquippedItem.Quantity);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Equipment Panel: Equipment display refreshed"));
}

void USHIEquipmentPanelWidget::RefreshStatsDisplay()
{
    if (!StatsComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipment Panel: Cannot refresh stats - no stats component"));
        return;
    }

    // Update stats text widgets
    if (GucText)
    {
        float CurrentGuc = StatsComponent->GetCurrentGuc();
        float BaseGuc = StatsComponent->GetBaseStats().Guc;
        FString GucDisplayText = FString::Printf(TEXT("GÜÇ: %.0f"), CurrentGuc);
        
        // Show equipment bonus if any
        if (CurrentGuc != BaseGuc)
        {
            float Bonus = CurrentGuc - BaseGuc;
            GucDisplayText = FString::Printf(TEXT("GÜÇ: %.0f (+%.0f)"), CurrentGuc, Bonus);
        }
        
        GucText->SetText(FText::FromString(GucDisplayText));
    }

    if (CeviklikText)
    {
        float CurrentCeviklik = StatsComponent->GetCurrentCeviklik();
        float BaseCeviklik = StatsComponent->GetBaseStats().Ceviklik;
        FString CeviklikDisplayText = FString::Printf(TEXT("ÇEVİKLİK: %.0f"), CurrentCeviklik);
        
        if (CurrentCeviklik != BaseCeviklik)
        {
            float Bonus = CurrentCeviklik - BaseCeviklik;
            CeviklikDisplayText = FString::Printf(TEXT("ÇEVİKLİK: %.0f (+%.0f)"), CurrentCeviklik, Bonus);
        }
        
        CeviklikText->SetText(FText::FromString(CeviklikDisplayText));
    }

    if (ZekaText)
    {
        float CurrentZeka = StatsComponent->GetCurrentZeka();
        float BaseZeka = StatsComponent->GetBaseStats().Zeka;
        FString ZekaDisplayText = FString::Printf(TEXT("ZEKA: %.0f"), CurrentZeka);
        
        if (CurrentZeka != BaseZeka)
        {
            float Bonus = CurrentZeka - BaseZeka;
            ZekaDisplayText = FString::Printf(TEXT("ZEKA: %.0f (+%.0f)"), CurrentZeka, Bonus);
        }
        
        ZekaText->SetText(FText::FromString(ZekaDisplayText));
    }

    if (OdaklanmaText)
    {
        float CurrentOdaklanma = StatsComponent->GetCurrentOdaklanma();
        float BaseOdaklanma = StatsComponent->GetBaseStats().Odaklanma;
        FString OdaklanmaDisplayText = FString::Printf(TEXT("ODAKLANMA: %.0f"), CurrentOdaklanma);
        
        if (CurrentOdaklanma != BaseOdaklanma)
        {
            float Bonus = CurrentOdaklanma - BaseOdaklanma;
            OdaklanmaDisplayText = FString::Printf(TEXT("ODAKLANMA: %.0f (+%.0f)"), CurrentOdaklanma, Bonus);
        }
        
        OdaklanmaText->SetText(FText::FromString(OdaklanmaDisplayText));
    }

    if (DayaniklilikText)
    {
        float CurrentDayaniklilik = StatsComponent->GetCurrentDayaniklilik();
        float BaseDayaniklilik = StatsComponent->GetBaseStats().Dayaniklilik;
        FString DayaniklilikDisplayText = FString::Printf(TEXT("BÜNYE: %.0f"), CurrentDayaniklilik);
        
        if (CurrentDayaniklilik != BaseDayaniklilik)
        {
            float Bonus = CurrentDayaniklilik - BaseDayaniklilik;
            DayaniklilikDisplayText = FString::Printf(TEXT("BÜNYE: %.0f (+%.0f)"), CurrentDayaniklilik, Bonus);
        }
        
        DayaniklilikText->SetText(FText::FromString(DayaniklilikDisplayText));
    }

    if (SaglikText)
    {
        float MaxSaglik = StatsComponent->GetMaxSaglik();
        SaglikText->SetText(FText::FromString(FString::Printf(TEXT("SAĞLIK: %.0f"), MaxSaglik)));
    }

    UE_LOG(LogTemp, Log, TEXT("Equipment Panel: Stats display refreshed"));
}

void USHIEquipmentPanelWidget::OnEquipmentChanged(ESHIEquipmentSlot SlotType, USHIItemData* NewItem, USHIItemData* OldItem)
{
    UE_LOG(LogTemp, Warning, TEXT("Equipment Panel - Equipment Changed: Slot %d"), (int32)SlotType);
    
    // Update the specific slot
    USHIEquipmentSlotWidget* SlotWidget = GetSlotWidget(SlotType);
    if (SlotWidget)
    {
        SlotWidget->SetItem(NewItem, NewItem ? 1 : 0);
        UE_LOG(LogTemp, Log, TEXT("Updated slot widget for slot %d"), (int32)SlotType);
    }

    // Force refresh stats display
    RefreshStatsDisplay();
    
    // Debug message to screen
    if (GEngine)
    {
        FString ChangeText = FString::Printf(TEXT("Equipment Updated: Slot %d"), (int32)SlotType);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, ChangeText);
    }
}

void USHIEquipmentPanelWidget::OnStatsRecalculated()
{
    UE_LOG(LogTemp, Warning, TEXT("Equipment Panel - Stats Recalculated Event"));
    RefreshStatsDisplay();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Stats UI Updated!"));
    }
}

USHIEquipmentSlotWidget* USHIEquipmentPanelWidget::GetSlotWidget(ESHIEquipmentSlot SlotType)
{
    switch (SlotType)
    {
    case ESHIEquipmentSlot::Kask: return KaskSlot;
    case ESHIEquipmentSlot::GoguslukZirhi: return GoguslukZirhiSlot;
    case ESHIEquipmentSlot::Kalkan: return KalkanSlot;
    case ESHIEquipmentSlot::Silah1: return Silah1Slot;
    case ESHIEquipmentSlot::Silah2: return Silah2Slot;
    case ESHIEquipmentSlot::Kolye: return KolyeSlot;
    case ESHIEquipmentSlot::Yuzuk: return YuzukSlot;
    case ESHIEquipmentSlot::Kupe: return KupeSlot;
    case ESHIEquipmentSlot::Eldiven: return EldivenSlot;        // ⬅️ NEW
    case ESHIEquipmentSlot::Pantolon: return PantolonSlot;      // ⬅️ NEW
    case ESHIEquipmentSlot::Ayakkabi: return AyakkabiSlot;      // ⬅️ NEW
    default: 
        UE_LOG(LogTemp, Warning, TEXT("Equipment Panel: Unknown slot type %d"), (int32)SlotType);
        return nullptr;
    }
}