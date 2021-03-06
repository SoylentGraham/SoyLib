#include "SoyUnity.h"
#include "SoyDebug.h"
#include <sstream>
#include "SoyExportManager.h"
#include "SoyFilesystem.h"

//	new interfaces in 5.2+
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"

#if defined(ENABLE_DIRECTX)
#include "SoyDirectx.h"
#include "Unity/IUnityGraphicsD3D11.h"

class ID3D12Device;
class ID3D12CommandQueue;
class ID3D12Fence;
class ID3D12Resource;
class D3D12_RESOURCE_STATES;
#include "Unity/IUnityGraphicsD3D12.h"

#endif


#if defined(ENABLE_DIRECTX9)
class IDirect3D9;
class IDirect3DDevice9;
#include "Unity/IUnityGraphicsD3D9.h"

#include "SoyDirectx9.h"

#endif

#if defined(ENABLE_OPENGL)
#include "SoyOpenglContext.h"
#endif


#if defined(ENABLE_GNM)
#include "SoyGnm.h"
#include "Unity/IUnityGraphicsPS4.h"
#endif

//	unity 5.4
/*
#if defined(ENABLE_METAL)
#include "Unity/IUnityGraphicsMetal.h"
#endif
*/
#if defined(ENABLE_METAL)
#include "SoyMetal.h"
#endif

#if defined(TARGET_ANDROID)
#include "SoyJava.h"
#endif


#if defined(TARGET_IOS)
extern "C" {
typedef	void	(*UnityPluginSetGraphicsDeviceFunc)(void* device, int deviceType, int eventType);
typedef	void	(*UnityPluginRenderMarkerFunc)(int marker);
void	UnityRegisterRenderingPlugin(UnityPluginSetGraphicsDeviceFunc setDevice, UnityPluginRenderMarkerFunc renderMarker);
}

__export void	UnitySetGraphicsDevice(void* device,Unity::sint deviceType,Unity::sint eventType);
#endif

//	for debugging new systems, quickly turn off the DLL calls
#define ENABLE_UNITY_INTERFACES

namespace Platform
{
	std::string		GetBundleIdentifier();
}



namespace Unity
{
	static int		gRenderEventTimerMs = 8;

	std::shared_ptr<Opengl::TContext>	OpenglContext;
	std::shared_ptr<Directx::TContext>	DirectxContext;
	std::shared_ptr<Directx9::TContext>	Directx9Context;
	std::shared_ptr<Metal::TContext>	MetalContext;
	std::shared_ptr<Cuda::TContext>		CudaContext;
	std::shared_ptr<Gnm::TContext>		GnmContext;
		
#if defined(TARGET_IOS)
	void				IosDetectGraphicsDevice();
#endif

	IUnityGraphics*		GraphicsDevice = nullptr;
	IUnityInterfaces*	Interfaces = nullptr;

#if defined(TARGET_PS4)
	void*				GpuAlloc(size_t Size);
	bool				GpuFree(void* Object);
#endif
}

std::function<void()>& Unity::GetOnDeviceShutdown()
{
	static std::function<void()> Event;
	return Event;
}


std::map<UnityDevice::Type, std::string> UnityDevice::EnumMap =
{
	{ UnityDevice::Invalid,				"Invalid" },
	{ UnityDevice::kGfxRendererOpenGL,	"Opengl" },
	{ UnityDevice::kGfxRendererD3D9,	"Directx9"	},
	{ UnityDevice::kGfxRendererD3D11,	"Directx11"	},
	{ UnityDevice::kGfxRendererGCM,		"PS3"	},
	{ UnityDevice::kGfxRendererNull,	"NullBatchMode"	},
	{ UnityDevice::kGfxRendererXenon,	"Xbox360"	},
	{ UnityDevice::kGfxRendererOpenGLES20,	"OpenglES2"	},
	{ UnityDevice::kGfxRendererOpenGLES30,	"OpenglES3"	},
	{ UnityDevice::kGfxRendererGXM,		"PSVita"	},
	{ UnityDevice::kGfxRendererPS4,		"PS4"	},
	{ UnityDevice::kGfxRendererXboxOne,	"XboxOne"	},
	{ UnityDevice::kGfxRendererMetal,	"Metal"	},
	{ UnityDevice::kGfxRendererOpenGLCore,		"Opengl Core"	},
	{ UnityDevice::kGfxRendererD3D12,		"Directx12"	},
};

