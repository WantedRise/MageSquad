#pragma once

#include "NativeGameplayTags.h"

namespace MSGameplayTags
{
	/* Player Ability Tags */
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_DefaultAttack);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Blink);

	/* Player Event Tags */
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Event_DefaultAttack);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Event_Blink);

	/* Player Cooldown Tags */
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Cooldown_Blink);

	/*Enemy State Tags*/
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Chase);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Attack);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Dead);
	
	/*Enemy Ability Tags*/
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_NormalAttack);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Ability_Dead);

	/*Enemy Tier Tags*/
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Tier_Normal);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Tier_Elite);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Tier_Boss);

	/* Skill Ability Tags */
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_Ability_IceSpear);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_Ability_Explosion);

	/* Skill Event Tags */
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_Event_IceSpear);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_Event_Explosion);
	
	/* GameplayCue Tags */
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_Explosion);
}
