
#include "MSGameplayTags.h"

namespace MSGameplayTags
{
	/* Player Ability Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_DefaultAttack, "Player.Ability.DefaultAttack");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Blink, "Player.Ability.Blink");

	/* Player Event Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_DefaultAttack, "Player.Event.DefaultAttack");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Blink, "Player.Event.Blink");

	/* Player Cooldown Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_Blink, "Player.Cooldown.Blink");
	
	/*Enemy Ability Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_NormalAttack, "Enemy.Ability.NormalAttack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Dead, "Enemy.Ability.Dead");
	
	/*Enemy State Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Chase, "Enemy.State.Chase");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Attack, "Enemy.State.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Dead, "Enemy.State.Dead");

	/*Enemy Tier Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Tier_Normal, "Enemy.Tier.Normal");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Tier_Elite, "Enemy.Tier.Elite");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Tier_Boss, "Enemy.Tier.Boss");

	/* Skill Ability Tags */
	UE_DEFINE_GAMEPLAY_TAG(Skill_Ability_IceSpear, "Skill.Ability.IceSpear");

	/* Skill Event Tags */
	UE_DEFINE_GAMEPLAY_TAG(Skill_Event_IceSpear, "Skill.Event.IceSpear");
}