std::map<UnityEvent::Type, std::string> UnityEvent::EnumMap =
{
	{ UnityEvent::Invalid,						"Invalid" },
	{ UnityEvent::kGfxDeviceEventInitialize,	"kGfxDeviceEventInitialize" },
	{ UnityEvent::kGfxDeviceEventShutdown,		"kGfxDeviceEventShutdown" },
	{ UnityEvent::kGfxDeviceEventBeforeReset,	"kGfxDeviceEventBeforeReset" },
	{ UnityEvent::kGfxDeviceEventAfterReset,	"kGfxDeviceEventAfterReset" },
};


//	from ios trampoline / UnityInterface.h
#if defined(TARGET_IOS)
typedef enum
UnityRenderingAPI
{
	apiOpenGLES2	= 2,
	apiOpenGLES3	= 3,
	apiMetal		= 4,
}
UnityRenderingAPI;
class EAGLContext;

typedef void* MTLDeviceRef;	//	id<MTLDevice>
extern "C" int					UnitySelectedRenderingAPI();
extern "C" MTLDeviceRef			UnityGetMetalDevice();
extern "C" EAGLContext*			UnityGetDataContextEAGL();
/*
extern "C" NSBundle*			UnityGetMetalBundle()		{ return _MetalBundle; }
extern "C" MTLDeviceRef			UnityGetMetalDevice()		{ return _MetalDevice; }
extern "C" MTLCommandQueueRef	UnityGetMetalCommandQueue()	{ return  ((UnityDisplaySurfaceMTL*)GetMainDisplaySurface())->commandQueue; }

extern "C" EAGLContext*			UnityGetDataContextEAGL()	{ return _GlesContext; }
extern "C" int					UnitySelectedRenderingAPI()	{ return _renderingAPI; }

extern "C" UnityRenderBuffer	UnityBackbufferColor()		{ return GetMainDisplaySurface()->unityColorBuffer; }
extern "C" UnityRenderBuffer	UnityBackbufferDepth()		{ return GetMainDisplaySurface()->unityDepthBuffer; }
*/
#endif

#if defined(TARGET_IOS)
void Unity::IosDetectGraphicsDevice()
{
	//	already decided
	if ( OpenglContext )
		return;
#if defined(ENABLE_METAL)
	if ( MetalContext )
		return;
#endif
	
	//	on ios UnitySetGraphicsDevice never gets called, so we do it ourselves depending on API
	//	gr: cannot find a more hard api (eg. active metal context, grabbing the system one seems like we'd just be using that, even if unity isnt)
	auto Api = UnitySelectedRenderingAPI();
	switch ( Api )
	{
		case apiOpenGLES2:
			UnitySetGraphicsDevice( UnityGetDataContextEAGL(), UnityDevice::kGfxRendererOpenGLES20, UnityEvent::kGfxDeviceEventInitialize );
			return;
			
		case apiOpenGLES3:
			UnitySetGraphicsDevice( UnityGetDataContextEAGL(), UnityDevice::kGfxRendererOpenGLES30, UnityEvent::kGfxDeviceEventInitialize );
			return;
			
		case apiMetal:
#if defined(ENABLE_METAL)
			UnitySetGraphicsDevice( UnityGetMetalDevice(), UnityDevice::kGfxRendererMetal, UnityEvent::kGfxDeviceEventInitialize );
			return;
#endif

		default:
			break;
	}
	
	std::stringstream Error;
	Error << "Unknown unity API selected: " << Api;
	throw Soy::AssertException( Error.str() );
}
#endif

bool Unity::HasOpenglContext()
{
#if defined(TARGET_IOS)
	IosDetectGraphicsDevice();
#endif
	return OpenglContext != nullptr;
}

Opengl::TContext& Unity::GetOpenglContext()
{
	auto Ptr = GetOpenglContextPtr();

	if (!Ptr)
		throw Soy::AssertException("Getting opengl context on non-opengl run");

	return *Ptr;
}


std::shared_ptr<Opengl::TContext> Unity::GetOpenglContextPtr()
{
#if defined(TARGET_IOS)
	IosDetectGraphicsDevice();
#endif
	
	return OpenglContext;
}

