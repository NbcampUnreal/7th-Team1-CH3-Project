#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "EquipWeaponMaster.generated.h"

class USceneComponent;
class UArrowComponent;
class APlayerController;
class UNiagaraSystem;




//탄이 없을 때 발사 막기
USTRUCT(BlueprintType)
struct FRecoilStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch = 0.0f; // 위로 들림(+)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Yaw = 0.0f;   // 좌(-) 우(+)
};

UCLASS()
class HJ_PROJECT_API AEquipWeaponMaster : public AActor
{
	GENERATED_BODY()

public:
	AEquipWeaponMaster();

	/* ================= Fire ================= */

	UFUNCTION(BlueprintCallable)
	virtual void Fire();

	virtual void StartFire();
	virtual void StopFire();

	//재장전
	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	virtual void Reload();


	//사운드
	UFUNCTION(BlueprintPure, Category = "Weapon|SFX")
	USoundBase* GetReloadSound() const { return ReloadSound2D; }

protected:
	//Tick 에서 반동 복구
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	void FinishReload();
	FVector GetMuzzleLocation() const;

	/* ================= Components ================= */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* Muzzle;

	//VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|VFX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashNiagara = nullptr;


	/* ================= Weapon Stat ================= */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float TraceDistance = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stat")
	float CurrentDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float FireRate = 0.1f;

	/* ================= Ammo ================= */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MagazineSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MaxAmmo = 120;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentMagazineAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	float ReloadTime = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
	bool bIsReloading = false;

	//총기 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|SFX")
	TObjectPtr<USoundBase> FireSound2D = nullptr;

	//리로딩 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|SFX")
	TObjectPtr<USoundBase> ReloadSound2D = nullptr;


	/* ================= Debug ================= */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawCameraDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Debug")
	bool bDrawMuzzleDebug = true;

	FTimerHandle FireTimerHandle;

	FTimerHandle ReloadTimerHandle;//장전완료 

	


	//탄 (45발 예비탄 무한)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MaxAmmoInMag = 45; //탄창 최대치

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmoInMag = 45;  //현재 탄창 탄약

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	bool bInfiniteReserveAmmo = true;//예비탄 무한
	

	//탄이 없을 때 발사 막기
	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	bool CanFire() const; //CurrentAmmoInMag > 0 인지 체크

	//발사 시 탄약 1발 감소
	void ConsumeAmmo(); 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	TArray<FRecoilStep> RecoilPattern;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Recoil")
	int32 ShotsFiredInBurst = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float RecoilOverflowMultiplier = 1.15f; // 
	

	// 반동이 무한정 커지지 않도록 상한선 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float MaxRecoilPitchPerShot = 0.35f; //한 발당 Pitch 반동 최대치

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float MaxRecoilYawPerShot = 0.12f;   // 한 발당 Yaw 반동 최대치

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	int32 RecoilOverflowStartShot = 20;  //이 발수 이후부터만 누적 패널티

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float RecoilOverflowSlope = 0.0f;   // 누적 증가 기울기

	//스프레드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float BaseSpreadDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float SpreadIncreasePerShotDeg = 0.012f; // 웨이브 여유를 위해 약간 완화

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float MaxSpreadDeg = 1.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Recoil")
	float CurrentSpreadDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float SpreadRecoverySpeed = 12.0f; // 회복 더 빠르게

	//  실제 반동을 컨트롤러 입력으로 적용
	virtual void ApplyRecoil(APlayerController* PC);

	// 연사 종료 시 버스트 카운트 리셋
	virtual void ResetBurst();

	// 스프레드 누적/복구
	virtual void IncreaseSpread();
	virtual void RecoverSpread(float DeltaTime);

};