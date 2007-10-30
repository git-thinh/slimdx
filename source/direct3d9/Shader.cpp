/*
* Copyright (c) 2007 SlimDX Group
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
#include <d3d9.h>
#include <d3dx9.h>
#include <vcclr.h>
#include <memory>

#include "Device.h"
#include "Buffer.h"
#include "Shader.h"

namespace SlimDX
{
	namespace Direct3D9
	{
		IncludeShim::IncludeShim( Include^ wrappedInterface )
		{
			m_WrappedInterface = wrappedInterface;
		}

		HRESULT IncludeShim::Open( D3DXINCLUDE_TYPE includeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes )
		{
			return E_FAIL;
		}

		HRESULT IncludeShim::Close( LPCVOID pData )
		{
			return E_FAIL;
		}

		//helper function to resolve array<Macro>^ to D3DXMACRO*
		D3DXMACRO* Macro::Marshal( array<Macro>^ macros, [Out] array<GCHandle>^% handles )
		{
			if( macros == nullptr )
			{
				handles = nullptr;
				return NULL;
			}

			//this array is null terminated, so we need to patch in an extra value
			D3DXMACRO* result = new D3DXMACRO[macros->Length + 1];
			handles = gcnew array<GCHandle>( macros->Length * 2 );

			for( int i = 0; i < macros->Length; ++i )
			{
				array<Byte>^ nameBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( macros[i].Name );
				array<Byte>^ defBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( macros[i].Definition );

				handles[2 * i] = GCHandle::Alloc( nameBytes, GCHandleType::Pinned );
				handles[2 * i + 1] = GCHandle::Alloc( defBytes, GCHandleType::Pinned );

				result[i].Name = (LPCSTR) handles[2 * i].AddrOfPinnedObject().ToPointer();
				result[i].Definition = (LPCSTR) handles[2 * i + 1].AddrOfPinnedObject().ToPointer();
			}

			result[macros->Length].Name = NULL;
			result[macros->Length].Definition = NULL;

			return result;
		}

		void Macro::Unmarshal( D3DXMACRO* macros, array<GCHandle>^ handles )
		{
			delete macros;
			if( handles != nullptr )
			{
				for( int i = 0; i < handles->Length; ++i )
				{
					handles[i].Free();
				}
			}
		}

		ConstantTable^ ShaderBytecode::GetConstantTable()
		{
			ID3DXConstantTable* constantTable;
			HRESULT hr = D3DXGetShaderConstantTable( (const DWORD*) m_Pointer->GetBufferPointer(), &constantTable );
			GraphicsException::CheckHResult( hr );
			if( FAILED( hr ) )
				return nullptr;

			return gcnew ConstantTable( constantTable );
		}

		array<ShaderSemantic>^ ShaderBytecode::GetInputSemantics()
		{
			UINT count = 0;
			const DWORD* function = (const DWORD*) m_Pointer->GetBufferPointer();

			HRESULT hr = D3DXGetShaderInputSemantics( function, NULL, &count );
			GraphicsException::CheckHResult( hr );
			if( FAILED( hr ) )
				return nullptr;
			
			array<ShaderSemantic>^ inputs = gcnew array<ShaderSemantic>( count );
			pin_ptr<ShaderSemantic> pinnedInputs = &inputs[0];

			hr = D3DXGetShaderInputSemantics( function, (D3DXSEMANTIC*) pinnedInputs, &count );
			GraphicsException::CheckHResult( hr );
			if( FAILED( hr ) )
				return nullptr;
			
			return inputs;
		}

		array<ShaderSemantic>^ ShaderBytecode::GetOutputSemantics()
		{
			UINT count = 0;
			const DWORD* function = (const DWORD*) m_Pointer->GetBufferPointer();

			HRESULT hr = D3DXGetShaderOutputSemantics( function, NULL, &count );
			GraphicsException::CheckHResult( hr );
			if( FAILED( hr ) )
				return nullptr;
			
			array<ShaderSemantic>^ outputs = gcnew array<ShaderSemantic>( count );
			pin_ptr<ShaderSemantic> pinnedOutputs = &outputs[0];

			hr = D3DXGetShaderOutputSemantics( function, (D3DXSEMANTIC*) pinnedOutputs, &count );
			GraphicsException::CheckHResult( hr );
			if( FAILED( hr ) )
				return nullptr;
			
			return outputs;
		}

		array<String^>^ ShaderBytecode::GetSamplers()
		{
			UINT count = 0;
			const DWORD* function = (const DWORD*) m_Pointer->GetBufferPointer();

			HRESULT hr = D3DXGetShaderSamplers( function, NULL, &count );
			GraphicsException::CheckHResult( hr );
			if( FAILED( hr ) )
				return nullptr;
			
			std::auto_ptr<LPCSTR> samplers = new LPCSTR[count];

			hr = D3DXGetShaderSamplers( function, samplers.get(), &count );
			GraphicsException::CheckHResult( hr );
			if( FAILED( hr ) )
				return nullptr;
			
			array<String^>^ outputSamplers = gcnew array<String^>( count );
			for( UINT i = 0; i < count; ++i )
			{
				outputSamplers[i] = gcnew String( samplers.get()[i] );
			}

			return outputSamplers;
		}

		int ShaderBytecode::Version::get()
		{
			const DWORD* function = (const DWORD*) m_Pointer->GetBufferPointer();
			return (int) D3DXGetShaderVersion( function );
		}

		//D3DXAssembleShader
		ShaderBytecode^ Shader::Assemble( array<Byte>^ sourceData, array<Macro>^ defines,
			Include^ includeFile, ShaderFlags flags, [Out] String^% errors )
		{
			ID3DXBuffer* shaderBuffer;
			ID3DXBuffer* errorBuffer;
			pin_ptr<Byte> pinnedData = &sourceData[0];

			IncludeShim includeShim = IncludeShim( includeFile );
			ID3DXInclude* includePtr = NULL;
			if( includeFile != nullptr )
				includePtr = &includeShim;

			array<GCHandle>^ handles;
			D3DXMACRO* macros = Macro::Marshal( defines, handles );

			HRESULT hr = D3DXAssembleShader( (char*) pinnedData, sourceData->Length, macros, includePtr,
				(DWORD) flags, &shaderBuffer, &errorBuffer );

			//clean up after marshaling macros
			Macro::Unmarshal( macros, handles );
			//marshal errors if necessary
			errors = BufferWrapper::ConvertToString( errorBuffer );
			
			GraphicsException::CheckHResult( hr, "Compilation Errors", errors );
			if( FAILED( hr ) )
				return nullptr;

			return gcnew ShaderBytecode( shaderBuffer );
		}

		ShaderBytecode^ Shader::Assemble( String^ sourceData, array<Macro>^ defines,
			Include^ includeFile, ShaderFlags flags, [Out] String^% errors )
		{
			array<Byte>^ data = System::Text::ASCIIEncoding::ASCII->GetBytes( sourceData );
			return Assemble( data, defines, includeFile, flags, errors );
		}

		//D3DXAssembleShaderFromFile
		ShaderBytecode^ Shader::AssembleFromFile( String^ fileName, array<Macro>^ defines,
			Include^ includeFile, ShaderFlags flags, [Out] String^% errors )
		{
			ID3DXBuffer* shaderBuffer;
			ID3DXBuffer* errorBuffer;
			pin_ptr<const wchar_t> pinnedFileName = PtrToStringChars( fileName );

			IncludeShim includeShim = IncludeShim( includeFile );
			ID3DXInclude* includePtr = NULL;
			if( includeFile != nullptr )
				includePtr = &includeShim;

			array<GCHandle>^ handles;
			D3DXMACRO* macros = Macro::Marshal( defines, handles );

			HRESULT hr = D3DXAssembleShaderFromFile( pinnedFileName, macros, includePtr,
				(DWORD) flags, &shaderBuffer, &errorBuffer );

			//clean up after marshaling macros
			Macro::Unmarshal( macros, handles );
			//marshal errors if necessary
			errors = BufferWrapper::ConvertToString( errorBuffer );
			
			GraphicsException::CheckHResult( hr, "Compilation Errors", errors );
			if( FAILED( hr ) )
				return nullptr;

			return gcnew ShaderBytecode( shaderBuffer );
	   }

		//D3DXCompileShader
		ShaderBytecode^ Shader::Compile( array<Byte>^ sourceData, array<Macro>^ defines,
			Include^ includeFile, String^ functionName, String^ profile, ShaderFlags flags,
			[Out] String^% errors, [Out] ConstantTable^% constantTable )
		{
			ID3DXBuffer* shaderBuffer;
			ID3DXBuffer* errorBuffer;
			ID3DXConstantTable* constants;
			pin_ptr<Byte> pinnedData = &sourceData[0];
			
			array<Byte>^ functionBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( functionName );
			pin_ptr<Byte> pinnedFunction = &functionBytes[0];
			array<Byte>^ profileBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( profile );
			pin_ptr<Byte> pinnedProfile = &profileBytes[0];

			IncludeShim includeShim = IncludeShim( includeFile );
			ID3DXInclude* includePtr = NULL;
			if( includeFile != nullptr )
				includePtr = &includeShim;

			array<GCHandle>^ handles;
			D3DXMACRO* macros = Macro::Marshal( defines, handles );

			HRESULT hr = D3DXCompileShader( (char*) pinnedData, sourceData->Length, macros, includePtr,
				(char*) pinnedFunction, (char*) pinnedProfile, (DWORD) flags,
				&shaderBuffer, &errorBuffer, &constants );

			//clean up after marshaling macros
			Macro::Unmarshal( macros, handles );
			//marshal errors if necessary
			errors = BufferWrapper::ConvertToString( errorBuffer );
			
			GraphicsException::CheckHResult( hr, "Compilation Errors", errors );
			if( FAILED( hr ) )
			{
				constantTable = nullptr;
				return nullptr;
			}

			return gcnew ShaderBytecode( shaderBuffer );
		}

		ShaderBytecode^ Shader::Compile( String^ sourceData, array<Macro>^ defines,
			Include^ includeFile, String^ functionName, String^ profile, ShaderFlags flags,
			[Out] String^% errors, [Out] ConstantTable^% constantTable )
		{
			array<Byte>^ data = System::Text::ASCIIEncoding::ASCII->GetBytes( sourceData );
			return Compile( data, defines, includeFile, functionName, profile, flags, errors, constantTable );
		}

		ShaderBytecode^ Shader::Compile( array<Byte>^ sourceData, array<Macro>^ defines,
			Include^ includeFile, String^ functionName, String^ profile, ShaderFlags flags,
			[Out] String^% errors )
		{
			ID3DXBuffer* shaderBuffer;
			ID3DXBuffer* errorBuffer;
			pin_ptr<Byte> pinnedData = &sourceData[0];
			
			array<Byte>^ functionBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( functionName );
			pin_ptr<Byte> pinnedFunction = &functionBytes[0];
			array<Byte>^ profileBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( profile );
			pin_ptr<Byte> pinnedProfile = &profileBytes[0];

			IncludeShim includeShim = IncludeShim( includeFile );
			ID3DXInclude* includePtr = NULL;
			if( includeFile != nullptr )
				includePtr = &includeShim;

			array<GCHandle>^ handles;
			D3DXMACRO* macros = Macro::Marshal( defines, handles );

			HRESULT hr = D3DXCompileShader( (char*) pinnedData, sourceData->Length, macros, includePtr,
				(char*) pinnedFunction, (char*) pinnedProfile, (DWORD) flags,
				&shaderBuffer, &errorBuffer, NULL );

			//clean up after marshaling macros
			Macro::Unmarshal( macros, handles );
			//marshal errors if necessary
			errors = BufferWrapper::ConvertToString( errorBuffer );
			
			GraphicsException::CheckHResult( hr, "Compilation Errors", errors );
			if( FAILED( hr ) )
			{
				return nullptr;
			}

			return gcnew ShaderBytecode( shaderBuffer );
		}

		ShaderBytecode^ Shader::Compile( String^ sourceData, array<Macro>^ defines,
			Include^ includeFile, String^ functionName, String^ profile, ShaderFlags flags,
			[Out] String^% errors )
		{
			array<Byte>^ data = System::Text::ASCIIEncoding::ASCII->GetBytes( sourceData );
			return Compile( data, defines, includeFile, functionName, profile, flags, errors );
		}

		//D3DXCompileShaderFromFile
		ShaderBytecode^ Shader::CompileFromFile( String^ fileName, array<Macro>^ defines,
			Include^ includeFile, String^ functionName, String^ profile, ShaderFlags flags,
			[Out] String^% errors, [Out] ConstantTable^% constantTable )
		{
			ID3DXBuffer* shaderBuffer;
			ID3DXBuffer* errorBuffer;
			ID3DXConstantTable* constants;
			pin_ptr<const wchar_t> pinnedFileName = PtrToStringChars( fileName );
			
			array<Byte>^ functionBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( functionName );
			pin_ptr<Byte> pinnedFunction = &functionBytes[0];
			array<Byte>^ profileBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( profile );
			pin_ptr<Byte> pinnedProfile = &profileBytes[0];

			IncludeShim includeShim = IncludeShim( includeFile );
			ID3DXInclude* includePtr = NULL;
			if( includeFile != nullptr )
				includePtr = &includeShim;

			array<GCHandle>^ handles;
			D3DXMACRO* macros = Macro::Marshal( defines, handles );

			HRESULT hr = D3DXCompileShaderFromFile( pinnedFileName, macros, includePtr,
				(char*) pinnedFunction, (char*) pinnedProfile, (DWORD) flags,
				&shaderBuffer, &errorBuffer, &constants );

			//clean up after marshaling macros
			Macro::Unmarshal( macros, handles );
			//marshal errors if necessary
			errors = BufferWrapper::ConvertToString( errorBuffer );
			
			GraphicsException::CheckHResult( hr, "Compilation Errors", errors );
			if( FAILED( hr ) )
			{
				constantTable = nullptr;
				return nullptr;
			}

			return gcnew ShaderBytecode( shaderBuffer );
		}

		ShaderBytecode^ Shader::CompileFromFile( String^ fileName, array<Macro>^ defines,
			Include^ includeFile, String^ functionName, String^ profile, ShaderFlags flags,
			[Out] String^% errors )
		{
			ID3DXBuffer* shaderBuffer;
			ID3DXBuffer* errorBuffer;
			pin_ptr<const wchar_t> pinnedFileName = PtrToStringChars( fileName );
			
			array<Byte>^ functionBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( functionName );
			pin_ptr<Byte> pinnedFunction = &functionBytes[0];
			array<Byte>^ profileBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( profile );
			pin_ptr<Byte> pinnedProfile = &profileBytes[0];

			IncludeShim includeShim = IncludeShim( includeFile );
			ID3DXInclude* includePtr = NULL;
			if( includeFile != nullptr )
				includePtr = &includeShim;

			array<GCHandle>^ handles;
			D3DXMACRO* macros = Macro::Marshal( defines, handles );

			HRESULT hr = D3DXCompileShaderFromFile( pinnedFileName, macros, includePtr,
				(char*) pinnedFunction, (char*) pinnedProfile, (DWORD) flags,
				&shaderBuffer, &errorBuffer, NULL );

			//clean up after marshaling macros
			Macro::Unmarshal( macros, handles );
			//marshal errors if necessary
			errors = BufferWrapper::ConvertToString( errorBuffer );
			
			GraphicsException::CheckHResult( hr, "Compilation Errors", errors );
			if( FAILED( hr ) )
			{
				return nullptr;
			}

			return gcnew ShaderBytecode( shaderBuffer );
		}

		int Shader::MajorVersion( int version )
		{
			return D3DSHADER_VERSION_MAJOR( version );
		}

		int Shader::MinorVersion( int version )
		{
			return D3DSHADER_VERSION_MINOR( version );
		}

		Version^ Shader::ParseVersion( int version )
		{
			return gcnew Version( MajorVersion( version ), MinorVersion( version ) );
		}
	}
}