bool Unity::HasDirectxContext()
{
	return DirectxContext != nullptr;
}

Directx::TContext& Unity::GetDirectxContext()
{
	if (!DirectxContext)
		throw Soy::AssertException("Getting directx context on non-directx run");

	return *DirectxContext;
}

std::shared_ptr<Directx::TContext> Unity::GetDirectxContextPtr()
{
	return DirectxContext;
}


bool Unity::HasDirectx9Context()
{
	return Directx9Context != nullptr;
}

Directx9::TContext& Unity::GetDirectx9Context()
{
	if (!Directx9Context)
		throw Soy::AssertException("Getting directx9 context on non-directx run");

	return *Directx9Context;
}

std::shared_ptr<Directx9::TContext> Unity::GetDirectx9ContextPtr()
{
	return Directx9Context;
}

bool Unity::HasGnmContext()
{
	return GnmContext != nullptr;
}

Gnm::TContext& Unity::GetGnmContext()
{
	if (!GnmContext)
		throw Soy::AssertException("Getting Gnm context on non-Gnm run");

	return *GnmContext;
}

std::shared_ptr<Gnm::TContext> Unity::GetGnmContextPtr()
{
	return GnmContext;
}


std::shared_ptr<Metal::TContext> Unity::GetMetalContextPtr()
{
#if defined(TARGET_IOS)
	IosDetectGraphicsDevice();
#endif
	return MetalContext;
}


Metal::TContext& Unity::GetMetalContext()
{
	auto Ptr = GetMetalContextPtr();
	
	if (!Ptr)
		throw Soy::AssertException("Getting metal context on non-metal run");
	
	return *Ptr;
}

bool Unity::HasMetalContext()
{
#if defined(TARGET_IOS)
	IosDetectGraphicsDevice();
#endif
	return MetalContext != nullptr;
}



std::shared_ptr<Cuda::TContext> Unity::GetCudaContextPtr()
{
#if defined(ENABLE_CUDA)
	if ( !CudaContext )
		CudaContext.reset(new Cuda::TContext);
#endif

	return CudaContext;
}


SoyPixelsFormat::Type Unity::GetPixelFormat(RenderTexturePixelFormat::Type Format)
{
	switch ( Format )
	{
		//	gr: this appears to be RGBA (opengl texture format is RGBA8 on osx)
		//	gr: possibly it IS ARGB but opengl reinterprets it?
		case RenderTexturePixelFormat::ARGB32:	return SoyPixelsFormat::RGBA;
	
		//	todo: test this
		case RenderTexturePixelFormat::R8:		return SoyPixelsFormat::Greyscale;

		//	floats, test all these!
		case RenderTexturePixelFormat::Depth:	return SoyPixelsFormat::Greyscale;
		case RenderTexturePixelFormat::ARGBHalf:	return SoyPixelsFormat::RGBA;
		case RenderTexturePixelFormat::ARGBFloat:	return SoyPixelsFormat::RGBA;
		case RenderTexturePixelFormat::RFloat:		return SoyPixelsFormat::Greyscale;
		case RenderTexturePixelFormat::RGHalf:		return SoyPixelsFormat::GreyscaleAlpha;
		case RenderTexturePixelFormat::RGFloat:		return SoyPixelsFormat::GreyscaleAlpha;
		case RenderTexturePixelFormat::RHalf:		return SoyPixelsFormat::Greyscale;
			
		//	test these!
		//case RenderTexturePixelFormat::Shadowmap:	return SoyPixelsFormat::RGBA;
		//case RenderTexturePixelFormat::DefaultHDR:	return SoyPixelsFormat::RGBA;
			
		//	16 bit treated as 2 channel
		case RenderTexturePixelFormat::ARGB4444:	return SoyPixelsFormat::GreyscaleAlpha;
		case RenderTexturePixelFormat::RGB565:		return SoyPixelsFormat::GreyscaleAlpha;
		case RenderTexturePixelFormat::ARGB1555:	return SoyPixelsFormat::GreyscaleAlpha;
			
		default:
			return SoyPixelsFormat::Invalid;
	}
}


