#include "BTTask_UseInventoryItemVerschuerenLain.h"
#include "Common/InventoryComponent.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Items/BaseItem.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTTask_UseInventoryItemVerschuerenLain::UBTTask_UseInventoryItemVerschuerenLain()
{
	NodeName = TEXT("Use Inventory Item VerschuerenLain");
}

EBTNodeResult::Type UBTTask_UseInventoryItemVerschuerenLain::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	// safety check: do not consume if stats are already high
	if (ItemType == EItemType::Medkit)
	{
		UHealthComponent* HealthComp = Pawn->GetComponentByClass<UHealthComponent>();
		if (HealthComp)
		{
			float HealthPct = (float)HealthComp->GetHealth() / HealthComp->GetMaxHealth();
			if (HealthPct >= 0.85f)
			{
				return EBTNodeResult::Failed;
			}
		}
	}
	else if (ItemType == EItemType::Food)
	{
		UStaminaComponent* StaminaComp = Pawn->GetComponentByClass<UStaminaComponent>();
		if (StaminaComp)
		{
			float StaminaPct = StaminaComp->GetCurrentStamina() / StaminaComp->GetMaxStamina();
			if (StaminaPct >= 0.85f)
			{
				return EBTNodeResult::Failed;
			}
		}
	}

	UInventoryComponent* InvComp = Pawn->GetComponentByClass<UInventoryComponent>();
	if (!InvComp) return EBTNodeResult::Failed;

	const TArray<ABaseItem*>& Inv = InvComp->GetInventory();
	for (int s = 0; s < Inv.Num(); ++s)
	{
		if (Inv[s] && Inv[s]->GetItemType() == ItemType)
		{
			if (InvComp->UseItem(s))
			{
				// remove item if fully used (value <= 0)
				if (Inv[s]->GetValue() <= 0)
				{
					InvComp->RemoveItem(s);
				}
				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
