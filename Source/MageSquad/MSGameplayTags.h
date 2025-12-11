#pragma once

#include "NativeGameplayTags.h"

namespace MSGameplayTags
{
	/* Input Tags */
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);

	/*Enemy State Tags*/
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Chase);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Attack);
	MAGESQUAD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Dead);
}
