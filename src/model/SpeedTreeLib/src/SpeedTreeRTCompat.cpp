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
      m_pInstanceOf(nullptr)
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
    m_nSeed = nSeed;
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
        std::string lower;
        for (char ch : tok) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        if (lower.find("compositeshadowmap") != std::string::npos || (lower.find("shadow") != std::string::npos && lower.find(".tga") != std::string::npos))
        {
            if (m_strSelfShadowTexture.empty())
                m_strSelfShadowTexture = tok;
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

void CSpeedTreeRT::GetGeometry(SGeometry& sGeometry, unsigned long ulBitVector, short sOverrideBranchLodValue, short sOverrideFrondLodValue, short sOverrideLeafLodValue)
{
    sGeometry.m_sBranches.m_usVertexCount = 0;
    sGeometry.m_sBranches.m_nDiscreteLodLevel = -1;
    sGeometry.m_sBranches.m_usNumStrips = 0;
    sGeometry.m_fBranchAlphaTestValue = 0.0f;

    sGeometry.m_sFronds.m_usVertexCount = 0;
    sGeometry.m_sFronds.m_nDiscreteLodLevel = -1;
    sGeometry.m_sFronds.m_usNumStrips = 0;
    sGeometry.m_fFrondAlphaTestValue = 0.0f;

    sGeometry.m_sLeaves0.m_usLeafCount = 0;
    sGeometry.m_sLeaves0.m_bIsActive = false;
    sGeometry.m_sLeaves0.m_nDiscreteLodLevel = -1;

    sGeometry.m_sLeaves1.m_usLeafCount = 0;
    sGeometry.m_sLeaves1.m_bIsActive = false;
    sGeometry.m_sLeaves1.m_nDiscreteLodLevel = -1;

    sGeometry.m_sBillboard0.m_bIsActive = false;
    sGeometry.m_sBillboard1.m_bIsActive = false;
    sGeometry.m_sHorizontalBillboard.m_bIsActive = false;
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

unsigned int CSpeedTreeRT::GetLeafTriangleCount(float fLodLevel) const { return 0; }
unsigned int CSpeedTreeRT::GetBranchTriangleCount(float fLodLevel) const { return 0; }
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
