#include <SpeedTreeLib/StdAfx.h>
#include <SpeedTreeLib/SpeedTreeRTCompat.h>
#include <SpeedTreeLib/SpeedTreeWrapper.h>
#include <SpeedTreeLib/SpeedTreeForest.h>
#include <SpeedTreeLib/CSpeedTreeDirectX.h>
#include <SpeedTreeLib/BoundaryShapeManager.h>

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

namespace {

void test_speedtreert_compat()
{
    CSpeedTreeRT rt;
    rt.SetTreePosition(10.0f, 20.0f, 30.0f);
    const float* pos = rt.GetTreePosition();
    assert(pos != nullptr);
    assert(pos[0] == 10.0f && pos[1] == 20.0f && pos[2] == 30.0f);

    rt.SetTreeSize(500.0f, 10.0f);
    float size = 0.0f, variance = 0.0f;
    rt.GetTreeSize(size, variance);
    assert(size == 500.0f);

    float bounds[6] = {0};
    rt.GetBoundingBox(bounds);
    assert(bounds[5] == 500.0f);
    assert(bounds[0] < 0.0f && bounds[3] > 0.0f);

    assert(rt.Compute(nullptr, 42));
    assert(rt.GetSeed() == 42);

    // Collision cylinder
    assert(rt.GetCollisionObjectCount() >= 1);
    CSpeedTreeRT::ECollisionObjectType colType;
    float colPos[3] = {0};
    float colDims[3] = {0};
    rt.GetCollisionObject(0, colType, colPos, colDims);
    assert(colType == CSpeedTreeRT::CO_CYLINDER);
    assert(colDims[0] > 0.0f && colDims[1] > 0.0f);

    // Materials
    const float* mat = rt.GetBranchMaterial();
    assert(mat != nullptr);
    assert(mat[0] == 1.0f); // Diffuse R

    // Instancing
    CSpeedTreeRT* inst = rt.MakeInstance();
    assert(inst != nullptr);
    assert(inst->InstanceOf() == &rt);
    delete inst;

    std::printf("[PASS] test_speedtreert_compat\n");
}

void test_speedtree_wrapper_and_spt_loading()
{
    CSpeedTreeWrapper wrapper;

    // Create a synthetic SPT buffer with mock tags and texture filenames
    std::string mockSpt = "__IdvSpt_02_\x00\x00\x00\x1b" "D:\\Tree\\BeechBark.tga\x00\x00" "CompositeMapB1\x00\x00" "CompositeShadowMapB1.tga\x00\x00" "BeechLeaves_RT_1.tga";
    mockSpt += std::string("\x00\x00\x00\x00\x00\x00\x00\x00", 8);

    bool loadOk = wrapper.LoadTree("test_tree.spt", reinterpret_cast<const BYTE*>(mockSpt.data()), static_cast<unsigned int>(mockSpt.size()));
    assert(loadOk);

    const float* bbox = wrapper.GetBoundingBox();
    assert(bbox != nullptr);

    wrapper.SetPosition(50.0f, 100.0f, 20.0f);

    D3DXVECTOR3 v3Center;
    float fRadius = 0.0f;
    bool bsOk = wrapper.GetBoundingSphere(v3Center, fRadius);
    assert(bsOk);
    assert(fRadius > 0.0f);
    assert(v3Center.x == 50.0f && v3Center.y == 100.0f);

    // Test wrapper instancing
    CSpeedTreeWrapper* pInst = wrapper.MakeInstance();
    assert(pInst != nullptr);
    assert(pInst->InstanceOf() == &wrapper);

    unsigned int instCount = 0;
    CSpeedTreeWrapper** ppInstances = wrapper.GetInstances(instCount);
    assert(instCount == 1);
    assert(ppInstances[0] == pInst);

    wrapper.DeleteInstance(pInst);
    ppInstances = wrapper.GetInstances(instCount);
    assert(instCount == 0);

    std::printf("[PASS] test_speedtree_wrapper_and_spt_loading\n");
}

void test_speedtree_forest()
{
    CSpeedTreeDirectX directxForest;

    directxForest.SetWindStrength(0.75f);
    assert(std::fabs(directxForest.GetWindStrength() - 0.75f) < 1e-4f);

    float dir[3] = {0.0f, 1.0f, 0.0f};
    float amb[3] = {0.2f, 0.2f, 0.2f};
    float diff[3] = {0.8f, 0.8f, 0.8f};
    directxForest.SetLight(dir, amb, diff);

    directxForest.SetFog(500.0f, 2000.0f);

    directxForest.UpdateSystem(0.016f);

    directxForest.Clear();

    std::printf("[PASS] test_speedtree_forest\n");
}

void test_boundary_shape_manager()
{
    CBoundaryShapeManager bsm;
    assert(!bsm.PointInside(100.0f, 100.0f));

    float rx = 0.0f, ry = 0.0f;
    assert(!bsm.RandomPoint(rx, ry));

    std::printf("[PASS] test_boundary_shape_manager\n");
}

} // namespace

int main()
{
    test_speedtreert_compat();
    test_speedtree_wrapper_and_spt_loading();
    test_speedtree_forest();
    test_boundary_shape_manager();

    std::printf("All SpeedTreeLib tests passed successfully!\n");
    return 0;
}
