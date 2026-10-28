#include <SpeedTreeLib/SpeedTreeRTCompat.h>
#include <cstdio>
#include <cctype>
#include <algorithm>

// Static member definitions
std::string   CSpeedTreeRT::ms_strError;
bool          CSpeedTreeRT::ms_bTextureFlip = false;
bool          CSpeedTreeRT::ms_bDropToBillboard = true;
float         CSpeedTreeRT::ms_afCameraPos[3] = {0.0f, 0.0f, 0.0f};
float         CSpeedTreeRT::ms_afCameraDir[3] = {0.0f, 0.0f, -1.0f};
float         CSpeedTreeRT::ms_afLightAttrs[16] = {0.0f};
bool          CSpeedTreeRT::ms_bLightOn = false;
unsigned int  CSpeedTreeRT::ms_unNumWindMatrices = 4;

// SGeometry implementations
CSpeedTreeRT::SGeometry::SIndexed::SIndexed()
    : m_nDiscreteLodLevel(-1),
      m_usNumStrips(0),
      m_pStripLengths(nullptr),
      m_pStrips(nullptr),
      m_usVertexCount(0),
      m_pColors(nullptr),
      m_pNormals(nullptr),
      m_pBinormals(nullptr),
      m_pTangents(nullptr),
      m_pCoords(nullptr),
      m_pTexCoords0(nullptr),
      m_pTexCoords1(nullptr),
      m_pWindWeights(nullptr),
      m_pWindMatrixIndices(nullptr)
{
}

CSpeedTreeRT::SGeometry::SIndexed::~SIndexed()
{
}

CSpeedTreeRT::SGeometry::SLeaf::SLeaf()
    : m_bIsActive(false),
      m_fAlphaTestValue(0.0f),
      m_nDiscreteLodLevel(-1),
      m_usLeafCount(0),
      m_pLeafMapIndices(nullptr),
      m_pLeafClusterIndices(nullptr),
      m_pCenterCoords(nullptr),
      m_pLeafMapTexCoords(nullptr),
      m_pLeafMapCoords(nullptr),
      m_pColors(nullptr),
      m_pNormals(nullptr),
      m_pBinormals(nullptr),
      m_pTangents(nullptr),
      m_pWindWeights(nullptr),
      m_pWindMatrixIndices(nullptr)
{
}

CSpeedTreeRT::SGeometry::SLeaf::~SLeaf()
{
}

CSpeedTreeRT::SGeometry::SBillboard::SBillboard()
    : m_bIsActive(false),
      m_pTexCoords(nullptr),
      m_pCoords(nullptr),
      m_fAlphaTestValue(0.0f)
{
}

CSpeedTreeRT::SGeometry::SBillboard::~SBillboard()
{
}

CSpeedTreeRT::SGeometry::SGeometry()
    : m_fBranchAlphaTestValue(0.0f),
      m_fFrondAlphaTestValue(0.0f)
{
}

CSpeedTreeRT::SGeometry::~SGeometry()
{
}

CSpeedTreeRT::STextures::STextures()
    : m_pBranchTextureFilename(nullptr),
      m_uiLeafTextureCount(0),
      m_pLeafTextureFilenames(nullptr),
      m_uiFrondTextureCount(0),
      m_pFrondTextureFilenames(nullptr),
      m_pCompositeFilename(nullptr),
      m_pSelfShadowFilename(nullptr)
{
}

CSpeedTreeRT::STextures::~STextures()
{
}

CSpeedTreeRT::CSpeedTreeRT()
    : m_fSize(400.0f),
      m_fSizeVariance(0.0f),
      m_nSeed(1),
      m_fWindStrength(0.0f),
      m_fLeafLightingAdjustment(1.0f),
      m_eBranchLighting(LIGHT_STATIC),
      m_eLeafLighting(LIGHT_STATIC),
      m_eFrondLighting(LIGHT_STATIC),
      m_eBranchWind(WIND_NONE),
      m_eLeafWind(WIND_NONE),
      m_eFrondWind(WIND_NONE),
      m_pInstanceOf(nullptr),
      m_bGeometryGenerated(false),
      m_usBranchStripLength(0)
{
    m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;
    m_afBoundingBox[0] = -100.0f;
    m_afBoundingBox[1] = -100.0f;
    m_afBoundingBox[2] = 0.0f;
    m_afBoundingBox[3] = 100.0f;
    m_afBoundingBox[4] = 100.0f;
    m_afBoundingBox[5] = 600.0f;

    // Default Material: Diffuse(1,1,1), Ambient(0.5,0.5,0.5), Specular(0.2,0.2,0.2), Emissive(0,0,0), Power(5.0)
    const float defaultMat[13] = {
        1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f,
        0.2f, 0.2f, 0.2f,
        0.0f, 0.0f, 0.0f,
        5.0f
    };
    std::memcpy(m_afBranchMaterial, defaultMat, sizeof(defaultMat));
    std::memcpy(m_afLeafMaterial, defaultMat, sizeof(defaultMat));
    std::memcpy(m_afFrondMaterial, defaultMat, sizeof(defaultMat));

    // Default trunk collision cylinder
    SCollisionObj obj;
    obj.m_eType = CO_CYLINDER;
    obj.m_afPos[0] = 0.0f;
    obj.m_afPos[1] = 0.0f;
    obj.m_afPos[2] = 0.0f;
    obj.m_afDims[0] = 25.0f;   // Radius
    obj.m_afDims[1] = 200.0f;  // Height
    obj.m_afDims[2] = 0.0f;
    m_vCollisions.push_back(obj);
}

