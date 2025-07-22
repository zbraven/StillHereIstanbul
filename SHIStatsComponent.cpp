#include "SHIStatsComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Data/SHIItemData.h"

void FSHICharacterStats::ApplyModifiers(const TArray<FSHIStatModifier>& Modifiers)
{
    for (const FSHIStatModifier& Modifier : Modifiers)
    {
        if (Modifier.StatName == "Guc")
        {
            Guc += Modifier.BonusAmount;
        }
        else if (Modifier.StatName == "Ceviklik")
        {
            Ceviklik += Modifier.BonusAmount;
        }
        else if (Modifier.StatName == "Zeka")
        {
            Zeka += Modifier.BonusAmount;
        }
        else if (Modifier.StatName == "Odaklanma")
        {
            Odaklanma += Modifier.BonusAmount;
        }
        else if (Modifier.StatName == "Dayaniklilik")
        {
            Dayaniklilik += Modifier.BonusAmount;
        }
    }
}

USHIStatsComponent::USHIStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    // Initialize base stats
    BaseStats = FSHICharacterStats();
    CurrentStats = BaseStats;
    EquipmentBonuses = FSHICharacterStats();
    TemporaryModifiers = FSHICharacterStats();
}

void USHIStatsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize current stats
    RecalculateCurrentStats();
    
    UE_LOG(LogTemp, Warning, TEXT("SHI Stats initialized - Güç: %f, Zeka: %f"), 
           GetCurrentGuc(), GetCurrentZeka());
}

void USHIStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USHIStatsComponent, BaseStats);
    DOREPLIFETIME(USHIStatsComponent, CurrentStats);
}

void USHIStatsComponent::Server_ModifyBaseStat_Implementation(FName StatName, float Amount)
{
    float OldValue = GetStatByName(BaseStats, StatName);
    float NewValue = FMath::Max(1.0f, OldValue + Amount);
    
    SetStatByName(BaseStats, StatName, NewValue);
    
    // Recalculate current stats
    RecalculateCurrentStats();
    
    // Broadcast change
    OnStatChanged.Broadcast(StatName, OldValue, NewValue);
    
    UE_LOG(LogTemp, Log, TEXT("Base stat %s modified: %f -> %f"), 
           *StatName.ToString(), OldValue, NewValue);
}

void USHIStatsComponent::ApplyEquipmentBonuses(const TArray<FSHIStatModifier>& EquipmentModifiers)
{
    // Clear current equipment bonuses
    EquipmentBonuses = FSHICharacterStats();
    
    // Apply new equipment modifiers
    EquipmentBonuses.ApplyModifiers(EquipmentModifiers);
    
    // Recalculate current stats
    RecalculateCurrentStats();
    
    UE_LOG(LogTemp, Log, TEXT("Equipment bonuses applied: Güç +%f, Zeka +%f, Çeviklik +%f"), 
           EquipmentBonuses.Guc, EquipmentBonuses.Zeka, EquipmentBonuses.Ceviklik);
}

void USHIStatsComponent::ClearEquipmentBonuses()
{
    EquipmentBonuses = FSHICharacterStats();
    RecalculateCurrentStats();
    
    UE_LOG(LogTemp, Log, TEXT("Equipment bonuses cleared"));
}

void USHIStatsComponent::ApplyTemporaryModifier(FName StatName, float Amount, float Duration)
{
    // Remove existing temporary modifier if exists
    if (TempModifierTimers.Contains(StatName))
    {
        GetWorld()->GetTimerManager().ClearTimer(TempModifierTimers[StatName]);
    }
    
    TempModifierValues.Add(StatName, Amount);
    RecalculateCurrentStats();
    
    // Set timer to remove modifier
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, StatName]()
    {
        RemoveTemporaryModifier(StatName);
    }, Duration, false);
    
    TempModifierTimers.Add(StatName, TimerHandle);
    
    UE_LOG(LogTemp, Log, TEXT("Temporary modifier applied: %s +%f for %f seconds"), 
           *StatName.ToString(), Amount, Duration);
}

