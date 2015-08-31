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
#ifndef RendererManager_h__
#define RendererManager_h__

#include "SimulationCommon.h"
#include "Core/Shapes.h"
#include "Core/Singleton.h"
#include "Core/Renderable.h"
#include "Core/CommandManager.h"
#include "Core/VaneMath.h"

#include "PointClouds.h"
#include "ParticleSystems.h"

#include "Simulation/VTI/VTITypes.h"

#include <vector>

namespace VANE
{
	typedef IRenderableID DebugRenderableID;
	const DebugRenderableID kInvalidDebugRenderableID = 0;
	
	typedef IRenderableID LineRenderableID;
	const LineRenderableID kInvalidLineRenderableID = 0;
	
	typedef IRenderableID TrackRenderableID;
	const TrackRenderableID kInvalidTrackRenderableID = 0;

	namespace Rendering
	{
		class IRenderer;
		class IRenderContext;
		class IRendererFactory;
		struct RenderRequestData;
		
		///A list of renderers
		typedef std::vector<VANE::Rendering::IRenderer*> RendererList;
		///A list of renderer-to-bes
		typedef std::vector<VANE::Rendering::IRendererFactory*> RendererFactoryList;


		///A manager class for various renderers.  We interact with them through
		///IRenderer interface, which allows us to have debug objects in multiple renderers
		///without any knowledge of the specific rendering systems involved
		class VaneSimExport RendererManager : public VANE::Singleton<RendererManager>
		{
		public:

			RendererManager();
			~RendererManager();

			///Add a renderer to the manager. This should happen at the creation of all
			///IRenderers
			void AddRenderer(IRenderer* pRenderer);
			
			///Remove a renderer from the manager
			void RemoveRenderer(IRenderer* pRenderer);
			
			/// Returns true if we have any renderers that are currently enabled.
			/// This can be used to skip rendering when we have no renderers currently active.
			bool HasEnabledRenderer();
			
			///Make an already existing renderable part of the User Interface
			///layer.
			void MoveRenderableToUI( IRenderableID );

			///Enable/disable User Interface rendering.
			void SetUserInterfaceVisibility( bool visible );

			/************************************************************************/
			/* Line Rendering                                                       */
			/************************************************************************/

			///Create a line renderable that will be tracked by ID
			///and owned by the rendering system
			LineRenderableID CreateLineRenderable(const vector<Vector3>& points, float32 width, const Color& color = Color::Green );

			///Update a line renderable
			void UpdateLineRenderable( LineRenderableID renderableID, const vector<Vector3>& newPoints, const vector<Color>& colors = vector<Color>() );

			///Destroy a line renderable
			void DestroyLineRenderable( LineRenderableID id );
			
			///Update the color of a line renderable
			void SetLineRenderableColor( LineRenderableID id, const Color& color );

			/************************************************************************/
			/* Track Rendering                                                       */
			/************************************************************************/

			///Create a track renderable that will be tracked by ID and owned by the rendering system
			TrackRenderableID CreateTrackRenderable( float32 width );

			///Destroy a track renderable
			void DestroyTrackRenderable( TrackRenderableID trackID );

			///Update a track renderable with the latest position of the corresponding wheel
			void UpdateTrackRenderablePosition( IRenderableID id, const Vector3& position, SurfaceID surfaceID, bool wheelOnGround );

			/************************************************************************/
			/* Debug Renderable                                                     */
			/************************************************************************/

			///Create a debug renderable that will be tracked by ID
			///and owned by the rendering system
			DebugRenderableID CreateDebugRenderable(const Shape& shape, const Color& color = Color::Green, bool isDebugOnly = true);

			///Destroy a debug renderable
			void DestroyDebugRenderable(DebugRenderableID id);

			///Update the position and orientation of a debug renderable
			void UpdateDebugRenderable(DebugRenderableID id, const Vector3& position, const Quaternion& orientation, const Vector3& scale = Vector3::UnitScale());
	
			///Add a debug line to a renderable, useful for drawing forces, etc
			//void AddDebugLine(DebugRenderableID id, const Vector3& start, const Vector3& end);
			void AddDebugLines(DebugRenderableID id, const std::vector<Vector3>& lines, const std::vector<Color>& colors);

			///Add a circle on the xy plane to a renderable, drawing ranges, etc
			void AddDebugCircle( DebugRenderableID id, const Color& circleColor, const Vector3& origin, float64 radius, uint32 faces = 12, const Vector3& orthoAxis = kWorldUp);
			
			///Clear all debug lines associated with a renderable
			void ClearDebugLines( DebugRenderableID id );
			
			/// Set a debug renderable to be orthographically drawn, ie, 2d. If this is the case
			/// then the coordinates must already be in device coords ( -1, 1), and Z will be ignored.
			///@param[in] id DebugRenderable to modify
			///@param[in] orthographic true if orthographic rendering should be used on this.
			void SetDebugRenderableOrthographic( DebugRenderableID id, bool orthographic );
			
			///Get a list of all the current renderers
			RendererList GetRenderers() const { return m_renderers; }

			void SetRenderableVisibility(DebugRenderableID id, bool isVisible);

			///Set the debug visibility for this renderer
			///This will turn off the entire debug rendering capability
			void SetDebugVisibility(bool visible);

			///Set the enabled state for the specified debug renderable. If disabled, a renderable will not be drawn.
			void EnableDebugRenderable(DebugRenderableID id, bool enable);

			///Create a point cloud renderable that will be tracked by ID
			///and owned by the rendering system
			PointCloudID CreatePointCloudRenderable( const PointCloudDataPtr& pData, const VANE::Color& color = Color::Green );