CSpeedTreeRT::~CSpeedTreeRT()
{
}

bool CSpeedTreeRT::Compute(const float* pTransform, unsigned int nSeed, bool bCompositeStrips)
{
    (void)pTransform;
    (void)bCompositeStrips;
    m_nSeed = nSeed;
    m_bGeometryGenerated = false;
    GenerateGeometry();
    return true;
}

CSpeedTreeRT* CSpeedTreeRT::Clone(float x, float y, float z, unsigned int nSeed) const
{
    CSpeedTreeRT* pClone = new CSpeedTreeRT(*this);
    pClone->SetTreePosition(x, y, z);
    pClone->m_nSeed = nSeed;
    return pClone;
}

const CSpeedTreeRT* CSpeedTreeRT::InstanceOf(void) const
{
    return m_pInstanceOf;
}

CSpeedTreeRT* CSpeedTreeRT::MakeInstance(void)
{
    CSpeedTreeRT* pInstance = new CSpeedTreeRT();
    pInstance->m_pInstanceOf = this;
    pInstance->m_fSize = m_fSize;
    pInstance->m_fSizeVariance = m_fSizeVariance;
    pInstance->m_nSeed = m_nSeed;
    pInstance->m_fWindStrength = m_fWindStrength;
    pInstance->m_fLeafLightingAdjustment = m_fLeafLightingAdjustment;
    std::memcpy(pInstance->m_afBoundingBox, m_afBoundingBox, sizeof(m_afBoundingBox));
    std::memcpy(pInstance->m_afBranchMaterial, m_afBranchMaterial, sizeof(m_afBranchMaterial));
    std::memcpy(pInstance->m_afLeafMaterial, m_afLeafMaterial, sizeof(m_afLeafMaterial));
    std::memcpy(pInstance->m_afFrondMaterial, m_afFrondMaterial, sizeof(m_afFrondMaterial));
    pInstance->m_strBranchTexture = m_strBranchTexture;
    pInstance->m_strCompositeTexture = m_strCompositeTexture;
    pInstance->m_strSelfShadowTexture = m_strSelfShadowTexture;
    pInstance->m_vLeafTextures = m_vLeafTextures;
    pInstance->m_vFrondTextures = m_vFrondTextures;
    pInstance->m_vCollisions = m_vCollisions;
    return pInstance;
}

void CSpeedTreeRT::DeleteTransientData(void)
{
}

static std::string ExtractBaseName(const std::string& path)
{
    size_t lastSlash = path.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
    return filename;
}

bool CSpeedTreeRT::LoadTree(const unsigned char* pBlock, unsigned int nNumBytes)
{
    if (!pBlock || nNumBytes == 0)
    {
        ms_strError = "Empty SPT block";
        return false;
    }

    // Scan for texture filename strings in the SPT binary
    std::vector<std::string> tokens;
    std::string current;
    for (unsigned int i = 0; i < nNumBytes; ++i)
    {
        char c = static_cast<char>(pBlock[i]);
        if (std::isprint(static_cast<unsigned char>(c)) && c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != '"' && c != '\'')
        {
            current += c;
        }
        else
        {
            if (current.length() >= 4)
            {
                tokens.push_back(current);
            }
            current.clear();
        }
    }
    if (current.length() >= 4)
        tokens.push_back(current);

    for (const auto& rawTok : tokens)
    {
        std::string tok = ExtractBaseName(rawTok);

        // SpeedTree appends a texture-set suffix ("CompositeMapB1#N.tga").
        // The shipped DDS files do not contain it, so drop '#'..'extension'.
        const std::string::size_type hashPos = tok.find('#');
        if (hashPos != std::string::npos)
        {
            const std::string::size_type dotPos = tok.find_last_of('.');
            if (dotPos != std::string::npos && dotPos > hashPos)
                tok = tok.substr(0, hashPos) + tok.substr(dotPos);
            else
                tok = tok.substr(0, hashPos);
        }

        std::string lower;
        for (char ch : tok) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        if (lower.find("compositeshadowmap") != std::string::npos || (lower.find("shadow") != std::string::npos && lower.find(".tga") != std::string::npos))
        {
            // Intentionally ignored: the generated geometry does not provide
            // matching self-shadow texcoords, so leave the stage unbound.
        }
        else if (lower.find("compositemap") != std::string::npos || lower.find("composite") != std::string::npos)
        {
            if (m_strCompositeTexture.empty())
                m_strCompositeTexture = tok;
        }
        else if (lower.find("bark") != std::string::npos || (m_strBranchTexture.empty() && lower.find(".tga") != std::string::npos && lower.find("leaf") == std::string::npos && lower.find("frond") == std::string::npos))
        {
            if (m_strBranchTexture.empty())
                m_strBranchTexture = tok;
        }
        else if (lower.find("leaf") != std::string::npos || lower.find("leaves") != std::string::npos)
        {
            m_vLeafTextures.push_back(tok);
        }
        else if (lower.find("frond") != std::string::npos)
        {
            m_vFrondTextures.push_back(tok);
        }
    }

    // Read the tree height from the parameter stream (property 2001 is the
    // overall tree size in world units, stored as a little-endian float).
    for (unsigned int i = 0; i + 8 <= nNumBytes; ++i)
    {
        if (pBlock[i] == 0xD1 && pBlock[i + 1] == 0x07 && pBlock[i + 2] == 0x00 && pBlock[i + 3] == 0x00)
        {
            float fSize = 0.0f;
            std::memcpy(&fSize, pBlock + i + 4, sizeof(float));
            if (fSize > 20.0f && fSize < 4000.0f)
            {
                SetTreeSize(std::max(200.0f, fSize), 0.0f);
                break;
            }
        }
    }

    return true;
}

