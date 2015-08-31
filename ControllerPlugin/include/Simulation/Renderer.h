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
#ifndef VANE_Renderer_h__
#define VANE_Renderer_h__

#include "Core/Renderable.h"
#include "Core/Resources.h"
#include "Core/Shapes.h"

//@TODO replace with camera types...
#include "Simulation/Camera/Camera.h"
#include "Simulation/Camera/CameraController.h"
#include "Simulation/InputEvents.h"
#include "Simulation/VTI/VTITypes.h"

#include "PointClouds.h"
#include "ParticleSystems.h"

namespace VANE
{
	namespace Rendering
	{
	
#if VANE_PLATFORM == VANE_PLATFORM_WIN32
		typedef HWND WindowHandle;
#else
		//@TODO: include all data needed for GTK display...
		typedef long WindowHandle;
#endif

		class IRenderWindow;

		/// Basic rendering state used to determine what parts of the scene should be drawn.
		struct RenderState
		{
			RenderState() 
			: m_drawGeometry(true)
			, m_drawDebug(false)
			, m_drawPointCloud(false)
			, m_drawParticles(true) 
			, m_drawUI(false)
			, m_drawVirtualObjects( true )
			, m_drawObjects( true )
			, m_drawVegetation( true )
			, m_drawProps( true )
			{;}
			
			RenderState( bool geom, bool debug, bool pointCloud, bool particles, bool drawUI, bool drawVirtualObjects )
			 : m_drawGeometry(geom)
			 , m_drawDebug(debug)
			 , m_drawPointCloud(pointCloud)
			 , m_drawParticles(particles) 
			 , m_drawUI( drawUI )
			 , m_drawVirtualObjects( drawVirtualObjects )
			 , m_drawObjects( true )
			 , m_drawVegetation( true )
			 , m_drawProps( true )
			 {;}

			bool m_drawGeometry;
			bool m_drawDebug;
			bool m_drawPointCloud;
			bool m_drawParticles;
			bool m_drawUI;
			bool m_drawVirtualObjects;
			bool m_drawObjects;
			bool m_drawVegetation;
			bool m_drawProps;
		};

		struct RenderRequestData
		{
			RenderRequestData() 
				: m_pOutputBuffer( NULL )
				, m_renderTimeStamp( 0 )
				, m_renderStatus( 0 )
			{	}

			uint8* m_pOutputBuffer;
			uint32 m_renderTimeStamp;
			uint32 m_renderStatus;
		};

		/************************************************************************/
		/* IRenderContext                                                       */
		/************************************************************************/

		///A context for rendering. Rendering Contexts reside in IRenderingWindows, and receive
		///notifications from them about resize events, and when to draw.
		class IRenderContext
		{
		public:
			
			///Set the camera for this RenderContext
			///@return True if camera was set successfully, false otherwise.
			virtual bool SetCamera(Cameras::Camera* pCamera) = 0;

			virtual Cameras::Camera* GetCamera() const = 0;

			virtual void ClearCamera() = 0;

			virtual void SetClearColor(const VANE::Color& clearColor) = 0;
		
			///Notification that a render window has been resized.
			virtual void OnWindowResize(uint32 width, uint32 height) = 0;

			///Notification to signal an update for this render context.
			virtual void Update() = 0;

			/// Returns the rendering window if we currently have one, otherwise NULL
			virtual IRenderWindow* GetRenderWindow() const = 0;
			virtual void           SetRenderWindow( IRenderWindow* pWindow ) = 0;

			///Enable or disable a visual effect
			virtual void SetEffectEnabled( const String& effectName, bool enabled ) =0;
			///Disable all rendering effects
			virtual void ClearRenderingEffects() =0;

			///Adding an item to the include list will put the render context in a 
			///special mode where it will ONLY render those items in the include list.
			///To revert this status, call ClearIncludeList
			virtual void AddToIncludeList( IRenderableID renderableID ) = 0;
			///Remove all items from the IncludeList, allowing full scene rendering to proceed
			///as normal.
			virtual void ClearIncludeList() = 0;

