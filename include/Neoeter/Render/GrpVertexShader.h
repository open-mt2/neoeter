#pragma once

#include "GrpBase.h"

class CVertexShader : public CGraphicBase
{
	public:
		CVertexShader();
		virtual ~CVertexShader();

		void Destroy();
		bool CreateFromDiskFile(const char* c_szFileName, const uint32_t* c_pdwVertexDecl);

		void Set();

	protected:
		void Initialize();

	protected:
		LPDIRECT3DVERTEXSHADER9 m_handle;
};
