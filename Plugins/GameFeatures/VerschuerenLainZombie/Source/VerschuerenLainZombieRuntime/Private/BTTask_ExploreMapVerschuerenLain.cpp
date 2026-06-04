#include "BTTask_ExploreMapVerschuerenLain.h"
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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("explore task: ExecuteTask called!"));
	
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	
	if (!Controller || !BlackboardComp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("explore task: no controller or blackboard"));
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("explore task: no possessed pawn"));
		return EBTNodeResult::Failed;
	}

	FVector Origin = Pawn->GetActorLocation();
	FVector TargetLocation = Origin;
	bool bFoundPoint = false;

	UWorld* World = OwnerComp.GetWorld();
	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
	{
		FNavLocation NavLoc;
		if (NavSys->GetRandomReachablePointInRadius(Origin, SearchRadius, NavLoc))
		{
			TargetLocation = NavLoc.Location;
			bFoundPoint = true;
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("explore task: nav system not found"));
	}

	if (bFoundPoint)
	{
		FString DebugMsg = FString::Printf(TEXT("explore task: going to %s (key: %s)"), 
			*TargetLocation.ToString(), *TargetLocationKey.SelectedKeyName.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, DebugMsg);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("explore task: failed to find random nav point"));
	}

	BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLocation);

	return EBTNodeResult::Succeeded;
}
