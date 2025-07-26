// SHIAbilityComponent.cpp
#include "Components/SHIAbilityComponent.h"
#include "Player/SHICharacter.h"
#include "Components/SHIEquipmentComponent.h"
#include "Engine/Engine.h"

USHIAbilityComponent::USHIAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
    
    // Initialize ability state
    AbilityState = FSHIAbilityState();
}

void USHIAbilityComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get character reference
    OwnerCharacter = Cast<ASHICharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("SHIAbilityComponent: Owner is not a valid SHICharacter!"));
        return;
    }
    
    // Initialize ability database with Turkish-themed abilities
    InitializeAbilityDatabase();
    
    UE_LOG(LogTemp, Log, TEXT("SHI Ability Component initialized for: %s"), 
           *OwnerCharacter->GetName());
}

void USHIAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update ability cooldowns
    UpdateCooldowns(DeltaTime);
}

void USHIAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USHIAbilityComponent, AbilityState);
}

void USHIAbilityComponent::UseQAbility()
{
    if (CanUseAbility(AbilityState.QAbility.AbilityType))
    {
        Server_UseAbility(AbilityState.QAbility.AbilityType);
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
                TEXT("Q Yeteneği henüz hazır değil!"));
        }
    }
}

void USHIAbilityComponent::UseRAbility()
{
    if (CanUseAbility(AbilityState.RAbility.AbilityType))
    {
        Server_UseAbility(AbilityState.RAbility.AbilityType);
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
                TEXT("R Yeteneği henüz hazır değil!"));
        }
    }
}

void USHIAbilityComponent::UseFAbility()
{
    if (CanUseAbility(AbilityState.FAbility.AbilityType))
    {
        Server_UseAbility(AbilityState.FAbility.AbilityType);
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
                TEXT("F Yeteneği henüz hazır değil!"));
        }
    }
}

void USHIAbilityComponent::Server_UseAbility_Implementation(ESHIWeaponAbilityType AbilityType)
{
    UseAbilityInternal(AbilityType);
}

void USHIAbilityComponent::UseAbilityInternal(ESHIWeaponAbilityType AbilityType)
{
    if (!CanUseAbility(AbilityType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot use ability: %d - on cooldown or invalid"), (int32)AbilityType);
        return;
    }

    // Find ability data
    FSHIAbilityData* AbilityData = nullptr;
    if (AbilityState.QAbility.AbilityType == AbilityType)
        AbilityData = &AbilityState.QAbility;
    else if (AbilityState.RAbility.AbilityType == AbilityType)
        AbilityData = &AbilityState.RAbility;
    else if (AbilityState.FAbility.AbilityType == AbilityType)
        AbilityData = &AbilityState.FAbility;

    if (!AbilityData)
    {
        UE_LOG(LogTemp, Error, TEXT("Ability data not found for type: %d"), (int32)AbilityType);
        return;
    }

    // Execute ability logic based on type
    float FinalDamage = AbilityData->BaseDamage;
    FString AbilityMessage;

    switch (AbilityType)
    {
        case ESHIWeaponAbilityType::KilicSlash:
            AbilityMessage = FString::Printf(TEXT("🗡️ Kılıç Doğrama! Hasar: %.0f"), FinalDamage);
            break;
        case ESHIWeaponAbilityType::KilicThrust:
            AbilityMessage = FString::Printf(TEXT("⚔️ Kılıç Saplanması! Hasar: %.0f"), FinalDamage);
            break;
        case ESHIWeaponAbilityType::KilicGuard:
            AbilityMessage = FString::Printf(TEXT("🛡️ Kılıç Savunması! Zırh: +%.0f"), FinalDamage * 0.5f);
            break;
        case ESHIWeaponAbilityType::BaltaChop:
            AbilityMessage = FString::Printf(TEXT("🪓 Balta Doğrama! Hasar: %.0f"), FinalDamage);
            break;
        case ESHIWeaponAbilityType::BaltaThrow:
            AbilityMessage = FString::Printf(TEXT("🎯 Balta Fırlatma! Hasar: %.0f"), FinalDamage);
            break;
        case ESHIWeaponAbilityType::BaltaWhirlwind:
            AbilityMessage = FString::Printf(TEXT("🌪️ Balta Kasırgası! Hasar: %.0f"), FinalDamage * 1.5f);
            break;
        case ESHIWeaponAbilityType::MecLunge:
            AbilityMessage = FString::Printf(TEXT("🤺 Meç Hamlesi! Hasar: %.0f"), FinalDamage);
            break;
        case ESHIWeaponAbilityType::MecParry:
            AbilityMessage = FString::Printf(TEXT("⚡ Meç Savması! Karşı Saldırı Hazır"), FinalDamage);
            break;
        case ESHIWeaponAbilityType::MecRiposte:
            AbilityMessage = FString::Printf(TEXT("💥 Meç Geri Saldırısı! Hasar: %.0f"), FinalDamage * 1.2f);
            break;
        case ESHIWeaponAbilityType::AtesFireball:
            AbilityMessage = FString::Printf(TEXT("🔥 Ateş Topu! Hasar: %.0f"), FinalDamage);
            break;
        case ESHIWeaponAbilityType::AtesBurn:
            AbilityMessage = FString::Printf(TEXT("🔥 Yakıcı Alan! DoT Hasar: %.0f"), FinalDamage * 0.3f);
            break;
        case ESHIWeaponAbilityType::AtesIgnite:
            AbilityMessage = FString::Printf(TEXT("🔥 Tutuşturma! Hasar: %.0f"), FinalDamage * 0.8f);
            break;
        default:
            AbilityMessage = FString::Printf(TEXT("Bilinmeyen yetenek kullanıldı"));
            break;
    }

    // Show ability effect
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, AbilityMessage);
    }

    // Start cooldown
    StartAbilityCooldown(AbilityType, AbilityData->CooldownTime);

    // Broadcast ability used event
    OnAbilityUsed.Broadcast(AbilityType, FinalDamage);

    UE_LOG(LogTemp, Log, TEXT("Ability used: %s"), *AbilityMessage);
}

