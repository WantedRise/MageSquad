#pragma once

#include "NativeGameplayTags.h"

namespace MSGameplayTags
{
	/* Player Ability Tags */
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_DefaultAttack);

	/*Enemy State Tags*/
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Chase);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Attack);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Dead);
	
	/*Enemy Tier Tags*/
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Tier_Normal);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Tier_Elite);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Tier_Boss);
}
