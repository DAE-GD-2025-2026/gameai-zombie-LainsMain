#include "BTTask_ExploreMapVerschuerenLain.h"
#include "ZombieSurvMemoryComponentVerschuerenLain.h"
#include "Village/House/House.h"
#include "Survivor/SurvivorPawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"

UBTTask_ExploreMapVerschuerenLain::UBTTask_ExploreMapVerschuerenLain()
{
	NodeName = TEXT("Explore Map VerschuerenLain");
}

EBTNodeResult::Type UBTTask_ExploreMapVerschuerenLain::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	FVector Origin = Pawn->GetActorLocation();
	FVector TargetLocation = Origin;
	bool bFoundPoint = false;
	bool bIsHouse = false;

	// try to find unexplored house first
	AHouse* House = nullptr;
	if (MemoryComp->GetNextUnexploredHouse(Origin, TargetLocation, House))
	{
		bFoundPoint = true;
		bIsHouse = true;
	}
	else
	{
		// wander in a direction if all houses explored
		UWorld* World = OwnerComp.GetWorld();
		if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
			FNavLocation NavLoc;
			FVector Direction = Pawn->GetActorForwardVector();
			FVector WanderOrigin = Origin + Direction * SearchRadius;
			if (NavSys->GetRandomReachablePointInRadius(WanderOrigin, SearchRadius * 0.5f, NavLoc))
			{
				TargetLocation = NavLoc.Location;
				bFoundPoint = true;
			}
			else if (NavSys->GetRandomReachablePointInRadius(Origin, SearchRadius, NavLoc))
			{
				TargetLocation = NavLoc.Location;
				bFoundPoint = true;
			}
		}
	}

	if (bFoundPoint)
	{
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
		BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLocation);
		if (bIsHouse)
		{
			BlackboardComp->SetValueAsBool(ExploringHouseKey.SelectedKeyName, true);
		}

		// stop running to save stamina
		if (ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Pawn))
		{
			Survivor->StopRunning();
		}
		
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
