#include "BufferContainer.h"

namespace Library
{
	BufferContainer::BufferContainer()
		: mBuffer(nullptr), mElementCount(0)
	{
	}

	ID3D11Buffer* BufferContainer::Buffer()
	{
		return mBuffer;
	}

	void BufferContainer::SetBuffer(ID3D11Buffer* buffer)
	{
		mBuffer = buffer;
	}

	UINT BufferContainer::ElementCount() const
	{
		return mElementCount;
	}

	void BufferContainer::SetElementCount(UINT elementCount)
	{
		mElementCount = elementCount;
	}

	void BufferContainer::ReleaseBuffer()
	{
		ReleaseObject(mBuffer);
		mElementCount = 0;
	}
}
