/*
* Copyright (c) 2007-2008 SlimDX Group
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include "VertexBufferBinding.h"

using namespace System;

namespace SlimDX
{
namespace Direct3D10
{ 
	VertexBufferBinding::VertexBufferBinding( Direct3D10::Buffer^ buffer, int stride, int offset )
	: m_Buffer( buffer ), m_Stride( stride ), m_Offset( offset )
	{
	}
	
	Direct3D10::Buffer^ VertexBufferBinding::Buffer::get()
	{
		return m_Buffer;
	}
	
	void VertexBufferBinding::Buffer::set( Direct3D10::Buffer^ value )
	{
		m_Buffer = value;
	}
	
	int VertexBufferBinding::Stride::get()
	{
		return m_Stride;
	}
	
	void VertexBufferBinding::Stride::set( int value )
	{
		m_Stride = value;
	}
	
	int VertexBufferBinding::Offset::get()
	{
		return m_Offset;
	}
	
	void VertexBufferBinding::Offset::set( int value )
	{
		m_Offset = value;
	}
}
}