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

#include "../DirectXObject.h"
#include "../Math/Math.h"
#include "GraphicsException.h"
#include "AnimationController.h"

namespace SlimDX
{
namespace Direct3D9
{
	AnimationSet::AnimationSet(ID3DXAnimationSet *set) : DirectXObject( set )
	{
	}

	int AnimationSet::GetAnimationIndex( String^ name )
	{
		array<unsigned char>^ nameBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( name );
		pin_ptr<unsigned char> pinnedName = &nameBytes[0];
		int result;

		HRESULT hr = m_Pointer->GetAnimationIndexByName( (LPCSTR) pinnedName, (UINT*) &result );
		GraphicsException::CheckHResult( hr );

		return result;
	}

	String^ AnimationSet::GetAnimationName( int index )
	{
		LPCSTR result;
		HRESULT hr = m_Pointer->GetAnimationNameByIndex( index, &result );
		GraphicsException::CheckHResult( hr );

		return gcnew String( result );
	}

	AnimationCallback^ AnimationSet::GetCallback( double position, CallbackSearchFlags flags, [Out] double% callbackPosition )
	{
		AnimationCallback^ callback;
		pin_ptr<double> pinPosition = &callbackPosition;
		void* functionPointer = Marshal::GetFunctionPointerForDelegate( callback ).ToPointer();

		HRESULT hr = m_Pointer->GetCallback( position, ( DWORD )flags, pinPosition, &functionPointer );
		GraphicsException::CheckHResult( hr );

		return callback;
	}

	AnimationOutput^ AnimationSet::GetTransformation( double periodicPosition, int animation )
	{
		Vector3 scale;
		Vector3 translation;
		Quaternion rotation;

		HRESULT hr = m_Pointer->GetSRT( periodicPosition, animation, (D3DXVECTOR3*) &scale, 
			(D3DXQUATERNION*) &rotation, (D3DXVECTOR3*) &translation );
		GraphicsException::CheckHResult( hr );

		AnimationOutput^ output = gcnew AnimationOutput();
		output->Flags = AnimationOutputFlags::Rotation | AnimationOutputFlags::Scale | AnimationOutputFlags::Translation;
		output->Scaling = scale;
		output->Translation = translation;
		output->Rotation = rotation;

		return output;
	}

	String^ AnimationSet::Name::get()
	{
		return gcnew String( m_Pointer->GetName() );
	}

	int AnimationSet::AnimationCount::get()
	{
		return m_Pointer->GetNumAnimations();
	}

	double AnimationSet::Period::get()
	{
		return m_Pointer->GetPeriod();
	}

	double AnimationSet::GetPeriodicPosition( double position )
	{
		return m_Pointer->GetPeriodicPosition( position );
	}

	AnimationController::AnimationController( ID3DXAnimationController *controller ) : DirectXObject( controller )
	{
	}

