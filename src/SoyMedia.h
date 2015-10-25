#pragma once

#include "SoyPixels.h"
#include "SoyThread.h"

namespace Soy
{
	class TYuvParams;
}

//	implement this per-platform
//	currently all this is used in PopMovieTexture/PopCast, but is forward declared
namespace Platform
{
	class TMediaFormat;
}

namespace Opengl
{
	class TContext;
	class TTexture;
}

namespace Directx
{
	class TContext;
	class TTexture;
}

//	merge this + pixel format at some point
namespace SoyMediaFormat
{
	enum Type
	{
		Invalid = SoyPixelsFormat::Invalid,
		
		Greyscale = SoyPixelsFormat::Greyscale,
		GreyscaleAlpha = SoyPixelsFormat::GreyscaleAlpha,
		RGB = SoyPixelsFormat::RGB,
		RGBA = SoyPixelsFormat::RGBA,
		BGRA = SoyPixelsFormat::BGRA,
		BGR = SoyPixelsFormat::BGR,
		KinectDepth = SoyPixelsFormat::KinectDepth,
		FreenectDepth10bit = SoyPixelsFormat::FreenectDepth10bit,
		FreenectDepth11bit = SoyPixelsFormat::FreenectDepth11bit,
		FreenectDepthmm = SoyPixelsFormat::FreenectDepthmm,
		Yuv420_Biplanar_Full = SoyPixelsFormat::Yuv420_Biplanar_Full,
		Yuv420_Biplanar_Video = SoyPixelsFormat::Yuv420_Biplanar_Video,
		Yuv422_Biplanar_Full = SoyPixelsFormat::Yuv422_Biplanar_Full,
		Yuv444_Biplanar_Full = SoyPixelsFormat::Yuv444_Biplanar_Full,
		LumaFull = SoyPixelsFormat::LumaFull,
		LumaVideo = SoyPixelsFormat::LumaVideo,
		Chroma2 = SoyPixelsFormat::Chroma2,
		Nv12 = SoyPixelsFormat::Nv12,
		
		NotPixels = SoyPixelsFormat::Count,
		
		H264,			//	specialise this? or go the other way and call it compressed-video? or should this just be a pixel format?
		H264Ts,			//	elementry/transport/annexb stream (H264 ES in MF)
		Mpeg2TS,
		Mpeg2,
		
		Audio,
		Text,
		Subtitle,
		ClosedCaption,
		Timecode,
		MetaData,
		Muxed,
	};
	
	SoyPixelsFormat::Type	GetPixelFormat(Type MediaFormat);
	Type					FromPixelFormat(SoyPixelsFormat::Type PixelFormat);
	
	bool		IsVideo(Type Format);	//	or pixels
	inline bool	IsPixels(Type Format)	{	return GetPixelFormat( Format ) != SoyPixelsFormat::Invalid;	}
	bool		IsAudio(Type Format);
	Type		FromFourcc(uint32 Fourcc);
	std::string	ToMime(Type Format);
	Type		FromMime(const std::string& Mime);
	
	DECLARE_SOYENUM(SoyMediaFormat);
}




class Soy::TYuvParams
{
private:
	TYuvParams(float LumaMin,float LumaMax,float ChromaVRed,float ChromaUGreen,float ChromaVGreen,float ChromaUBlue) :
	mLumaMin		( LumaMin ),
	mLumaMax		( LumaMax ),
	mChromaVRed		( ChromaVRed ),
	mChromaUGreen	( ChromaUGreen ),
	mChromaVGreen	( ChromaVGreen ),
	mChromaUBlue	( ChromaUBlue )
	{
	}
public:
	static TYuvParams	Video()
	{
		float LumaMin = 16.0/255.0;
		float LumaMax = 253.0/255.0;
		float ChromaVRed = 1.5958;
		float ChromaUGreen = -0.39173;
		float ChromaVGreen = -0.81290;
		float ChromaUBlue = 2.017;
		return TYuvParams( LumaMin, LumaMax, ChromaVRed, ChromaUGreen, ChromaVGreen, ChromaUBlue );
	}
	static TYuvParams	Full()
	{
		float LumaMin = 0;
		float LumaMax = 1;
		float ChromaVRed = 1.4;
		float ChromaUGreen = -0.343;
		float ChromaVGreen = -0.711;
		float ChromaUBlue = 1.765;
		return TYuvParams( LumaMin, LumaMax, ChromaVRed, ChromaUGreen, ChromaVGreen, ChromaUBlue );
	}
	