bool CSpeedTreeRT::LoadTree(const char* pFilename)
{
    if (!pFilename || !pFilename[0])
    {
        ms_strError = "Invalid tree filename";
        return false;
    }

    FILE* fp = std::fopen(pFilename, "rb");
    if (!fp)
    {
        ms_strError = std::string("Cannot open tree file: ") + pFilename;
        return false;
    }

    std::fseek(fp, 0, SEEK_END);
    long sz = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);

    if (sz <= 0)
    {
        std::fclose(fp);
        ms_strError = "Empty tree file";
        return false;
    }

    std::vector<unsigned char> buf(static_cast<size_t>(sz));
    size_t readBytes = std::fread(buf.data(), 1, buf.size(), fp);
    std::fclose(fp);

    return LoadTree(buf.data(), static_cast<unsigned int>(readBytes));
}

unsigned char* CSpeedTreeRT::SaveTree(unsigned int& nNumBytes, bool bSaveLeaves) const
{
    nNumBytes = 0;
    return nullptr;
}

void CSpeedTreeRT::GetTreeSize(float& fSize, float& fVariance) const
{
    fSize = m_fSize;
    fVariance = m_fSizeVariance;
}

void CSpeedTreeRT::SetTreeSize(float fNewSize, float fNewVariance)
{
    m_fSize = fNewSize;
    m_fSizeVariance = fNewVariance;
    m_bGeometryGenerated = false;
    if (fNewSize > 0.0f)
    {
        m_afBoundingBox[0] = -fNewSize * 0.25f;
        m_afBoundingBox[1] = -fNewSize * 0.25f;
        m_afBoundingBox[2] = 0.0f;
        m_afBoundingBox[3] = fNewSize * 0.25f;
        m_afBoundingBox[4] = fNewSize * 0.25f;
        m_afBoundingBox[5] = fNewSize;
    }
}

unsigned int CSpeedTreeRT::GetSeed() const
{
    return m_nSeed;
}

const float* CSpeedTreeRT::GetTreePosition(void) const
{
    return m_afPos;
}

void CSpeedTreeRT::SetTreePosition(float x, float y, float z)
{
    m_afPos[0] = x;
    m_afPos[1] = y;
    m_afPos[2] = z;
}

void CSpeedTreeRT::SetLeafTargetAlphaMask(unsigned char ucMask)
{
}

CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetBranchLightingMethod(void) const { return m_eBranchLighting; }
void CSpeedTreeRT::SetBranchLightingMethod(ELightingMethod eMethod) { m_eBranchLighting = eMethod; }
CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetLeafLightingMethod(void) const { return m_eLeafLighting; }
void CSpeedTreeRT::SetLeafLightingMethod(ELightingMethod eMethod) { m_eLeafLighting = eMethod; }
CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetFrondLightingMethod(void) const { return m_eFrondLighting; }
void CSpeedTreeRT::SetFrondLightingMethod(ELightingMethod eMethod) { m_eFrondLighting = eMethod; }

CSpeedTreeRT::EStaticLightingStyle CSpeedTreeRT::GetStaticLightingStyle(void) const { return SLS_BASIC; }
void CSpeedTreeRT::SetStaticLightingStyle(EStaticLightingStyle eStyle) {}

float CSpeedTreeRT::GetLeafLightingAdjustment() const { return m_fLeafLightingAdjustment; }
void CSpeedTreeRT::SetLeafLightingAdjustment(float fScalar) { m_fLeafLightingAdjustment = fScalar; }

bool CSpeedTreeRT::GetLightState(unsigned int nLightIndex) { return ms_bLightOn; }
void CSpeedTreeRT::SetLightState(unsigned int nLightIndex, bool bLightOn) { ms_bLightOn = bLightOn; }
const float* CSpeedTreeRT::GetLightAttributes(unsigned int nLightIndex) { return ms_afLightAttrs; }
void CSpeedTreeRT::SetLightAttributes(unsigned int nLightIndex, const float* pLightAttributes)
{
    if (pLightAttributes)
        std::memcpy(ms_afLightAttrs, pLightAttributes, sizeof(ms_afLightAttrs));
}

const float* CSpeedTreeRT::GetBranchMaterial(void) const { return m_afBranchMaterial; }
void CSpeedTreeRT::SetBranchMaterial(const float* pMaterial)
{
    if (pMaterial) std::memcpy(m_afBranchMaterial, pMaterial, sizeof(m_afBranchMaterial));
}

const float* CSpeedTreeRT::GetLeafMaterial(void) const { return m_afLeafMaterial; }
void CSpeedTreeRT::SetLeafMaterial(const float* pMaterial)
{
    if (pMaterial) std::memcpy(m_afLeafMaterial, pMaterial, sizeof(m_afLeafMaterial));
}

