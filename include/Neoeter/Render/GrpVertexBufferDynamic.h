#pragma once

#include "GrpVertexBuffer.h"

class CDynamicVertexBuffer : public CGraphicVertexBuffer
{
	public:
		CDynamicVertexBuffer();
		virtual ~CDynamicVertexBuffer();

		using CGraphicVertexBuffer::Create;

		bool Create(int vtxCount, int fvf);

	protected:
		int m_vtxCount;
		int m_fvf;
};