			///Create a point cloud renderable that will be tracked by ID
			///and owned by the rendering system
			PointCloudID CreatePointCloudRenderable( const String& filename, const VANE::Color& color = Color::Green );

			///Destroy a point cloud renderable
			void DestroyPointCloudRenderable(PointCloudID id);

			///Update the position and orientation of a point cloud.
			void UpdatePointCloudRenderable(PointCloudID id, const Vector3& position, const Quaternion& orientation);

			///Set the point cloud visibility for this renderer
			void SetPointCloudVisibility(bool visible);

			///Set the visibility of particle systems for this renderer
			void SetParticleSystemVisibility(bool visible);

			///Render text in the world view
			///@param[in] text The text to render
			///@param[in] text The layer the text will be added to
			///@param[in] screenLocation The location on the screen to render
			///@param[in] fadeTime The amount of time before the text fades away (with 0 never fading)
			///@param[in] showInActiveRenderWindow If true, the text will be shown in the active render window (the one with the most recent mouse activity), otherwise it will default to the primary render window
			void RenderText2D( const String& text, const String& layer, const Vector2& screenLocation, float32 fadeTime, const Color& textColor = Color::White, bool showInActiveRenderWindow = false );

			///Render text on the screen in the world view at the given world location, which will be transformed into screen space
			///@param[in] text The text to render
			///@param[in] text The layer the text will be added to
			///@param[in] worldLocation The world location we want to transform into screen space and render the text at
			///@param[in] fadeTime The amount of time before the text fades away (with 0 never fading)
			///@param[in] showInActiveRenderWindow If true, the text will be shown in the active render window (the one with the most recent mouse activity), otherwise it will default to the primary render window
			void RenderText2D( const String& text, const String& layer, const Vector3& worldLocation, float32 fadeTime, const Color& textColor = Color::White, bool showInActiveRenderWindow = false );

			///Set the position of text in a given layer
			///@param[in] layer The layer of the text we wish to move
			///@param[in] screenLocation The location we want to move the text to
			void SetRenderText2DPosition( const String& layer, const Vector2& screenLocation );

			///Set the position of text in a given layer using a world location, which will be transformed into screen space
			///@param[in] layer The layer of the text we wish to move
			///@param[in] worldLocation The world location we want to transform into screen space and render the text at
			void SetRenderText2DPosition( const String& layer, const Vector3& worldLocation );

			///Clear the text in a given layer
			///@param[in] layer The layer of text we wish to clear
			void ClearRenderText2D( const String& layer );

			/// Get a list of names for every render context in existence.
			/// @return Vector of render context names.
			StringVector GetRenderContextNames() const;

			/// Search through all known renderers for a render context.
			/// @param[in] name Name of the render context.
			/// @return Pointer to the first render context with the given name, NULL otherwise.
			IRenderContext* FindRenderContext( const String& name );

			///Get a specific render based on a string name
			///@param[in] the name of the renderer to retrieve
			///@return NULL if none exist with that name, otherwise ptr to the renderer
			IRenderer* GetRenderer(const String& name);
			
			///Get a list of current renderer names
			StringVector GetRendererNames() const;

			///Calls the Update(dt) of all ACTIVE renderers. This uses two separate times
			///as different effects can be based on the real clock time, or only the sim
			///time that has elapsed. 
			///@param[in] simDt The amount of simulation time that has passed since last update
			///@param[in] appDt The amount of application time that has passed 
			void Update(TimeValue simDt, TimeValue appDt);

			///Adds the renderer factory, for later use
			void AddRendererFactory(IRendererFactory* pRendererFac);

			///Removes a factory from the manager
			void RemoveRendererFactory(IRendererFactory* pRendererFac);

			///Checks if each factory's type is already a Renderer,
			/// and if not runs that factory's CreateIRenderer method.
			///@param factoryName The type of the Factory to be used in creating the Renderer
			///@param newRendererName A UNIQUE name for the new renderer; If there is an XML
			/// Config file, this is the name of the XML Element containing all the settings.
			///@param configFileName An optional parameter that specifies some configuration
			/// file to be used in setting up the new renderer. Because a renderer is allowed
			/// to have any kind of configuration file it wants, reading and parsing it are
			/// entirely up to the renderer or it's factory. The Manager knows nothing about
			/// even what kind of file it is.
			void CreateRendererFromFactory(const String& factoryName, const String& newRendererName, const String& configFileName = "");

			/************************************************************************/
			/* External Render Targets                                              */
			/************************************************************************/

			///@see IRenderer::CreateRenderTarget
			RenderTargetID CreateRenderTarget( uint32 xSize, uint32 ySize );
			///@see IRenderer::DestroyRenderTarget
			void DestroyRenderTarget( RenderTargetID renderTarget );
			///@see IRenderer::RenderToRenderTarget
			void RenderToRenderTarget( RenderTargetID target, ViewData& viewData, Rendering::RenderRequestData* pRenderRequest );			
			///@see IRenderer::ClearAllRequestsForRenderTarget
			void ClearAllRequestsForRenderTarget( RenderTargetID target );

		public: //Singleton Interface
			static RendererManager* GetSingletonPtr();
			static RendererManager& GetSingleton();

		private:

			//the current count/ID of various renderables.  These are incremented on
			//the creation of each new renderable ID.
			static DebugRenderableID ms_debugRenderableID;
			static PointCloudID      ms_pointCloudID;
			static ParticleSystemID  ms_particleSystemID;
			static LineRenderableID  ms_lineRenderableID;
			static TrackRenderableID ms_trackRenderableID;

			//our renderers
			RendererList m_renderers;

			//our renderer-to-bes
			RendererFactoryList m_rendererFactories;
		};
	}
}

#endif // RendererManager_h__
