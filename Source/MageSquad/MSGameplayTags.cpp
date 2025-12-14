
#include "MSGameplayTags.h"

namespace MSGameplayTags
{
	/* Player Ability Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_DefaultAttack, "Player.Ability.DefaultAttack");

	/* Player Event Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_DefaultAttack, "Player.Event.DefaultAttack");

	/*Enemy State Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Chase, "Enemy.State.Chase");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Attack, "Enemy.State.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Dead, "Enemy.State.Dead");

	/*Enemy Tier Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Tier_Normal, "Enemy.Tier.Normal");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Tier_Elite, "Enemy.Tier.Elite");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Tier_Boss, "Enemy.Tier.Boss");
}