	float	mLumaMin;
	float	mLumaMax;
	float	mChromaVRed;
	float	mChromaUGreen;
	float	mChromaVGreen;
	float	mChromaUBlue;
};



//	now this is in soy, give it a better name!
class TStreamMeta
{
public:
	TStreamMeta() :
	mCodec				( SoyMediaFormat::Invalid ),
	mMediaTypeIndex		( 0 ),
	mStreamIndex		( 0 ),
	mCompressed			( false ),
	mFramesPerSecond	( 0 ),
	mChannelCount		( 0 ),
	mInterlaced			( false ),
	mVideoClockWiseRotationDegrees	( 0 ),
	m3DVideo			( false ),
	mDrmProtected		( false ),
	mMaxKeyframeSpacing	( 0 ),
	mAverageBitRate		( 0 ),
	mYuvMatrix			( Soy::TYuvParams::Full() ),
	mEncodingBitRate	( 0 )
	{
	};
	
	void				SetMime(const std::string& Mime)	{	mCodec = SoyMediaFormat::FromMime( Mime );	}
	std::string			GetMime() const						{	return SoyMediaFormat::ToMime( mCodec );	}
	
public:
	SoyMediaFormat::Type	mCodec;
	Array<uint8>		mExtensions;		//	codec extensions
	std::string			mDescription;		//	other meta that doesnt fit here (eg. unsupported type)
	bool				mCompressed;
	float				mFramesPerSecond;	//	0 when not known. in audio this is samples per second (hz)
	SoyTime				mDuration;
	size_t				mEncodingBitRate;
	
	//	windows media foundation
	size_t				mStreamIndex;		//	windows MediaFoundation can have multiple metas for a single stream (MediaType index splits this), otherwise this would be EXTERNAL from the meta
	size_t				mMediaTypeIndex;	//	applies to Windows MediaFoundation streams
	
	//	video
	SoyPixelsMeta		mPixelMeta;			//	could be invalid format (if unknown, so just w/h) or because its audio etc
	bool				mInterlaced;
	float				mVideoClockWiseRotationDegrees;	//	todo: change for a 3x3 matrix
	bool				m3DVideo;
	Soy::TYuvParams		mYuvMatrix;
	bool				mDrmProtected;
	size_t				mMaxKeyframeSpacing;	//	gr: not sure of entropy yet
	size_t				mAverageBitRate;	//	gr: not sure of entropy yet
	float3x3			mTransform;
	
	//	audio
	size_t				mChannelCount;		//	for audio. Maybe expand to planes? but mPixelMeta tell us this
};
std::ostream& operator<<(std::ostream& out,const TStreamMeta& in);



//	merge this with TMediaPacket so the content can be abstract like TPixelBuffer
//	but cover more than just pixels, like TMediaPacket
class TPixelBuffer
{
public:
	//	different paths return arrays now - shader/fbo blit is pretty generic now so move it out of pixel buffer
	//	generic array, handle that internally (each implementation tends to have it's own lock info anyway)
	//	for future devices (metal, dx), expand these
	//	if 1 texture assume RGB/BGR greyscale etc
	//	if multiple, assuming YUV
	//virtual void		Lock(ArrayBridge<Metal::TTexture>&& Textures)=0;
	//virtual void		Lock(ArrayBridge<Cuda::TTexture>&& Textures)=0;
	//virtual void		Lock(ArrayBridge<Opencl::TTexture>&& Textures)=0;
	virtual void		Lock(ArrayBridge<Opengl::TTexture>&& Textures,Opengl::TContext& Context)=0;
	virtual void		Lock(ArrayBridge<Directx::TTexture>&& Textures,Directx::TContext& Context)=0;
	virtual void		Lock(ArrayBridge<SoyPixelsImpl*>&& Textures)=0;
	virtual void		Unlock()=0;
};


class TPixelBufferFrame
{
public:
	std::shared_ptr<TPixelBuffer>	mPixels;
	SoyTime							mTimestamp;
};


class TPixelBufferManagerBase
{
public:
	virtual bool		PushPixelBuffer(TPixelBufferFrame& PixelBuffer,std::function<bool()> Block)=0;
};


class TMediaPacket
{
public:
	TMediaPacket() :
	mIsKeyFrame		( false ),
	mEncrypted		( false ),
	mEof			( false )
	{
	}
	