			/// Get the render state for this rendering context.
			RenderState GetRenderState() const { return m_renderState; }

			/// Set the render state.
			void SetRenderState( RenderState renderState ) { m_renderState = renderState; }

			/// Enable/disable geometry rendering.
			void SetRenderGeometry( bool render ) { m_renderState.m_drawGeometry = render; m_renderState.m_drawVegetation = render; m_renderState.m_drawObjects = render; m_renderState.m_drawProps = render; }

			/// Enable/disable debug rendering.
			void SetRenderDebug( bool render ) { m_renderState.m_drawDebug = render; }

			/// Enable/disable point cloud rendering.
			void SetRenderPointClouds( bool render ) { m_renderState.m_drawPointCloud = render; }

			/// Enable/disable UI rendering.
			void SetRenderUserInterface( bool render ) { m_renderState.m_drawUI = render; }

			/************************************************************************/
			/* Functions for interacting with what is rendered                      */
			/*  These functions are optional to implement                           */
			/************************************************************************/

			///Notification about mouse events. Useful for enabling interactivity with rendered world.
			///@param[in] mouseEvt The mouse event, oriented on the render context with 0,0 in the upper left
			///@return Whether or not the input event was handeled, indicating if the event should propagate to
			///        to other potential listeners.
			virtual bool OnMouseEvent(const MouseInputEvent& mouseEvt) { VANE_UNUSED( mouseEvt ); return false;}

			///Using selection, which pre-selects targets for Commands, renderers can be interactive
			///pretty easily, by just helping other commands find targets. This tells the renderer that
			///we want to select an object at the target screen coordinates
			///@param[in] x The x position of the desired screen coordinate
			///@param[in] y The y position of the desired screen coordinate
			virtual VaneID GetIdAtRendererCoordinates(const int64 x, const int64 y) const { VANE_UNUSED(x); VANE_UNUSED(y); return kInvalidVaneID; }

			///Using selection, which pre-selects targets for Commands, renderers can be interactive
			///pretty easily, by just helping other commands find targets. This tells the renderer that
			///we want to select all objects at the specified box (note the two coordinates may be the same)
			///@param[in] x1 The x position of the desired screen coordinate box corner
			///@param[in] y1 The y position of the desired screen coordinate box corner
			///@param[in] x2 The x position of the desired screen coordinate box corner
			///@param[in] y2 The y position of the desired screen coordinate box corner
			///@return A list of VaneIDs within the specified box
			virtual VaneIdVector GetIdsAtRendererBox(int64 x1, int64 y1, int64 x2, int64 y2) const { VANE_UNUSED(x1); VANE_UNUSED(y1); VANE_UNUSED(x2); VANE_UNUSED(y2); return VaneIdVector(); }

			///Convert screen coordinates on the renderer context to regular world coordinates
			///@param[in] x The x position of the desired screen coordinate
			///@param[in] y The y position of the desired screen coordinate
			///@return A Vector3 containing the world coordinates
			virtual Vector3 ScreenCoordinatesToWorldCoordinates(const int64 x, const int64 y, bool terrainOnly = true) const { VANE_UNUSED(x); VANE_UNUSED(y); VANE_UNUSED(terrainOnly); return Vector3::Zero(); }

			///Get a ray straight out from the camera's perspective for the given pixel
			///@param[in] x x value of pixel
			///@param[in] y y value of pixel
			///@return Ray in world space
			virtual VANE::Ray GetWorldRayThroughScreen(const int64 x, const int64 y) const { VANE_UNUSED(x); VANE_UNUSED(y); return VANE::Ray(); }

			/// Informs the render context when it is in a window that has focus.
			virtual void ContextHasFocus( ) const = 0;

		protected:
			mutable RenderState	m_renderState;
		};
		
		/************************************************************************/
		/* IRenderWindow                                                        */
		/************************************************************************/

