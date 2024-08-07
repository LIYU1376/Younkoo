﻿#pragma once
#include <SDK.hpp>
#include <wrapper/versions/1_18_1/net/minecraft/world/level/ChunkPos.h>
BEGIN_1_18_1


BEGIN_KLASS_DEF(SectionPos, return SRGParser::get().getObfuscatedClassName("net/minecraft/core/SectionPos"))

JNI::Method<ChunkPos, JNI::NOT_STATIC, DECLARE_NAME(
	return SRGParser::get().getObfuscatedMethodName("net/minecraft/core/SectionPos", "chunk", "()Lnet/minecraft/world/level/ChunkPos;").first
)> chunk{ *this };

END_KLASS_DEF();
END_1_18_1