const float* CSpeedTreeRT::GetFrondMaterial(void) const { return m_afFrondMaterial; }
void CSpeedTreeRT::SetFrondMaterial(const float* pMaterial)
{
    if (pMaterial) std::memcpy(m_afFrondMaterial, pMaterial, sizeof(m_afFrondMaterial));
}

void CSpeedTreeRT::GetCamera(float* pPosition, float* pDirection)
{
    if (pPosition) std::memcpy(pPosition, ms_afCameraPos, sizeof(ms_afCameraPos));
    if (pDirection) std::memcpy(pDirection, ms_afCameraDir, sizeof(ms_afCameraDir));
}

void CSpeedTreeRT::SetCamera(const float* pPosition, const float* pDirection)
{
    if (pPosition) std::memcpy(ms_afCameraPos, pPosition, sizeof(ms_afCameraPos));
    if (pDirection) std::memcpy(ms_afCameraDir, pDirection, sizeof(ms_afCameraDir));
}

void CSpeedTreeRT::SetTime(float fTime) {}
void CSpeedTreeRT::ComputeWindEffects(bool bBranches, bool bLeaves, bool bFronds) {}
void CSpeedTreeRT::ResetLeafWindState(void) {}
bool CSpeedTreeRT::GetLeafRockingState(void) const { return true; }
void CSpeedTreeRT::SetLeafRockingState(bool bFlag) {}
void CSpeedTreeRT::SetNumLeafRockingGroups(unsigned int nRockingGroups) {}

CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetLeafWindMethod(void) const { return m_eLeafWind; }
void CSpeedTreeRT::SetLeafWindMethod(EWindMethod eMethod) { m_eLeafWind = eMethod; }
CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetBranchWindMethod(void) const { return m_eBranchWind; }
void CSpeedTreeRT::SetBranchWindMethod(EWindMethod eMethod) { m_eBranchWind = eMethod; }
CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetFrondWindMethod(void) const { return m_eFrondWind; }
void CSpeedTreeRT::SetFrondWindMethod(EWindMethod eMethod) { m_eFrondWind = eMethod; }

float CSpeedTreeRT::GetWindStrength(void) const { return m_fWindStrength; }
float CSpeedTreeRT::SetWindStrength(float fNewStrength, float fOldStrength, float fFrequencyTimeOffset)
{
    m_fWindStrength = fNewStrength;
    return m_fWindStrength;
}

void CSpeedTreeRT::SetNumWindMatrices(unsigned int nNumMatrices) { ms_unNumWindMatrices = nNumMatrices; }
void CSpeedTreeRT::SetWindMatrix(unsigned int nMatrixIndex, const float* pMatrix) {}
void CSpeedTreeRT::GetLocalMatrices(unsigned int& nStartingIndex, unsigned int& nMatrixSpan)
{
    nStartingIndex = 0;
    nMatrixSpan = 0;
}
void CSpeedTreeRT::SetLocalMatrices(unsigned int nStartingMatrix, unsigned int nMatrixSpan) {}

void CSpeedTreeRT::ComputeLodLevel(void) {}
float CSpeedTreeRT::GetLodLevel(void) const { return 1.0f; }
void CSpeedTreeRT::SetLodLevel(float fLodLevel) {}
void CSpeedTreeRT::SetDropToBillboard(bool bFlag) { ms_bDropToBillboard = bFlag; }

void CSpeedTreeRT::GetLodLimits(float& fNear, float& fFar) const { fNear = 100.0f; fFar = 1000.0f; }
void CSpeedTreeRT::SetLodLimits(float fNear, float fFar) {}

short CSpeedTreeRT::GetDiscreteBranchLodLevel(float fLodLevel) const { return 0; }
unsigned short CSpeedTreeRT::GetDiscreteLeafLodLevel(float fLodLevel) const { return 0; }
short CSpeedTreeRT::GetDiscreteFrondLodLevel(float fLodLevel) const { return 0; }

unsigned short CSpeedTreeRT::GetNumBranchLodLevels(void) const { return 1; }
unsigned short CSpeedTreeRT::GetNumLeafLodLevels(void) const { return 1; }
unsigned short CSpeedTreeRT::GetNumFrondLodLevels(void) const { return 1; }

void CSpeedTreeRT::DeleteBranchGeometry(void) {}
void CSpeedTreeRT::DeleteFrondGeometry(void) {}
unsigned char* CSpeedTreeRT::GetFrondGeometryMapIndexes(int nLodLevel) const { return nullptr; }
const float* CSpeedTreeRT::GetLeafBillboardTable(unsigned int& nEntryCount) const { nEntryCount = 0; return nullptr; }
const float* CSpeedTreeRT::GetLeafLodSizeAdjustments(void) { return nullptr; }

