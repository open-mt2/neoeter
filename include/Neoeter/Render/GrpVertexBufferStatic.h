#pragma once

#include "GrpVertexBuffer.h"

class CStaticVertexBuffer : public CGraphicVertexBuffer
{
	public:
		CStaticVertexBuffer();
		virtual ~CStaticVertexBuffer();

		using CGraphicVertexBuffer::Create;

		bool Create(int vtxCount, DWORD fvf, bool isManaged = true);
};