void USHIAbilityComponent::UpdateAbilitiesForWeapon(USHIItemData* WeaponData)
{
    if (!WeaponData)
    {
        // No weapon equipped - clear abilities
        AbilityState.QAbility = FSHIAbilityData();
        AbilityState.RAbility = FSHIAbilityData();
        AbilityState.FAbility = FSHIAbilityData();
        
        UE_LOG(LogTemp, Log, TEXT("No weapon equipped - abilities cleared"));
    }
    else
    {
        // NEW: Get abilities from weapon data instead of hard-coded name matching
        TArray<FSHIAbilityData> WeaponAbilities = GetAbilitiesFromWeaponData(WeaponData);
        
        // Clear current abilities
        AbilityState.QAbility = FSHIAbilityData();
        AbilityState.RAbility = FSHIAbilityData();
        AbilityState.FAbility = FSHIAbilityData();
        
        // Assign abilities to Q, R, F slots
        if (WeaponAbilities.Num() >= 1)
            AbilityState.QAbility = WeaponAbilities[0];
        if (WeaponAbilities.Num() >= 2)
            AbilityState.RAbility = WeaponAbilities[1];
        if (WeaponAbilities.Num() >= 3)
            AbilityState.FAbility = WeaponAbilities[2];
        
        UE_LOG(LogTemp, Log, TEXT("Abilities updated for weapon: %s (%d abilities loaded from data)"), 
               *WeaponData->ItemName.ToString(), WeaponAbilities.Num());
        
        // Show detailed ability info
        if (WeaponAbilities.Num() > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Q: %s | R: %s | F: %s"), 
                   *AbilityState.QAbility.AbilityName.ToString(),
                   *AbilityState.RAbility.AbilityName.ToString(),
                   *AbilityState.FAbility.AbilityName.ToString());
        }
    }
    
    // Broadcast abilities changed
    OnAbilitiesChanged.Broadcast(AbilityState);
}

bool USHIAbilityComponent::IsAbilityOnCooldown(ESHIWeaponAbilityType AbilityType) const
{
    if (AbilityState.QAbility.AbilityType == AbilityType)
        return AbilityState.QAbilityCooldown.bIsOnCooldown;
    else if (AbilityState.RAbility.AbilityType == AbilityType)
        return AbilityState.RAbilityCooldown.bIsOnCooldown;
    else if (AbilityState.FAbility.AbilityType == AbilityType)
        return AbilityState.FAbilityCooldown.bIsOnCooldown;
    
    return false;
}