		///A generic Render Window that can be used with any ANVEL rendering system.
		class IRenderWindow
		{
		public:
			///Get the System specific Window Handle that we can use to create 
			///a rendering context
			virtual WindowHandle GetWindowHandle() const = 0;

			///Get the size from this window, that we can use to create a new 
			///rendering context in. 
			virtual void GetWindowSize( uint32& width, uint32& height) const = 0;

			///Returns the rendering context if we currently have one, otherwise NULL
			virtual IRenderContext* GetRenderContext() const = 0;

			///Set the current IRenderingContext
			virtual void SetRenderContext(IRenderContext* pContext) = 0;
			
			///Set a controller for the camera that this window is using.
			///This is needed to update focus.
			virtual void SetCameraController(Cameras::CameraController* pController) = 0;

			///Enable or disable mouse interaction with this window.
			virtual void SetMouseInteractionEnabled( bool enabled ) = 0;
		};
	
		/************************************************************************/
		/* IRenderer                                                            */
		/************************************************************************/
	
		///Abstract interface for a renderer
		class IRenderer
		{
		public:
			///Get the name of this renderer
			virtual String GetName()=0;

			///Renderers must be able to initialize both with additional input AND without any.
			///To reinforce this, each case has it's own initialization function, so both cases
			///should be implemented. This function is for initialization with no additional input.
			///Additionally, it is expected that either here, or in the Renderer's constructor,
			///that the Renderer add itself to the RendererManager.
			virtual void Initialize()=0;

			///This function should be passed a filename from which the renderer can initialize itself.
			virtual void Initialize(const String& fileName)=0;
			
			///Returns true if this renderer is currently enabled.
			///@return true if this renderer is currently enabled.
			virtual bool IsEnabled() const =0;
		
			///Set the visibility for a particular renderable
			///@param[in] id the renderable ID
			///@param[in] isVisible true if the renderable should be visible, false if you wish it to be hidden
			virtual void SetRenderableVisibility(IRenderableID id, bool isVisible) = 0;

			///Calls the Update function of each Renderer
			///@param[in] simDt The amount of simulation time that has passed since last update
			///@param[in] appDt The amount of application time that has passed 
			virtual bool Update( TimeValue simDt, TimeValue appDt )=0;

			/// Setup the renderer's resource paths.
			/// @param[in] resourcePaths A list of resource paths to add to the renderer.
			virtual void SetupResources( const Resources::ResourcePathVector& resourcePaths ) = 0;
			
			///Create a rendering context in the given IRenderWindow
			virtual IRenderContext* CreateRenderingContext( const String& name, IRenderWindow* pTargetWindow) = 0;
			
			/// Get a render context by its name.
			/// @param[in] name Name of the render context to find.
			/// @return Pointer to the render context if it exists, NULL otherwise.
			virtual IRenderContext* GetRenderContext( const String& name ) = 0;

			/// Get a list of names for all this renderer's render contexts.
			/// @return Vector of render context names.
			virtual StringVector GetRenderContextNames() const = 0;

			///Create a camera implementation
			virtual Cameras::CameraImplementation* CreateCameraImplementation( const Cameras::CameraViewParams& params) =0;

			///Get the list of currently supported effects available
			///@return the list of currently supported effects available
			virtual StringVector GetSupportedEffects() const = 0;
			
			///Make an already existing renderable part of the User Interface
			///layer.
			virtual void MoveRenderableToUI( IRenderableID ) = 0;
			
			///Enable/disable User Interface rendering.
			virtual void SetUserInterfaceVisibility( bool visible ) = 0;

			///Set the virtual object visibility for this renderer
			virtual void SetVirtualObjectVisibility(bool visible) = 0;

			/****************************************/
			/*  Debug								*/
			/****************************************/
			///Create a debug renderable
			virtual void CreateDebugShape(IRenderableID id, const Shape& shape, const Color& color, bool isDebugOnly )=0;

			///Update the position / orientation of an already made debug renderable
			virtual void UpdateDebugShape(IRenderableID id, const Vector3& position, const Quaternion& orientation, const Vector3& scale)=0;

