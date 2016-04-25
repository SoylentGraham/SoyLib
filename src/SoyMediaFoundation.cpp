#include "SoyMediaFoundation.h"
#include <SoyDirectx.h>

#include <Mferror.h>
#include <Codecapi.h>


#define DEFINE_GUID_CUSTOM(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define DEFINE_MEDIATYPE_GUID_CUSTOM(name, format) \
    DEFINE_GUID_CUSTOM(name,                       \
    format, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);


//	some fourcc's not declared in MediaFoundation
DEFINE_MEDIATYPE_GUID_CUSTOM( MFVideoFormat_VIDS,      FCC('vids') );
DEFINE_MEDIATYPE_GUID_CUSTOM( MFAudioFormat_AUDS,      FCC('auds') );


std::string GetFourCCString(uint32 MediaFormatFourCC)
{
	//	gr: these are in MFAPi.h, not sure why compiler isn't resolving them
#ifndef DIRECT3D_VERSION
#define D3DFMT_R8G8B8       20
#define D3DFMT_A8R8G8B8     21
#define D3DFMT_X8R8G8B8     22
#define D3DFMT_R5G6B5       23
#define D3DFMT_X1R5G5B5     24
#define D3DFMT_P8           41
#define LOCAL_D3DFMT_DEFINES 1
#endif	
	//	special cases
	switch ( MediaFormatFourCC )
	{
		case D3DFMT_R8G8B8:		return "D3DFMT_R8G8B8";
		case D3DFMT_A8R8G8B8:	return "D3DFMT_A8R8G8B8";
		case D3DFMT_X8R8G8B8:	return "D3DFMT_X8R8G8B8";
		case D3DFMT_R5G6B5:		return "D3DFMT_R5G6B5";
		case D3DFMT_X1R5G5B5:	return "D3DFMT_X1R5G5B5";
		case D3DFMT_P8:			return "D3DFMT_P8";
	}

	//	extract fourcc
	return Soy::FourCCToString( MediaFormatFourCC );
}	



const char* MediaGuidSpecialToString(REFGUID guid)
{
	if ( guid == MFVideoFormat_MPEG2 )		return "MFVideoFormat_MPEG2";
	if ( guid == MFVideoFormat_H264_ES )	return "MFVideoFormat_H264_ES";
	if ( guid == MFMediaType_Default )		return "Default";
	if ( guid == MFMediaType_Audio )		return "Audio";
	if ( guid == MFMediaType_Video )		return "Video";
	if ( guid == MFMediaType_Protected )	return "Protected";
	if ( guid == MFMediaType_SAMI )			return "SAMI";
	if ( guid == MFMediaType_Script )		return "Script";
	if ( guid == MFMediaType_Image )		return "Image";
	if ( guid == MFMediaType_HTML )			return "Html";
	if ( guid == MFMediaType_Binary )		return "Binary";
	if ( guid == MFMediaType_FileTransfer )	return "FileTransfer";
	if ( guid == MFImageFormat_JPEG )		return "MFImageFormat_JPEG";
	//	if ( guid == MFImageFormat_RGB32 )		return "MFImageFormat_RGB32";
	if ( guid == MFStreamFormat_MPEG2Transport )	return "MPEG2Transport";	//	.ts
	if ( guid == MFStreamFormat_MPEG2Program )		return "MPEG2Program";

	return nullptr;
}

bool GetMediaFormatGuidFourcc(GUID Guid,uint32& Part)
{
	//	data1 is the specific one. Pop it and if 0 matches Base then we know it's a MediaFormat guid
	Part = Guid.Data1;
	Guid.Data1 = 0;
	if ( Guid == MFVideoFormat_Base )
		return true;
	if ( Guid == MFAudioFormat_Base )
		return true;
	return false;
}


