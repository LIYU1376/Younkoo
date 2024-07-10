﻿#pragma once
#include <SDK.hpp>
#include "wrapper/Object.h"
#include "wrapper/net/minecraft/entity/player/EntityPlayer.h"
#include <wrapper/net/minecraft/block/state/BlockState.h>

BEGIN_WRAP
class World :public Object {
public:
	using Object::Object;
	std::vector<EntityPlayer> getPlayerList();
	BlockState getBlockState(const Math::Vector3D& pos);
};

END_WRAP