			///Add a line to a debug renderable, useful for showing debug forces, etc.
			//virtual void AddDebugLine(IRenderableID id, const Vector3& start, const Vector3& end)=0;
		
			///Add a line to a debug renderable, useful for showing debug forces, etc.
			virtual void AddDebugLines(IRenderableID id, const std::vector<Vector3>& lines, const std::vector<Color>& colors)=0;

			///Removes all lines from this debug renderable. This will destroy any shape information that
			///existed in this debug renderable, so this should probably be only called on shapes that
			///the user is manually adding to with AddDebugLine.
			virtual void ClearDebugLines(IRenderableID id)=0;
		
			///Remove a debug renderable from this rendering system
			virtual void DestroyDebugShape(IRenderableID id)=0;
			
			///Remove a debug renderable from this rendering system
			virtual void SetDebugRenderableOrthographic( IRenderableID id, bool ortho )=0;

			///Set the debug visibility for this renderer
			virtual void SetDebugVisibility(bool visible)=0;

			///Set the enabled state for the specified debug renderable. If disabled, a renderable will not be drawn.
			virtual void EnableDebugRenderable(IRenderableID id, bool enable)=0;

			/****************************************/
			/*  Point Clouds						*/
			/****************************************/
			///Create a point cloud
			virtual void CreatePointCloudRenderable(IRenderableID id, const PointCloudDataPtr& pData, const Color& color ) = 0;

			///Update the position / orientation of an already made point cloud renderable
			virtual void UpdatePointCloud(IRenderableID id, const Vector3& position, const Quaternion& orientation)=0;

			///Remove a point cloud  renderable from this rendering system
			virtual void DestroyPointCloud(IRenderableID id)=0;

			///Set the point cloud visibility for this renderer
			virtual void SetPointCloudVisibility(bool visible)=0;

			/****************************************/
			/*  Particle Systems					*/
			/****************************************/
			///Set the particle system visibility for this renderer
			virtual void SetParticleSystemVisibility(bool visible)=0;

			/****************************************/
			/*  Line Rendering   					*/
			/****************************************/

			///Create a nicely drawn line renderable
			virtual void CreateLineRenderable( IRenderableID id, const vector<Vector3>& points, float32 width, const Color& color ) = 0;

			///Update the position / orientation of an already made line
			virtual void UpdateLineRenderable( IRenderableID id, const vector<Vector3>& points, const vector<Color>& colors )=0;

			///Update the color of a line renderable
			virtual void SetLineRenderableColor( IRenderableID id, const Color& color )=0;

			///Remove a line renderable from this rendering system
			virtual void DestroyLineRenderable( IRenderableID id )=0;

			///Set the line visibility for this renderer
			virtual void SetLineRenderableVisible( IRenderableID id, bool visible )=0;

			/****************************************/
			/*  Track Rendering   					*/
			/****************************************/

			///Create a track renderable for tire tracks on the terrain
			virtual void CreateTrackRenderable( IRenderableID id, float32 width ) = 0;
			
			///Remove a track renderable from the rendering system
			virtual void DestroyTrackRenderable( IRenderableID trackID ) = 0;

			///Update a track renderable with the latest position of it's corresponding wheel
			virtual void UpdateTrackRenderablePosition( IRenderableID id, const Vector3& position, SurfaceID surfaceID, bool wheelOnGround ) = 0;

			/************************************************************************/
			/* Text Rendering                                                       */
			/************************************************************************/

			///Render text on the screen in the world view at the given screen coords
			///@param[in] text The text to render
			///@param[in] text The layer the text will be added to
			///@param[in] screenLocation The location on the screen to render it at
			///@param[in] fadeTime The amount of time before the text fades away (with 0 never fading)
			///@param[in] showInActiveRenderWindow If true, the text will be shown in the active render window (the one with the most recent mouse activity), otherwise it will default to the primary render window
			virtual void RenderText2D( const String& text, const String& layer, const Vector2& screenLocation, float32 fadeTime, const Color& textColor = Color::White, bool showInActiveRenderWindow = false )=0;