	SoyTime					GetSortTimecode() const		{	return mDecodeTimecode.IsValid() ? mDecodeTimecode : mTimecode;	}
	
public:
	bool					mEof;
	SoyTime					mTimecode;	//	presentation time
	SoyTime					mDuration;
	SoyTime					mDecodeTimecode;
	bool					mIsKeyFrame;
	bool					mEncrypted;
	
	std::shared_ptr<Platform::TMediaFormat>	mFormat;
	TStreamMeta				mMeta;			//	format info
	
	Array<uint8>			mData;
};
std::ostream& operator<<(std::ostream& out,const TMediaPacket& in);



//	abstracted so later we can handle multiple streams at once as seperate buffers
class TMediaPacketBuffer
{
public:
	TMediaPacketBuffer(size_t MaxBufferSize=10) :
	mMaxBufferSize	( MaxBufferSize )
	{
	}
	
	//	todo: options here so we can get the next packet we need
	//		where we skip over all frames until the prev-to-Time keyframe
	//std::shared_ptr<TMediaPacket>	PopPacket(SoyTime Time);
	std::shared_ptr<TMediaPacket>	PopPacket();
	void							PushPacket(std::shared_ptr<TMediaPacket> Packet,std::function<bool()> Block);
	bool							HasPackets() const	{	return !mPackets.IsEmpty();	}
	
public:
	SoyEvent<std::shared_ptr<TMediaPacket>>	mOnNewPacket;
	
private:
	//	make this a ring buffer of objects
	//	gr: but it also needs to be sorted!
	size_t									mMaxBufferSize;
	Array<std::shared_ptr<TMediaPacket>>	mPackets;
	std::mutex								mPacketsLock;
};


class TMediaExtractor : public SoyWorkerThread
{
public:
	TMediaExtractor(const std::string& ThreadName);
	virtual ~TMediaExtractor()
	{
		WaitToFinish();
	}
	
	void							Seek(SoyTime Time);				//	keep calling this, controls the packet read-ahead
	
	virtual void					GetStreams(ArrayBridge<TStreamMeta>&& Streams)=0;
	TStreamMeta						GetVideoStream(size_t Index);
	virtual std::shared_ptr<Platform::TMediaFormat>	GetStreamFormat(size_t StreamIndex)=0;
	bool							HasFatalError(std::string& Error)
	{
		Error = mFatalError;
		return !Error.empty();
	}
	
	//	change this to one-per stream
	std::shared_ptr<TMediaPacketBuffer>	GetVideoStreamBuffer()		{	return mBuffer;	}
	
	
protected:
	void							OnEof();
	void							OnError(const std::string& Error);
	
	//virtual void					ResetTo(SoyTime Time);			//	for when we seek backwards, assume a stream needs resetting
	void							ReadPacketsUntil(SoyTime Time,std::function<bool()> While);
	virtual std::shared_ptr<TMediaPacket>	ReadNextPacket()=0;
	
private:
	virtual bool					Iteration() override;
	
protected:
	std::shared_ptr<TMediaPacketBuffer>		mBuffer;
	
private:
	std::string						mFatalError;
	SoyTime							mSeekTime;
};


//	transcoder
//	todo: change output buffer to a packet buffer - esp when it becomes audio
//	gr: this is dumb atm, it processes packets on the thread ASAP.
//		output decides what's discarded, input decides what's skipped.
//		maybe it should be more intelligent as this should be the SLOWEST part of the chain?...
class TMediaEncoder : public SoyWorkerThread
{
public:
	TMediaEncoder(const std::string& ThreadName,std::shared_ptr<TMediaPacketBuffer>& InputBuffer,std::shared_ptr<TPixelBufferManagerBase> OutputBuffer);
	virtual ~TMediaEncoder();
	
	bool							HasFatalError(std::string& Error)
	{
		Error = mFatalError.str();
		return !Error.empty();
	}
	
protected:
	virtual void					ProcessPacket(const TMediaPacket& Packet)=0;
	
	//	gr: added this for android as I'm not sure about the thread safety, but other platforms generally have a callback
	//		if we can do input&output on different threads, remove this
	virtual void					ProcessOutputPacket(TPixelBufferManagerBase& FrameBuffer)
	{
	}
	
private:
	virtual bool					Iteration() final;
	virtual bool					CanSleep() override;
	
protected:
	std::shared_ptr<TMediaPacketBuffer>		mInput;
	std::shared_ptr<TPixelBufferManagerBase>	mOutput;
	std::stringstream					mFatalError;
	
private:
	SoyListenerId						mOnNewPacketListener;
};



