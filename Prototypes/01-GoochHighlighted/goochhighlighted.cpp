/*
 * Copyright 2025 Soumitra Goswami. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "common.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"


namespace
{

#define RENDER_PASS_MAIN	0


struct Settings
{
	float m_warmColor[3];
	float m_coolColor[3];
	float m_highlightColor[3];
	float m_surfaceColor[3];

	Settings()
	{
		// Default Gooch shading parameters
		m_surfaceColor[0] = 0.6f; m_surfaceColor[1] = 0.6f; m_surfaceColor[2] = 0.6f; // Grey
		m_warmColor[0] = 0.3f; m_warmColor[1] = 0.3f; m_warmColor[2] = 0.0f; // Yellowish
		m_coolColor[0] = 0.0f; m_coolColor[1] = 0.0f; m_coolColor[2] = 0.55f; // Bluish
		m_highlightColor[0] = 1.0f; m_highlightColor[1] = 1.0f; m_highlightColor[2] = 1.0f; // White
	}
};

struct Uniforms // Constant Buffers
{
	enum { NumVec4 = 4 };

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

	union
	{
		struct
		{
			/* 0	*/	struct { float m_time; float m_warmColor[3]; };
			/* 1    */  struct { float m_coolColor[3]; float m_lightDirX; };
			/* 2    */  struct { float m_highlightColor[3]; float m_lightDirY;};
			/* 3    */  struct { float m_surfaceColor[3]; float m_lightDirZ;};
			///* 4    */  struct { float m_lightDir[3]; float m_unused3; };  // Light Dir could be packed into the other 3 float channels to save a float4 slot.
		};	
		float m_params[NumVec4 * 4];
	};
	bgfx::UniformHandle u_params;
};

class GoochHighlighted : public entry::AppI 
{
public:
	entry::MouseState m_mouseState;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;

	int64_t m_timeOffset;
	Mesh* m_mesh;
	bgfx::ProgramHandle m_program;

	Uniforms m_uniforms;

	// UI
	Settings m_Settings;
	float m_lightAngle;

	void updateUniforms(int _pass, float _time)
	{
		m_uniforms.m_time = _time;
		// Gooch shading parameters
		m_uniforms.m_surfaceColor[0] = m_Settings.m_surfaceColor[0];		m_uniforms.m_surfaceColor[1] = m_Settings.m_surfaceColor[1];		m_uniforms.m_surfaceColor[2] = m_Settings.m_surfaceColor[2];
		m_uniforms.m_warmColor[0] = m_Settings.m_warmColor[0];				m_uniforms.m_warmColor[1] = m_Settings.m_warmColor[1];				m_uniforms.m_warmColor[2] = m_Settings.m_warmColor[2];
		m_uniforms.m_coolColor[0] = m_Settings.m_coolColor[0];				m_uniforms.m_coolColor[1] = m_Settings.m_coolColor[1];				m_uniforms.m_coolColor[2] = m_Settings.m_coolColor[2];
		m_uniforms.m_highlightColor[0] = m_Settings.m_highlightColor[0];	m_uniforms.m_highlightColor[1] = m_Settings.m_highlightColor[1];	m_uniforms.m_highlightColor[2] = m_Settings.m_highlightColor[2];
		
		const bx::Vec3 at = { 0.0f, 1.0f,  0.0f };
		const bx::Vec3 eye = { 0.0f, 1.0f, -2.5f };

		// Set view and projection matrix for view 0.
		{
			float view[16];
			bx::mtxLookAt(view, eye, at);

			float proj[16];
			bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
			bgfx::setViewTransform(_pass, view, proj);
		}

		// 60 degree altitude for light
		bx::Vec3 pos = bx::normalize(bx::fromLatLong(m_lightAngle, bx::toRad(30)));
		pos = bx::normalize(pos);
		// Light direction
		m_uniforms.m_lightDirX = -pos.x;
		m_uniforms.m_lightDirY = -pos.y;
		m_uniforms.m_lightDirZ = -pos.z;
	}


	GoochHighlighted(const char* _name, const char* _description, const char* _url)
		: entry::AppI(_name, _description, _url), m_lightAngle(bx::toRad(0.0f))
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
			// Set view 0 clear sate.
			bgfx::setViewClear(RENDER_PASS_MAIN
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x303030ff
				, 1.0f
				, 0
			);

			//u_time = bgfx::createUniform("u_time", bgfx::UniformFreq::Frame, bgfx::UniformType::Vec4);
			m_uniforms.init();
			// Create program from shaders
			m_program = loadProgram("vs_goochhighlighted", "fs_goochhighlighted");
			m_mesh = meshLoad("meshes/bunny.bin");
			m_timeOffset = bx::getHPCounter();

		}
		imguiCreate();


	}

	int shutdown() override
	{
		imguiDestroy();

		meshUnload(m_mesh);

		// Cleanup
		bgfx::destroy(m_program);
		// bgfx::destroy(u_time);
		m_uniforms.destroy();

		// Shutdown bgfx
		bgfx::shutdown();

		return 0;

	}

	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState))
		{
		
			// Set main render pass 
			bgfx::setViewRect(RENDER_PASS_MAIN, 0, 0, uint16_t(m_width), uint16_t(m_height));

			bgfx::touch(RENDER_PASS_MAIN);

			float time = (float)((bx::getHPCounter() - m_timeOffset) / double(bx::getHPFrequency()));
			//bgfx::setFrameUniform(u_time, &time);

			updateUniforms(RENDER_PASS_MAIN, time);

			// Set view 0 default viewport.
			bgfx::setViewRect(RENDER_PASS_MAIN, 0, 0, uint16_t(m_width), uint16_t(m_height));
			

			// Update model matrix. Rotate over time.
			float mtx[16];
			bx::mtxRotateXY(mtx, 0.0f, time * 0.37f);

			m_uniforms.submit();
			meshSubmit(m_mesh, RENDER_PASS_MAIN, m_program, mtx);

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
				ImVec2(m_width - m_width / 4.0f - 10.0f, 10.0f)
				, ImGuiCond_FirstUseEver
			);
			ImGui::SetNextWindowSize(
				ImVec2(m_width / 4.0f, m_height / 1.3f)
				, ImGuiCond_FirstUseEver
			);
			ImGui::Begin("Settings", NULL, 0);

			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
			ImGui::Text("Light Parms");
			ImGui::SliderAngle("Sun Angle (Azimuth)", &m_lightAngle, -45.0f, 45.0f);
			ImGui::ColorEdit3("Gooch Warm Color", &m_Settings.m_warmColor[0], ImGuiColorEditFlags_NoSidePreview);
			ImGui::ColorEdit3("Gooch Cool Color", &m_Settings.m_coolColor[0],  ImGuiColorEditFlags_NoSidePreview);
			ImGui::ColorEdit3("Highlight Color", &m_Settings.m_highlightColor[0], ImGuiColorEditFlags_NoSidePreview);
			

			ImGui::Separator();
			ImGui::Text("Surface Params");
			ImGui::ColorEdit3("Surface Color", &m_Settings.m_surfaceColor[0], ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview);

			ImGui::End();

			imguiEndFrame();

			bgfx::frame();
			return true;

		}
		return false;
	}
};


}// namespace


ENTRY_IMPLEMENT_MAIN(
	GoochHighlighted,
	"SGTestBed 01-gooshhighlighted",
	"Loading Mesh and applying gooch shading",
	""
);
