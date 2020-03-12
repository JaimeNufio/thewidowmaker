#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"

const idEventDef EV_Railgun_RestoreHum( "<railgunRestoreHum>", "" );

class rvWeaponRailgun : public rvWeapon {
public:

	CLASS_PROTOTYPE( rvWeaponRailgun );

	rvWeaponRailgun ( void );

	virtual void			Spawn				( void );
	virtual void			Think				( void );
	void					Save				( idSaveGame *savefile ) const;
	void					Restore				( idRestoreGame *savefile );
	void					PreSave				( void );
	void					PostSave			( void );
	void					ClientUnstale		( void );

protected:
	jointHandle_t			jointBatteryView;

private:

	stateResult_t		State_Idle		( const stateParms_t& parms );
	stateResult_t		State_Fire		( const stateParms_t& parms );
	stateResult_t		State_Reload	( const stateParms_t& parms );

//	void				Event_RestoreHum	( void );

	CLASS_STATES_PROTOTYPE ( rvWeaponRailgun );
};

CLASS_DECLARATION( rvWeapon, rvWeaponRailgun )
//	EVENT( EV_Railgun_RestoreHum,			rvWeaponRailgun::Event_RestoreHum )
END_CLASS

/*
================
rvWeaponRailgun::rvWeaponRailgun
================
*/
rvWeaponRailgun::rvWeaponRailgun ( void ) {
}

void rvWeaponRailgun::ClientUnstale(void) {
}


/*
================
rvWeaponRailgun::Spawn
================
*/
void rvWeaponRailgun::Spawn ( void ) {
	SetState ( "Raise", 0 );	
}

/*
================
rvWeaponRailgun::Save
================
*/
void rvWeaponRailgun::Save ( idSaveGame *savefile ) const {
	savefile->WriteJoint( jointBatteryView );
}

/*
================
rvWeaponRailgun::Restore
================
*/
void rvWeaponRailgun::Restore ( idRestoreGame *savefile ) {
	savefile->ReadJoint( jointBatteryView );
}

/*
================
rvWeaponRailgun::PreSave
================
*/
void rvWeaponRailgun::PreSave ( void ) {

	//this should shoosh the humming but not the shooting sound.
	StopSound( SND_CHANNEL_BODY2, 0);
}

/*
================
rvWeaponRailgun::PostSave
================
*/
void rvWeaponRailgun::PostSave ( void ) {

	//restore the humming
	//PostEventMS( &EV_Railgun_RestoreHum, 10);
}



/*
================
rvWeaponRailgun::Think
================
*/
void rvWeaponRailgun::Think ( void ) {

	// Let the real weapon think first
	rvWeapon::Think ( );

	if ( zoomGui && wsfl.zoom && !gameLocal.isMultiplayer ) {
		int ammo = AmmoInClip();
		if ( ammo >= 0 ) {
			zoomGui->SetStateInt( "player_ammo", ammo );
		}			
	}
}


/*
===============================================================================

States

===============================================================================
*/

CLASS_STATES_DECLARATION(rvWeaponRailgun)
STATE("Idle", rvWeaponRailgun::State_Idle)
STATE("Fire", rvWeaponRailgun::State_Fire)
STATE("Reload", rvWeaponRailgun::State_Reload)
END_CLASS_STATES

/*
================
rvWeaponRailgun::State_Idle
================
*/
stateResult_t rvWeaponRailgun::State_Idle(const stateParms_t& parms) {
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
rvWeaponRailgun::State_Fire
================
*/
stateResult_t rvWeaponRailgun::State_Fire(const stateParms_t& parms) {
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
rvWeaponRailgun::State_Reload
================
*/
stateResult_t rvWeaponRailgun::State_Reload(const stateParms_t& parms) {
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

		if (mods & true) {
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

