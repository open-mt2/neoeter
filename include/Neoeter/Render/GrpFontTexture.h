#pragma once

#include "GrpTexture.h"
#include "GrpImageTexture.h"
#include "GrpDIB.h"

#include <map>
#include <memory>
#include <vector>

class CFontRasterizer;

class CGraphicFontTexture : public CGraphicTexture
{
	public:
		typedef std::pair<uint16_t, wchar_t> TCharacterKey;

		typedef struct SCharacterInfomation
		{
				short index;
				short width;
				short height;
				float left;
				float top;
				float right;
				float bottom;
				float advance;
		} TCharacterInfomation;

		typedef std::vector<TCharacterInfomation*> TPCharacterInfomationVector;

	public:
		CGraphicFontTexture();
		virtual ~CGraphicFontTexture();

		void Destroy();
		bool Create(const char* c_szFontName, int fontSize, bool bItalic);

		bool CreateDeviceObjects();
		void DestroyDeviceObjects();

		bool CheckTextureIndex(uint32_t dwTexture);
		void SelectTexture(uint32_t dwTexture);

		bool UpdateTexture();

		TCharacterInfomation* GetCharacterInfomation(uint16_t codePage, wchar_t keyValue);
		TCharacterInfomation* UpdateCharacterInfomation(TCharacterKey code);

		bool IsEmpty() const;

	protected:
		void Initialize();

		bool AppendTexture();

	protected:
		typedef std::vector<CGraphicImageTexture*> TGraphicImageTexturePointerVector;
		typedef std::map<TCharacterKey, TCharacterInfomation> TCharacterInfomationMap;

	protected:
		CGraphicDib m_dib;
		std::unique_ptr<CFontRasterizer> m_pRasterizer;

		TGraphicImageTexturePointerVector m_pFontTextureVector;

		TCharacterInfomationMap m_charInfoMap;

		int m_x;
		int m_y;
		int m_step;
		bool m_isDirty;

		char m_fontName[LF_FACESIZE];
		int32_t m_fontSize;
		bool m_bItalic;
};
