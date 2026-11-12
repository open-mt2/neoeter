#include <Neoeter/Model/Grn/StdAfx.h>
#include <Neoeter/Model/Grn/Material.h>
#include <Neoeter/Model/Grn/Util.h>

#include <support/TestAssert.h>

#include <cstring>

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

	return 0;
}