void USHIStatsComponent::RemoveTemporaryModifier(FName StatName)
{
    TempModifierValues.Remove(StatName);
    TempModifierTimers.Remove(StatName);
    RecalculateCurrentStats();
    
    UE_LOG(LogTemp, Log, TEXT("Temporary modifier removed: %s"), *StatName.ToString());
}

float USHIStatsComponent::GetMaxSaglik() const
{
    return 100.0f + (GetCurrentDayaniklilik() * 15.0f);
}

float USHIStatsComponent::GetMaxEnerji() const
{
    return 50.0f + (GetCurrentZeka() * 8.0f) + (GetCurrentOdaklanma() * 5.0f);
}

float USHIStatsComponent::GetHasarBonusu() const
{
    return (GetCurrentGuc() * 0.8f) + (GetCurrentCeviklik() * 0.3f);
}

float USHIStatsComponent::GetSavunma() const
{
    return (GetCurrentDayaniklilik() * 0.5f) + (GetCurrentCeviklik() * 0.2f);
}

int32 USHIStatsComponent::GetStatThresholdLevel(FName StatName) const
{
    float StatValue = GetStatByName(CurrentStats, StatName);
    return FMath::FloorToInt(StatValue / 50.0f);
}

TArray<FText> USHIStatsComponent::GetActiveThresholdBonuses() const
{
    TArray<FText> ActiveBonuses;
    
    // Check Güç thresholds
    int32 GucLevel = GetStatThresholdLevel(FName("Guc"));
    if (GucLevel >= 1) ActiveBonuses.Add(FText::FromString(TEXT("Güç 50+: Ağır saldırı +15% stamina hasarı")));
    if (GucLevel >= 2) ActiveBonuses.Add(FText::FromString(TEXT("Güç 100+: Ağır saldırı +20% hasar")));
    if (GucLevel >= 3) ActiveBonuses.Add(FText::FromString(TEXT("Güç 150+: Hafif saldırı %10 yavaşlatma")));
    
    // Check other stats...
    int32 CeviklikLevel = GetStatThresholdLevel(FName("Ceviklik"));
    if (CeviklikLevel >= 1) ActiveBonuses.Add(FText::FromString(TEXT("Çeviklik 50+: Arkadan saldırı +15% hasar")));
    if (CeviklikLevel >= 2) ActiveBonuses.Add(FText::FromString(TEXT("Çeviklik 100+: Kaçınma sonrası +20% hasar")));
    
    return ActiveBonuses;
}

void USHIStatsComponent::OnRep_CurrentStats()
{
    // Broadcast that stats have been updated
    OnStatsRecalculated.Broadcast();
    UE_LOG(LogTemp, VeryVerbose, TEXT("Current stats replicated"));
}

void USHIStatsComponent::RecalculateCurrentStats()
{
    // Start with base stats
    CurrentStats = BaseStats;
    
    // Add equipment bonuses
    CurrentStats = CurrentStats + EquipmentBonuses;
    
    // Add temporary modifiers
    for (const auto& TempModifier : TempModifierValues)
    {
        FName StatName = TempModifier.Key;
        float Amount = TempModifier.Value;
        
        float CurrentValue = GetStatByName(CurrentStats, StatName);
        SetStatByName(CurrentStats, StatName, FMath::Max(1.0f, CurrentValue + Amount));
    }
    
    // Ensure minimum stat values
    CurrentStats.Guc = FMath::Max(1.0f, CurrentStats.Guc);
    CurrentStats.Ceviklik = FMath::Max(1.0f, CurrentStats.Ceviklik);
    CurrentStats.Zeka = FMath::Max(1.0f, CurrentStats.Zeka);
    CurrentStats.Odaklanma = FMath::Max(1.0f, CurrentStats.Odaklanma);
    CurrentStats.Dayaniklilik = FMath::Max(1.0f, CurrentStats.Dayaniklilik);
    
    // Force replication if we're on the server (FIXED)
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        OnRep_CurrentStats();
    }
    
    // Broadcast recalculation event
    OnStatsRecalculated.Broadcast();
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Stats recalculated - Total Güç: %f (Base: %f + Equipment: %f)"), 
           CurrentStats.Guc, BaseStats.Guc, EquipmentBonuses.Guc);
}

