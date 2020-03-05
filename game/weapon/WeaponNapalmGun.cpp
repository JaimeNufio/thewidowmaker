#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"
#include "../client/ClientEffect.h"

#ifndef __GAME_PROJECTILE_H__
#include "../Projectile.h"
#endif


const int NAPALM_GUN_NUM_CYLINDERS = 5;

class WeaponNapalmGun : public rvWeapon {
public:

	CLASS_PROTOTYPE(WeaponNapalmGun);

	WeaponNapalmGun(void);
	~WeaponNapalmGun(void);

	virtual void			Spawn(void);
	virtual void			Think(void);
	virtual void			MuzzleRise(idVec3 &origin, idMat3 &axis);

	virtual void			SpectatorCycle(void);

	void					Save(idSaveGame *saveFile) const;
	void					Restore(idRestoreGame *saveFile);

protected:

	void					UpdateCylinders(void);

	typedef enum { CYLINDER_RESET_POSITION, CYLINDER_MOVE_POSITION, CYLINDER_UPDATE_POSITION } CylinderState;
	CylinderState								cylinderState;

private:

	stateResult_t		State_Idle(const stateParms_t& parms);
	stateResult_t		State_Fire(const stateParms_t& parms);
	stateResult_t		State_Reload(const stateParms_t& parms);
	stateResult_t		State_EmptyReload(const stateParms_t& parms);

	stateResult_t		Frame_MoveCylinder(const stateParms_t& parms);
	stateResult_t		Frame_ResetCylinder(const stateParms_t& parms);


	float								cylinderMaxOffsets[NAPALM_GUN_NUM_CYLINDERS];
	idInterpolate<float>				cylinderOffsets[NAPALM_GUN_NUM_CYLINDERS];
	jointHandle_t						cylinderJoints[NAPALM_GUN_NUM_CYLINDERS];


	int									cylinderMoveTime;
	int									previousAmmo;
	bool								zoomed;

	CLASS_STATES_PROTOTYPE(WeaponNapalmGun);
};

CLASS_DECLARATION(rvWeapon, WeaponNapalmGun)
END_CLASS

/*
================
WeaponNapalmGun::WeaponNapalmGun
================
*/
WeaponNapalmGun::WeaponNapalmGun(void) { }

/*
================
WeaponNapalmGun::~WeaponNapalmGun
================
*/
WeaponNapalmGun::~WeaponNapalmGun(void) { }

/*
================
WeaponNapalmGun::Spawn
================
*/
void WeaponNapalmGun::Spawn(void) {
	assert(viewModel);
	idAnimator* animator = viewModel->GetAnimator();
	assert(animator);

	SetState("Raise", 0);

	for (int i = 0; i < NAPALM_GUN_NUM_CYLINDERS; ++i)
	{
		idStr argName = "cylinder_offset";
		argName += i;
		cylinderMaxOffsets[i] = spawnArgs.GetFloat(argName, "0.0");

		argName = "cylinder_joint";
		argName += i;
		cylinderJoints[i] = animator->GetJointHandle(spawnArgs.GetString(argName, ""));

		cylinderOffsets[i].Init(gameLocal.time, 0.0f, 0, 0);
	}

	previousAmmo = AmmoInClip();
	cylinderMoveTime = spawnArgs.GetFloat("cylinderMoveTime", "500");
	cylinderState = CYLINDER_RESET_POSITION;
	zoomed = false;
}

/*
================
WeaponNapalmGun::Think
================
*/
void WeaponNapalmGun::Think(void) {

	rvWeapon::Think();

	//Check to see if the ammo level has changed.
	//This is to account for ammo pickups.
	if (previousAmmo != AmmoInClip()) {
		// don't do this in MP, the weap script doesn't sync the canisters anyway
		if (!gameLocal.isMultiplayer) {
			//change the cylinder state to reflect the new change in ammo.
			cylinderState = CYLINDER_MOVE_POSITION;
		}
		previousAmmo = AmmoInClip();
	}

	UpdateCylinders();
}

/*
===============
WeaponNapalmGun::MuzzleRise
===============
*/
void WeaponNapalmGun::MuzzleRise(idVec3 &origin, idMat3 &axis) {
	if (wsfl.zoom)
		return;

	rvWeapon::MuzzleRise(origin, axis);
}