SoyPixelsFormat::Type Unity::GetPixelFormat(Texture2DPixelFormat::Type Format)
{
	switch ( Format )
	{
		case Texture2DPixelFormat::RGBA32:	return SoyPixelsFormat::RGBA;
		case Texture2DPixelFormat::RGB24:	return SoyPixelsFormat::RGB;
		case Texture2DPixelFormat::BGRA32:	return SoyPixelsFormat::BGRA;
		case Texture2DPixelFormat::Alpha8:	return SoyPixelsFormat::Greyscale;

		//	gr: this was commented out... but it comes up...
		case Texture2DPixelFormat::ARGB32:	return SoyPixelsFormat::ARGB;
			
		default:
			return SoyPixelsFormat::Invalid;
	}
}


//	try and help debug DLL loading in windows
#if defined(TARGET_WINDOWS)
BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved)
{
	Platform::SetDllPath();
	return TRUE;
}
#endif

void Unity::RenderEvent(Unity::sint eventID)
{
	ofScopeTimerWarning Timer(__func__,gRenderEventTimerMs);

	//	iterate current context
#if defined(ENABLE_OPENGL)
	if (Unity::OpenglContext)
	{
		Unity::OpenglContext->Iteration();
	}
#endif

#if defined(ENABLE_DIRECTX)
	if ( Unity::DirectxContext )
	{
		Unity::DirectxContext->Iteration();
	}
#endif
#if defined(ENABLE_DIRECTX9)
	if ( Unity::Directx9Context )
	{
		Unity::Directx9Context->Iteration();
	}
#endif

#if defined(ENABLE_METAL)
	if ( Unity::MetalContext )
	{
		Unity::MetalContext->Iteration();
	}
#endif	

#if defined(ENABLE_GNM)
	if ( Unity::GnmContext )
	{
		Gnm::IterateContext( *Unity::GnmContext );
	}
#endif
}

__export void UnitySetGraphicsDevice(void* device,Unity::sint deviceType,Unity::sint eventType)
{
	try
	{
	#if defined(ENABLE_UNITY_INTERFACES)
		auto DeviceType = UnityDevice::Validate(deviceType);
		auto DeviceEvent = UnityEvent::Validate(eventType);

		std::Debug << __func__ << "(" << UnityDevice::ToString(DeviceType) << "," << UnityEvent::ToString(DeviceEvent) << ")" << std::endl;

		switch (DeviceEvent)
		{
		case UnityEvent::kGfxDeviceEventShutdown:
			Unity::Shutdown(DeviceType);
			break;

		case UnityEvent::kGfxDeviceEventInitialize:
			Unity::Init(DeviceType, device);
			break;

		default:
			break;
		};
	#else
		std::Debug << __func__ << " disabled" << std::endl;
	#endif
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
	}
}

//	gr: this is deprecated now, and instead unity lets you explicitly call a function.
//	gr: check this is okay with multiple plugins linking, which was the original reason for a unique function name...
#if defined(TARGET_IOS)
__export void UnityRenderEvent_Ios(Unity::sint eventID)
#else
__export void UnityRenderEvent(Unity::sint eventID)
#endif
{
	try
		{
	#if defined(ENABLE_UNITY_INTERFACES)
		//	event triggered by other plugin
		if ( eventID != Unity::GetPluginEventId() )
		{
			if ( Unity::IsDebugPluginEventEnabled() )
				std::Debug << "UnityRenderEvent(" << eventID << ") Not ours " << Unity::GetPluginEventId() << std::endl;
			return;
		}
	
		Unity::RenderEvent( eventID );
	#else
		std::Debug << __func__ << " disabled" << std::endl;
	#endif
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
	}
}