float USHIAbilityComponent::GetAbilityCooldownRemaining(ESHIWeaponAbilityType AbilityType) const
{
    if (AbilityState.QAbility.AbilityType == AbilityType)
        return AbilityState.QAbilityCooldown.RemainingCooldown;
    else if (AbilityState.RAbility.AbilityType == AbilityType)
        return AbilityState.RAbilityCooldown.RemainingCooldown;
    else if (AbilityState.FAbility.AbilityType == AbilityType)
        return AbilityState.FAbilityCooldown.RemainingCooldown;
    
    return 0.0f;
}

bool USHIAbilityComponent::CanUseAbility(ESHIWeaponAbilityType AbilityType) const
{
    return AbilityType != ESHIWeaponAbilityType::None && !IsAbilityOnCooldown(AbilityType);
}

void USHIAbilityComponent::DebugPrintAbilities() const
{
    UE_LOG(LogTemp, Log, TEXT("=== CURRENT ABILITIES ==="));
    UE_LOG(LogTemp, Log, TEXT("Q: %s (Cooldown: %.1fs)"), 
           *AbilityState.QAbility.AbilityName.ToString(),
           AbilityState.QAbilityCooldown.RemainingCooldown);
    UE_LOG(LogTemp, Log, TEXT("R: %s (Cooldown: %.1fs)"), 
           *AbilityState.RAbility.AbilityName.ToString(),
           AbilityState.RAbilityCooldown.RemainingCooldown);
    UE_LOG(LogTemp, Log, TEXT("F: %s (Cooldown: %.1fs)"), 
           *AbilityState.FAbility.AbilityName.ToString(),
           AbilityState.FAbilityCooldown.RemainingCooldown);
    UE_LOG(LogTemp, Log, TEXT("=== END ABILITIES ==="));
}

void USHIAbilityComponent::OnRep_AbilityState()
{
    // Notify UI of ability state changes
    OnAbilitiesChanged.Broadcast(AbilityState);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Ability state replicated"));
}

void USHIAbilityComponent::StartAbilityCooldown(ESHIWeaponAbilityType AbilityType, float CooldownTime)
{
    FSHIActiveAbility* TargetCooldown = nullptr;
    
    if (AbilityState.QAbility.AbilityType == AbilityType)
        TargetCooldown = &AbilityState.QAbilityCooldown;
    else if (AbilityState.RAbility.AbilityType == AbilityType)
        TargetCooldown = &AbilityState.RAbilityCooldown;
    else if (AbilityState.FAbility.AbilityType == AbilityType)
        TargetCooldown = &AbilityState.FAbilityCooldown;
    
    if (TargetCooldown)
    {
        TargetCooldown->AbilityType = AbilityType;
        TargetCooldown->RemainingCooldown = CooldownTime;
        TargetCooldown->bIsOnCooldown = true;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Started cooldown for ability %d: %.1fs"), 
               (int32)AbilityType, CooldownTime);
    }
}