void CSpeedTreeRT::GenerateGeometry()
{
    if (m_bGeometryGenerated)
        return;
    m_bGeometryGenerated = true;

    const float H = (m_fSize > 1.0f) ? m_fSize : 400.0f;

    m_vBranchCoords.clear();
    m_vBranchColors.clear();
    m_vBranchTexCoords0.clear();
    m_vBranchTexCoords1.clear();
    m_vBranchStrip.clear();

    m_vLeafCenters.clear();
    m_vLeafColors.clear();
    m_vLeafSize.clear();
    m_vLeafAngle.clear();
    m_vLeafCoords.clear();
    m_vLeafTexCoords.clear();
    m_vLeafMapIndices.clear();
    m_vLeafClusterIndices.clear();

    // ------------------------------------------------------------------
    // Branches: tapered tubes stitched into a single triangle strip.
    // ------------------------------------------------------------------
    auto addTube = [&](float sx, float sy, float sz,
                       float ex, float ey, float ez,
                       float r0, float r1, float vTile, bool bFlip)
    {
        float dx = ex - sx, dy = ey - sy, dz = ez - sz;
        float len = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (len < 1e-4f)
            return;
        dx /= len; dy /= len; dz /= len;

        float rx = 0.0f, ry = 0.0f, rz = 1.0f;
        if (std::fabs(dz) > 0.9f) { rx = 1.0f; rz = 0.0f; }

        // right = normalize(cross(ref, dir))
        float rght[3] = { ry * dz - rz * dy, rz * dx - rx * dz, rx * dy - ry * dx };
        float rl = std::sqrt(rght[0] * rght[0] + rght[1] * rght[1] + rght[2] * rght[2]);
        if (rl < 1e-6f) return;
        rght[0] /= rl; rght[1] /= rl; rght[2] /= rl;

        // up = cross(dir, right)
        float up[3] = { dy * rght[2] - dz * rght[1],
                        dz * rght[0] - dx * rght[2],
                        dx * rght[1] - dy * rght[0] };

        const int sides = 6;
        const unsigned short base = static_cast<unsigned short>(m_vBranchCoords.size() / 3);
        const float sign = bFlip ? 1.0f : -1.0f;

        for (int i = 0; i <= sides; ++i) {
            const float a = sign * 2.0f * 3.14159265358979f * static_cast<float>(i) / static_cast<float>(sides);
            const float ca = std::cos(a), sa = std::sin(a);
            const float ox = rght[0] * ca + up[0] * sa;
            const float oy = rght[1] * ca + up[1] * sa;
            const float oz = rght[2] * ca + up[2] * sa;
            const float u = 2.0f * static_cast<float>(i) / static_cast<float>(sides);

            m_vBranchCoords.push_back(sx + ox * r0);
            m_vBranchCoords.push_back(sy + oy * r0);
            m_vBranchCoords.push_back(sz + oz * r0);
            m_vBranchColors.push_back(0xFFFFFFFFul);
            m_vBranchTexCoords0.push_back(u);
            m_vBranchTexCoords0.push_back(0.0f);
            m_vBranchTexCoords1.push_back(u);
            m_vBranchTexCoords1.push_back(0.0f);

            m_vBranchCoords.push_back(ex + ox * r1);
            m_vBranchCoords.push_back(ey + oy * r1);
            m_vBranchCoords.push_back(ez + oz * r1);
            m_vBranchColors.push_back(0xFFFFFFFFul);
            m_vBranchTexCoords0.push_back(u);
            m_vBranchTexCoords0.push_back(vTile);
            m_vBranchTexCoords1.push_back(u);
            m_vBranchTexCoords1.push_back(vTile);
        }

        const unsigned short endIdx = base + static_cast<unsigned short>(2 * (sides + 1));
        if (m_vBranchStrip.empty()) {
            for (unsigned short k = base; k < endIdx; ++k)
                m_vBranchStrip.push_back(k);
        } else {
            const unsigned short last = m_vBranchStrip.back();
            m_vBranchStrip.push_back(last);   // degenerate stitch
            m_vBranchStrip.push_back(base);
            m_vBranchStrip.push_back(base);
            for (unsigned short k = static_cast<unsigned short>(base + 1); k < endIdx; ++k)
                m_vBranchStrip.push_back(k);
        }
    };

    // Double sided tubes: the branch pass uses backface culling.
    auto addTaperedTube = [&](float sx, float sy, float sz,
                              float ex, float ey, float ez,
                              float r0, float r1, float vTile)
    {
        addTube(sx, sy, sz, ex, ey, ez, r0, r1, vTile, false);
        addTube(sx, sy, sz, ex, ey, ez, r0, r1, vTile, true);
    };

    // Trunk
    addTaperedTube(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.52f * H,
                   0.040f * H, 0.020f * H, 0.75f * H / 100.0f);

    // Primary limbs
    for (int i = 0; i < 4; ++i) {
        const float ang = static_cast<float>(i) * 1.5707963f + 0.35f;
        const float h0 = (0.40f + 0.04f * static_cast<float>(i)) * H;
        float dirx = std::cos(ang) * 0.62f;
        float diry = std::sin(ang) * 0.62f;
        float dirz = 0.78f;
        const float dl = std::sqrt(dirx * dirx + diry * diry + dirz * dirz);
        dirx /= dl; diry /= dl; dirz /= dl;
        const float l = 0.30f * H;
        const float ex = dirx * l;
        const float ey = diry * l;
        const float ez = h0 + dirz * l;
        addTaperedTube(0.0f, 0.0f, h0, ex, ey, ez, 0.014f * H, 0.005f * H, 0.35f * H / 100.0f);
    }

    // Secondary twigs
    for (int i = 0; i < 8; ++i) {
        const float ang = static_cast<float>(i) * 0.7853982f + 0.7f;
        const float h0 = (0.52f + 0.02f * static_cast<float>(i % 3)) * H;
        float dirx = std::cos(ang) * 0.80f;
        float diry = std::sin(ang) * 0.80f;
        float dirz = 0.60f;
        const float dl = std::sqrt(dirx * dirx + diry * diry + dirz * dirz);
        dirx /= dl; diry /= dl; dirz /= dl;
        const float l = 0.16f * H;
        addTaperedTube(std::cos(ang) * 0.10f * H, std::sin(ang) * 0.10f * H, h0,
                       std::cos(ang) * 0.10f * H + dirx * l,
                       std::sin(ang) * 0.10f * H + diry * l,
                       h0 + dirz * l,
                       0.006f * H, 0.002f * H, 0.20f * H / 100.0f);
    }

    m_usBranchStripLength = static_cast<unsigned short>(m_vBranchStrip.size());
    m_vpBranchStrips.clear();
    m_vpBranchStrips.push_back(m_vBranchStrip.data());

    // ------------------------------------------------------------------
    // Leaves: camera facing cards distributed in an ellipsoidal crown.
    // ------------------------------------------------------------------
    const int nLeafCount = 96;
    unsigned int seed = 0x9E3779B9u ^ m_nSeed ^
                        (static_cast<unsigned int>(m_afPos[0] * 13.0f) * 2654435761u) ^
                        (static_cast<unsigned int>(m_afPos[1] * 7.0f) * 40503u);
    auto rnd = [&seed]() -> float {
        seed = seed * 1664525u + 1013904223u;
        return static_cast<float>((seed >> 8) & 0xFFFFFF) / 16777216.0f;
    };

    const float crownZ = 0.70f * H;
    const float crownRxy = 0.32f * H;
    const float crownRz = 0.24f * H;

    for (int i = 0; i < nLeafCount; ++i) {
        const float theta = rnd() * 6.2831853f;
        const float zc = rnd() * 2.0f - 1.0f;
        const float rr = std::sqrt(std::max(0.0f, 1.0f - zc * zc));
        const float cx = crownRxy * rr * std::cos(theta);
        const float cy = crownRxy * rr * std::sin(theta);
        const float cz = crownZ + crownRz * zc;

        m_vLeafCenters.push_back(cx);
        m_vLeafCenters.push_back(cy);
        m_vLeafCenters.push_back(cz);
        m_vLeafColors.push_back(0xFFFFFFFFul);
        m_vLeafSize.push_back((0.15f + 0.09f * rnd()) * H);
        m_vLeafAngle.push_back(rnd() * 6.2831853f);

        std::array<float, 16> coords{};
        coords[3] = coords[7] = coords[11] = coords[15] = 0.0f;
        m_vLeafCoords.push_back(coords);

        static const float kUvRects[6][4] = {
            { 0.00f, 0.25f, 0.25f, 0.50f },
            { 0.25f, 0.25f, 0.50f, 0.50f },
            { 0.00f, 0.50f, 0.25f, 0.75f },
            { 0.25f, 0.50f, 0.50f, 0.75f },
            { 0.00f, 0.75f, 0.25f, 1.00f },
            { 0.25f, 0.75f, 0.50f, 1.00f },
        };
        const float* uv = kUvRects[i % 6];
        std::array<float, 8> tex{};
        tex[0] = uv[0]; tex[1] = uv[3]; // bottom left  (v=1 is bottom of the atlas)
        tex[2] = uv[2]; tex[3] = uv[3]; // bottom right
        tex[4] = uv[2]; tex[5] = uv[1]; // top right
        tex[6] = uv[0]; tex[7] = uv[1]; // top left
        m_vLeafTexCoords.push_back(tex);

        m_vLeafMapIndices.push_back(0);
        m_vLeafClusterIndices.push_back(static_cast<unsigned char>(i % 4));
    }

    m_vpLeafCoords.clear();
    m_vpLeafTexCoords.clear();
    for (size_t i = 0; i < m_vLeafCoords.size(); ++i) {
        m_vpLeafCoords.push_back(m_vLeafCoords[i].data());
        m_vpLeafTexCoords.push_back(m_vLeafTexCoords[i].data());
    }

    UpdateLeafBillboards();

    // bounds + trunk collision
    m_afBoundingBox[0] = -0.35f * H;
    m_afBoundingBox[1] = -0.35f * H;
    m_afBoundingBox[2] = 0.0f;
    m_afBoundingBox[3] = 0.35f * H;
    m_afBoundingBox[4] = 0.35f * H;
    m_afBoundingBox[5] = 1.0f * H;

    m_vCollisions.clear();
    SCollisionObj obj;
    obj.m_eType = CO_CYLINDER;
    obj.m_afPos[0] = 0.0f;
    obj.m_afPos[1] = 0.0f;
    obj.m_afPos[2] = 0.0f;
    obj.m_afDims[0] = 0.045f * H;  // radius
    obj.m_afDims[1] = 0.60f * H;   // height
    obj.m_afDims[2] = 0.0f;
    m_vCollisions.push_back(obj);
}