void Unity::Init(UnityDevice::Type Device,void* DevicePtr)
{
	if (!Platform::Init())
		throw Soy::AssertException("Soy Failed to init platform");

	/*
	if ( !DebugListener.IsValid() )
	{
		auto& Event = std::Debug.GetOnFlushEvent();
		
		auto PushDebug = [](const std::string& Debug)
		{
			PushDebugString( Debug );
		};
		
		DebugListener = Event.AddListener( PushDebug );
	}
	*/
	
	//	ios needs to manually register callbacks
	//	http://gamedev.stackexchange.com/questions/100485/how-do-i-get-gl-issuerenderevent-to-work-on-ios-with-unity-5
#if defined(TARGET_IOS)
	UnityRegisterRenderingPlugin( nullptr, &UnityRenderEvent_Ios );
#endif

	//	allocate appropriate context and init
	switch ( Device )
	{
#if defined(ENABLE_OPENGL)
		case UnityDevice::kGfxRendererOpenGL:
		case UnityDevice::kGfxRendererOpenGLES20:
		case UnityDevice::kGfxRendererOpenGLES30:
		case UnityDevice::kGfxRendererOpenGLCore:
		{
			//	init context on first run
			auto InitContext = []
			{
				Unity::GetOpenglContext().Init();
			};
			
			//	already initialised
			if ( OpenglContext )
				break;
			
			OpenglContext.reset(new Opengl::TContext);
			Unity::GetOpenglContext().PushJob(InitContext);

		}
		break;
#endif

	#if defined(ENABLE_DIRECTX)
		case UnityDevice::kGfxRendererD3D11:
		case UnityDevice::kGfxRendererD3D12:
		{
			auto* DeviceDx = static_cast<ID3D11Device*>( DevicePtr );
			Soy::Assert( DeviceDx != nullptr, "Missing device pointer to create directx context");

			//	already setup (going through new & legacy interface)
			if ( DirectxContext && DirectxContext->mDevice == DeviceDx )
			{
				std::Debug << "DX11 graphics device already created." << std::endl;
				break;
			}

			DirectxContext.reset(new Directx::TContext( *DeviceDx ) );
		}
		break;
	#endif

	#if defined(ENABLE_DIRECTX9)
		case UnityDevice::kGfxRendererD3D9:
		{
			auto* DeviceDx = static_cast<IDirect3DDevice9*>( DevicePtr );
			Soy::Assert( DeviceDx != nullptr, "Missing device pointer to create directx context");

			//	already setup (going through new & legacy interface)
			if ( Directx9Context && Directx9Context->mDevice == DeviceDx )
			{
				std::Debug << "DX9 graphics device already created." << std::endl;
				break;
			}

			if ( Directx9Context )
				std::Debug << "DX9 context changed" << std::endl;
			Directx9Context.reset(new Directx9::TContext( *DeviceDx ) );
		}
		break;
	#endif

#if defined(ENABLE_METAL)
		case UnityDevice::kGfxRendererMetal:
		{
			MetalContext.reset(new Metal::TContext( DevicePtr ) );
		}
		break;
#endif

#if defined(ENABLE_GNM)
		case UnityDevice::kGfxRendererPS4:
		{
			GnmContext = Gnm::AllocContext( DevicePtr, Unity::GpuAlloc, Unity::GpuFree );
		}
		break;
#endif

		default:
		{
			std::string DeviceName;
			try
			{
				DeviceName = UnityDevice::ToString(Device);
			}
			catch(...)
			{
				std::stringstream Error;
				Error << "(Unknown device id " << static_cast<int>(Device) << ")";
				DeviceName = Error.str();
			}
			
			std::Debug << "Unsupported device type " << DeviceName << std::endl;
		}
		break;
	};

	std::Debug << "Unity::Init(" << UnityDevice::ToString(Device) << ") finished" << std::endl;
}

void Unity::Shutdown(UnityDevice::Type Device)
{
	//std::Debug.GetOnFlushEvent().RemoveListener( DebugListener );

	{
		GetOnDeviceShutdown();
	}
	
	//	free all contexts
	//	gr: may need to defer some of these!
	OpenglContext.reset();
	MetalContext.reset();
	DirectxContext.reset();
	Directx9Context.reset();
	CudaContext.reset();
	GnmContext.reset();
}


template<typename INTERFACETYPE>
void* GetDeviceContext()
{
	if ( !Unity::Interfaces )
		return nullptr;

	auto* Interface = Unity::Interfaces->Get<INTERFACETYPE>();
	return Interface->GetDevice();
}

#if defined(ENABLE_GNM)
template<>
void* GetDeviceContext<IUnityGraphicsPS4>()
{
	if ( !Unity::Interfaces )
		return nullptr;

	auto* Interface = Unity::Interfaces->Get<IUnityGraphicsPS4>();
	return Interface->GetGfxContext();
}
#endif


#if defined(TARGET_PS4)
void* Unity::GpuAlloc(size_t Size)
{
	if ( !Unity::Interfaces )
		return nullptr;
	auto* Interface = Unity::Interfaces->Get<IUnityGraphicsPS4>();
	if ( !Interface )
		return nullptr;

	int kAlignmentOfShaderInBytes = 256;

	return Interface->AllocateGPUMemory( Size, kAlignmentOfShaderInBytes );
}
#endif

