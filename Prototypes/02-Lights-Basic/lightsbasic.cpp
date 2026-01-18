/*
 * Copyright 2025 Soumitra Goswami. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "common.h"
#include "camera.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"

namespace
{
#define RENDER_PASS_MAIN	0

	struct Material
	{
		float albedo[3];
		float roughness;
		float f0[3];
		float metallic;
	};


	struct Uniforms // Constant Buffers
	{
		enum {NumVec4 = 2};
		bgfx::UniformHandle u_params;
		
		union
		{
			struct {
				/*0-1*/	struct { Material gold; };
			};
			float m_params[NumVec4 * 4];
		};

		void init()
		{
			u_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, NumVec4);
		}

		void submit()
		{
			bgfx::setUniform(u_params, m_params, NumVec4);
		}

		void destroy()
		{
			bgfx::destroy(u_params);
		}
	};

	class LightsBasic : public entry::AppI
	{
	public:
		entry::MouseState m_mouseState;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_debug;
		uint32_t m_reset;

		int64_t m_timeOffset;
		Mesh* m_ground;
		
		float m_groundTransform[16];
		float m_fovY;
		bgfx::ProgramHandle m_program;
		bgfx::UniformHandle u_time;

		LightsBasic(const char* _name, const char* _description, const char* _url)
			: entry::AppI(_name, _description, _url)
		{
		}

		void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
		{
			Args args(_argc, _argv);
			m_width = _width;
			m_height = _height;
			m_debug = BGFX_DEBUG_NONE;
			m_reset = BGFX_RESET_VSYNC;

			bgfx::Init init;
			init.type = args.m_type;
			init.vendorId = args.m_pciId;
			init.platformData.nwh = entry::getNativeWindowHandle(entry::kDefaultWindowHandle);
			init.platformData.ndt = entry::getNativeDisplayHandle();
			init.platformData.type = entry::getNativeWindowHandleType();
			init.resolution.width = m_width;
			init.resolution.height = m_height;
			init.resolution.reset = m_reset;
			bgfx::init(init);
			// Enable debug Text
			bgfx::setDebug(m_debug);
			
			// Setup Main pass
			{
				// Set view 0 clear state.
				bgfx::setViewClear(RENDER_PASS_MAIN
					, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
					, 0x303030ff
					, 1.0f
					, 0
				);
				// Create program from shaders
				u_time = bgfx::createUniform("u_time", bgfx::UniformFreq::Frame, bgfx::UniformType::Vec4);
				m_program = loadProgram("vs_lightsbasic", "fs_lightsbasic");
				m_ground = meshLoad("meshes/cube.bin");
				m_timeOffset = bx::getHPCounter();
			}

			// Initialize camera
			cameraCreate();
			cameraSetPosition({ 0.0f, 3.0f, -6.0f });
			cameraSetVerticalAngle(-0.3f);
			m_fovY = 60.0f;

			// Set ground transform
			float mtxScale[16];
			const float scale = 10.0f;
			bx::mtxScale(mtxScale, scale, scale, scale);

			float mtxTranslate[16];
			bx::mtxTranslate(mtxTranslate, 0.0f, -10.0f, 0.0f);
			bx::mtxMul(m_groundTransform, mtxScale, mtxTranslate);

			imguiCreate();
		}


		int shutdown() override
		{
			cameraDestroy();
			imguiDestroy();
			meshUnload(m_ground);
			
			// Cleanup
			bgfx::destroy(m_program);
			bgfx::destroy(u_time);
			// Shutdown bgfx
			bgfx::shutdown();

			return 0;
		}

		bool update() override
		{
			if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState))
			{
				// Update frame timer
				const double freq = double(bx::getHPFrequency());
				int64_t now = bx::getHPCounter();
				static int64_t last = now;
				const float deltaTime = float(now - last) / freq;
				last = now;
				// Update camera
				cameraUpdate(deltaTime * 0.15f, m_mouseState, ImGui::MouseOverArea());

				bgfx::setViewRect(RENDER_PASS_MAIN, 0, 0, uint16_t(m_width), uint16_t(m_height));
				bgfx::touch(RENDER_PASS_MAIN);
				bgfx::setFrameUniform(u_time, &deltaTime);

				// Set up matrices for view
				float view[16];
				cameraGetViewMtx(view);
				float proj[16];
				bx::mtxProj(proj, m_fovY, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
				bgfx::setViewTransform(RENDER_PASS_MAIN, view, proj);
				
				float mtxScale[16];
				const float scale = 10.0f;
				bx::mtxScale(mtxScale, scale, scale, scale);
				float mtx[16];
				float mtxTranslate[16];
				bx::mtxTranslate(mtxTranslate, 0.0f, -10.0f, 0.0f);
				bx::mtxMul(mtx, mtxScale, mtxTranslate);

				// Draw ground
 				meshSubmit(m_ground, RENDER_PASS_MAIN, m_program, mtx);

				//draw UI
				imguiBeginFrame(m_mouseState.m_mx
					, m_mouseState.m_my
					, (m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
					| (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
					| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
					, m_mouseState.m_mz
					, uint16_t(m_width)
					, uint16_t(m_height)
				);
				showExampleDialog(this);
				imguiEndFrame();
				
				// Submit frame
				bgfx::frame();
				return true;
			}
			return false;
		}
	};

}

ENTRY_IMPLEMENT_MAIN(
	  LightsBasic
	, "lightsbasic"
	, "Basic lighting example."
	, ""
);