#define FORMAT_MAP(Enum,SoyFormat)	TPlatformFormatMap<GUID>( Enum, #Enum, SoyFormat )
template<typename PLATFORMTYPE>
class TPlatformFormatMap
{
public:
	TPlatformFormatMap(PLATFORMTYPE Enum,const char* EnumName,SoyMediaFormat::Type SoyFormat) :
		mPlatformFormat		( Enum ),
		mName				( EnumName ),
		mSoyFormat			( SoyFormat )
	{
		Soy::Assert( IsValid(), "Expected valid enum - or invalid enum is bad" );
	}
	TPlatformFormatMap() :
		mPlatformFormat		( 0 ),
		mName				( "Invalid enum" ),
		mSoyFormat			( SoyPixelsFormat::Invalid )
	{
	}

	bool		IsValid() const		{	return mPlatformFormat != PLATFORMTYPE();	}

	bool		operator==(const PLATFORMTYPE& Enum) const				{	return mPlatformFormat == Enum;	}
	bool		operator==(const SoyPixelsFormat::Type& Format) const	{	return *this == SoyMediaFormat::FromPixelFormat(Format);	}
	bool		operator==(const SoyMediaFormat::Type& Format) const	{	return mSoyFormat == Format;	}

public:
	PLATFORMTYPE			mPlatformFormat;
	SoyMediaFormat::Type	mSoyFormat;
	std::string				mName;
};



TPlatformFormatMap<GUID> PlatformFormatMap[] =
{
	FORMAT_MAP( MFVideoFormat_RGB32,	SoyMediaFormat::RGBA ),
	FORMAT_MAP( MFVideoFormat_RGB24,	SoyMediaFormat::RGB ),

	//	YUV format explanations
	//	https://msdn.microsoft.com/en-us/library/windows/desktop/aa370819(v=vs.85).aspx
	//	MFVideoFormat_NV11	NV11	4:1:1	Planar	8
	//	MFVideoFormat_NV12	NV12	4:2:0	Planar	8
	FORMAT_MAP( MFVideoFormat_NV12,	SoyMediaFormat::Yuv_8_88_Full ),
	FORMAT_MAP( MFVideoFormat_NV12,	SoyMediaFormat::Yuv_8_88_Ntsc ),
	FORMAT_MAP( MFVideoFormat_NV12,	SoyMediaFormat::Yuv_8_88_Smptec ),

	//	MFVideoFormat_YUY2	YUY2	4:2:2	Packed	8
	FORMAT_MAP( MFVideoFormat_YUY2,	SoyMediaFormat::YYuv_8888_Full ),
	FORMAT_MAP( MFVideoFormat_YUY2,	SoyMediaFormat::YYuv_8888_Ntsc ),
	FORMAT_MAP( MFVideoFormat_YUY2,	SoyMediaFormat::YYuv_8888_Smptec ),

	//	gr: not actually YYuv_8888?
	//	MFVideoFormat_UYVY	UYVY	4:2:2	Packed	8
	FORMAT_MAP( MFVideoFormat_IYUV,	SoyMediaFormat::YYuv_8888_Full ),
	FORMAT_MAP( MFVideoFormat_IYUV,	SoyMediaFormat::YYuv_8888_Ntsc ),
	FORMAT_MAP( MFVideoFormat_IYUV,	SoyMediaFormat::YYuv_8888_Smptec ),

	//	gr: not actually YYuv_8888
	//	MFVideoFormat_Y42T	Y42T	4:2:2	Packed	8
	FORMAT_MAP( MFVideoFormat_Y42T,	SoyMediaFormat::YYuv_8888_Full ),
	FORMAT_MAP( MFVideoFormat_Y42T,	SoyMediaFormat::YYuv_8888_Ntsc ),
	FORMAT_MAP( MFVideoFormat_Y42T,	SoyMediaFormat::YYuv_8888_Smptec ),

	//	from an apple sample movie
	//	http://www.fourcc.org/codecs.php
	//	YUV 4:2:2 CCIR 601 for V422 (no, I don't understand this either) 
	FORMAT_MAP( MFVideoFormat_VIDS,	SoyMediaFormat::Yuv_8_88_Full ),
	FORMAT_MAP( MFVideoFormat_VIDS,	SoyMediaFormat::Yuv_8_88_Ntsc ),
	FORMAT_MAP( MFVideoFormat_VIDS,	SoyMediaFormat::Yuv_8_88_Smptec ),


	/*
	//	alpha4 index4 ?
	//MFVideoFormat_AI44	AI44	4:4:4	Packed	Palettized
	//	alpha yuv a844?
	//MFVideoFormat_AYUV	AYUV	4:4:4	Packed	8
	//MFVideoFormat_I420	I420	4:2:0	Planar	8

	//	MFVideoFormat_IYUV	IYUV	4:2:0	Planar	8
	if ( Minor == MFVideoFormat_IYUV )
	return SoyMediaFormat::Yuv_8_44_Full;

	//	MFVideoFormat_Y41P	Y41P	4:1:1	Packed	8
	if ( Minor == MFVideoFormat_Y41P )
	return SoyMediaFormat::Yuv_822_Full;

	//	MFVideoFormat_Y41T	Y41T	4:1:1	Packed	8
	if ( Minor == MFVideoFormat_Y41P )
	return SoyMediaFormat::Yuv_822_Full;

	//	MFVideoFormat_YV12	YV12	4:2:0	Planar	8
	if ( Minor == MFVideoFormat_YV12 )
	return SoyMediaFormat::Yuv_8_44_Full;
	*/
	
	FORMAT_MAP( MFVideoFormat_MP4S,	SoyMediaFormat::Mpeg4 ),
	FORMAT_MAP( MFVideoFormat_H264,	SoyMediaFormat::H264_8 ),
	FORMAT_MAP( MFVideoFormat_H264,	SoyMediaFormat::H264_16 ),
	FORMAT_MAP( MFVideoFormat_H264,	SoyMediaFormat::H264_32 ),
	FORMAT_MAP( MFVideoFormat_H264,	SoyMediaFormat::H264_ES ),

};