void CSpeedTreeRT::UpdateLeafBillboards()
{
    const size_t nLeafCount = m_vLeafCenters.size() / 3;
    if (nLeafCount == 0 || m_vLeafCoords.size() != nLeafCount)
        return;

    // Camera facing basis around the tree position.
    float vx = m_afPos[0] - ms_afCameraPos[0];
    float vy = m_afPos[1] - ms_afCameraPos[1];
    float vz = m_afPos[2] - ms_afCameraPos[2];
    float vl = std::sqrt(vx * vx + vy * vy + vz * vz);
    if (vl < 1e-4f) { vx = 0.0f; vy = 1.0f; vz = 0.0f; }
    else { vx /= vl; vy /= vl; vz /= vl; }

    // right = normalize(cross(worldUp, view))
    float rght[3] = { 0.0f * vz - 1.0f * vy, 1.0f * vx - 0.0f * vz, 0.0f * vy - 0.0f * vx };
    float rl = std::sqrt(rght[0] * rght[0] + rght[1] * rght[1] + rght[2] * rght[2]);
    if (rl < 1e-4f) { rght[0] = 1.0f; rght[1] = 0.0f; rght[2] = 0.0f; rl = 1.0f; }
    rght[0] /= rl; rght[1] /= rl; rght[2] /= rl;

    // up = cross(view, right)
    const float up[3] = { vy * rght[2] - vz * rght[1],
                          vz * rght[0] - vx * rght[2],
                          vx * rght[1] - vy * rght[0] };

    for (size_t i = 0; i < nLeafCount; ++i) {
        const float ca = std::cos(m_vLeafAngle[i]);
        const float sa = std::sin(m_vLeafAngle[i]);
        const float rx = rght[0] * ca + up[0] * sa;
        const float ry = rght[1] * ca + up[1] * sa;
        const float rz = rght[2] * ca + up[2] * sa;
        const float ux = up[0] * ca - rght[0] * sa;
        const float uy = up[1] * ca - rght[1] * sa;
        const float uz = up[2] * ca - rght[2] * sa;

        const float hw = m_vLeafSize[i] * 0.5f;
        const float hh = m_vLeafSize[i] * 0.5f;

        std::array<float, 16>& c = m_vLeafCoords[i];
        // corner 0: -r -u
        c[0] = -rx * hw - ux * hh; c[1] = -ry * hw - uy * hh; c[2] = -rz * hw - uz * hh; c[3] = 0.0f;
        // corner 1: +r -u
        c[4] =  rx * hw - ux * hh; c[5] =  ry * hw - uy * hh; c[6] =  rz * hw - uz * hh; c[7] = 0.0f;
        // corner 2: +r +u
        c[8] =  rx * hw + ux * hh; c[9] =  ry * hw + uy * hh; c[10] = rz * hw + uz * hh; c[11] = 0.0f;
        // corner 3: -r +u
        c[12] = -rx * hw + ux * hh; c[13] = -ry * hw + uy * hh; c[14] = -rz * hw + uz * hh; c[15] = 0.0f;
    }
}

