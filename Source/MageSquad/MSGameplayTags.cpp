
#include "MSGameplayTags.h"

namespace MSGameplayTags
{
	/* Player Ability Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_DefaultAttack, "Player.Ability.DefaultAttack");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Blink, "Player.Ability.Blink");
	
	/*Pleyer State Tahgs*/
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Invincible, "Player.State.Invincible"); // 무적 상태
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Dead, "Player.State.Dead");
	
	/* Player Event Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_DefaultAttack, "Player.Event.DefaultAttack");
	UE_DEFINE_GAMEPLAY_TAG(Player_Event_Blink, "Player.Event.Blink");

	/* Player Cooldown Tags */
	UE_DEFINE_GAMEPLAY_TAG(Player_Cooldown_Blink, "Player.Cooldown.Blink");
	
#pragma  region Enemy Tags Section
	/*Enemy Ability Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_NormalAttack, "Enemy.Ability.NormalAttack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Dead, "Enemy.Ability.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Spawn, "Enemy.Ability.Spawn");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Groggy, "Enemy.Ability.Groggy");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Idle, "Enemy.Ability.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Move, "Enemy.Ability.Move");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Pattern1, "Enemy.Ability.Pattern1");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Pattern2, "Enemy.Ability.Pattern2");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Pattern3, "Enemy.Ability.Pattern3");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Ability_Pattern4, "Enemy.Ability.Pattern4");
	
	/*Enemy State Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Idle, "Enemy.State.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Move, "Enemy.State.Move");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Chase, "Enemy.State.Chase");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Attack, "Enemy.State.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Dead, "Enemy.State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Spawn, "Enemy.State.Spawn");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Groggy, "Enemy.State.Groggy");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Phase2, "Enemy.State.Phase2");
	
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Pattern1, "Enemy.State.Pattern1");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Pattern2, "Enemy.State.Pattern2");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Pattern3, "Enemy.State.Pattern3");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Pattern4, "Enemy.State.Pattern4");
	
	/*Enemy Tier Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Tier_Normal, "Enemy.Tier.Normal");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Tier_Elite, "Enemy.Tier.Elite");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Tier_Boss, "Enemy.Tier.Boss");
	
	/*Enemy Event Tags*/
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_NormalAttack, "Enemy.Event.NormalAttack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_HealthDepleted, "Enemy.Event.HealthDepleted");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_Death,"Enemy.Event.Death");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_Groggy,"Enemy.Event.Groggy");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_PhaseChanged,"Enemy.Event.PhaseChanged");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_Idle, "Enemy.Event.Idle");
#pragma  endregion

	/* Skill Ability Tags */
	UE_DEFINE_GAMEPLAY_TAG(Skill_Ability_IceSpear, "Skill.Ability.IceSpear");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Ability_Lightning, "Skill.Ability.Lightning");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Ability_Explosion, "Skill.Ability.Explosion");

	/* Skill Event Tags */
	UE_DEFINE_GAMEPLAY_TAG(Skill_Event_IceSpear, "Skill.Event.IceSpear");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Event_Lightning, "Skill.Event.Lightning");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Event_Explosion, "Skill.Event.Explosion");

	/* Skill Upgrade Type Tags */
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_Damage, "Upgrade.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_CoolTime, "Upgrade.CoolTime");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_ProjectileNum, "Upgrade.ProjectileNum");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_Penetration, "Upgrade.Penetration");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_Radius, "Upgrade.Radius");
	UE_DEFINE_GAMEPLAY_TAG(Upgrade_Duration, "Upgrade.Duration");

	/* Shared Tags */
	UE_DEFINE_GAMEPLAY_TAG(Shared_Ability_DrawDamageNumber, "Shared.Ability.DrawDamageNumber");
	UE_DEFINE_GAMEPLAY_TAG(Shared_Event_DrawDamageNumber, "Shared.Event.DrawDamageNumber");
	UE_DEFINE_GAMEPLAY_TAG(Shared_State_Init, "Shared.State.Init");


	/* TEST Tags */
	UE_DEFINE_GAMEPLAY_TAG(TEST_Ability_HpIncrease, "TEST.Ability.HpIncrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Ability_HpDecrease, "TEST.Ability.HpDecrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Ability_MaxHpIncrease, "TEST.Ability.MaxHpIncrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Ability_MaxHpDecrease, "TEST.Ability.MaxHpDecrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Event_HpIncrease, "TEST.Event.HpIncrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Event_HpDecrease, "TEST.Event.HpDecrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Event_MaxHpIncrease, "TEST.Event.MaxHpIncrease");
	UE_DEFINE_GAMEPLAY_TAG(TEST_Event_MaxHpDecrease, "TEST.Event.MaxHpDecrease");
	
	/* GameplayCue Tags */
	//UE_DEFINE_GAMEPLAY_TAG(GameplayCue_HitFlash,"GameplayCue.HitFlash");
	
	/* Hit Result Tags */
	UE_DEFINE_GAMEPLAY_TAG(Hit_Critical, "Hit.Critical");
	
	/* Data Tags */
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
}
