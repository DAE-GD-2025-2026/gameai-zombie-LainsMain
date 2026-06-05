#include "BTTask_LookAtZombieVerschuerenLain.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTTask_LookAtZombieVerschuerenLain::UBTTask_LookAtZombieVerschuerenLain()
{
	NodeName = TEXT("Look At Zombie VerschuerenLain");
}

EBTNodeResult::Type UBTTask_LookAtZombieVerschuerenLain::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!Controller || !BlackboardComp) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	AActor* Zombie = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetZombieKey.SelectedKeyName));
	if (!Zombie) return EBTNodeResult::Failed;

	// smooth face zombie using AI focus
	Controller->SetFocus(Zombie);

	return EBTNodeResult::Succeeded;
}
