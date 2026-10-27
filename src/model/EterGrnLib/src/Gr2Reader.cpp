#include "EterGrnLib/Gr2Reader.h"
#include "EterGrnLib/Gr2Decompress.h"
#include "EterGrnLib/GrannyCompat.h"

#include <vector>
#include <deque>
#include <map>
#include <string>
#include <memory>
#include <cstring>
#include <cstdint>

namespace {

struct GR2ParsedContext
{
    granny_file file;
    granny_file_info fileInfo;
    std::vector<std::vector<uint8_t>> sections;
    std::vector<std::unique_ptr<granny_model>> models;
    std::vector<granny_model*> modelPtrs;
    std::vector<std::unique_ptr<granny_skeleton>> skeletons;
    std::vector<std::vector<granny_bone>> skeletonBones;
    std::vector<std::vector<granny_model_mesh_binding>> modelMeshBindings;
    std::vector<std::unique_ptr<granny_mesh>> meshes;
    std::vector<std::unique_ptr<granny_vertex_data>> vertexDatas;
    std::vector<std::unique_ptr<granny_tri_topology>> topologies;
    std::vector<std::vector<granny_tri_material_group>> triGroups;
    std::vector<std::vector<granny_material_binding>> meshMaterialBindings;
    std::vector<std::unique_ptr<granny_material>> materials;
    std::vector<std::vector<granny_material_map>> materialMaps;
    std::vector<std::unique_ptr<granny_texture>> textures;
    std::vector<std::vector<granny_bone_binding>> meshBoneBindings;
    std::vector<std::unique_ptr<granny_animation>> animations;
    std::vector<granny_animation*> animPtrs;
    std::deque<std::string> stringStore;

    const char* StoreString(const char* s)
    {
        if (!s || !*s) return "";
        stringStore.emplace_back(s);
        return stringStore.back().c_str();
    }
};

static const void* ResolveRef(
    const std::vector<std::vector<uint8_t>>& sections,
    const std::vector<std::map<uint32_t, std::pair<uint32_t, uint32_t>>>& fixups,
    uint32_t secIdx, uint32_t offset)
{
    if (secIdx >= fixups.size()) return nullptr;
    auto it = fixups[secIdx].find(offset);
    if (it == fixups[secIdx].end()) return nullptr;
    uint32_t toSec = it->second.first;
    uint32_t toOff = it->second.second;
    if (toSec >= sections.size() || toOff >= sections[toSec].size()) return nullptr;
    return &sections[toSec][toOff];
}

static const char* ResolveString(
    const std::vector<std::vector<uint8_t>>& sections,
    const std::vector<std::map<uint32_t, std::pair<uint32_t, uint32_t>>>& fixups,
    uint32_t secIdx, uint32_t offset)
{
    const void* ptr = ResolveRef(sections, fixups, secIdx, offset);
    return ptr ? (const char*)ptr : "";
}

} // namespace

