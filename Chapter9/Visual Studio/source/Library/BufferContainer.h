#pragma once

#include "Common.h"

namespace Library
{
	class BufferContainer
	{
	public:
		BufferContainer();

		ID3D11Buffer* Buffer();
		void SetBuffer(ID3D11Buffer* buffer);

		UINT ElementCount() const;
		void SetElementCount(UINT elementCount);

		void ReleaseBuffer();

	private:
		BufferContainer(const BufferContainer& rhs);
        BufferContainer& operator=(const BufferContainer& rhs);

		ID3D11Buffer* mBuffer;
		UINT mElementCount;
	};
}