void USHIAbilityComponent::UpdateCooldowns(float DeltaTime)
{
    // Update Q ability cooldown
    if (AbilityState.QAbilityCooldown.bIsOnCooldown)
    {
        AbilityState.QAbilityCooldown.RemainingCooldown -= DeltaTime;
        if (AbilityState.QAbilityCooldown.RemainingCooldown <= 0.0f)
        {
            AbilityState.QAbilityCooldown.bIsOnCooldown = false;
            AbilityState.QAbilityCooldown.RemainingCooldown = 0.0f;
            OnAbilityCooldownUpdated.Broadcast(AbilityState.QAbility.AbilityType, 0.0f);
        }
        else
        {
            OnAbilityCooldownUpdated.Broadcast(AbilityState.QAbility.AbilityType, 
                                             AbilityState.QAbilityCooldown.RemainingCooldown);
        }
    }
    
    // Update R ability cooldown
    if (AbilityState.RAbilityCooldown.bIsOnCooldown)
    {
        AbilityState.RAbilityCooldown.RemainingCooldown -= DeltaTime;
        if (AbilityState.RAbilityCooldown.RemainingCooldown <= 0.0f)
        {
            AbilityState.RAbilityCooldown.bIsOnCooldown = false;
            AbilityState.RAbilityCooldown.RemainingCooldown = 0.0f;
            OnAbilityCooldownUpdated.Broadcast(AbilityState.RAbility.AbilityType, 0.0f);
        }
        else
        {
            OnAbilityCooldownUpdated.Broadcast(AbilityState.RAbility.AbilityType, 
                                             AbilityState.RAbilityCooldown.RemainingCooldown);
        }
    }
    
    // Update F ability cooldown
    if (AbilityState.FAbilityCooldown.bIsOnCooldown)
    {
        AbilityState.FAbilityCooldown.RemainingCooldown -= DeltaTime;
        if (AbilityState.FAbilityCooldown.RemainingCooldown <= 0.0f)
        {
            AbilityState.FAbilityCooldown.bIsOnCooldown = false;
            AbilityState.FAbilityCooldown.RemainingCooldown = 0.0f;
            OnAbilityCooldownUpdated.Broadcast(AbilityState.FAbility.AbilityType, 0.0f);
        }
        else
        {
            OnAbilityCooldownUpdated.Broadcast(AbilityState.FAbility.AbilityType, 
                                             AbilityState.FAbilityCooldown.RemainingCooldown);
        }
    }
}

TArray<FSHIAbilityData> USHIAbilityComponent::GetAbilitiesForWeaponType(const FString& WeaponTypeName) const
{
    // This function is now DEPRECATED - we'll use GetAbilitiesFromWeaponData instead
    // But kept for backward compatibility
    UE_LOG(LogTemp, Warning, TEXT("Using deprecated GetAbilitiesForWeaponType - should use GetAbilitiesFromWeaponData"));
    
    TArray<FSHIAbilityData> WeaponAbilities;
    
    // Fallback to hard-coded system if needed
    if (WeaponTypeName.Contains(TEXT("Kılıç")) || WeaponTypeName.Contains(TEXT("Sword")))
    {
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::KilicSlash));
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::KilicThrust));
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::KilicGuard));
    }
    // Balta abilities
    else if (WeaponTypeName.Contains(TEXT("Balta")) || WeaponTypeName.Contains(TEXT("Axe")))
    {
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::BaltaChop));
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::BaltaThrow));
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::BaltaWhirlwind));
    }
    // Meç abilities
    else if (WeaponTypeName.Contains(TEXT("Meç")) || WeaponTypeName.Contains(TEXT("Rapier")))
    {
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::MecLunge));
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::MecParry));
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::MecRiposte));
    }
    // Ateş Asası abilities
    else if (WeaponTypeName.Contains(TEXT("Ateş")) || WeaponTypeName.Contains(TEXT("Fire")) || WeaponTypeName.Contains(TEXT("Asa")))
    {
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::AtesFireball));
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::AtesBurn));
        WeaponAbilities.Add(CreateAbilityData(ESHIWeaponAbilityType::AtesIgnite));
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Found %d abilities for weapon type: %s"), 
           WeaponAbilities.Num(), *WeaponTypeName);
    
    return WeaponAbilities;
}

TArray<FSHIAbilityData> USHIAbilityComponent::GetAbilitiesFromWeaponData(USHIItemData* WeaponData) const
{
    TArray<FSHIAbilityData> AbilityDataArray;
    
    if (!WeaponData)
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponData is null - no abilities to load"));
        return AbilityDataArray;
    }

    // Check if weapon has abilities defined
    if (!WeaponData->HasAbilities())
    {
        UE_LOG(LogTemp, Log, TEXT("Weapon %s has no abilities defined - using fallback"), 
               *WeaponData->ItemName.ToString());
        
        // Fallback to hard-coded system
        return GetAbilitiesForWeaponType(WeaponData->ItemName.ToString());
    }

    // Convert FSHIWeaponAbility to FSHIAbilityData
    TArray<FSHIWeaponAbility> WeaponAbilities = WeaponData->GetWeaponAbilities();
    for (const FSHIWeaponAbility& WeaponAbility : WeaponAbilities)
    {
        FSHIAbilityData AbilityData;
        AbilityData.AbilityType = WeaponAbility.AbilityType;
        AbilityData.AbilityName = WeaponAbility.AbilityName;
        AbilityData.AbilityDescription = WeaponAbility.AbilityDescription;
        AbilityData.CooldownTime = WeaponAbility.CooldownTime;
        AbilityData.BaseDamage = WeaponAbility.BaseDamage;
        AbilityData.Range = WeaponAbility.Range;
        AbilityData.AbilityIcon = WeaponAbility.AbilityIcon;
        
        AbilityDataArray.Add(AbilityData);
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded %d abilities from weapon data: %s"), 
           AbilityDataArray.Num(), *WeaponData->ItemName.ToString());
    
    return AbilityDataArray;
}