/*
===============
WeaponNapalmGun::UpdateCylinders
===============
*/
void WeaponNapalmGun::UpdateCylinders(void)
{
	idAnimator* animator;
	animator = viewModel->GetAnimator();
	assert(animator);

	float ammoInClip = AmmoInClip();
	float clipSize = ClipSize();
	if (clipSize <= idMath::FLOAT_EPSILON) {
		clipSize = maxAmmo;
	}

	for (int i = 0; i < NAPALM_GUN_NUM_CYLINDERS; ++i)
	{
		// move the local position of the joint along the x-axis.
		float currentOffset = cylinderOffsets[i].GetCurrentValue(gameLocal.time);

		switch (cylinderState)
		{
		case CYLINDER_MOVE_POSITION:
		{
									   float cylinderMaxOffset = cylinderMaxOffsets[i];
									   float endValue = cylinderMaxOffset * (1.0f - (ammoInClip / clipSize));
									   cylinderOffsets[i].Init(gameLocal.time, cylinderMoveTime, currentOffset, endValue);
		}
			break;

		case CYLINDER_RESET_POSITION:
		{
										float cylinderMaxOffset = cylinderMaxOffsets[i];
										float endValue = cylinderMaxOffset * (1.0f - (ammoInClip / clipSize));
										cylinderOffsets[i].Init(gameLocal.time, 0, endValue, endValue);
		}
			break;
		}


		animator->SetJointPos(cylinderJoints[i], JOINTMOD_LOCAL, idVec3(currentOffset, 0.0f, 0.0f));
	}

	cylinderState = CYLINDER_UPDATE_POSITION;
}


/*
=====================
WeaponNapalmGun::Save
=====================
*/
void WeaponNapalmGun::Save(idSaveGame *saveFile) const
{
	for (int i = 0; i < NAPALM_GUN_NUM_CYLINDERS; i++)
	{
		saveFile->WriteFloat(cylinderMaxOffsets[i]);
		saveFile->WriteInterpolate(cylinderOffsets[i]);
		saveFile->WriteJoint(cylinderJoints[i]);
	}

	saveFile->WriteInt(cylinderMoveTime);
	saveFile->WriteInt(previousAmmo);
}

/*
=====================
WeaponNapalmGun::Restore
=====================
*/
void WeaponNapalmGun::Restore(idRestoreGame *saveFile) {

	for (int i = 0; i < NAPALM_GUN_NUM_CYLINDERS; i++)
	{
		saveFile->ReadFloat(cylinderMaxOffsets[i]);
		saveFile->ReadInterpolate(cylinderOffsets[i]);
		saveFile->ReadJoint(cylinderJoints[i]);
	}

	saveFile->ReadInt(cylinderMoveTime);
	saveFile->ReadInt(previousAmmo);
}

/*
===============================================================================

States

===============================================================================
*/

CLASS_STATES_DECLARATION(WeaponNapalmGun)
STATE("Idle", WeaponNapalmGun::State_Idle)
STATE("Fire", WeaponNapalmGun::State_Fire)
STATE("Reload", WeaponNapalmGun::State_Reload)
END_CLASS_STATES

