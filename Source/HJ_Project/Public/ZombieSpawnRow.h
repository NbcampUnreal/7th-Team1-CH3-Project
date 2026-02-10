#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "ZombieSpawnRow.generated.h"

USTRUCT(BlueprintType) 
struct FZombieSpawnRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 ZombieCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float PreparationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    TSubclassOf<AActor> ZombieClass;
};