extern "C" granny_file* GrannyParseGR2FromMemory(int size, const void* memory)
{
    if (!memory || size < 88)
        return nullptr;

    const uint8_t* buf = static_cast<const uint8_t*>(memory);

    // Granny magic check: B8 67 B0 CA F8 6D B1 0F 84 72 8C 7E 5E 19 00 1E
    static const uint8_t GR2_MAGIC[16] = {
        0xb8, 0x67, 0xb0, 0xca, 0xf8, 0x6d, 0xb1, 0x0f,
        0x84, 0x72, 0x8c, 0x7e, 0x5e, 0x19, 0x00, 0x1e
    };

    if (std::memcmp(buf, GR2_MAGIC, 16) != 0)
        return nullptr;

    uint32_t secOffset   = *reinterpret_cast<const uint32_t*>(&buf[32 + 12]);
    uint32_t numSec      = *reinterpret_cast<const uint32_t*>(&buf[32 + 16]);
    uint32_t rootNodeSec = *reinterpret_cast<const uint32_t*>(&buf[32 + 28]);
    uint32_t rootNodeOff = *reinterpret_cast<const uint32_t*>(&buf[32 + 32]);

    if (32 + secOffset + numSec * 44 > static_cast<size_t>(size))
        return nullptr;

    auto ctx = std::make_unique<GR2ParsedContext>();
    ctx->sections.resize(numSec);
    std::vector<std::map<uint32_t, std::pair<uint32_t, uint32_t>>> fixups(numSec);

    for (uint32_t i = 0; i < numSec; ++i) {
        const uint8_t* pSec = &buf[32 + secOffset + i * 44];
        uint32_t fmt      = *reinterpret_cast<const uint32_t*>(pSec + 0);
        uint32_t off      = *reinterpret_cast<const uint32_t*>(pSec + 4);
        uint32_t compSz   = *reinterpret_cast<const uint32_t*>(pSec + 8);
        uint32_t uncompSz = *reinterpret_cast<const uint32_t*>(pSec + 12);
        uint32_t f16      = *reinterpret_cast<const uint32_t*>(pSec + 20);
        uint32_t f8       = *reinterpret_cast<const uint32_t*>(pSec + 24);
        uint32_t relocOff = *reinterpret_cast<const uint32_t*>(pSec + 28);
        uint32_t numReloc = *reinterpret_cast<const uint32_t*>(pSec + 32);

        ctx->sections[i].resize(uncompSz);
        if (compSz > 0 && uncompSz > 0 && off + compSz <= static_cast<size_t>(size)) {
            if (fmt == 0) {
                std::memcpy(ctx->sections[i].data(), &buf[off], std::min(compSz, uncompSz));
            } else if (fmt == 2) {
                gr2_decompress(compSz, &buf[off], f16, f8, uncompSz, ctx->sections[i].data());
            }
        }

        if (numReloc > 0 && relocOff + numReloc * 12 <= static_cast<size_t>(size)) {
            for (uint32_t r = 0; r < numReloc; ++r) {
                const uint8_t* pReloc = &buf[relocOff + r * 12];
                uint32_t fromOff = *reinterpret_cast<const uint32_t*>(pReloc + 0);
                uint32_t toSec   = *reinterpret_cast<const uint32_t*>(pReloc + 4);
                uint32_t toOff   = *reinterpret_cast<const uint32_t*>(pReloc + 8);
                fixups[i][fromOff] = {toSec, toOff};
            }
        }
    }

    if (rootNodeSec >= ctx->sections.size() || rootNodeOff + 84 > ctx->sections[rootNodeSec].size())
        return nullptr;

    const uint8_t* pRoot = &ctx->sections[rootNodeSec][rootNodeOff];
    int32_t modelCount = *reinterpret_cast<const int32_t*>(pRoot + 0x3C);
    int32_t animCount  = *reinterpret_cast<const int32_t*>(pRoot + 0x4C);

    ctx->fileInfo.ModelCount     = std::max(0, modelCount);
    ctx->fileInfo.AnimationCount = std::max(0, animCount);

    // Unmarshal Models
    if (modelCount > 0 && fixups[rootNodeSec].count(rootNodeOff + 0x40)) {
        auto [mArrSec, mArrOff] = fixups[rootNodeSec][rootNodeOff + 0x40];
        ctx->models.reserve(modelCount);
        ctx->modelMeshBindings.resize(modelCount);

        for (int m = 0; m < modelCount; ++m) {
            if (!fixups[mArrSec].count(mArrOff + m * 4)) continue;
            auto [mSec, mOff] = fixups[mArrSec][mArrOff + m * 4];
            if (mSec >= ctx->sections.size() || mOff + 84 > ctx->sections[mSec].size()) continue;

            const uint8_t* pModel = &ctx->sections[mSec][mOff];
            auto model = std::make_unique<granny_model>();
            std::memset(model.get(), 0, sizeof(granny_model));
            model->Name = ctx->StoreString(ResolveString(ctx->sections, fixups, mSec, mOff + 0));
            std::memcpy(&model->InitialPlacement, pModel + 8, sizeof(granny_transform));

            // Skeleton
            if (fixups[mSec].count(mOff + 4)) {
                auto [skSec, skOff] = fixups[mSec][mOff + 4];
                if (skSec < ctx->sections.size() && skOff + 12 <= ctx->sections[skSec].size()) {
                    const uint8_t* pSkel = &ctx->sections[skSec][skOff];
                    auto skel = std::make_unique<granny_skeleton>();
                    std::memset(skel.get(), 0, sizeof(granny_skeleton));
                    skel->Name = ctx->StoreString(ResolveString(ctx->sections, fixups, skSec, skOff + 0));
                    int32_t boneCount = *reinterpret_cast<const int32_t*>(pSkel + 4);
                    skel->BoneCount = std::max(0, boneCount);

                    if (boneCount > 0 && fixups[skSec].count(skOff + 8)) {
                        auto [bnSec, bnOff] = fixups[skSec][skOff + 8];
                        ctx->skeletonBones.emplace_back(boneCount);
                        auto& bones = ctx->skeletonBones.back();
                        for (int b = 0; b < boneCount; ++b) {
                            uint32_t bOff = bnOff + b * 156;
                            if (bnSec < ctx->sections.size() && bOff + 140 <= ctx->sections[bnSec].size()) {
                                const uint8_t* pBone = &ctx->sections[bnSec][bOff];
                                bones[b].Name = ctx->StoreString(ResolveString(ctx->sections, fixups, bnSec, bOff + 0));
                                bones[b].ParentIndex = *reinterpret_cast<const int32_t*>(pBone + 4);
                                std::memcpy(&bones[b].LocalTransform, pBone + 8, sizeof(granny_transform));
                                std::memcpy(bones[b].InverseWorld4x4, pBone + 76, sizeof(granny_matrix_4x4));
                            }
                        }
                        skel->Bones = bones.data();
                    }
                    model->Skeleton = skel.get();
                    ctx->skeletons.push_back(std::move(skel));
                }
            }

            // Mesh Bindings
            int32_t mbCount = *reinterpret_cast<const int32_t*>(pModel + 76);
            model->MeshBindingCount = std::max(0, mbCount);
            if (mbCount > 0 && fixups[mSec].count(mOff + 80)) {
                auto [mbSec, mbOff] = fixups[mSec][mOff + 80];
                auto& bindings = ctx->modelMeshBindings[m];
                bindings.resize(mbCount);

                for (int b = 0; b < mbCount; ++b) {
                    if (!fixups[mbSec].count(mbOff + b * 4)) continue;
                    auto [meshSec, meshOff] = fixups[mbSec][mbOff + b * 4];
                    if (meshSec >= ctx->sections.size() || meshOff + 36 > ctx->sections[meshSec].size()) continue;

                    const uint8_t* pMesh = &ctx->sections[meshSec][meshOff];
                    auto mesh = std::make_unique<granny_mesh>();
                    std::memset(mesh.get(), 0, sizeof(granny_mesh));
                    mesh->Name = ctx->StoreString(ResolveString(ctx->sections, fixups, meshSec, meshOff + 0));

                    // Vertex Data
                    if (fixups[meshSec].count(meshOff + 4)) {
                        auto [vtxSec, vtxOff] = fixups[meshSec][meshOff + 4];
                        if (vtxSec < ctx->sections.size() && vtxOff + 12 <= ctx->sections[vtxSec].size()) {
                            const uint8_t* pVtx = &ctx->sections[vtxSec][vtxOff];
                            auto vData = std::make_unique<granny_vertex_data>();
                            std::memset(vData.get(), 0, sizeof(granny_vertex_data));
                            vData->VertexCount = *reinterpret_cast<const int32_t*>(pVtx + 4);
                            if (fixups[vtxSec].count(vtxOff + 8)) {
                                auto [dSec, dOff] = fixups[vtxSec][vtxOff + 8];
                                if (dSec < ctx->sections.size() && dOff < ctx->sections[dSec].size()) {
                                    vData->Vertices = &ctx->sections[dSec][dOff];
                                }
                            }
                            mesh->PrimaryVertexData = vData.get();
                            ctx->vertexDatas.push_back(std::move(vData));
                        }
                    }

                    // Topology
                    if (fixups[meshSec].count(meshOff + 16)) {
                        auto [topoSec, topoOff] = fixups[meshSec][meshOff + 16];
                        if (topoSec < ctx->sections.size() && topoOff + 24 <= ctx->sections[topoSec].size()) {
                            const uint8_t* pTopo = &ctx->sections[topoSec][topoOff];
                            auto topo = std::make_unique<granny_tri_topology>();
                            std::memset(topo.get(), 0, sizeof(granny_tri_topology));
                            int32_t grpCount   = *reinterpret_cast<const int32_t*>(pTopo + 0);
                            int32_t idxCount   = *reinterpret_cast<const int32_t*>(pTopo + 8);
                            int32_t idxCount16 = *reinterpret_cast<const int32_t*>(pTopo + 16);

                            if (idxCount16 > 0 && fixups[topoSec].count(topoOff + 20)) {
                                auto [iSec, iOff] = fixups[topoSec][topoOff + 20];
                                if (iSec < ctx->sections.size() && iOff < ctx->sections[iSec].size()) {
                                    topo->Indices16 = reinterpret_cast<uint16_t*>(&ctx->sections[iSec][iOff]);
                                    topo->IndexCount = idxCount16;
                                }
                            } else if (idxCount > 0 && fixups[topoSec].count(topoOff + 12)) {
                                auto [iSec, iOff] = fixups[topoSec][topoOff + 12];
                                if (iSec < ctx->sections.size() && iOff < ctx->sections[iSec].size()) {
                                    topo->Indices = reinterpret_cast<int32_t*>(&ctx->sections[iSec][iOff]);
                                    topo->IndexCount = idxCount;
                                }
                            }

                            if (grpCount > 0 && fixups[topoSec].count(topoOff + 4)) {
                                auto [gSec, gOff] = fixups[topoSec][topoOff + 4];
                                if (gSec < ctx->sections.size() && gOff + grpCount * sizeof(granny_tri_material_group) <= ctx->sections[gSec].size()) {
                                    ctx->triGroups.emplace_back(grpCount);
                                    auto& groups = ctx->triGroups.back();
                                    std::memcpy(groups.data(), &ctx->sections[gSec][gOff], grpCount * sizeof(granny_tri_material_group));
                                    topo->GroupCount = grpCount;
                                    topo->Groups = groups.data();
                                }
                            }
                            mesh->PrimaryTopology = topo.get();
                            ctx->topologies.push_back(std::move(topo));
                        }
                    }

                    // Materials
                    int32_t matCount = *reinterpret_cast<const int32_t*>(pMesh + 20);
                    mesh->MaterialBindingCount = std::max(0, matCount);
                    if (matCount > 0 && fixups[meshSec].count(meshOff + 24)) {
                        auto [matArrSec, matArrOff] = fixups[meshSec][meshOff + 24];
                        ctx->meshMaterialBindings.emplace_back(matCount);
                        auto& matBindings = ctx->meshMaterialBindings.back();

                        for (int m_idx = 0; m_idx < matCount; ++m_idx) {
                            if (!fixups[matArrSec].count(matArrOff + m_idx * 4)) continue;
                            auto [matSec, matOff] = fixups[matArrSec][matArrOff + m_idx * 4];
                            if (matSec >= ctx->sections.size() || matOff + 12 > ctx->sections[matSec].size()) continue;

                            const uint8_t* pMat = &ctx->sections[matSec][matOff];
                            auto mat = std::make_unique<granny_material>();
                            std::memset(mat.get(), 0, sizeof(granny_material));
                            mat->Name = ctx->StoreString(ResolveString(ctx->sections, fixups, matSec, matOff + 0));
                            int32_t mapCount = *reinterpret_cast<const int32_t*>(pMat + 4);
                            mat->MapCount = std::max(0, mapCount);

                            if (mapCount > 0 && fixups[matSec].count(matOff + 8)) {
                                auto [mapSec, mapOff] = fixups[matSec][matOff + 8];
                                ctx->materialMaps.emplace_back(mapCount);
                                auto& maps = ctx->materialMaps.back();
                                for (int mp = 0; mp < mapCount; ++mp) {
                                    uint32_t curMapOff = mapOff + mp * 8;
                                    maps[mp].Usage = ctx->StoreString(ResolveString(ctx->sections, fixups, mapSec, curMapOff + 0));
                                    if (fixups[mapSec].count(curMapOff + 4)) {
                                        auto [subMSec, subMOff] = fixups[mapSec][curMapOff + 4];
                                        auto subMat = std::make_unique<granny_material>();
                                        std::memset(subMat.get(), 0, sizeof(granny_material));
                                        if (fixups[subMSec].count(subMOff + 12)) {
                                            auto [texSec, texOff] = fixups[subMSec][subMOff + 12];
                                            auto tex = std::make_unique<granny_texture>();
                                            std::memset(tex.get(), 0, sizeof(granny_texture));
                                            tex->FromFileName = const_cast<char*>(ctx->StoreString(ResolveString(ctx->sections, fixups, texSec, texOff + 0)));
                                            subMat->Texture = tex.get();
                                            ctx->textures.push_back(std::move(tex));
                                        }
                                        maps[mp].Material = subMat.get();
                                        ctx->materials.push_back(std::move(subMat));
                                    }
                                }
                                mat->Maps = maps.data();
                            }
                            matBindings[m_idx].Material = mat.get();
                            ctx->materials.push_back(std::move(mat));
                        }
                        mesh->MaterialBindings = matBindings.data();
                    }

                    // Bone Bindings
                    int32_t bbCount = *reinterpret_cast<const int32_t*>(pMesh + 28);
                    mesh->BoneBindingCount = std::max(0, bbCount);
                    if (bbCount > 0 && fixups[meshSec].count(meshOff + 32)) {
                        auto [bbArrSec, bbArrOff] = fixups[meshSec][meshOff + 32];
                        ctx->meshBoneBindings.emplace_back(bbCount);
                        auto& bBindings = ctx->meshBoneBindings.back();
                        for (int bb = 0; bb < bbCount; ++bb) {
                            uint32_t curBbOff = bbArrOff + bb * 36;
                            if (bbArrSec < ctx->sections.size() && curBbOff + 28 <= ctx->sections[bbArrSec].size()) {
                                const uint8_t* pBb = &ctx->sections[bbArrSec][curBbOff];
                                bBindings[bb].BoneName = const_cast<char*>(ctx->StoreString(ResolveString(ctx->sections, fixups, bbArrSec, curBbOff + 0)));
                                std::memcpy(bBindings[bb].OBBMin, pBb + 4, sizeof(granny_triple));
                                std::memcpy(bBindings[bb].OBBMax, pBb + 16, sizeof(granny_triple));
                            }
                        }
                        mesh->BoneBindings = bBindings.data();
                    }

                    // Assign vertex type definition based on rigidity
                    if (mesh->PrimaryVertexData) {
                        if (mesh->BoneBindingCount <= 1) {
                            mesh->PrimaryVertexData->VertexType = GrannyPNT332VertexType;
                        } else {
                            mesh->PrimaryVertexData->VertexType = GrannyPWNT3432VertexType;
                        }
                    }

                    bindings[b].Mesh = mesh.get();
                    ctx->meshes.push_back(std::move(mesh));
                }
                model->MeshBindings = bindings.data();
            }

            ctx->modelPtrs.push_back(model.get());
            ctx->models.push_back(std::move(model));
        }
        ctx->fileInfo.Models = ctx->modelPtrs.data();
    }

    // Unmarshal Animations
    if (animCount > 0 && fixups[rootNodeSec].count(rootNodeOff + 0x50)) {
        auto [aArrSec, aArrOff] = fixups[rootNodeSec][rootNodeOff + 0x50];
        ctx->animations.reserve(animCount);

        for (int a = 0; a < animCount; ++a) {
            if (!fixups[aArrSec].count(aArrOff + a * 4)) continue;
            auto [aSec, aOff] = fixups[aArrSec][aArrOff + a * 4];
            if (aSec >= ctx->sections.size() || aOff + 16 > ctx->sections[aSec].size()) continue;

            const uint8_t* pAnim = &ctx->sections[aSec][aOff];
            auto anim = std::make_unique<granny_animation>();
            std::memset(anim.get(), 0, sizeof(granny_animation));
            anim->Name         = ctx->StoreString(ResolveString(ctx->sections, fixups, aSec, aOff + 0));
            anim->Duration     = *reinterpret_cast<const float*>(pAnim + 4);
            anim->TimeStep     = *reinterpret_cast<const float*>(pAnim + 8);
            anim->Oversampling = *reinterpret_cast<const float*>(pAnim + 12);
            anim->TrackGroupCount = 0;

            ctx->animPtrs.push_back(anim.get());
            ctx->animations.push_back(std::move(anim));
        }
        ctx->fileInfo.Animations = ctx->animPtrs.data();
    }

    ctx->file.FileInfo     = &ctx->fileInfo;
    ctx->file.OwnsFileInfo = false;
    ctx->file.InternalData = ctx.get();

    return &ctx.release()->file;
}

extern "C" void GrannyFreeParsedGR2(granny_file* file)
{
    if (!file || !file->InternalData)
        return;

    auto* ctx = static_cast<GR2ParsedContext*>(file->InternalData);
    delete ctx;
}
