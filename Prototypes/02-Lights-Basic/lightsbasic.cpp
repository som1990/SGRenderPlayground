/*
 * Copyright 2025 Soumitra Goswami. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "common.h"
#include "camera.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"
#include <debugdraw/debugdraw.h>

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

	
	struct Uniforms // Constant Buffer
	{
		enum {NumVec4 = 4};
		bgfx::UniformHandle u_params;
		
		union
		{
			struct 
			{
				/*0-1*/	struct { Material gold; };
				/*2*/	struct { float lightPos[3];  float influenceRadiusMin; };
				/*3*/	struct { float lightColor[3]; float influenceRadiusMax; };
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

	struct Settings
	{
		// Material Properties
		float m_albedo[3];
		float m_roughness;
		float m_f0[3];
		float m_metallic;

		// Point Light Properties
		float m_lightColor[3];
		float m_lightLatAngle;
		float m_lightLongAngle;
		float m_lightDistance;
		float m_influenceRadiusMin;
		float m_influenceRadiusMax;

		Settings()
		{
			//Default material parameters for gold
			m_albedo[0] = 1.0f; m_albedo[1] = 0.782f; m_albedo[2] = 0.344f;
			m_roughness = 0.2f;
			m_f0[0] = 1.02f; m_f0[1] = 0.782f; m_f0[2] = 0.344f;
			m_metallic = 1.0f;

			//Default Light Properties
			m_lightColor[0] = 1.0f; m_lightColor[1] = 1.0f; m_lightColor[2] = 1.0f;
			m_lightLatAngle = 0.0f; m_lightLongAngle = bx::toRad(30.0f); m_lightDistance = 20.0f;
			m_influenceRadiusMin = 1.0; m_influenceRadiusMax = 50.0f;

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
		
		Uniforms m_uniforms;

		float m_groundTransform[16];
		float m_fovY;
		float m_lightPos[3];
		float m_lightLatAngle, m_lightLongAngle;

		bgfx::ProgramHandle m_program;
		bgfx::UniformHandle u_time;

		//UI 
		Settings m_settings;

		LightsBasic(const char* _name, const char* _description, const char* _url)
			: entry::AppI(_name, _description, _url)
		{
		}

		void updateUniforms(uint32_t _pass, float _time)
		{  
			//Material Attributes
			m_uniforms.m_params[0] = m_settings.m_albedo[0]; m_uniforms.m_params[1] = m_settings.m_albedo[1]; m_uniforms.m_params[2] = m_settings.m_albedo[2]; 
			m_uniforms.m_params[3] = m_settings.m_roughness;
			m_uniforms.m_params[4] = m_settings.m_f0[0]; m_uniforms.m_params[5] = m_settings.m_f0[1]; m_uniforms.m_params[6] = m_settings.m_f0[2];
			m_uniforms.m_params[7] = m_settings.m_metallic;

			m_uniforms.lightColor[0] = m_settings.m_lightColor[0]; m_uniforms.lightColor[1] = m_settings.m_lightColor[1]; m_uniforms.lightColor [2] = m_settings.m_lightColor[2];
			m_uniforms.influenceRadiusMax = m_settings.m_influenceRadiusMax;
			m_uniforms.influenceRadiusMin = m_settings.m_influenceRadiusMin;
			
			m_lightLatAngle = m_settings.m_lightLatAngle;
			m_lightLongAngle = m_settings.m_lightLongAngle;
			bx::Vec3 lightPos = bx::normalize(bx::fromLatLong(m_lightLatAngle, -m_lightLongAngle)) ;
			lightPos = bx::mul(lightPos, m_settings.m_lightDistance);


			m_uniforms.lightPos[0] = lightPos.x; m_uniforms.lightPos[1] = lightPos.y; m_uniforms.lightPos[2] = lightPos.z;
			m_lightPos[0] = lightPos.x; m_lightPos[1] = lightPos.y; m_lightPos[2] = lightPos.z;


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

				m_uniforms.init();
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


			ddInit();

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

			// Direct Draw Cleanup
			ddShutdown();

			meshUnload(m_ground);
			
			// Cleanup
			bgfx::destroy(m_program);
			bgfx::destroy(u_time);
			m_uniforms.destroy();

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
				ImGui::SetNextWindowPos(
					ImVec2(m_width - m_width / 4.0 - 10.f, 10.f)
					, ImGuiCond_FirstUseEver
				);
				ImGui::SetNextWindowSize(
					ImVec2(m_width / 4.0f, m_height / 4.0f)
					, ImGuiCond_FirstUseEver
				);

				ImGui::Begin("Settings", NULL, 0);
				ImGui::Text("This example shows basic lighting with a single point light source.");
				ImGui::Separator();

				ImGui::Text("Material Parms");
				ImGui::SliderFloat("Roughness", &m_settings.m_roughness, 0.0f, 1.0f);
				ImGui::SliderFloat("Metallic", &m_settings.m_metallic, 0.0f, 1.0f);
				ImGui::ColorEdit3("Albedo", &m_settings.m_albedo[0], ImGuiColorEditFlags_NoSidePreview);
				ImGui::SliderFloat3("f0", &m_settings.m_f0[0], 0.0, 2.0);
				ImGui::Separator();

				ImGui::Text("Light Parms");
				ImGui::ColorEdit3("Color", &m_settings.m_lightColor[0], ImGuiColorEditFlags_NoSidePreview);
				ImGui::SliderAngle("Light Lat Angle", &m_settings.m_lightLatAngle, -45.0f, 45.0f);
				ImGui::SliderAngle("Light Long Angle", &m_settings.m_lightLongAngle, 0.0f, 90.0f);
				ImGui::SliderFloat("Light Distance", &m_settings.m_lightDistance, 1.0f, 60.0f);
				ImGui::SliderFloat("Light Min Radius", &m_settings.m_influenceRadiusMin, 0.5f, 10.0f);
				ImGui::SliderFloat("Light Max Radius", &m_settings.m_influenceRadiusMax, 1.0f, 100.0f);
				ImGui::End();

				imguiEndFrame();

				// Update camera
				cameraUpdate(deltaTime * 0.15f, m_mouseState, ImGui::MouseOverArea());

				
				// Render Scene
				bgfx::setViewRect(RENDER_PASS_MAIN, 0, 0, uint16_t(m_width), uint16_t(m_height));
				bgfx::touch(RENDER_PASS_MAIN);
				bgfx::setFrameUniform(u_time, &deltaTime);

				updateUniforms(RENDER_PASS_MAIN, deltaTime);
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

				DebugDrawEncoder dde;

				dde.begin(0);
				dde.push();
					bx::Sphere sphere = bx::Sphere{ {m_lightPos[0], m_lightPos[1], m_lightPos[2]} , 0.5f};
					dde.setColor(0xff0000ff);
					dde.setWireframe(true);
					dde.draw(sphere);
					dde.setWireframe(false);
				dde.pop();
				dde.end();



				m_uniforms.submit();

				// Draw ground
 				meshSubmit(m_ground, RENDER_PASS_MAIN, m_program, mtx);

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