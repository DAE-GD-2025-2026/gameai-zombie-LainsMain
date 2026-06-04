#include "BTTask_FindClosestItemVerschuerenLain.h"
#include "ZombieSurvMemoryComponentVerschuerenLain.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Common/InventoryComponent.h"

UBTTask_FindClosestItemVerschuerenLain::UBTTask_FindClosestItemVerschuerenLain()
{
	NodeName = TEXT("Find Closest Item VerschuerenLain");
}

EBTNodeResult::Type UBTTask_FindClosestItemVerschuerenLain::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!Controller || !BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UZombieSurvMemoryComponentVerschuerenLain* MemoryComp = Pawn->GetComponentByClass<UZombieSurvMemoryComponentVerschuerenLain>();
	if (!MemoryComp)
	{
		return EBTNodeResult::Failed;
	}

	UHealthComponent* HealthComp = Pawn->GetComponentByClass<UHealthComponent>();
	UStaminaComponent* StaminaComp = Pawn->GetComponentByClass<UStaminaComponent>();
	UInventoryComponent* InvComp = Pawn->GetComponentByClass<UInventoryComponent>();

	FVector PawnLoc = Pawn->GetActorLocation();
	FVector TargetLoc;
	ABaseItem* TargetItem = nullptr;
	bool bFoundTarget = false;

	// health check
	float HealthPercent = 1.0f;
	if (HealthComp)
	{
		HealthPercent = (float)HealthComp->GetHealth() / HealthComp->GetMaxHealth();
	}

	// stamina check
	float StaminaPercent = 1.0f;
	if (StaminaComp)
	{
		StaminaPercent = StaminaComp->GetCurrentStamina() / StaminaComp->GetMaxStamina();
	}

	// weapon check
	bool bHasWeapon = false;
	if (InvComp)
	{
		for (ABaseItem* InvItem : InvComp->GetInventory())
		{
			if (InvItem && (InvItem->GetItemType() == EItemType::Pistol || InvItem->GetItemType() == EItemType::Shotgun))
			{
				bHasWeapon = true;
				break;
			}
		}
	}

	// priority 1: low health -> get medkit
	if (HealthPercent < 0.4f)
	{
		bFoundTarget = MemoryComp->FindClosestItem(PawnLoc, EItemType::Medkit, TargetLoc, TargetItem);
	}

	// priority 2: low stamina -> get food
	if (!bFoundTarget && StaminaPercent < 0.4f)
	{
		bFoundTarget = MemoryComp->FindClosestItem(PawnLoc, EItemType::Food, TargetLoc, TargetItem);
	}

	// priority 3: no weapon -> get weapon
	if (!bFoundTarget && !bHasWeapon)
	{
		FVector ShotgunLoc, PistolLoc;
		ABaseItem *ShotgunItem = nullptr, *PistolItem = nullptr;
		bool bFoundShotgun = MemoryComp->FindClosestItem(PawnLoc, EItemType::Shotgun, ShotgunLoc, ShotgunItem);
		bool bFoundPistol = MemoryComp->FindClosestItem(PawnLoc, EItemType::Pistol, PistolLoc, PistolItem);

		if (bFoundShotgun && bFoundPistol)
		{
			if (FVector::Dist(PawnLoc, ShotgunLoc) < FVector::Dist(PawnLoc, PistolLoc))
			{
				TargetLoc = ShotgunLoc;
				TargetItem = ShotgunItem;
				bFoundTarget = true;
			}
			else
			{
				TargetLoc = PistolLoc;
				TargetItem = PistolItem;
				bFoundTarget = true;
			}
		}
		else if (bFoundShotgun)
		{
			TargetLoc = ShotgunLoc;
			TargetItem = ShotgunItem;
			bFoundTarget = true;
		}
		else if (bFoundPistol)
		{
			TargetLoc = PistolLoc;
			TargetItem = PistolItem;
			bFoundTarget = true;
		}
	}

	// priority 4: get any item
	if (!bFoundTarget)
	{
		bFoundTarget = MemoryComp->FindClosestItem(PawnLoc, EItemType::Garbage, TargetLoc, TargetItem);
	}

	if (bFoundTarget)
	{
		BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLoc);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
