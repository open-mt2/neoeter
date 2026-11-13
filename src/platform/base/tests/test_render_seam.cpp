#include <support/TestAssert.h>

#include <cstring>

#include <Platform/DirectXMathCompat.h>
#include <Platform/RenderBackend.h>

// T1.7 — renderer seam contract.
//
// The façade must describe state in value snapshots that carry no GL types, and
// a draw without vertex data must stay a no-op (it must not reach the backend,
// so this test needs no GPU or GL context).

int main()
{
	TEST_SECTION("RenderHandle is opaque");

	RenderHandle none{0, 0};
	TEST_ASSERT(!none.IsValid());

	RenderHandle texture{3, 1};
	TEST_ASSERT(texture.IsValid());
	TEST_ASSERT(texture.id == 3);
	TEST_ASSERT(texture.generation == 1);
	TEST_PASS("invalid handle is id 0, valid handles carry a generation");

	TEST_SECTION("RenderState is a value snapshot");

	RenderState state{};
	TEST_ASSERT(state.fvf == 0);
	TEST_ASSERT(state.viewportWidth == 0);

	state.fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	state.zEnable = true;
	state.stages[0].colorOp = D3DTOP_MODULATE;
	state.textures[1] = RenderHandle{7, 2};

	RenderState copy = state;
	TEST_ASSERT(copy.fvf == state.fvf);
	TEST_ASSERT(copy.zEnable);
	TEST_ASSERT(copy.stages[0].colorOp == D3DTOP_MODULATE);
	TEST_ASSERT(copy.textures[1].id == 7);
	TEST_ASSERT(copy.textures[1].generation == 2);
	TEST_PASS("snapshot copies every stage, sampler and handle");

	TEST_SECTION("DrawCall carries its state");

	DrawCall call{};
	call.state.zFunc = D3DCMP_LESS;
	call.primitiveType = D3DPT_TRIANGLELIST;
	call.vertexCount = 3;
	TEST_ASSERT(call.state.zFunc == D3DCMP_LESS);
	TEST_ASSERT(call.vertexData == nullptr);
	TEST_PASS("draw and its snapshot travel together");

	TEST_SECTION("D3D9 facade tracks state without GL");

	IDirect3DDevice9 device;
	device.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
	device.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	device.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	DWORD value = 0;
	TEST_ASSERT(device.GetRenderState(D3DRS_CULLMODE, &value) == S_OK);
	TEST_ASSERT(value == D3DCULL_NONE);
	TEST_ASSERT(device.GetRenderState(D3DRS_ALPHABLENDENABLE, &value) == S_OK);
	TEST_ASSERT(value == 1);
	TEST_ASSERT(device.GetSamplerState(0, D3DSAMP_ADDRESSU, &value) == S_OK);
	TEST_ASSERT(value == D3DTADDRESS_CLAMP);
	TEST_ASSERT(device.GetTextureStageState(0, D3DTSS_COLOROP, &value) == S_OK);
	TEST_ASSERT(value == D3DTOP_SELECTARG1);
	TEST_PASS("state round-trips through the facade");

	TEST_SECTION("lights and materials round-trip through the facade");

	{
		D3DLIGHT9 light;
		std::memset(&light, 0, sizeof(light));
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Diffuse.r = 1.0f;
		light.Diffuse.g = 0.5f;
		light.Diffuse.b = 0.25f;
		light.Diffuse.a = 1.0f;
		light.Ambient.r = 0.1f;
		light.Ambient.g = 0.2f;
		light.Ambient.b = 0.3f;
		light.Ambient.a = 1.0f;
		light.Direction.x = 0.25f;
		light.Direction.y = 0.5f;
		light.Direction.z = -0.75f;

		TEST_ASSERT(device.SetLight(1, &light) == S_OK);
		TEST_ASSERT(device.LightEnable(1, TRUE) == S_OK);

		D3DLIGHT9 stored;
		std::memset(&stored, 0, sizeof(stored));
		TEST_ASSERT(device.GetLight(1, &stored) == S_OK);
		TEST_ASSERT(stored.Type == D3DLIGHT_DIRECTIONAL);
		TEST_ASSERT(stored.Diffuse.r == 1.0f && stored.Diffuse.g == 0.5f && stored.Diffuse.b == 0.25f);
		TEST_ASSERT(stored.Direction.x == 0.25f && stored.Direction.y == 0.5f && stored.Direction.z == -0.75f);
		TEST_ASSERT(device.SetLight(8, &light) == E_INVALIDARG);
		TEST_ASSERT(device.LightEnable(8, TRUE) == E_INVALIDARG);

		D3DMATERIAL9 material;
		std::memset(&material, 0, sizeof(material));
		material.Diffuse.r = 0.8f;
		material.Ambient.g = 0.6f;
		material.Emissive.b = 0.4f;
		material.Power = 5.0f;
		TEST_ASSERT(device.SetMaterial(&material) == S_OK);

		D3DMATERIAL9 storedMaterial;
		std::memset(&storedMaterial, 0, sizeof(storedMaterial));
		TEST_ASSERT(device.GetMaterial(&storedMaterial) == S_OK);
		TEST_ASSERT(storedMaterial.Diffuse.r == 0.8f);
		TEST_ASSERT(storedMaterial.Ambient.g == 0.6f);
		TEST_ASSERT(storedMaterial.Emissive.b == 0.4f);
		TEST_ASSERT(storedMaterial.Power == 5.0f);

		device.SetRenderState(D3DRS_SPECULARENABLE, TRUE);
		DWORD specular = 0;
		TEST_ASSERT(device.GetRenderState(D3DRS_SPECULARENABLE, &specular) == S_OK);
		TEST_ASSERT(specular == 1);
		TEST_PASS("light, material and specular state survive the facade");
	}

	TEST_SECTION("lighting can be disabled for a render section");

	TEST_ASSERT(!Platform_IsLightingDisabled());
	Platform_PushLightingDisabled();
	TEST_ASSERT(Platform_IsLightingDisabled());
	Platform_PushLightingDisabled();
	TEST_ASSERT(Platform_IsLightingDisabled());
	Platform_PopLightingDisabled();
	TEST_ASSERT(Platform_IsLightingDisabled());
	Platform_PopLightingDisabled();
	TEST_ASSERT(!Platform_IsLightingDisabled());
	Platform_PopLightingDisabled();
	TEST_ASSERT(!Platform_IsLightingDisabled());
	TEST_PASS("the lighting scope nests and never underflows");

	TEST_SECTION("empty draws stay no-ops");

	TEST_ASSERT(device.DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1) == S_OK);
	TEST_ASSERT(device.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 0, 0, 1) == S_OK);
	TEST_ASSERT(device.DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, nullptr, 0) == S_OK);
	TEST_ASSERT(device.DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 0, 1, nullptr,
											  D3DFMT_INDEX16, nullptr, 0) == S_OK);
	TEST_PASS("draws without vertex data return S_OK");

	TEST_SECTION("device viewport comes from the presentation parameters");

	{
		IDirect3D9 d3d;
		D3DPRESENT_PARAMETERS present;
		std::memset(&present, 0, sizeof(present));
		present.BackBufferWidth = 1920;
		present.BackBufferHeight = 1080;

		IDirect3DDevice9* created = nullptr;
		TEST_ASSERT(d3d.CreateDevice(0, D3DDEVTYPE_HAL, nullptr, 0, &present, &created) == S_OK);
		TEST_ASSERT(created != nullptr);

		D3DVIEWPORT9 viewport;
		std::memset(&viewport, 0, sizeof(viewport));
		TEST_ASSERT(created->GetViewport(&viewport) == S_OK);
		TEST_ASSERT(viewport.Width == 1920);
		TEST_ASSERT(viewport.Height == 1080);

		delete created;
		TEST_PASS("viewport starts at the requested back-buffer size");
	}

	TEST_SECTION("textures keep their D3D9 description");

	{
		IDirect3DTexture9 texture(4, 8, D3DFMT_A8R8G8B8);
		TEST_ASSERT(texture.GetWidth() == 4);
		TEST_ASSERT(texture.GetHeight() == 8);
		TEST_ASSERT(texture.GetFormat() == D3DFMT_A8R8G8B8);
		TEST_ASSERT(texture.HasMipMaps() == false);
		TEST_PASS("texture metadata stays on the facade side");
	}

	return 0;
}