void CSpeedTreeRT::GetGeometry(SGeometry& sGeometry, unsigned long ulBitVector, short sOverrideBranchLodValue, short sOverrideFrondLodValue, short sOverrideLeafLodValue)
{
    (void)sOverrideBranchLodValue;
    (void)sOverrideFrondLodValue;
    (void)sOverrideLeafLodValue;

    if (!m_bGeometryGenerated)
        GenerateGeometry();

    if (ulBitVector & SpeedTree_BranchGeometry)
    {
        sGeometry.m_sBranches.m_usVertexCount = static_cast<unsigned short>(m_vBranchCoords.size() / 3);
        sGeometry.m_sBranches.m_nDiscreteLodLevel = 0;
        sGeometry.m_sBranches.m_usNumStrips = 1;
        sGeometry.m_sBranches.m_pStripLengths = &m_usBranchStripLength;
        sGeometry.m_sBranches.m_pStrips = m_vpBranchStrips.data();
        sGeometry.m_sBranches.m_pColors = m_vBranchColors.data();
        sGeometry.m_sBranches.m_pCoords = m_vBranchCoords.data();
        sGeometry.m_sBranches.m_pTexCoords0 = m_vBranchTexCoords0.data();
        sGeometry.m_sBranches.m_pTexCoords1 = m_vBranchTexCoords1.data();
        sGeometry.m_sBranches.m_pNormals = nullptr;
        sGeometry.m_sBranches.m_pBinormals = nullptr;
        sGeometry.m_sBranches.m_pTangents = nullptr;
        sGeometry.m_sBranches.m_pWindWeights = nullptr;
        sGeometry.m_sBranches.m_pWindMatrixIndices = nullptr;
        sGeometry.m_fBranchAlphaTestValue = 1.0f;
    }

    if (ulBitVector & SpeedTree_FrondGeometry)
    {
        sGeometry.m_sFronds.m_usVertexCount = 0;
        sGeometry.m_sFronds.m_nDiscreteLodLevel = -1;
        sGeometry.m_sFronds.m_usNumStrips = 0;
        sGeometry.m_fFrondAlphaTestValue = 0.0f;
    }

    if (ulBitVector & SpeedTree_LeafGeometry)
    {
        UpdateLeafBillboards();

        sGeometry.m_sLeaves0.m_bIsActive = true;
        sGeometry.m_sLeaves0.m_fAlphaTestValue = 128.0f;
        sGeometry.m_sLeaves0.m_nDiscreteLodLevel = 0;
        sGeometry.m_sLeaves0.m_usLeafCount = static_cast<unsigned short>(m_vLeafCenters.size() / 3);
        sGeometry.m_sLeaves0.m_pLeafMapIndices = m_vLeafMapIndices.data();
        sGeometry.m_sLeaves0.m_pLeafClusterIndices = m_vLeafClusterIndices.data();
        sGeometry.m_sLeaves0.m_pCenterCoords = m_vLeafCenters.data();
        sGeometry.m_sLeaves0.m_pLeafMapTexCoords = m_vpLeafTexCoords.data();
        sGeometry.m_sLeaves0.m_pLeafMapCoords = m_vpLeafCoords.data();
        sGeometry.m_sLeaves0.m_pColors = m_vLeafColors.data();
        sGeometry.m_sLeaves0.m_pNormals = nullptr;
        sGeometry.m_sLeaves0.m_pBinormals = nullptr;
        sGeometry.m_sLeaves0.m_pTangents = nullptr;
        sGeometry.m_sLeaves0.m_pWindWeights = nullptr;
        sGeometry.m_sLeaves0.m_pWindMatrixIndices = nullptr;

        sGeometry.m_sLeaves1.m_bIsActive = false;
        sGeometry.m_sLeaves1.m_usLeafCount = 0;
        sGeometry.m_sLeaves1.m_nDiscreteLodLevel = -1;
    }

    if (ulBitVector & SpeedTree_BillboardGeometry)
    {
        sGeometry.m_sBillboard0.m_bIsActive = false;
        sGeometry.m_sBillboard1.m_bIsActive = false;
        sGeometry.m_sHorizontalBillboard.m_bIsActive = false;
    }
}