	AnimationController::AnimationController( int maxAnimationOutputs, int maxAnimationSets, int maxTracks, int maxEvents )
	{
		HRESULT hr = D3DXCreateAnimationController( maxAnimationOutputs, maxAnimationSets, maxTracks, maxEvents,
			( LPD3DXANIMATIONCONTROLLER* ) m_Pointer );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::AdvanceTime( double time, AnimationCallback^ handler )
	{
		HRESULT hr = m_Pointer->AdvanceTime( time, ( LPD3DXANIMATIONCALLBACKHANDLER )
			Marshal::GetFunctionPointerForDelegate( handler ).ToPointer() );
		GraphicsException::CheckHResult( hr );
	}

	AnimationController^ AnimationController::Clone( int maxAnimationOutputs, int maxAnimationSets, int maxTracks, int maxEvents )
	{
		LPD3DXANIMATIONCONTROLLER pointer;

		HRESULT hr = m_Pointer->CloneAnimationController( maxAnimationOutputs, maxAnimationSets, maxTracks, maxEvents, &pointer );
		GraphicsException::CheckHResult( hr );

		return gcnew AnimationController( pointer );
	}

	AnimationSet^ AnimationController::GetAnimationSet( int index )
	{
		LPD3DXANIMATIONSET set;

		HRESULT hr = m_Pointer->GetAnimationSet( index, &set );
		GraphicsException::CheckHResult( hr );

		return gcnew AnimationSet( set );
	}

	AnimationSet^ AnimationController::GetAnimationSet( String^ name )
	{
		LPD3DXANIMATIONSET set;
		array<unsigned char>^ nameBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( name );
		pin_ptr<unsigned char> pinnedName = &nameBytes[0];

		HRESULT hr = m_Pointer->GetAnimationSetByName( (LPCSTR) pinnedName, &set );
		GraphicsException::CheckHResult( hr );

		return gcnew AnimationSet( set );
	}

	int AnimationController::GetCurrentTrackEvent( int track, EventType eventType )
	{
		return m_Pointer->GetCurrentTrackEvent( track, ( D3DXEVENT_TYPE )eventType );
	}

	EventDescription AnimationController::GetEventDescription( int handle )
	{
		EventDescription result;

		HRESULT hr = m_Pointer->GetEventDesc( handle, (LPD3DXEVENT_DESC) &result );
		GraphicsException::CheckHResult( hr );

		return result;
	}

	AnimationSet^ AnimationController::GetTrackAnimationSet( int track )
	{
		LPD3DXANIMATIONSET set;

		HRESULT hr = m_Pointer->GetTrackAnimationSet( track, &set );
		GraphicsException::CheckHResult( hr );

		return gcnew AnimationSet( set );
	}

	TrackDescription AnimationController::GetTrackDescription( int track )
	{
		TrackDescription result;

		HRESULT hr = m_Pointer->GetTrackDesc( track, (LPD3DXTRACK_DESC) &result );
		GraphicsException::CheckHResult( hr );

		return result;
	}

	int AnimationController::GetUpcomingPriorityBlend( int handle )
	{
		return m_Pointer->GetUpcomingPriorityBlend( handle );
	}

	int AnimationController::GetUpcomingTrackEvent( int track, int handle )
	{
		return m_Pointer->GetUpcomingTrackEvent( track, handle );
	}

	int AnimationController::KeyPriorityBlend( float newBlendWeight, double startTime, double duration, TransitionType transition )
	{
		return m_Pointer->KeyPriorityBlend( newBlendWeight, startTime, duration, (D3DXTRANSITION_TYPE) transition );
	}

	int AnimationController::KeyTrackEnable( int track, bool enable, double startTime )
	{
		return m_Pointer->KeyTrackEnable( track, enable, startTime );
	}

	int AnimationController::KeyTrackPosition( int track, double position, double startTime )
	{
		return m_Pointer->KeyTrackPosition( track, position, startTime );
	}

	int AnimationController::KeyTrackSpeed( int track, float newSpeed, double startTime, double duration, TransitionType transition )
	{
		return m_Pointer->KeyTrackSpeed( track, newSpeed, startTime, duration, (D3DXTRANSITION_TYPE) transition );
	}

	int AnimationController::KeyTrackWeight( int track, float newWeight, double startTime, double duration, TransitionType transition )
	{
		return m_Pointer->KeyTrackWeight( track, newWeight, startTime, duration, (D3DXTRANSITION_TYPE) transition );
	}

	void AnimationController::RegisterAnimationOutput( String^ name, AnimationOutput^ output )
	{
		D3DXMATRIX *matrix = NULL;
		D3DXVECTOR3 *scale = NULL;
		D3DXVECTOR3 *translation = NULL;
		D3DXQUATERNION *rotation = NULL;
		array<unsigned char>^ nameBytes = System::Text::ASCIIEncoding::ASCII->GetBytes( name );
		pin_ptr<unsigned char> pinnedName = &nameBytes[0];
		pin_ptr<Matrix> pinMatrix;
		pin_ptr<Vector3> pinScale;
		pin_ptr<Vector3> pinTranslation;
		pin_ptr<Quaternion> pinQuaternion;

		if( (output->Flags & AnimationOutputFlags::Transformation) == AnimationOutputFlags::Transformation )
		{
			pinMatrix = &output->Transformation;
			matrix = (D3DXMATRIX*) pinMatrix;
		}

		if( (output->Flags & AnimationOutputFlags::Scale) == AnimationOutputFlags::Scale )
		{
			pinScale = &output->Scaling;
			scale = (D3DXVECTOR3*) pinScale;
		}

		if( (output->Flags & AnimationOutputFlags::Translation) == AnimationOutputFlags::Translation )
		{
			pinTranslation = &output->Translation;
			translation = (D3DXVECTOR3*) pinTranslation;
		}

		if( (output->Flags & AnimationOutputFlags::Rotation) == AnimationOutputFlags::Rotation )
		{
			pinQuaternion = &output->Rotation;
			rotation = (D3DXQUATERNION*) pinQuaternion;
		}

		HRESULT hr = m_Pointer->RegisterAnimationOutput( (LPCSTR) pinnedName, matrix, scale, rotation, translation );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::RegisterAnimationSet( AnimationSet^ set )
	{
		HRESULT hr = m_Pointer->RegisterAnimationSet( (LPD3DXANIMATIONSET) set->ComPointer.ToPointer() );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::ResetTime()
	{
		HRESULT hr = m_Pointer->ResetTime();
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::SetTrackAnimationSet( int track, AnimationSet^ set )
	{
		HRESULT hr = m_Pointer->SetTrackAnimationSet( track, (LPD3DXANIMATIONSET) set->ComPointer.ToPointer() );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::SetTrackDescription( int track, TrackDescription description )
	{
		HRESULT hr = m_Pointer->SetTrackDesc( track, (LPD3DXTRACK_DESC) &description);
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::EnableTrack( int track )
	{
		HRESULT hr = m_Pointer->SetTrackEnable( track, true );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::DisableTrack( int track )
	{
		HRESULT hr = m_Pointer->SetTrackEnable( track, false );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::SetTrackPosition( int track, double position )
	{
		HRESULT hr = m_Pointer->SetTrackPosition( track, position );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::SetTrackPriority( int track, TrackPriority priority )
	{
		HRESULT hr = m_Pointer->SetTrackPriority( track, (D3DXPRIORITY_TYPE) priority );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::SetTrackSpeed( int track, float speed )
	{
		HRESULT hr = m_Pointer->SetTrackSpeed( track, speed );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::SetTrackWeight( int track, float weight )
	{
		HRESULT hr = m_Pointer->SetTrackWeight( track, weight );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::UnkeyAllPriorityBlends()
	{
		HRESULT hr = m_Pointer->UnkeyAllPriorityBlends();
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::UnkeyAllTrackEvents( int track )
	{
		HRESULT hr = m_Pointer->UnkeyAllTrackEvents( track );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::UnkeyEvent( int handle )
	{
		HRESULT hr = m_Pointer->UnkeyEvent( handle );
		GraphicsException::CheckHResult( hr );
	}

	void AnimationController::UnregisterAnimationSet( AnimationSet^ set )
	{
		HRESULT hr = m_Pointer->UnregisterAnimationSet( (LPD3DXANIMATIONSET) set->ComPointer.ToPointer() );
		GraphicsException::CheckHResult( hr );
	}

	bool AnimationController::ValidateEvent( int handle )
	{
		HRESULT hr = m_Pointer->ValidateEvent( handle );

		return (hr == S_OK);
	}

	double AnimationController::Time::get()
	{
		return m_Pointer->GetTime();
	}

	int AnimationController::CurrentPriorityBlend::get()
	{
		return m_Pointer->GetCurrentPriorityBlend();
	}

	float AnimationController::PriorityBlend::get()
	{
		return m_Pointer->GetPriorityBlend();
	}

	void AnimationController::PriorityBlend::set( float value )
	{
		m_Pointer->SetPriorityBlend( value );
	}

	int AnimationController::AnimationSetCount::get()
	{
		return m_Pointer->GetNumAnimationSets();
	}

	int AnimationController::MaxAnimationOutputs::get()
	{
		return m_Pointer->GetMaxNumAnimationOutputs();
	}

	int AnimationController::MaxAnimationSets::get()
	{
		return m_Pointer->GetMaxNumAnimationSets();
	}

	int AnimationController::MaxTracks::get()
	{
		return m_Pointer->GetMaxNumTracks();
	}

	int AnimationController::MaxEvents::get()
	{
		return m_Pointer->GetMaxNumEvents();
	}
}
}