namespace MediaFoundation
{
	namespace Private
	{
		std::shared_ptr<TContext> Context;
	}
}



std::shared_ptr<MediaFoundation::TContext> MediaFoundation::GetContext()
{
	if ( !Private::Context )
	{
		Private::Context.reset(new TContext() );
	}
	return Private::Context;
}

void MediaFoundation::Shutdown()
{
	//	free last context
	if ( Private::Context.unique() )
		Private::Context.reset();
}

MediaFoundation::TContext::TContext()
{
	auto Result = MFStartup( MF_VERSION, MFSTARTUP_FULL );
	Directx::IsOkay(Result, "MFStartup");
}

MediaFoundation::TContext::~TContext()
{
	auto Result = MFShutdown();
	Directx::IsOkay(Result, "MFShutdown", false );
}


SoyMediaFormat::Type MediaFoundation::GetFormat(GUID Format)
{
	auto Table = GetRemoteArray( PlatformFormatMap );
	auto* Meta = GetArrayBridge(Table).Find( Format );

	if ( !Meta )
		return SoyMediaFormat::Invalid;

	return Meta->mSoyFormat;
}

SoyPixelsFormat::Type MediaFoundation::GetPixelFormat(GUID Format)
{
	auto Table = GetRemoteArray( PlatformFormatMap );
	auto* Meta = GetArrayBridge(Table).Find( Format );

	if ( !Meta )
		return SoyPixelsFormat::Invalid;

	return SoyMediaFormat::GetPixelFormat( Meta->mSoyFormat );
}

GUID MediaFoundation::GetFormat(SoyPixelsFormat::Type Format)
{
	auto Table = GetRemoteArray( PlatformFormatMap );
	auto* Meta = GetArrayBridge(Table).Find( Format );

	if ( !Meta )
		return GUID();

	return Meta->mPlatformFormat;
}

