#include "SHICharacterStatsWidget.h"
#include "Components/TextBlock.h"
#include "Player/SHICharacter.h"
#include "Components/SHIStatsComponent.h"

void USHICharacterStatsWidget::NativeConstruct()
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
    
    UE_LOG(LogTemp, Log, TEXT("Character Stats Widget constructed"));
}

void USHICharacterStatsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // Update stats every frame (can be optimized later)
    UpdateAllStats();
}

void USHICharacterStatsWidget::UpdateAllStats()
{
    if (!OwnerCharacter)
        return;
    
    USHIStatsComponent* StatsComp = OwnerCharacter->GetStatsComponent();
    if (!StatsComp)
        return;
    
    // Update all text blocks if they exist
    if (GucValue)
        GucValue->SetText(GetGucText());
    
    if (CeviklikValue)
        CeviklikValue->SetText(GetCeviklikText());
    
    if (ZekaValue)
        ZekaValue->SetText(GetZekaText());
    
    if (OdaklanmaValue)
        OdaklanmaValue->SetText(GetOdaklanmaText());
    
    if (DayaniklilikValue)
        DayaniklilikValue->SetText(GetDayaniklilikText());
    
    if (SaglikValue)
        SaglikValue->SetText(GetSaglikText());
    
    if (EnerjiValue)
        EnerjiValue->SetText(GetEnerjiText());
    
    if (HasarValue)
        HasarValue->SetText(GetHasarText());
    
    if (SavunmaValue)
        SavunmaValue->SetText(GetSavunmaText());
}

void USHICharacterStatsWidget::SetOwnerCharacter(ASHICharacter* Character)
{
    OwnerCharacter = Character;
    UE_LOG(LogTemp, Log, TEXT("Stats widget owner set: %s"), 
           Character ? TEXT("Valid") : TEXT("Null"));
}

FText USHICharacterStatsWidget::GetGucText() const
{
    if (OwnerCharacter && OwnerCharacter->GetStatsComponent())
    {
        float Value = OwnerCharacter->GetStatsComponent()->GetCurrentGuc();
        return FText::FromString(FString::Printf(TEXT("%.0f"), Value));
    }
    return FText::FromString(TEXT("0"));
}

FText USHICharacterStatsWidget::GetCeviklikText() const
{
    if (OwnerCharacter && OwnerCharacter->GetStatsComponent())
    {
        float Value = OwnerCharacter->GetStatsComponent()->GetCurrentCeviklik();
        return FText::FromString(FString::Printf(TEXT("%.0f"), Value));
    }
    return FText::FromString(TEXT("0"));
}

FText USHICharacterStatsWidget::GetZekaText() const
{
    if (OwnerCharacter && OwnerCharacter->GetStatsComponent())
    {
        float Value = OwnerCharacter->GetStatsComponent()->GetCurrentZeka();
        return FText::FromString(FString::Printf(TEXT("%.0f"), Value));
    }
    return FText::FromString(TEXT("0"));
}

FText USHICharacterStatsWidget::GetOdaklanmaText() const
{
    if (OwnerCharacter && OwnerCharacter->GetStatsComponent())
    {
        float Value = OwnerCharacter->GetStatsComponent()->GetCurrentOdaklanma();
        return FText::FromString(FString::Printf(TEXT("%.0f"), Value));
    }
    return FText::FromString(TEXT("0"));
}

FText USHICharacterStatsWidget::GetDayaniklilikText() const
{
    if (OwnerCharacter && OwnerCharacter->GetStatsComponent())
    {
        float Value = OwnerCharacter->GetStatsComponent()->GetCurrentDayaniklilik();
        return FText::FromString(FString::Printf(TEXT("%.0f"), Value));
    }
    return FText::FromString(TEXT("0"));
}

FText USHICharacterStatsWidget::GetSaglikText() const
{
    if (OwnerCharacter && OwnerCharacter->GetStatsComponent())
    {
        float Value = OwnerCharacter->GetStatsComponent()->GetMaxSaglik();
        return FText::FromString(FString::Printf(TEXT("%.0f"), Value));
    }
    return FText::FromString(TEXT("0"));
}

FText USHICharacterStatsWidget::GetEnerjiText() const
{
    if (OwnerCharacter && OwnerCharacter->GetStatsComponent())
    {
        float Value = OwnerCharacter->GetStatsComponent()->GetMaxEnerji();
        return FText::FromString(FString::Printf(TEXT("%.0f"), Value));
    }
    return FText::FromString(TEXT("0"));
}

FText USHICharacterStatsWidget::GetHasarText() const
{
    if (OwnerCharacter && OwnerCharacter->GetStatsComponent())
    {
        float Value = OwnerCharacter->GetStatsComponent()->GetHasarBonusu();
        return FText::FromString(FString::Printf(TEXT("%.1f"), Value));
    }
    return FText::FromString(TEXT("0"));
}

FText USHICharacterStatsWidget::GetSavunmaText() const
{
    if (OwnerCharacter && OwnerCharacter->GetStatsComponent())
    {
        float Value = OwnerCharacter->GetStatsComponent()->GetSavunma();
        return FText::FromString(FString::Printf(TEXT("%.1f"), Value));
    }
    return FText::FromString(TEXT("0"));
}