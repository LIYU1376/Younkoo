﻿#include "CommonData.h"
#include <base/features/modules/ModuleManager.h>
#include <base/Younkoo.hpp>

#include <wrapper/net/minecraft/client/Minecraft.h>

CommonData::CommonData()
{
	REGISTER_EVENT(EventRender3D, CommonData::onRender3D);
	renderPos = {};
	renderPartialTicks = 0;
	fov = 0;
	projection = {};
	modelView = {};
	eyeHeight = 0;
}
void CommonData::onRender3D(const EventRender3D& e)
{
	renderPartialTicks = e.TICK_DELTA;
	projection = e.PROJECTION_MATRIX;
	modelView = e.MODELVIEW_MATRIX;
	cameraPos = e.CAMERA_POS;
}
#include <base/render/Renderer.hpp>

void CommonData::onUpdate()
{
	mc = Wrapper::Minecraft::getMinecraft();
	if (mc.isNULL()) return;
	auto settings = mc.getSettings();
	if (settings.isNULL()) return;
	this->fov = settings.getFov();
	auto player = mc.getPlayer();
	if (player.isNULL()) return;
	float ySubtractValue = 3.4;
	if (player.isSneaking())
		ySubtractValue -= .175f;

	inChatScreen = mc.isInChatGui();

	this->eyeHeight = player.getEyeHeight();
	this->renderPos = /*player.getPosition(renderPartialTicks) -*/ cameraPos /*+ Math::Vector3D{ 0, ySubtractValue - eyeHeight, 0 }*/;

	const auto& renderContext = Renderer::get().renderContext;
	this->viewport = { 0, 0, renderContext.winSize.first, renderContext.winSize.second };

}



