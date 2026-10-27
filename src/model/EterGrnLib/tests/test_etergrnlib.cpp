#include <EterGrnLib/StdAfx.h>
#include <EterGrnLib/Thing.h>
#include <EterGrnLib/ThingInstance.h>
#include <EterGrnLib/LODController.h>
#include <EterGrnLib/Model.h>
#include <EterGrnLib/ModelInstance.h>
#include <EterGrnLib/Mesh.h>
#include <EterGrnLib/Motion.h>
#include <EterGrnLib/Material.h>
#include <EterLib/ResourceManager.h>

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace {

void test_granny_skeleton_and_world_pose_math()
{
    // Build a 2-bone hierarchy: Root -> Hand
    granny_bone bones[2];
    std::memset(bones, 0, sizeof(bones));

    bones[0].Name = "Bip01";
    bones[0].ParentIndex = GrannyNoParentBone;
    bones[0].LocalTransform.Flags = GrannyHasPosition | GrannyHasOrientation;
    bones[0].LocalTransform.Position[0] = 10.0f;
    bones[0].LocalTransform.Position[1] = 20.0f;
    bones[0].LocalTransform.Position[2] = 30.0f;
    bones[0].LocalTransform.Orientation[3] = 1.0f;
    GrannyMathInternal::LoadIdentity4x4(bones[0].InverseWorld4x4);

    bones[1].Name = "Bip01_R_Hand";
    bones[1].ParentIndex = 0;
    bones[1].LocalTransform.Flags = GrannyHasPosition | GrannyHasOrientation;
    bones[1].LocalTransform.Position[0] = 5.0f;
    bones[1].LocalTransform.Position[1] = -3.0f;
    bones[1].LocalTransform.Position[2] = 2.0f;
    bones[1].LocalTransform.Orientation[3] = 1.0f;
    GrannyMathInternal::LoadIdentity4x4(bones[1].InverseWorld4x4);

    granny_skeleton skel;
    skel.Name = "WarriorSkeleton";
    skel.BoneCount = 2;
    skel.Bones = bones;

    granny_int32x foundIdx = -1;
    bool foundBone = GrannyFindBoneByName(&skel, "bip01_r_hand", &foundIdx);
    assert(foundBone);
    assert(foundIdx == 1);

    granny_local_pose* localPose = GrannyNewLocalPose(2);
    granny_world_pose* worldPose = GrannyNewWorldPose(2);

    localPose->Transforms[0] = bones[0].LocalTransform;
    localPose->Transforms[1] = bones[1].LocalTransform;

    GrannyBuildWorldPose(&skel, 0, 2, localPose, nullptr, worldPose);

    const float* rootMat = GrannyGetWorldPose4x4(worldPose, 0);
    assert(rootMat != nullptr);
    assert(std::fabs(rootMat[12] - 10.0f) < 1e-4f);
    assert(std::fabs(rootMat[13] - 20.0f) < 1e-4f);
    assert(std::fabs(rootMat[14] - 30.0f) < 1e-4f);

    const float* handMat = GrannyGetWorldPose4x4(worldPose, 1);
    assert(handMat != nullptr);
    assert(std::fabs(handMat[12] - 15.0f) < 1e-4f);
    assert(std::fabs(handMat[13] - 17.0f) < 1e-4f);
    assert(std::fabs(handMat[14] - 32.0f) < 1e-4f);

    GrannyFreeWorldPose(worldPose);
    GrannyFreeLocalPose(localPose);

    std::printf("[PASS] test_granny_skeleton_and_world_pose_math\n");
}

void test_granny_vertex_deformer_skinning()
{
    granny_pwnt3432_vertex srcVert;
    std::memset(&srcVert, 0, sizeof(srcVert));
    srcVert.Position[0] = 1.0f;
    srcVert.Position[1] = 2.0f;
    srcVert.Position[2] = 3.0f;
    srcVert.BoneWeights[0] = 255;
    srcVert.BoneIndices[0] = 0;
    srcVert.Normal[0] = 0.0f;
    srcVert.Normal[1] = 0.0f;
    srcVert.Normal[2] = 1.0f;
    srcVert.UV[0] = 0.25f;
    srcVert.UV[1] = 0.75f;

    granny_matrix_4x4 boneMat;
    GrannyMathInternal::LoadIdentity4x4(boneMat);
    boneMat[3][0] = 100.0f;
    boneMat[3][1] = 200.0f;
    boneMat[3][2] = 300.0f;

    granny_int32x boneMap[1] = {0};

    granny_mesh_deformer* deformer = GrannyNewMeshDeformer(
        GrannyPWNT3432VertexType,
        GrannyPNT332VertexType,
        GrannyDeformPositionNormal);
    assert(deformer != nullptr);
    assert(deformer->HasBoneWeights);

    granny_pnt332_vertex dstVert;
    std::memset(&dstVert, 0, sizeof(dstVert));

    GrannyDeformVertices(deformer, boneMap, &boneMat[0][0], 1, &srcVert, &dstVert);

    assert(std::fabs(dstVert.Position[0] - 101.0f) < 1e-3f);
    assert(std::fabs(dstVert.Position[1] - 202.0f) < 1e-3f);
    assert(std::fabs(dstVert.Position[2] - 303.0f) < 1e-3f);
    assert(std::fabs(dstVert.Normal[2] - 1.0f) < 1e-3f);
    assert(std::fabs(dstVert.UV[0] - 0.25f) < 1e-3f);
    assert(std::fabs(dstVert.UV[1] - 0.75f) < 1e-3f);

    GrannyFreeMeshDeformer(deformer);

    std::printf("[PASS] test_granny_vertex_deformer_skinning\n");
}

void test_motion_and_thing_loading()
{
    // Test CGrannyMotion text track and duration
    granny_text_track_entry entries[2] = {
        {0.15f, "sound_swing1"},
        {0.45f, "Attack_Hit"}
    };
    granny_text_track textTrack = {"Events", 2, entries};
    granny_track_group trackGroup;
    std::memset(&trackGroup, 0, sizeof(trackGroup));
    trackGroup.Name = "WarriorTrackGroup";
    trackGroup.TextTrackCount = 1;
    trackGroup.TextTracks = &textTrack;

    granny_track_group* trackGroups[1] = {&trackGroup};

    granny_animation anim;
    std::memset(&anim, 0, sizeof(anim));
    anim.Name = "combo_attack_01";
    anim.Duration = 1.25f;
    anim.TimeStep = 1.0f / 30.0f;
    anim.TrackGroupCount = 1;
    anim.TrackGroups = trackGroups;

    CGrannyMotion motion;
    assert(motion.IsEmpty());
    bool bound = motion.BindGrannyAnimation(&anim);
    assert(bound);
    assert(!motion.IsEmpty());
    assert(std::strcmp(motion.GetName(), "combo_attack_01") == 0);
    assert(std::fabs(motion.GetDuration() - 1.25f) < 1e-4f);

    int hitCount = 0;
    float hitTimes[4] = {0.0f};
    motion.GetTextTrack("attack_hit", &hitCount, hitTimes);
    assert(hitCount == 1);
    assert(std::fabs(hitTimes[0] - 0.45f) < 1e-4f);

    // Test loading a synthetic model + animation into CGraphicThing via OpenGrannyMemoryEnvelope
    granny_bone rootBone;
    std::memset(&rootBone, 0, sizeof(rootBone));
    rootBone.Name = "Bip01";
    rootBone.ParentIndex = GrannyNoParentBone;
    rootBone.LocalTransform.Flags = GrannyHasPosition | GrannyHasOrientation;
    rootBone.LocalTransform.Orientation[3] = 1.0f;
    GrannyMathInternal::LoadIdentity4x4(rootBone.InverseWorld4x4);

    granny_skeleton skel = {"TestSkel", 1, &rootBone};

    granny_pnt332_vertex triVerts[3] = {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{10.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.0f, 10.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    granny_vertex_data vtxData;
    std::memset(&vtxData, 0, sizeof(vtxData));
    vtxData.VertexType = GrannyPNT332VertexType;
    vtxData.VertexCount = 3;
    vtxData.Vertices = reinterpret_cast<granny_uint8*>(triVerts);

    granny_uint16 triIndices[3] = {0, 1, 2};
    granny_tri_material_group triGroup = {0, 0, 1};
    granny_tri_topology topo;
    std::memset(&topo, 0, sizeof(topo));
    topo.GroupCount = 1;
    topo.Groups = &triGroup;
    topo.IndexCount16 = 3;
    topo.Indices16 = triIndices;

    granny_bone_binding boneBinding = {"Bip01", {-5.0f, -5.0f, -5.0f}, {15.0f, 15.0f, 5.0f}, 0, nullptr};

    granny_mesh mesh;
    std::memset(&mesh, 0, sizeof(mesh));
    mesh.Name = "SwordMesh";
    mesh.PrimaryVertexData = &vtxData;
    mesh.PrimaryTopology = &topo;
    mesh.BoneBindingCount = 1;
    mesh.BoneBindings = &boneBinding;

    granny_model_mesh_binding meshBinding = {&mesh};
    granny_model model;
    std::memset(&model, 0, sizeof(model));
    model.Name = "SwordModel";
    model.Skeleton = &skel;
    model.MeshBindingCount = 1;
    model.MeshBindings = &meshBinding;

    granny_model* models[1] = {&model};
    granny_animation* anims[1] = {&anim};

    granny_file_info fileInfo;
    std::memset(&fileInfo, 0, sizeof(fileInfo));
    fileInfo.ModelCount = 1;
    fileInfo.Models = models;
    fileInfo.AnimationCount = 1;
    fileInfo.Animations = anims;

    OpenGrannyMemoryEnvelope env = {OPEN_GRANNY_MEM_MAGIC, 1, &fileInfo};

    struct ScopedD3DContext : public CGraphicBase
    {
        ScopedD3DContext()
        {
            ms_lpd3dDevice = new IDirect3DDevice9();
            D3DXCreateMatrixStack(0, &ms_lpd3dMatStack);
            D3DXMatrixIdentity(&ms_matIdentity);
        }
        ~ScopedD3DContext()
        {
            if (ms_lpd3dMatStack)
            {
                ms_lpd3dMatStack->Release();
                ms_lpd3dMatStack = nullptr;
            }
            if (ms_lpd3dDevice)
            {
                ms_lpd3dDevice->Release();
                ms_lpd3dDevice = nullptr;
            }
        }
    } d3dCtx;

    class TestGraphicThing : public CGraphicThing
    {
    public:
        explicit TestGraphicThing(const char* name) : CGraphicThing(name) {}
        bool LoadMemory(int size, const void* buf) { return OnLoad(size, buf); }
    };

    TestGraphicThing thing("item/weapon/sword_001.gr2");
    bool loaded = thing.LoadMemory(sizeof(env), &env);
    assert(loaded);
    assert(thing.GetModelCount() == 1);
    assert(thing.GetMotionCount() == 1);
    assert(thing.GetModelPointer(0) != nullptr);
    assert(thing.GetModelPointer(0)->GetRigidVertexCount() == 3);
    assert(thing.GetMotionPointer(0) != nullptr);

    // Test CGrannyModelInstance & bone queries
    CGrannyModelInstance* modelInst = CGrannyModelInstance::New();
    assert(modelInst != nullptr);
    modelInst->SetMainModelPointer(thing.GetModelPointer(0), nullptr);

    int boneIdx = -1;
    bool gotBone = modelInst->GetBoneIndexByName("Bip01", &boneIdx);
    assert(gotBone);
    assert(boneIdx == 0);

    D3DXMATRIX worldMat;
    D3DXMatrixIdentity(&worldMat);
    modelInst->DeformNoSkin(&worldMat);

    D3DXVECTOR3 vtMin, vtMax;
    modelInst->GetBoundBox(&vtMin, &vtMax);
    assert(vtMax.x > vtMin.x);

    modelInst->SetMotionPointer(thing.GetMotionPointer(0), 0.1f, 1, 1.0f);
    assert(modelInst->IsMotionPlaying());
    modelInst->UpdateLocalTime(0.5f);
    modelInst->Update(CGrannyModelInstance::ANIFPS_MIN);

    CGrannyModelInstance::Delete(modelInst);
    CGrannyModelInstance::DestroySystem();
    thing.DestroyDeviceObjects();

    std::printf("[PASS] test_motion_and_thing_loading\n");
}

void test_native_gr2_parsing_and_model_loading()
{
    CResourceManager resMgr;

    class TestGraphicThing : public CGraphicThing
    {
    public:
        explicit TestGraphicThing(const char* name) : CGraphicThing(name) {}
        bool LoadMemory(int size, const void* buf) { return OnLoad(size, buf); }
    };

    auto try_read_file = [](const std::vector<std::string>& candidates) -> std::vector<uint8_t> {
        for (const auto& path : candidates) {
            FILE* fp = std::fopen(path.c_str(), "rb");
            if (!fp) continue;
            std::fseek(fp, 0, SEEK_END);
            long sz = std::ftell(fp);
            std::fseek(fp, 0, SEEK_SET);
            if (sz > 0) {
                std::vector<uint8_t> data(sz);
                size_t readBytes = std::fread(data.data(), 1, sz, fp);
                std::fclose(fp);
                if (readBytes == static_cast<size_t>(sz))
                    return data;
            } else {
                std::fclose(fp);
            }
        }
        return {};
    };

    // Test 1: Rigid mesh (general_obj_fence03.gr2)
    std::vector<uint8_t> fenceData = try_read_file({
        "bin/pack/Zone/ymir work/zone/b/obj/general_obj_fence03.gr2",
        "../bin/pack/Zone/ymir work/zone/b/obj/general_obj_fence03.gr2",
        "../../bin/pack/Zone/ymir work/zone/b/obj/general_obj_fence03.gr2"
    });

    if (!fenceData.empty()) {
        TestGraphicThing thingFence("general_obj_fence03.gr2");
        bool loaded = thingFence.LoadMemory(fenceData.size(), fenceData.data());
        assert(loaded);
        assert(thingFence.GetModelCount() >= 1);
        CGrannyModel* model = thingFence.GetModelPointer(0);
        assert(model != nullptr);
        assert(model->GetMeshCount() >= 1);
        assert(model->GetRigidVertexCount() > 0);
        assert(model->GetRigidVertexCount() == 59);
        assert(model->GetMeshCount() == 1);

        CGrannyModelInstance* inst = CGrannyModelInstance::New();
        assert(inst != nullptr);
        inst->SetMainModelPointer(model, nullptr);

        D3DXMATRIX worldMat;
        D3DXMatrixIdentity(&worldMat);
        inst->DeformNoSkin(&worldMat);

        D3DXVECTOR3 vtMin, vtMax;
        inst->GetBoundBox(&vtMin, &vtMax);
        assert(vtMax.x > vtMin.x);

        CGrannyModelInstance::Delete(inst);
        thingFence.DestroyDeviceObjects();
        std::printf("[PASS] test_native_gr2_parsing_and_model_loading: rigid mesh verified\n");
    }

    // Test 2: Deformable skinned character mesh (warrior_novice.gr2)
    std::vector<uint8_t> warriorData = try_read_file({
        "bin/pack/PC/ymir work/pc/warrior/warrior_novice.gr2",
        "../bin/pack/PC/ymir work/pc/warrior/warrior_novice.gr2",
        "../../bin/pack/PC/ymir work/pc/warrior/warrior_novice.gr2"
    });

    if (!warriorData.empty()) {
        TestGraphicThing thingWarrior("warrior_novice.gr2");
        bool loaded = thingWarrior.LoadMemory(warriorData.size(), warriorData.data());
        assert(loaded);
        assert(thingWarrior.GetModelCount() >= 1);
        CGrannyModel* model = thingWarrior.GetModelPointer(0);
        assert(model != nullptr);
        assert(model->GetMeshCount() >= 1);
        assert(model->GetDeformVertexCount() > 0);
        assert(model->GetDeformVertexCount() == 2207);
        assert(model->GetMeshCount() == 3);

        CGrannyModelInstance* inst = CGrannyModelInstance::New();
        assert(inst != nullptr);
        inst->SetMainModelPointer(model, nullptr);

        D3DXMATRIX worldMat;
        D3DXMatrixIdentity(&worldMat);
        inst->Deform(&worldMat);

        D3DXVECTOR3 vtMin, vtMax;
        inst->GetBoundBox(&vtMin, &vtMax);
        assert(vtMax.z > vtMin.z);

        int boneIdx = -1;
        bool foundBip = inst->GetBoneIndexByName("Bip01", &boneIdx);
        assert(foundBip);
        assert(boneIdx >= 0);

        CGrannyModelInstance::Delete(inst);
        thingWarrior.DestroyDeviceObjects();
        std::printf("[PASS] test_native_gr2_parsing_and_model_loading: deformable skinned character verified\n");
    }

    CGrannyModelInstance::DestroySystem();
}

} // namespace

int main()
{
    test_granny_skeleton_and_world_pose_math();
    test_granny_vertex_deformer_skinning();
    test_motion_and_thing_loading();
    test_native_gr2_parsing_and_model_loading();

    std::printf("All EterGrnLib smoke tests passed!\n");
    return 0;
}