void CSpeedTreeRT::GetTextures(STextures& sTextures) const
{
    sTextures.m_pBranchTextureFilename = m_strBranchTexture.empty() ? nullptr : m_strBranchTexture.c_str();
    sTextures.m_pCompositeFilename = m_strCompositeTexture.empty() ? nullptr : m_strCompositeTexture.c_str();
    sTextures.m_pSelfShadowFilename = m_strSelfShadowTexture.empty() ? nullptr : m_strSelfShadowTexture.c_str();

    m_vpLeafTextures.clear();
    for (const auto& s : m_vLeafTextures)
        m_vpLeafTextures.push_back(s.c_str());
    sTextures.m_uiLeafTextureCount = static_cast<unsigned int>(m_vpLeafTextures.size());
    sTextures.m_pLeafTextureFilenames = m_vpLeafTextures.empty() ? nullptr : m_vpLeafTextures.data();

    m_vpFrondTextures.clear();
    for (const auto& s : m_vFrondTextures)
        m_vpFrondTextures.push_back(s.c_str());
    sTextures.m_uiFrondTextureCount = static_cast<unsigned int>(m_vpFrondTextures.size());
    sTextures.m_pFrondTextureFilenames = m_vpFrondTextures.empty() ? nullptr : m_vpFrondTextures.data();
}

void CSpeedTreeRT::SetLeafTextureCoords(unsigned int nLeafMapIndex, const float* pTexCoords) {}
void CSpeedTreeRT::SetFrondTextureCoords(unsigned int nFrondMapIndex, const float* pTexCoords) {}
bool CSpeedTreeRT::GetTextureFlip(void) { return ms_bTextureFlip; }
void CSpeedTreeRT::SetTextureFlip(bool bFlag) { ms_bTextureFlip = bFlag; }

void CSpeedTreeRT::SetBranchTextureFilename(const char* pFilename)
{
    m_strBranchTexture = pFilename ? pFilename : "";
}

void CSpeedTreeRT::SetLeafTextureFilename(unsigned int nLeafMapIndex, const char* pFilename)
{
    if (nLeafMapIndex < m_vLeafTextures.size() && pFilename)
        m_vLeafTextures[nLeafMapIndex] = pFilename;
}

void CSpeedTreeRT::SetFrondTextureFilename(unsigned int nFrondMapIndex, const char* pFilename)
{
    if (nFrondMapIndex < m_vFrondTextures.size() && pFilename)
        m_vFrondTextures[nFrondMapIndex] = pFilename;
}

void CSpeedTreeRT::Authorize(const char* pKey) {}
bool CSpeedTreeRT::IsAuthorized(void) { return true; }
const char* CSpeedTreeRT::GetCurrentError(void) { return ms_strError.c_str(); }
void CSpeedTreeRT::ResetError(void) { ms_strError.clear(); }

void CSpeedTreeRT::GetBoundingBox(float* pBounds) const
{
    if (pBounds)
        std::memcpy(pBounds, m_afBoundingBox, sizeof(m_afBoundingBox));
}

unsigned int CSpeedTreeRT::GetLeafTriangleCount(float fLodLevel) const { (void)fLodLevel; return (unsigned int)(m_vLeafCenters.size() / 3) * 2; }
unsigned int CSpeedTreeRT::GetBranchTriangleCount(float fLodLevel) const { (void)fLodLevel; return m_usBranchStripLength > 2 ? (unsigned int)(m_usBranchStripLength - 2) : 0; }
unsigned int CSpeedTreeRT::GetFrondTriangleCount(float fLodLevel) const { return 0; }

unsigned int CSpeedTreeRT::GetCollisionObjectCount(void)
{
    return static_cast<unsigned int>(m_vCollisions.size());
}

void CSpeedTreeRT::GetCollisionObject(unsigned int nIndex, ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
    if (nIndex < m_vCollisions.size())
    {
        eType = m_vCollisions[nIndex].m_eType;
        if (pPosition) std::memcpy(pPosition, m_vCollisions[nIndex].m_afPos, 3 * sizeof(float));
        if (pDimensions) std::memcpy(pDimensions, m_vCollisions[nIndex].m_afDims, 3 * sizeof(float));
    }
}

const char* CSpeedTreeRT::GetUserData(void) const
{
    return nullptr;
}