float USHIStatsComponent::GetStatByName(const FSHICharacterStats& Stats, FName StatName) const
{
    if (StatName == "Guc") return Stats.Guc;
    if (StatName == "Ceviklik") return Stats.Ceviklik;
    if (StatName == "Zeka") return Stats.Zeka;
    if (StatName == "Odaklanma") return Stats.Odaklanma;
    if (StatName == "Dayaniklilik") return Stats.Dayaniklilik;
    return 0.0f;
}

void USHIStatsComponent::SetStatByName(FSHICharacterStats& Stats, FName StatName, float Value)
{
    if (StatName == "Guc") Stats.Guc = Value;
    else if (StatName == "Ceviklik") Stats.Ceviklik = Value;
    else if (StatName == "Zeka") Stats.Zeka = Value;
    else if (StatName == "Odaklanma") Stats.Odaklanma = Value;
    else if (StatName == "Dayaniklilik") Stats.Dayaniklilik = Value;
}

void USHIStatsComponent::DebugPrintStats() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER STATS DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("BASE STATS:"));
    UE_LOG(LogTemp, Warning, TEXT("  Güç: %f"), BaseStats.Guc);
    UE_LOG(LogTemp, Warning, TEXT("  Çeviklik: %f"), BaseStats.Ceviklik);
    UE_LOG(LogTemp, Warning, TEXT("  Zeka: %f"), BaseStats.Zeka);
    UE_LOG(LogTemp, Warning, TEXT("  Odaklanma: %f"), BaseStats.Odaklanma);
    UE_LOG(LogTemp, Warning, TEXT("  Dayanıklılık: %f"), BaseStats.Dayaniklilik);
    
    UE_LOG(LogTemp, Warning, TEXT("EQUIPMENT BONUSES:"));
    UE_LOG(LogTemp, Warning, TEXT("  Güç: +%f"), EquipmentBonuses.Guc);
    UE_LOG(LogTemp, Warning, TEXT("  Çeviklik: +%f"), EquipmentBonuses.Ceviklik);
    UE_LOG(LogTemp, Warning, TEXT("  Zeka: +%f"), EquipmentBonuses.Zeka);
    UE_LOG(LogTemp, Warning, TEXT("  Odaklanma: +%f"), EquipmentBonuses.Odaklanma);
    UE_LOG(LogTemp, Warning, TEXT("  Dayanıklılık: +%f"), EquipmentBonuses.Dayaniklilik);
    
    UE_LOG(LogTemp, Warning, TEXT("CURRENT STATS (Total):"));
    UE_LOG(LogTemp, Warning, TEXT("  Güç: %f"), CurrentStats.Guc);
    UE_LOG(LogTemp, Warning, TEXT("  Çeviklik: %f"), CurrentStats.Ceviklik);
    UE_LOG(LogTemp, Warning, TEXT("  Zeka: %f"), CurrentStats.Zeka);
    UE_LOG(LogTemp, Warning, TEXT("  Odaklanma: %f"), CurrentStats.Odaklanma);
    UE_LOG(LogTemp, Warning, TEXT("  Dayanıklılık: %f"), CurrentStats.Dayaniklilik);
    
    UE_LOG(LogTemp, Warning, TEXT("DERIVED STATS:"));
    UE_LOG(LogTemp, Warning, TEXT("  Max Sağlık: %f"), GetMaxSaglik());
    UE_LOG(LogTemp, Warning, TEXT("  Max Enerji: %f"), GetMaxEnerji());
    UE_LOG(LogTemp, Warning, TEXT("  Hasar Bonusu: %f"), GetHasarBonusu());
    UE_LOG(LogTemp, Warning, TEXT("  Savunma: %f"), GetSavunma());
    UE_LOG(LogTemp, Warning, TEXT("=== END STATS DEBUG ==="));
}

FString USHIStatsComponent::GetStatsDebugString() const
{
    return FString::Printf(TEXT("Güç:%0.f Çev:%0.f Zeka:%0.f Odak:%0.f Bünye:%0.f (Sağlık:%0.f)"), 
                          GetCurrentGuc(), GetCurrentCeviklik(), GetCurrentZeka(), 
                          GetCurrentOdaklanma(), GetCurrentDayaniklilik(), GetMaxSaglik());
}