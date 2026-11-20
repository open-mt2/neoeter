#pragma once

#include "GrpVertexBuffer.h"

class CStaticVertexBuffer : public CGraphicVertexBuffer
{
	public:
		CStaticVertexBuffer();
		virtual ~CStaticVertexBuffer();

		using CGraphicVertexBuffer::Create;

		bool Create(int vtxCount, uint32_t fvf, bool isManaged = true);
};
