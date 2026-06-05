#include "BTService_UpdateSensorDataVerschuerenLain.h"
#include "ZombieSurvMemoryComponentVerschuerenLain.h"
#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Common/InventoryComponent.h"

UBTService_UpdateSensorDataVerschuerenLain::UBTService_UpdateSensorDataVerschuerenLain()
{
	NodeName = TEXT("Update Sensor Data VerschuerenLain");
	
	// standard tick interval (e.g. 0.1s to 0.2s)
	Interval = 0.1f;
	RandomDeviation = 0.05f;
}

void UBTService_UpdateSensorDataVerschuerenLain::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!Controller || !BlackboardComp) return;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return;

	// health
	UHealthComponent* HealthComp = Pawn->GetComponentByClass<UHealthComponent>();
	if (HealthComp)
	{
		float HealthPercent = (float)HealthComp->GetHealth() / HealthComp->GetMaxHealth();
		BlackboardComp->SetValueAsFloat(HealthKey.SelectedKeyName, HealthPercent);
	}

	// stamina
	UStaminaComponent* StaminaComp = Pawn->GetComponentByClass<UStaminaComponent>();
	if (StaminaComp)
	{
		float StaminaPercent = StaminaComp->GetCurrentStamina() / StaminaComp->GetMaxStamina();
		BlackboardComp->SetValueAsFloat(StaminaKey.SelectedKeyName, StaminaPercent);
	}

	// weapon
	UInventoryComponent* InvComp = Pawn->GetComponentByClass<UInventoryComponent>();
	bool bHasWeapon = false;
	if (InvComp)
	{
		for (ABaseItem* Item : InvComp->GetInventory())
		{
			if (Item && (Item->GetItemType() == EItemType::Pistol || Item->GetItemType() == EItemType::Shotgun))
			{
				if (Item->GetValue() > 0)
				{
					bHasWeapon = true;
					break;
				}
			}
		}
	}
	BlackboardComp->SetValueAsBool(HasWeaponKey.SelectedKeyName, bHasWeapon);

	// closest zombie
	UZombieSurvMemoryComponentVerschuerenLain* MemoryComp = Pawn->GetComponentByClass<UZombieSurvMemoryComponentVerschuerenLain>();
	ABaseZombie* ClosestZombie = nullptr;
	bool bFoundZombie = false;
	if (MemoryComp)
	{
		bFoundZombie = MemoryComp->FindClosestZombie(Pawn->GetActorLocation(), ClosestZombie);
	}

	BlackboardComp->SetValueAsBool(HasTargetZombieKey.SelectedKeyName, bFoundZombie);
	if (bFoundZombie)
	{
		BlackboardComp->SetValueAsObject(TargetZombieKey.SelectedKeyName, ClosestZombie);
	}
	else
	{
		BlackboardComp->ClearValue(TargetZombieKey.SelectedKeyName);
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}