#if defined(TARGET_PS4)
bool Unity::GpuFree(void* Object)
{
	if ( !Unity::Interfaces )
		return nullptr;
	auto* Interface = Unity::Interfaces->Get<IUnityGraphicsPS4>();
	if ( !Interface )
		return nullptr;

	int Alignment = 0;
	Interface->ReleaseGPUMemory( Object );
	return true;
}
#endif

void* Unity::GetPlatformDeviceContext(UnityDevice::Type Device)
{
	switch ( Device )
	{
	#if defined(ENABLE_DIRECTX)
		case kUnityGfxRendererD3D11:	return GetDeviceContext<IUnityGraphicsD3D11>();
		case kUnityGfxRendererD3D12:	return GetDeviceContext<IUnityGraphicsD3D12>();
	#endif
	#if defined(ENABLE_DIRECTX9)
		case kUnityGfxRendererD3D9:		return GetDeviceContext<IUnityGraphicsD3D9>();
	#endif
	#if defined(ENABLE_GNM)
		case kUnityGfxRendererPS4:		return GetDeviceContext<IUnityGraphicsPS4>();
	#endif		
	
		default:
			return nullptr;
	}
}

void* Unity::GetPlatformDeviceContext()
{
	auto Device = Unity::GraphicsDevice;
	if ( !Device )
		throw Soy::AssertException("missing graphics device");
	auto DeviceType = static_cast<UnityDevice::Type>( Unity::GraphicsDevice->GetRenderer() );
	return GetPlatformDeviceContext( DeviceType );
}

void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	try
	{
		std::Debug << __func__ << "(" << eventType << ")" << std::endl;
		
	#if defined(ENABLE_UNITY_INTERFACES)
		auto Device = Unity::GraphicsDevice;
		if ( !Device )
		{
			std::Debug << __func__ << " missing graphics device" << std::endl;
			return;
		}

		auto DeviceType = static_cast<UnityDevice::Type>( Unity::GraphicsDevice->GetRenderer() );
		void* DeviceContext = Unity::GetPlatformDeviceContext(DeviceType);

		UnitySetGraphicsDevice( DeviceContext, DeviceType, eventType );
	#else
		std::Debug << __func__ << " disabled" << std::endl;
	#endif
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
	}
}

// Unity plugin load event
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	try
		{
	#if defined(ENABLE_UNITY_INTERFACES)
		Unity::Interfaces = unityInterfaces;
		Unity::GraphicsDevice = Unity::Interfaces->Get<IUnityGraphics>();

		Unity::GraphicsDevice->RegisterDeviceEventCallback( OnGraphicsDeviceEvent );

		// Run OnGraphicsDeviceEvent(initialize) manually on plugin load
		// to not miss the event in case the graphics device is already initialized
		OnGraphicsDeviceEvent( kUnityGfxDeviceEventInitialize );
	#else
		std::Debug << __func__ << " disabled" << std::endl;
	#endif
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
	}
}

// Unity plugin unload event
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	try
	{
	#if defined(ENABLE_UNITY_INTERFACES)
		if ( Unity::GraphicsDevice )
		{
			Unity::GraphicsDevice->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
			//Unity::GraphicsDevice = nullptr;
		}
	#else
		std::Debug << __func__ << " disabled" << std::endl;
	#endif
	}
	catch(std::exception& e)
	{
		std::Debug << __func__ << " exception: " << e.what() << std::endl;
	}
}

void Unity::GetSystemFileExtensions(ArrayBridge<std::string>&& Extensions)
{
	Extensions.PushBack(".meta");
}


#if defined(TARGET_ANDROID)
std::string Platform::GetBundleIdentifier()
{
	return Java::GetBundleIdentifier();
}
#elif defined(TARGET_IOS)||defined(TARGET_OSX)
//	in mm
#else
std::string Platform::GetBundleIdentifier()
{
	return "Todo:BundleIdentifier";
}
#endif

const std::string& Unity::GetBundleIdentifier()
{
	//	cache this
	static std::string CachedIdentifier = Soy::StringToLower( Platform::GetBundleIdentifier() );
	return CachedIdentifier;
}