FSHIAbilityData USHIAbilityComponent::CreateAbilityData(ESHIWeaponAbilityType AbilityType) const
{
    if (AbilityDatabase.Contains(AbilityType))
    {
        return AbilityDatabase[AbilityType];
    }
    
    // Fallback for unknown abilities
    FSHIAbilityData DefaultAbility;
    DefaultAbility.AbilityType = AbilityType;
    DefaultAbility.AbilityName = FText::FromString("Bilinmeyen Yetenek");
    DefaultAbility.AbilityDescription = FText::FromString("Bu yetenek henüz tanımlanmamış.");
    return DefaultAbility;
}

void USHIAbilityComponent::InitializeAbilityDatabase()
{
    AbilityDatabase.Empty();
    
    // Kılıç abilities - Turkish themed
    FSHIAbilityData KilicSlash;
    KilicSlash.AbilityType = ESHIWeaponAbilityType::KilicSlash;
    KilicSlash.AbilityName = FText::FromString("Kılıç Doğrama");
    KilicSlash.AbilityDescription = FText::FromString("Hızlı ve keskin bir doğrama saldırısı yapar.");
    KilicSlash.CooldownTime = 3.0f;
    KilicSlash.BaseDamage = 75.0f;
    KilicSlash.Range = 200.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::KilicSlash, KilicSlash);
    
    FSHIAbilityData KilicThrust;
    KilicThrust.AbilityType = ESHIWeaponAbilityType::KilicThrust;
    KilicThrust.AbilityName = FText::FromString("Kılıç Saplanması");
    KilicThrust.AbilityDescription = FText::FromString("Düşmanı delerek yüksek hasar verir.");
    KilicThrust.CooldownTime = 5.0f;
    KilicThrust.BaseDamage = 120.0f;
    KilicThrust.Range = 250.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::KilicThrust, KilicThrust);
    
    FSHIAbilityData KilicGuard;
    KilicGuard.AbilityType = ESHIWeaponAbilityType::KilicGuard;
    KilicGuard.AbilityName = FText::FromString("Kılıç Savunması");
    KilicGuard.AbilityDescription = FText::FromString("Savunma duruşu alır ve gelen hasarı azaltır.");
    KilicGuard.CooldownTime = 8.0f;
    KilicGuard.BaseDamage = 0.0f; // Defensive ability
    KilicGuard.Range = 0.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::KilicGuard, KilicGuard);
    
    // Balta abilities
    FSHIAbilityData BaltaChop;
    BaltaChop.AbilityType = ESHIWeaponAbilityType::BaltaChop;
    BaltaChop.AbilityName = FText::FromString("Balta Doğrama");
    BaltaChop.AbilityDescription = FText::FromString("Güçlü bir balta darbesi indirir.");
    BaltaChop.CooldownTime = 4.0f;
    BaltaChop.BaseDamage = 100.0f;
    BaltaChop.Range = 180.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::BaltaChop, BaltaChop);
    
    FSHIAbilityData BaltaThrow;
    BaltaThrow.AbilityType = ESHIWeaponAbilityType::BaltaThrow;
    BaltaThrow.AbilityName = FText::FromString("Balta Fırlatma");
    BaltaThrow.AbilityDescription = FText::FromString("Baltayı düşmana fırlatır.");
    BaltaThrow.CooldownTime = 6.0f;
    BaltaThrow.BaseDamage = 90.0f;
    BaltaThrow.Range = 500.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::BaltaThrow, BaltaThrow);
    
    FSHIAbilityData BaltaWhirlwind;
    BaltaWhirlwind.AbilityType = ESHIWeaponAbilityType::BaltaWhirlwind;
    BaltaWhirlwind.AbilityName = FText::FromString("Balta Kasırgası");
    BaltaWhirlwind.AbilityDescription = FText::FromString("Etraftaki tüm düşmanlara dönerek saldırır.");
    BaltaWhirlwind.CooldownTime = 10.0f;
    BaltaWhirlwind.BaseDamage = 80.0f;
    BaltaWhirlwind.Range = 300.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::BaltaWhirlwind, BaltaWhirlwind);
    
    // Meç abilities
    FSHIAbilityData MecLunge;
    MecLunge.AbilityType = ESHIWeaponAbilityType::MecLunge;
    MecLunge.AbilityName = FText::FromString("Meç Hamlesi");
    MecLunge.AbilityDescription = FText::FromString("Ani bir hamle ile düşmana saldırır.");
    MecLunge.CooldownTime = 3.5f;
    MecLunge.BaseDamage = 85.0f;
    MecLunge.Range = 220.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::MecLunge, MecLunge);
    
    FSHIAbilityData MecParry;
    MecParry.AbilityType = ESHIWeaponAbilityType::MecParry;
    MecParry.AbilityName = FText::FromString("Meç Savması");
    MecParry.AbilityDescription = FText::FromString("Gelen saldırıyı savarak karşı saldırıya hazırlanır.");
    MecParry.CooldownTime = 7.0f;
    MecParry.BaseDamage = 0.0f; // Counter ability
    MecParry.Range = 0.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::MecParry, MecParry);
    
    FSHIAbilityData MecRiposte;
    MecRiposte.AbilityType = ESHIWeaponAbilityType::MecRiposte;
    MecRiposte.AbilityName = FText::FromString("Meç Geri Saldırısı");
    MecRiposte.AbilityDescription = FText::FromString("Savma sonrası güçlü bir geri saldırı yapar.");
    MecRiposte.CooldownTime = 5.5f;
    MecRiposte.BaseDamage = 110.0f;
    MecRiposte.Range = 200.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::MecRiposte, MecRiposte);
    
    // Ateş Asası abilities
    FSHIAbilityData AtesFireball;
    AtesFireball.AbilityType = ESHIWeaponAbilityType::AtesFireball;
    AtesFireball.AbilityName = FText::FromString("Ateş Topu");
    AtesFireball.AbilityDescription = FText::FromString("Büyülü bir ateş topu fırlatır.");
    AtesFireball.CooldownTime = 4.5f;
    AtesFireball.BaseDamage = 95.0f;
    AtesFireball.Range = 600.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::AtesFireball, AtesFireball);
    
    FSHIAbilityData AtesBurn;
    AtesBurn.AbilityType = ESHIWeaponAbilityType::AtesBurn;
    AtesBurn.AbilityName = FText::FromString("Yakıcı Alan");
    AtesBurn.AbilityDescription = FText::FromString("Yerde yanıcı bir alan oluşturur.");
    AtesBurn.CooldownTime = 8.0f;
    AtesBurn.BaseDamage = 60.0f; // DoT damage
    AtesBurn.Range = 400.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::AtesBurn, AtesBurn);
    
    FSHIAbilityData AtesIgnite;
    AtesIgnite.AbilityType = ESHIWeaponAbilityType::AtesIgnite;
    AtesIgnite.AbilityName = FText::FromString("Tutuşturma");
    AtesIgnite.AbilityDescription = FText::FromString("Düşmanı tutuşturarak sürekli hasar verir.");
    AtesIgnite.CooldownTime = 6.5f;
    AtesIgnite.BaseDamage = 70.0f;
    AtesIgnite.Range = 300.0f;
    AbilityDatabase.Add(ESHIWeaponAbilityType::AtesIgnite, AtesIgnite);
    
    UE_LOG(LogTemp, Log, TEXT("Ability database initialized with %d Turkish-themed abilities"), 
           AbilityDatabase.Num());
}