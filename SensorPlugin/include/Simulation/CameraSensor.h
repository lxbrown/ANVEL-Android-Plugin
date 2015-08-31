//////////////////////////////////////////////////////////////////////////
// 
// Header Copyright (c) 2013 Quantum Signal, LLC
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this script file for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
// 
// 1. That the authors recognize that ANVEL is owned and licensed 
// by Quantum Signal, LLC, and respect ANVEL trademark and copyright.
// 
// 2. An acknowledgment of the ANVEL website (www.anvelsim.com) is included 
// in any source distribution or product documentation
// 
// 3. This notice may not be removed or altered from any source
// distribution.
//
// For more information, contact Quantum Signal, LLC at info@anvelsim.com
// 
//////////////////////////////////////////////////////////////////////////
#ifndef Vane_Sim_CameraSensor_h__
#define Vane_Sim_CameraSensor_h__

#include "SimulationCommon.h"

#include "Sensor.h"

//@todo: replace with RenderingTypes
#include "Core/Renderable.h"
#include "Renderer.h"

namespace VANE
{
	///Our Sensor type 
	extern const VaneSimExport SensorType kSensorTypeCamera;
	
	enum MultiLensImageLayout
	{
		kLayoutHorizontal, ///< Default pattern where multiple lenses are displayed side by side
		kLayoutVertical    ///< Layout the images vertically
	};
	
	///Per lens configuration
	struct LensParams
	{
		Vector3 position;
		Quaternion orientation;

		String m_renderingEffects;

		float64 m_verticalFOV;
		float64 m_horizontalFOV;
		float64 m_sensorWidthMM;  //width of the CCD in mm
		float64 m_sensorHeightMM; //height of the CCD in mm
		float64 m_focalLengthMM;    //focal length of the lens

		uint32  m_resolutionX; //pixel resolution in the x direction per Lens
		uint32  m_resolutionY; //pixel resolution in the y direction per Lens
		
		bool m_enabled; ///If this lens is currently enabled.
	};
	
	///Visualization for a Camera Sensor
	class ICameraVisualizer : public ISensorVisualizer
	{
	public:
		/// Get the type of sensor this visualizer is used for.
		virtual SensorType GetType() const { return GetStaticType(); }
		static SensorType GetStaticType() { return "CameraSensor"; }
		
		/// Callback for when sensor is destroyed
		virtual void OnSensorDestroyed() = 0 ; 
		
		virtual void SetImageBuffer( uint32 lens, const uint8* pImageBuffer )=0;
		
		virtual void SetMultiLensLayout( MultiLensImageLayout layout )=0;
		
		virtual void SetLensParams( vector<LensParams>& params )=0;
	};
	
	///Per lens data buffer
	struct LensData
	{
		LensData() 
			: m_renderTargetID( Rendering::kInvalidRenderTargetID )
			, m_lastVisualizationTimestamp( 0 )
		{	}
		
		~LensData()
		{
			if ( m_renderRequest.m_pOutputBuffer )
			{
				delete[] m_renderRequest.m_pOutputBuffer;
			}
		}
		
		Rendering::RenderRequestData m_renderRequest;
		Rendering::RenderTargetID m_renderTargetID;
		uint32 m_lastVisualizationTimestamp;
	};
	
	//////////////////////////////////////////////////////////////////////////
	//
	//  Camera Parameters
	//
	//////////////////////////////////////////////////////////////////////////
	
	///Camera specific configuration parameters
	class CameraSensorStaticAssetParams
		: public SensorStaticAssetParams
	{
	public:
			
		vector<LensParams> m_lensParams;
		
		uint16  m_numLenses; 
		
	public:
		bool IsValid( ) { return ( !m_lensParams.empty() && m_numLenses > 0 ); }	
	};
	
	//////////////////////////////////////////////////////////////////////////
	//
	//  Camera Sensor
	//
	//////////////////////////////////////////////////////////////////////////
	
	///A sensor that simulates a video camera
	class VaneSimExport CameraSensor : public Sensor
	{
		friend class DefaultSensorFactory;

	public: //[Sensor methods]

		virtual void Update( TimeValue dt );
		
		///Get our current set of lens parameters
		///@return The current set of lens parameters
		const std::vector<LensParams>& GetLensParams() const { return m_lensParams; }
		
		///Get our current set of lens parameters
		///@return The current set of lens parameters
		const std::vector<LensData>& GetLensData() const { return m_lensData; }

	protected:
		
		///Create a new camera sensor (can only be called by default sensor factory )
		///@param[in] specificId
		///@param[in] params
		///@param[in] dynamicParams
		CameraSensor( VaneID specificId, CameraSensorStaticAssetParams& params, DynamicAssetParams& dynamicParams );

		~CameraSensor();
	protected:
	
		//create our initial camera setup and rendering info
		void InitializeData();

	protected:	
	
		//Our configuration for each lens
		std::vector<LensParams> m_lensParams;
		std::vector<LensData>   m_lensData;

		ICameraVisualizer* m_pVisualizer;
		
		bool m_initialized;
	};	
	
}


#endif // Vane_Sim_CameraSensor_h__