			///Render text on the screen in the world view at the given world location, which will be transformed into screen space
			///@param[in] text The text to render
			///@param[in] text The layer the text will be added to
			///@param[in] worldLocation The world location we want to transform into screen space and render the text at
			///@param[in] fadeTime The amount of time before the text fades away (with 0 never fading)
			///@param[in] showInActiveRenderWindow If true, the text will be shown in the render window (the one with the most recent mouse activity), otherwise it will default to the primary render window
			virtual void RenderText2D( const String& text, const String& layer, const Vector3& worldLocation, float32 fadeTime, const Color& textColor = Color::White, bool showInActiveRenderWindow = false )=0;

			///Set the position of text in a given layer
			///@param[in] layer The layer of the text we wish to move
			///@param[in] screenLocation The location we want to move the text to
			virtual void SetRenderText2DPosition( const String& layer, const Vector2& screenLocation )=0;

			///Set the position of text in a given layer using a world location, which will be transformed into screen space
			///@param[in] layer The layer of the text we wish to move
			///@param[in] worldLocation The world location we want to transform into screen space and render the text at
			virtual void SetRenderText2DPosition( const String& layer, const Vector3& worldLocation )=0;

			///Clear the text in a given layer
			///@param[in] layer The layer of text we wish to clear
			virtual void ClearRenderText2D( const String& layer )=0;
			
			/************************************************************************/
			/* Render Targets                                                       */
			/************************************************************************/
			
			///Create a Render Target, which is basically a texture to render to
			///@param[in] xSize width, in pixels, of the render target
			///@param[in] ySize height, in pixels of the render target
			///@return ID of the newly created render target
			virtual RenderTargetID CreateRenderTarget( uint32 xSize, uint32 ySize ) { VANE_UNUSED( xSize); VANE_UNUSED(ySize); return kInvalidRenderTargetID;}

			///Destroy a render target
			///@param[in] ID of the render target to destroy
			virtual void DestroyRenderTarget( RenderTargetID /*renderTarget*/ ) { }			

			///Render to a render target, based upon the passed in view data. Note that this
			///is not an immediate call, and can be delayed.  
			///@todo: potentially use a smart pointer, or something like that to help
			///in the situation of a delayed write to a sensor that has been destroyed.
			///@param[in] render target to use
			///@param[in] the View parameters to render the scene with
			///@param[in] output buffer to write the results too.
			///@param[in] pStatusFlag a flag that will be updated with the current rendering status
			///@return true if this render system can handle this update, which means we can stop looking
			///through the render systems.
			virtual bool RenderToRenderTarget( RenderTargetID /*target*/, ViewData& /*lensData*/, RenderRequestData* /*pRenderRequestData*/ ) { return false; }			

			///Clear all outstanding requests for the given render target ID. This can be used when the 
			///buffer has been resized, or a target is being deleted, etc.
			virtual void ClearAllRequestsForRenderTarget( RenderTargetID /*target*/ ) {;}
		};

		/************************************************************************/
		/*  IRendererFactory                                                    */
		/************************************************************************/
		
		///Abstract Factory class for a IRenderer.
		///A renderer factory should take care to actually destroy any Rendering System
		///it created when the factory is destroyed.
		class IRendererFactory
		{
		public:
			virtual ~IRendererFactory() {;}

			///Create a IRenderer; which is in charge of registering itself
			///with the RendererManager (so it can control it's flow of initialization).
			///The Factory will call the new IRenderer's Initialization function.
			virtual void    CreateIRenderer(const String& rendererName)=0;

			///Create a IRenderer, just as above; However the Initialize function
			///taking a String filename is used instead.
			virtual void    CreateIRendererWithFile(const String& rendererName, const String& fileName)=0;

			///Get the type of IRenderer this Factory is responsible for
			virtual String  GetType()=0;
		};
	}
}


#endif // Renderer_h__