SoyMediaFormat::Type MediaFoundation::GetFormat(GUID Major,GUID Minor,size_t H264NaluLengthSize)
{
	if (Major == MFMediaType_Audio)
	{
		if ( Minor == MFAudioFormat_AUDS )
			return SoyMediaFormat::Audio_AUDS;

		if (Minor == MFAudioFormat_AAC)
			return SoyMediaFormat::Aac;

		if (Minor == MFAudioFormat_Float)
		{
			//throw Soy::AssertException("Gr: Verify MFAudioFormat_Float is PCM");
			return SoyMediaFormat::PcmLinear_float;
		}

		if (Minor == MFAudioFormat_PCM)
		{
			auto SampleByteSize = H264NaluLengthSize;
			switch (SampleByteSize)
			{
				case 1:		return SoyMediaFormat::PcmLinear_8;
				case 2:		return SoyMediaFormat::PcmLinear_16;
				default:	break;
			};
			std::stringstream Error;
			Error << "Cannot determine PCM format with sample size=" << SampleByteSize << std::endl;
			throw Soy::AssertException(Error.str());
		}
	}

	if ( Major == MFMediaType_Video )
	{
		if ( Minor == MFVideoFormat_H264 )
		{
			switch( H264NaluLengthSize )
			{
				case 1:		return SoyMediaFormat::H264_8;
				case 2:		return SoyMediaFormat::H264_16;
				case 4:		return SoyMediaFormat::H264_32;
				default:	break;
			};
			std::stringstream Error;
			Error << "Cannot determine h264 format with nalulength=" << H264NaluLengthSize << std::endl;
			throw Soy::AssertException( Error.str() );
		}

		if ( Minor == MFVideoFormat_MPEG2 )
			return SoyMediaFormat::Mpeg2;

		if (  Minor == MFVideoFormat_H264_ES )
			return SoyMediaFormat::H264_ES;

		if ( Minor == MFStreamFormat_MPEG2Transport )
			return SoyMediaFormat::Mpeg2TS;

		//	apple sample 3gp movie comes out as this
		if ( Minor == MFVideoFormat_M4S2 )
			return SoyMediaFormat::Mpeg4;

		//	look for pixel formats
		auto PixelFormat = MediaFoundation::GetPixelFormat( Minor );
		if ( PixelFormat != SoyPixelsFormat::Invalid )
		{
			return SoyMediaFormat::FromPixelFormat( PixelFormat );
		}


		//	extract fourcc
		//	gr: on win7 with bjork, we get a fourcc of avc1, but the rest doesn't match MediaFormat Base, so this func fails, but secretly has what we want...s
		uint32 Fourcc = 0;
		GetMediaFormatGuidFourcc( Minor, Fourcc );
		{
			auto Format = SoyMediaFormat::FromFourcc( Fourcc, H264NaluLengthSize );
			if ( Format != SoyMediaFormat::Invalid )
				return Format;
		}
	}

	std::string GuidString;
	{
		std::stringstream ss;
		ss << Major << "/" << Minor;
		GuidString = ss.str();
	}

	std::stringstream Error;
	Error << "Cannot determine media format from guids " << GuidString;
	throw Soy::AssertException( Error.str() );
}


std::ostream& operator<<(std::ostream& os, REFGUID guid)
{
	//	some other special guids
	if ( guid == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID )
	{
		os << "MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID";
		return os;
	}

	if ( guid == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID )
	{
		os << "MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID";
		return os;
	}


	//	media format guids are explicit
	{
		uint32 MediaFormatFourCC = 0;
		if ( GetMediaFormatGuidFourcc( guid, MediaFormatFourCC ) )
		{
			os << GetFourCCString( MediaFormatFourCC );
			return os;
		}
	}

	{
		auto* MediaTypeName = MediaGuidSpecialToString( guid );
		if ( MediaTypeName )
		{
			os << MediaTypeName;
			return os;
		}
	}

	OLECHAR Buffer[100] = {0};
	StringFromGUID2( guid, Buffer, sizeofarray(Buffer) );
	os << Buffer;
	/*
	os << std::uppercase;
	os.width(8);
	os << std::hex << guid.Data1 << '-';

	os.width(4);
	os << std::hex << guid.Data2 << '-';

	os.width(4);
	os << std::hex << guid.Data3 << '-';

	os.width(2);
	os << std::hex
	<< static_cast<short>(guid.Data4[0])
	<< static_cast<short>(guid.Data4[1])
	<< '-'
	<< static_cast<short>(guid.Data4[2])
	<< static_cast<short>(guid.Data4[3])
	<< static_cast<short>(guid.Data4[4])
	<< static_cast<short>(guid.Data4[5])
	<< static_cast<short>(guid.Data4[6])
	<< static_cast<short>(guid.Data4[7]);
	os << std::nouppercase;
	*/
	return os;
}



