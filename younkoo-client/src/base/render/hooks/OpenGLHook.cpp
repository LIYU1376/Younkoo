#include "OpenGLHook.hpp"

#include "../Renderer.hpp"
#include "../nano/NanovgHelper.hpp"


typedef bool(__stdcall* template_wglSwapBuffers) (HDC hdc);
static TitanHook<template_wglSwapBuffers> wglSwapBuffersHook;
static LPVOID wglSwapBuffers{};

#include <tuple>

static auto getWindowSize(const HWND& window) {
	RECT windowRect;
	GetWindowRect(window, &windowRect);
	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;
	return std::make_tuple(windowWidth, windowHeight, windowWidth / windowHeight);
}


#include <mutex>
#include <memory>
#include <GL/glew.h>
#include <nanovg.h>
static auto flagInit = std::make_unique<std::once_flag>();

bool OpenGLHook::Detour_wglSwapBuffers(_In_ HDC hdc) {

	auto &renderer = Renderer::get();
	renderer.OriginalGLContext = wglGetCurrentContext();
	renderer.HandleDeviceContext = hdc;
	renderer.HandleWindow = WindowFromDC(hdc);
	
	if(!renderer.Initialized){

		renderer.MenuGLContext = wglCreateContext(hdc);
		wglCopyContext(renderer.OriginalGLContext, renderer.MenuGLContext,GL_ALL_ATTRIB_BITS); 
		wglMakeCurrent(hdc, renderer.MenuGLContext);
		NanoVGHelper::InitContext(renderer.HandleWindow);
		wglMakeCurrent(renderer.HandleDeviceContext, renderer.OriginalGLContext);
		/*



		GLint viewport[4]{};
		glGetIntegerv(GL_VIEWPORT, viewport);
		glViewport(0, 0, viewport[2], viewport[3]);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);*/

		renderer.Initialized = true;
	}
	else {

		wglCopyContext(renderer.OriginalGLContext, renderer.MenuGLContext, GL_ALL_ATTRIB_BITS);
	}
	
	wglMakeCurrent(renderer.HandleDeviceContext, renderer.MenuGLContext);

	

	GLint viewport[4]{};
	glGetIntegerv(GL_VIEWPORT, viewport);
	int winWidth = viewport[2];
	int winHeight = viewport[3];

	auto& vg = NanoVGHelper::Context;
	nvgBeginFrame(vg, winWidth, winHeight, /*devicePixelRatio*/ 1.0f);
	nvgSave(vg);


	nvgBeginPath(vg);
	nvgRect(vg, winWidth / static_cast<float>(2) - 50, winHeight / static_cast<float>(2) - 50, 100, 100); // ���ľ���
	nvgFillColor(vg, nvgRGBA(220, 160, 0, 200)); // ��ɫ���
	nvgFill(vg);
	nvgClosePath(vg);

	nvgBeginPath(vg);
	nvgFillColor(vg, nvgRGB(255, 255, 255));
	nvgFontFaceId(vg, NanoVGHelper::fontHarmony);
	nvgFontSize(vg, 66.f);
	nvgText(vg, 50, 50, "Younkoo", NULL);
	nvgClosePath(vg);
	nvgRestore(vg);
	nvgEndFrame(vg);

	//glDepthFunc(GL_LEQUAL);

	wglMakeCurrent(renderer.HandleDeviceContext, renderer.OriginalGLContext);
	return wglSwapBuffersHook.GetOrignalFunc()(hdc);
}


bool OpenGLHook::Init() 
{

	static auto gl = GetModuleHandleW(L"opengl32.dll");

	if (gl) wglSwapBuffers = (LPVOID)GetProcAddress(gl, "wglSwapBuffers");

	if (!wglSwapBuffers) return false;

	wglSwapBuffersHook.InitHook(wglSwapBuffers, Detour_wglSwapBuffers);
	wglSwapBuffersHook.SetHook();
	return true;
}

bool OpenGLHook::Clean()
{
	wglSwapBuffersHook.RemoveHook();
	return true;
}
