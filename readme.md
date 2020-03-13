#The Widowmaker Project

##Universal Deliverables
* In Game Visuals showcasing your edits
* A README with the description of your changes and instructions on how to see them
* A Shortcut that auto-launches your mod
* Compiling with no additional Warnings (Quake's original warnings are fine)

##Personal Deliverables
* Replace weapons w/ shotgun variants (10)
	* Pew Pew - Blaster with more hitscans, even more on charge. (Similar to blaster.)
	* Extra Spicy - Napalm Launcher, fires multiple blasts. 
	* Apology Accepted - DMG, with multiple objects being fired.
	* Wall of Blue - Super Scattershot weapon, almost like fireworks in the air.
	* Fully Operational - Super laser. Hard to tell, but it fires multiple hitscan.
	* Not So Reliable - Shotgun with a slower rate of fire. 
	* Grapeshot - no reload required shotgun.
	* Demoltion Man - Multiple shot gernade launcher.
	* Diplomatic Solution - automatic Shotgun of hyperblast rounds.
	* The Classic - Stronger Shotgun, plain and simple.
	* Unlimited Power - Faster automatic machine gun, with multiple hitscans per fire.


* Combo Leveling System Perks Based (5)
	* Level 0 - Nothing.
	* Level 1 - You may activate the abilities.
	* Level 2 - You boost to ammo on hit.
	* Level 3 - Ammo cost reduction. (coded, not scaled well.)
	* Level 4 - Speed boost
	* Level 5 - Jump boost

	All Abilities Scale by player level.

* Unlockable player abilities (5) (Bleed)
	* Abilities are tied to levels, mapped to the buymenu key
		* Level 1 - Double Speed
		* Level 2 - Double Damage Output
		* Level 3 - "Double Tap" (twice the hitscans, twice the entities created on fire)
		* Level 4 - Healing (Armor increases rapidly while ability active)
		* Level 5 - Invincibility
	* Abilities cascade; At level 1 you only get the speed boost, at level 5, you get every ability triggered at once.

* Score (Record peak)
	* Chaining kills with hitscan weapons increases your score. Every 5 points is a new level. 

* Health System
	* Health removed, Armor is now the sole stat that govern's life. Hitscan weapons pull from Armor instead of having their own ammo.

#Install

* drag ./widowmaker into your Quake4 Folder in steamapps.
* Command `restock` grants the user 10,000 armor
* Command `score [num]` sets the player's score to [num]