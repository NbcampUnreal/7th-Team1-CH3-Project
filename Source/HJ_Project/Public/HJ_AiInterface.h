#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HJ_AIInterface.generated.h"

UINTERFACE(MinimalAPI)
class UHJ_AIInterface : public UInterface
{
	GENERATED_BODY()
};

class HJ_PROJECT_API IHJ_AIInterface
{
	GENERATED_BODY()

public:
	virtual void NotifyDeath() = 0;
};
