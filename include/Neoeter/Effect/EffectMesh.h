#pragma once

#include <Platform/Platform.h>
#include <Platform/DirectXMathCompat.h>

#include <Neoeter/Render/GrpScreen.h>
#include <Neoeter/Render/Resource.h>
#include <Neoeter/Render/GrpImageInstance.h>
#include <Neoeter/Render/TextFileLoader.h>

#include "Type.h"
#include "EffectElementBase.h"

class CEffectMesh : public CResource
{
	public:
		typedef struct SEffectFrameData
		{
				uint8_t byChangedFrame;
				float fVisibility;
				uint32_t dwVertexCount;
				uint32_t dwTextureVertexCount;
				uint32_t dwIndexCount;
				std::vector<TPTVertex> PDTVertexVector;
		} TEffectFrameData;

		typedef struct SEffectMeshData
		{
				char szObjectName[32];
				char szDiffuseMapFileName[128];

				std::vector<TEffectFrameData> EffectFrameDataVector;
				std::vector<CGraphicImage*> pImageVector;

				static SEffectMeshData* New();
				static void Delete(SEffectMeshData* pkData);

				static void DestroySystem();

				static CDynamicPool<SEffectMeshData> ms_kPool;
		} TEffectMeshData;

		// About Resource Code
	public:
		typedef CRef<CEffectMesh> TRef;

	public:
		static TType Type();

	public:
		CEffectMesh(const char* c_szFileName);
		virtual ~CEffectMesh();

		uint32_t GetFrameCount();
		uint32_t GetMeshCount();
		TEffectMeshData* GetMeshDataPointer(uint32_t dwMeshIndex);

		std::vector<CGraphicImage*>* GetTextureVectorPointer(uint32_t dwMeshIndex);
		std::vector<CGraphicImage*>& GetTextureVectorReference(uint32_t dwMeshIndex);

		// Exceptional function for tool
		int32_t GetMeshElementPointer(uint32_t dwMeshIndex, TEffectMeshData** ppMeshData);

	protected:
		bool OnLoad(int iSize, const void* c_pvBuf);

		void OnClear();
		bool OnIsEmpty() const;
		bool OnIsType(TType type);

		int32_t __LoadData_Ver001(int iSize, const uint8_t* c_pbBuf);
		int32_t __LoadData_Ver002(int iSize, const uint8_t* c_pbBuf);

	protected:
		int m_iGeomCount;
		int m_iFrameCount;
		std::vector<TEffectMeshData*> m_pEffectMeshDataVector;

		bool m_isData;
};

class CEffectMeshScript : public CEffectElementBase
{
	public:
		typedef struct SMeshData
		{
				uint8_t byBillboardType;

				int32_t bBlendingEnable;
				uint8_t byBlendingSrcType;
				uint8_t byBlendingDestType;
				int32_t bTextureAlphaEnable;

				uint8_t byColorOperationType;
				D3DXCOLOR ColorFactor;

				int32_t bTextureAnimationLoopEnable;
				float fTextureAnimationFrameDelay;

				uint32_t dwTextureAnimationStartFrame;

				TTimeEventTableFloat TimeEventAlpha;

				SMeshData()
				{
					TimeEventAlpha.clear();
				}
		} TMeshData;
		typedef std::vector<TMeshData> TMeshDataVector;

	public:
		CEffectMeshScript();
		virtual ~CEffectMeshScript();

		const char* GetMeshFileName();

		void ReserveMeshData(uint32_t dwMeshCount);
		bool CheckMeshIndex(uint32_t dwMeshIndex);
		bool GetMeshDataPointer(uint32_t dwMeshIndex, TMeshData** ppMeshData);
		int GetMeshDataCount();

		int GetBillboardType(uint32_t dwMeshIndex);
		int32_t isBlendingEnable(uint32_t dwMeshIndex);
		uint8_t GetBlendingSrcType(uint32_t dwMeshIndex);
		uint8_t GetBlendingDestType(uint32_t dwMeshIndex);
		int32_t isTextureAlphaEnable(uint32_t dwMeshIndex);
		int32_t GetColorOperationType(uint32_t dwMeshIndex, uint8_t* pbyType);
		int32_t GetColorFactor(uint32_t dwMeshIndex, D3DXCOLOR* pColor);
		int32_t GetTimeTableAlphaPointer(uint32_t dwMeshIndex, TTimeEventTableFloat** pTimeEventAlpha);

		int32_t isMeshAnimationLoop();
		int32_t GetMeshAnimationLoopCount();
		float GetMeshAnimationFrameDelay();
		int32_t isTextureAnimationLoop(uint32_t dwMeshIndex);
		float GetTextureAnimationFrameDelay(uint32_t dwMeshIndex);
		uint32_t GetTextureAnimationStartFrame(uint32_t dwMeshIndex);

	protected:
		void OnClear();
		bool OnIsData();
		int32_t OnLoadScript(CTextFileLoader& rTextFileLoader);

	protected:
		int32_t m_isMeshAnimationLoop;
		int m_iMeshAnimationLoopCount;
		float m_fMeshAnimationFrameDelay;
		TMeshDataVector m_MeshDataVector;

		std::string m_strMeshFileName;

	public:
		static void DestroySystem();

		static CEffectMeshScript* New();
		static void Delete(CEffectMeshScript* pkData);

		static CDynamicPool<CEffectMeshScript> ms_kPool;
};