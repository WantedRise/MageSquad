
#include "MSGameplayTags.h"

namespace MSGameplayTags
{
	/* Player Ability Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_DefaultAttack, "Player.Ability.DefaultAttack");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Blink, "Player.Ability.Blink");
	
	/*Pleyer State Tahgs*/
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Invincible, "Player.State.Invincible");
	
	/* Player Event Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_DefaultAttack, "Player.Event.DefaultAttack");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Blink, "Player.Event.Blink");

	/* Player Cooldown Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_Blink, "Player.Cooldown.Blink");
	
#pragma  region Enemy Tags Section
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
	
	/*Enemy Event Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_NormalAttack, "Enemy.Event.NormalAttack");
#pragma  endregion

	/* Skill Ability Tags */
	UE_DEFINE_GAMEPLAY_TAG(Skill_Ability_IceSpear, "Skill.Ability.IceSpear");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Ability_Explosion, "Skill.Ability.Explosion");

	/* Skill Event Tags */
	UE_DEFINE_GAMEPLAY_TAG(Skill_Event_IceSpear, "Skill.Event.IceSpear");



	/* TEST Tags */
	UE_DEFINE_GAMEPLAY_TAG(TEST_Ability_HpIncrease, "TEST.Ability.HpIncrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Ability_HpDecrease, "TEST.Ability.HpDecrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Ability_MaxHpIncrease, "TEST.Ability.MaxHpIncrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Ability_MaxHpDecrease, "TEST.Ability.MaxHpDecrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Event_HpIncrease, "TEST.Event.HpIncrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Event_HpDecrease, "TEST.Event.HpDecrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Event_MaxHpIncrease, "TEST.Event.MaxHpIncrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Event_MaxHpDecrease, "TEST.Event.MaxHpDecrease");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Event_Explosion, "Skill.Event.Explosion");
	
	/* GameplayCue Tags */
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_Explosion,"GameplayCue.Skill.Explosion");
	//UE_DEFINE_GAMEPLAY_TAG(GameplayCue_HitFlash,"GameplayCue.HitFlash");
	
	/* Hit Result Tags */
	UE_DEFINE_GAMEPLAY_TAG(Hit_Critical, "Hit.Critical");
	
	/* Data Tags */
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
}
