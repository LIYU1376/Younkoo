﻿#pragma once

#include <SDK.hpp>
#include "entity/EntityPlayerSP.h"
#include "../world/HitResult.h"
BEGIN_KLASS_DEF(Minecraft, return SRGParser::get().getObfuscatedClassName("net/minecraft/client/Minecraft"))
JNI::Field<EntityPlayerSP, JNI::NOT_STATIC> thePlayer{ []() {
	return SRGParser::get().getObfuscatedFieldName(SRGParser::get().getObfuscatedClassName("net/minecraft/client/Minecraft"),"player");
	} ,*this };
JNI::Field<HitResult, JNI::NOT_STATIC> mouseOver{ []() {

	if (SRGParser::get().GetVersion() == Versions::FORGE_1_18_1) {
		return SRGParser::get().getObfuscatedFieldName(SRGParser::get().getObfuscatedClassName("net/minecraft/client/Minecraft"), "hitResult");
	}
	else {
		return SRGParser::get().getObfuscatedFieldName(SRGParser::get().getObfuscatedClassName("net/minecraft/client/Minecraft"),"objectMouseOver");
}
} ,*this };
JNI::Method<Minecraft, JNI::STATIC> getMinecraft{ []() {
	return SRGParser::get().GetVersion() == Versions::FORGE_1_18_1 ?
		SRGParser::get().getObfuscatedMethodName("net/minecraft/client/Minecraft", "getInstance", "()Lnet/minecraft/client/Minecraft;").first :
		SRGParser::get().getObfuscatedMethodName("net/minecraft/client/Minecraft", "getMinecraft", "()Lnet/minecraft/client/Minecraft;").first;
	} ,*this };
END_KLASS_DEF();

