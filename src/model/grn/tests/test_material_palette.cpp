#include <Neoeter/Model/Grn/StdAfx.h>
#include <Neoeter/Model/Grn/Material.h>
#include <Neoeter/Model/Grn/Util.h>

#include <Neoeter/Render/ResourceManager.h>

#include <support/TestAssert.h>

#include <cstring>

namespace
{

	void setup_shared_texture_material(granny_material* pMaterial, granny_material_map* pMap, granny_material* pMapMaterial, granny_texture* pTexture)
	{
		std::memset(pTexture, 0, sizeof(*pTexture));
		pTexture->FromFileName = "d:/ymir work/pc/warrior/warrior_4-1.dds";

		std::memset(pMapMaterial, 0, sizeof(*pMapMaterial));
		pMapMaterial->Texture = pTexture;

		std::memset(pMap, 0, sizeof(*pMap));
		pMap->Usage = "Diffuse Color";
		pMap->Material = pMapMaterial;

		std::memset(pMaterial, 0, sizeof(*pMaterial));
		pMaterial->Name = "warrior_4-1";
		pMaterial->MapCount = 1;
		pMaterial->Maps = pMap;
	}

	void test_material_data_covers_every_material_sharing_a_texture()
	{
		TEST_SECTION("material data reaches every material sharing a texture");

		CResourceManager resMgr;

		granny_texture texture;
		granny_material mapMaterial;
		granny_material_map map;
		granny_material body;
		granny_material shoulders;

		setup_shared_texture_material(&body, &map, &mapMaterial, &texture);
		setup_shared_texture_material(&shoulders, &map, &mapMaterial, &texture);

		CGrannyMaterialPalette palette;
		TEST_ASSERT(palette.RegisterMaterial(&body) == 0);
		TEST_ASSERT(palette.RegisterMaterial(&shoulders) == 1);

		CGrannyMaterial* bodyBefore = &palette.GetMaterialRef(0);
		CGrannyMaterial* shouldersBefore = &palette.GetMaterialRef(1);

		SMaterialData materialData;
		materialData.pImage = nullptr;
		materialData.isSpecularEnable = TRUE;
		materialData.fSpecularPower = 1.0f;
		materialData.bSphereMapIndex = 0;
		palette.SetMaterialData(texture.FromFileName, materialData);

		TEST_ASSERT(&palette.GetMaterialRef(0) != bodyBefore);
		TEST_ASSERT(&palette.GetMaterialRef(1) != shouldersBefore);
		TEST_PASS("SetMaterialData updates all matching materials");

		bodyBefore = &palette.GetMaterialRef(0);
		shouldersBefore = &palette.GetMaterialRef(1);
		palette.SetMaterialImagePointer(texture.FromFileName, nullptr);

		TEST_ASSERT(&palette.GetMaterialRef(0) != bodyBefore);
		TEST_ASSERT(&palette.GetMaterialRef(1) != shouldersBefore);
		TEST_PASS("SetMaterialImagePointer updates all matching materials");
	}

}

int main()
{
	TEST_SECTION("specular state does not leak between model instances");

	granny_material grannyMaterial;
	std::memset(&grannyMaterial, 0, sizeof(grannyMaterial));
	grannyMaterial.Name = "Material";

	CGrannyMaterialPalette source;
	const DWORD index = source.RegisterMaterial(&grannyMaterial);
	TEST_ASSERT(index == 0);

	CGrannyMaterialPalette first;
	CGrannyMaterialPalette second;
	first.Copy(source);
	second.Copy(source);

	TEST_ASSERT(&first.GetMaterialRef(0) == &second.GetMaterialRef(0));
	TEST_ASSERT(&first.GetMaterialRef(0) == &source.GetMaterialRef(0));

	SMaterialData materialData;
	materialData.pImage = nullptr;
	materialData.isSpecularEnable = TRUE;
	materialData.fSpecularPower = 0.0f;
	materialData.bSphereMapIndex = 1;
	first.SetMaterialData(nullptr, materialData);

	TEST_ASSERT(&first.GetMaterialRef(0) != &second.GetMaterialRef(0));
	TEST_ASSERT(&second.GetMaterialRef(0) == &source.GetMaterialRef(0));
	TEST_PASS("SetMaterialData gives the instance its own material");

	CGrannyMaterialPalette third;
	third.Copy(source);
	third.SetSpecularInfo(nullptr, TRUE, 1.0f);

	TEST_ASSERT(&third.GetMaterialRef(0) != &source.GetMaterialRef(0));
	TEST_ASSERT(&second.GetMaterialRef(0) == &source.GetMaterialRef(0));
	TEST_PASS("SetSpecularInfo gives the instance its own material");

	test_material_data_covers_every_material_sharing_a_texture();

	return 0;
}