/*
================
rvWeaponShotgun::State_Idle
================
*/
stateResult_t WeaponNapalmGun::State_Idle(const stateParms_t& parms) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};
	switch (parms.stage) {
	case STAGE_INIT:
		if (!AmmoAvailable()) {
			SetStatus(WP_OUTOFAMMO);
		}
		else {
			SetStatus(WP_READY);
		}

		PlayCycle(ANIMCHANNEL_ALL, "idle", parms.blendFrames);
		return SRESULT_STAGE(STAGE_WAIT);

	case STAGE_WAIT:
		if (wsfl.lowerWeapon) {
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		if (!clipSize) {
			if (gameLocal.time > nextAttackTime && wsfl.attack && AmmoAvailable()) {
				SetState("Fire", 0);
				return SRESULT_DONE;
			}
		}
		else {
			if (gameLocal.time > nextAttackTime && wsfl.attack && AmmoInClip()) {
				SetState("Fire", 0);
				return SRESULT_DONE;
			}
			if (wsfl.attack && AutoReload() && !AmmoInClip() && AmmoAvailable()) {
				SetState("Reload", 4);
				return SRESULT_DONE;
			}
			if (wsfl.netReload || (wsfl.reload && AmmoInClip() < ClipSize() && AmmoAvailable()>AmmoInClip())) {
				SetState("Reload", 4);
				return SRESULT_DONE;
			}
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponShotgun::State_Fire
================
*/
stateResult_t WeaponNapalmGun::State_Fire(const stateParms_t& parms) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};
	switch (parms.stage) {
	case STAGE_INIT:
		nextAttackTime = gameLocal.time + (fireRate * owner->PowerUpModifier(PMOD_FIRERATE));
		Attack(false, hitscans, spread, 0, 1.0f);
		PlayAnim(ANIMCHANNEL_ALL, "fire", 0);
		return SRESULT_STAGE(STAGE_WAIT);

	case STAGE_WAIT:
		if ((!gameLocal.isMultiplayer && (wsfl.lowerWeapon || AnimDone(ANIMCHANNEL_ALL, 0))) || AnimDone(ANIMCHANNEL_ALL, 0)) {
			SetState("Idle", 0);
			return SRESULT_DONE;
		}
		if (wsfl.attack && gameLocal.time >= nextAttackTime && AmmoInClip()) {
			SetState("Fire", 0);
			return SRESULT_DONE;
		}
		if (clipSize) {
			if ((wsfl.netReload || (wsfl.reload && AmmoInClip() < ClipSize() && AmmoAvailable()>AmmoInClip()))) {
				SetState("Reload", 4);
				return SRESULT_DONE;
			}
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponShotgun::State_Reload
================
*/
stateResult_t WeaponNapalmGun::State_Reload(const stateParms_t& parms) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
		STAGE_RELOADSTARTWAIT,
		STAGE_RELOADLOOP,
		STAGE_RELOADLOOPWAIT,
		STAGE_RELOADDONE,
		STAGE_RELOADDONEWAIT
	};
	switch (parms.stage) {
	case STAGE_INIT:
		if (wsfl.netReload) {
			wsfl.netReload = false;
		}
		else {
			NetReload();
		}

		SetStatus(WP_RELOAD);

		if (mods & false) {
			PlayAnim(ANIMCHANNEL_ALL, "reload_clip", parms.blendFrames);
		}
		else {
			PlayAnim(ANIMCHANNEL_ALL, "reload_start", parms.blendFrames);
			return SRESULT_STAGE(STAGE_RELOADSTARTWAIT);
		}
		return SRESULT_STAGE(STAGE_WAIT);

	case STAGE_WAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 4)) {
			AddToClip(ClipSize());
			SetState("Idle", 4);
			return SRESULT_DONE;
		}
		if (wsfl.lowerWeapon) {
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;

	case STAGE_RELOADSTARTWAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 0)) {
			return SRESULT_STAGE(STAGE_RELOADLOOP);
		}
		if (wsfl.lowerWeapon) {
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;

	case STAGE_RELOADLOOP:
		if ((wsfl.attack && AmmoInClip()) || AmmoAvailable() <= AmmoInClip() || AmmoInClip() == ClipSize()) {
			return SRESULT_STAGE(STAGE_RELOADDONE);
		}
		PlayAnim(ANIMCHANNEL_ALL, "reload_loop", 0);
		return SRESULT_STAGE(STAGE_RELOADLOOPWAIT);

	case STAGE_RELOADLOOPWAIT:
		if ((wsfl.attack && AmmoInClip()) || wsfl.netEndReload) {
			return SRESULT_STAGE(STAGE_RELOADDONE);
		}
		if (wsfl.lowerWeapon) {
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		if (AnimDone(ANIMCHANNEL_ALL, 0)) {
			AddToClip(1);
			return SRESULT_STAGE(STAGE_RELOADLOOP);
		}
		return SRESULT_WAIT;

	case STAGE_RELOADDONE:
		NetEndReload();
		PlayAnim(ANIMCHANNEL_ALL, "reload_end", 0);
		return SRESULT_STAGE(STAGE_RELOADDONEWAIT);

	case STAGE_RELOADDONEWAIT:
		if (wsfl.lowerWeapon) {
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		if (wsfl.attack && AmmoInClip() && gameLocal.time > nextAttackTime) {
			SetState("Fire", 0);
			return SRESULT_DONE;
		}
		if (AnimDone(ANIMCHANNEL_ALL, 4)) {
			SetState("Idle", 4);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

stateResult_t WeaponNapalmGun::Frame_MoveCylinder(const stateParms_t& parms) {
	cylinderState = CYLINDER_MOVE_POSITION;
	return SRESULT_OK;
}

stateResult_t WeaponNapalmGun::Frame_ResetCylinder(const stateParms_t& parms) {
	cylinderState = CYLINDER_RESET_POSITION;
	return SRESULT_OK;
}

void WeaponNapalmGun::SpectatorCycle(void) {
	cylinderState = CYLINDER_RESET_POSITION;
}