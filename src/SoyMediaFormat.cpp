#include "SoyMediaFormat.h"
#include "SoyFourcc.h"

//	android sdk define
#define MIMETYPE_AUDIO_RAW	"audio/raw"


//	media foundation fourcc's
#if !defined(WAVE_FORMAT_PCM)
#define  WAVE_FORMAT_PCM						0x0001 /* Microsoft Corporation */
#elif WAVE_FORMAT_PCM!=0x0001
#error WAVE_FORMAT_PCM define mis match
#endif
#define  WAVE_FORMAT_IEEE_FLOAT                 0x0003 /* Microsoft Corporation */
#define  WAVE_FORMAT_MPEG                       0x0050 /* Microsoft Corporation */
#define  WAVE_FORMAT_DTS                        0x0008 /* Microsoft Corporation */
#define  WAVE_FORMAT_WMAUDIO2                   0x0161 /* Microsoft Corporation */
#define  WAVE_FORMAT_WMAUDIO3                   0x0162 /* Microsoft Corporation */
#define  WAVE_FORMAT_WMAUDIO_LOSSLESS           0x0163 /* Microsoft Corporation */
#define  WAVE_FORMAT_MPEG_HEAAC                 0x1610 /* Microsoft Corporation (MPEG-2 AAC or MPEG-4 HE-AAC v1/v2 streams with any payload (ADTS, ADIF, LOAS/LATM, RAW). Format block includes MP4 AudioSpecificConfig() -- see HEAACWAVEFORMAT below */
#define  WAVE_FORMAT_DOLBY_AC3_SPDIF            0x0092 /* Sonic Foundry */
#define  WAVE_FORMAT_DRM                        0x0009 /* Microsoft Corporation */
#define  WAVE_FORMAT_WMASPDIF                   0x0164 /* Microsoft Corporation */
#define  WAVE_FORMAT_WMAVOICE9                  0x000A /* Microsoft Corporation */
#define  WAVE_FORMAT_MPEG_ADTS_AAC              0x1600 /* Microsoft Corporation */
#define  WAVE_FORMAT_AMR_WP                     0x7363 /* AMR Wideband Plus */
#define  WAVE_FORMAT_AMR_NB                     0x7361 /* AMR Narrowband */
#define  WAVE_FORMAT_AMR_WB                     0x7362 /* AMR Wideband */
#define  WAVE_FORMAT_MPEGLAYER3                 0x0055 /* ISO/MPEG Layer3 Format Tag */

//	gr: this appears in BigBuckBunny surround AC3 5.1, but doesn't match any other fourcc's...
#define FOURCC_AC3_SURROUND						0x00200000

namespace Mime
{
	const char*	Aac_Android = "audio/mp4a-latm";
	const char*	Aac_Other = "audio/aac";
	const char*	Aac_x = "audio/x-aac";
#if defined(TARGET_ANDROID)
	const char*	Aac_Default = Aac_Android;
#else
	const char*	Aac_Default = Aac_Other;
#endif
}


namespace SoyMediaMetaFlags
{
	//	copy & paste this to c#
	enum Type
	{
		None						= 0,
		IsVideo						= 1<<0,
		IsAudio						= 1<<1,
		IsH264						= 1<<2,
		IsText						= 1<<3,
		IsImage						= 1<<4,
	};
}

class SoyMediaFormatMeta
{
public:
	SoyMediaFormatMeta() :
		mFormat			( SoyMediaFormat::Invalid ),
		mFlags			( 0 ),
		mSubtypeSize	( 0 )
	{
	}	
	SoyMediaFormatMeta(SoyMediaFormat::Type Format,const std::initializer_list<std::string>& FileExtensions,const std::initializer_list<std::string>& Mimes,const std::initializer_list<uint32_t>& Fourccs,int Flags,size_t SubtypeSize) :
		mFormat			( Format ),
		mFlags			( Flags ),
		mSubtypeSize	( SubtypeSize )
	{
		for ( auto FileExtension : FileExtensions )
			mFileExtensions.PushBack( FileExtension );
		for ( auto Fourcc : Fourccs )
			mFourccs.PushBack( Fourcc );
		for ( auto Mime : Mimes )
			mMimes.PushBack( Mime );
	}
	SoyMediaFormatMeta(SoyMediaFormat::Type Format,const std::initializer_list<std::string>& FileExtensions,const std::initializer_list<std::string>& Mimes,const uint32_t& Fourcc,int Flags,size_t SubtypeSize) :
		mFormat			( Format ),
		mFlags			( Flags ),
		mSubtypeSize	( SubtypeSize )
	{
		for ( auto FileExtension : FileExtensions )
			mFileExtensions.PushBack( FileExtension );
		mFourccs.PushBack( Fourcc );
		for ( auto Mime : Mimes )
			mMimes.PushBack( Mime );
	}
	SoyMediaFormatMeta(SoyMediaFormat::Type Format,const std::initializer_list<std::string>& FileExtensions,const std::string& Mime,const std::initializer_list<uint32_t>& Fourccs,int Flags,size_t SubtypeSize) :
		mFormat			( Format ),
		mFlags			( Flags ),
		mSubtypeSize	( SubtypeSize )
	{
		for ( auto FileExtension : FileExtensions )
			mFileExtensions.PushBack( FileExtension );
		for ( auto Fourcc : Fourccs )
			mFourccs.PushBack( Fourcc );
		mMimes.PushBack( Mime );
	}
	SoyMediaFormatMeta(SoyMediaFormat::Type Format,const std::initializer_list<std::string>& FileExtensions,const std::string& Mime,const uint32_t& Fourcc,int Flags,size_t SubtypeSize) :
		mFormat			( Format ),
		mFlags			( Flags ),
		mSubtypeSize	( SubtypeSize )
	{
		for ( auto FileExtension : FileExtensions )
			mFileExtensions.PushBack( FileExtension );
		mFourccs.PushBack( Fourcc );
		mMimes.PushBack( Mime );
	}
	
	SoyMediaFormat::Type		mFormat;
	BufferArray<uint32_t,15>	mFourccs;
	BufferArray<std::string,5>	mMimes;
	BufferArray<std::string,5>	mFileExtensions;
	uint32_t					mFlags;
	size_t						mSubtypeSize;	//	zero is non-specific
	
	bool					Is(SoyMediaMetaFlags::Type Flag) const		{	return bool_cast( mFlags & Flag );	}
	
	bool					operator==(const SoyMediaFormat::Type Format) const	{	return mFormat == Format;	}
};



namespace SoyMediaFormat
{
	const Array<SoyMediaFormatMeta>&	GetFormatMap();
	void								GetFormatMetas(ArrayBridge<const SoyMediaFormatMeta*>&& MetaMatches,uint32_t Fourcc,size_t Size);
	const SoyMediaFormatMeta&			GetFormatMeta(SoyMediaFormat::Type Format);
	const SoyMediaFormatMeta&			GetFormatMetaFromMime(const std::string& Mime);
	const SoyMediaFormatMeta&			GetFormatMetaFromExtension(const std::string& Extension);
}


const Array<SoyMediaFormatMeta>& SoyMediaFormat::GetFormatMap()
{
	static SoyMediaFormatMeta _FormatMap[] =
	{
		SoyMediaFormatMeta(),
		
		SoyMediaFormatMeta( SoyMediaFormat::H264_8,			{"h264"},	"video/avc",	'avc1', SoyMediaMetaFlags::IsVideo|SoyMediaMetaFlags::IsH264, 1 ),
		SoyMediaFormatMeta( SoyMediaFormat::H264_16,		{"h264"},	"video/avc",	'avc1', SoyMediaMetaFlags::IsVideo|SoyMediaMetaFlags::IsH264, 2 ),

		//	win7 mediafoundation gives H264 with unknown size, so we assume 32
		//	gr^^ this should change to ES and re-resolve if not annex-b
		SoyMediaFormatMeta( SoyMediaFormat::H264_32,		{"h264"},	"video/avc",	{'avc1','H264'}, SoyMediaMetaFlags::IsVideo|SoyMediaMetaFlags::IsH264, 4 ),
		SoyMediaFormatMeta( SoyMediaFormat::H264_ES,		{"h264"},	"video/avc",	'avc1', SoyMediaMetaFlags::IsVideo|SoyMediaMetaFlags::IsH264, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::H264_PPS_ES,	{"h264"},	"video/avc",	'avc1', SoyMediaMetaFlags::IsVideo|SoyMediaMetaFlags::IsH264, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::H264_SPS_ES,	{"h264"},	"video/avc",	'avc1', SoyMediaMetaFlags::IsVideo|SoyMediaMetaFlags::IsH264, 0 ),

		SoyMediaFormatMeta( SoyMediaFormat::H265,			{"h265","hevc"},	"video/hevc",	{'HEVC','HEVS'}, SoyMediaMetaFlags::IsVideo, 0 ),

		SoyMediaFormatMeta( SoyMediaFormat::Mpeg2TS,		{"ts"},		"video/ts",		'xxxx', SoyMediaMetaFlags::IsVideo, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Mpeg2TS_PSI,	{"ts"},		"video/ts",		'xxxx', SoyMediaMetaFlags::IsVideo, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Mpeg2,			{"mp2"},	"video/mpeg2",	'xxxx', SoyMediaMetaFlags::IsVideo, 0 ),
		
		//	windows media foundation has this fourcc in caps (all?)
		//	FMP4 is FFmpeg-mp4
		SoyMediaFormatMeta( SoyMediaFormat::Mpeg4,			{"mp4"},	"video/mp4",		{'mp4v','MP4V','FMP4'}, SoyMediaMetaFlags::IsVideo, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Mpeg4_v3,		{},			"video/mp43",		'MP43', SoyMediaMetaFlags::IsVideo, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::VC1,			{},			"video/xxx",		'xxxx', SoyMediaMetaFlags::IsVideo, 0 ),
		
		//	verify mime
		SoyMediaFormatMeta( SoyMediaFormat::Divx,			{"divx"},	"video/divx",		'divx', SoyMediaMetaFlags::IsVideo, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::MotionJpeg,		{"mjpg","mjpeg"},	"video/mjpg",		'MJPG', SoyMediaMetaFlags::IsVideo, 0 ),

		
		SoyMediaFormatMeta( SoyMediaFormat::Wave,			{"wav"},	"audio/wave",		'xxxx', SoyMediaMetaFlags::IsAudio, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Audio_AUDS,		{"auds"},	"audio/Audio_AUDS",	'xxxx', SoyMediaMetaFlags::IsAudio, 0 ),
		
		//	verify mime
		SoyMediaFormatMeta( SoyMediaFormat::Ac3,			{"ac3"},	"audio/ac3",	'xxxx', SoyMediaMetaFlags::IsAudio, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Mpeg2Audio,		{"mp2"},	"audio/mpeg",	WAVE_FORMAT_MPEG, SoyMediaMetaFlags::IsAudio, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Dts,			{"dts"},	"audio/dts",	WAVE_FORMAT_DTS, SoyMediaMetaFlags::IsAudio, 0 ),

		//	try and encompass all formats that we don't need to specifically handle and can throw around
		SoyMediaFormatMeta( SoyMediaFormat::Audio_Platform,	{},	"audio/xxx",	{FOURCC_AC3_SURROUND,WAVE_FORMAT_DOLBY_AC3_SPDIF,WAVE_FORMAT_DRM,WAVE_FORMAT_WMAUDIO2,WAVE_FORMAT_WMAUDIO3,WAVE_FORMAT_WMAUDIO_LOSSLESS,WAVE_FORMAT_WMASPDIF,WAVE_FORMAT_WMAVOICE9,WAVE_FORMAT_MPEG_ADTS_AAC,WAVE_FORMAT_AMR_NB,WAVE_FORMAT_AMR_WB,WAVE_FORMAT_AMR_WP}, SoyMediaMetaFlags::IsAudio, 0 ),

		//	gr: change this to handle multiple mime types per format
		SoyMediaFormatMeta( SoyMediaFormat::Aac,			{"aac"},	{ Mime::Aac_Default, Mime::Aac_Android, Mime::Aac_x, Mime::Aac_Other},	{'aac ',WAVE_FORMAT_MPEG_HEAAC}, SoyMediaMetaFlags::IsAudio, 0 ),

		//	https://en.wikipedia.org/wiki/Pulse-code_modulation
		SoyMediaFormatMeta( SoyMediaFormat::PcmLinear_8,	{},	"audio/L8",		{'lpcm',WAVE_FORMAT_PCM}, SoyMediaMetaFlags::IsAudio, 8  ),
		SoyMediaFormatMeta( SoyMediaFormat::PcmLinear_16,	{},	"audio/L16",	{'lpcm',WAVE_FORMAT_PCM}, SoyMediaMetaFlags::IsAudio, 16  ),
		SoyMediaFormatMeta( SoyMediaFormat::PcmLinear_20,	{},	"audio/L20",	{'lpcm',WAVE_FORMAT_PCM}, SoyMediaMetaFlags::IsAudio, 20  ),
		SoyMediaFormatMeta( SoyMediaFormat::PcmLinear_24,	{},	"audio/L24",	{'lpcm',WAVE_FORMAT_PCM}, SoyMediaMetaFlags::IsAudio, 24  ),
		SoyMediaFormatMeta( SoyMediaFormat::PcmAndroidRaw,	{},	MIMETYPE_AUDIO_RAW,	{'lpcm',WAVE_FORMAT_PCM}, SoyMediaMetaFlags::IsAudio, 0 ),

		//	find mime
		SoyMediaFormatMeta( SoyMediaFormat::PcmLinear_float,	{},	"audio/L32",	WAVE_FORMAT_IEEE_FLOAT, SoyMediaMetaFlags::IsAudio, 0 ),

		//	audio/mpeg is what android reports when I try and open mp3
		SoyMediaFormatMeta( SoyMediaFormat::Mp3,			{"mp3"},	"audio/mpeg",	WAVE_FORMAT_MPEGLAYER3, SoyMediaMetaFlags::IsAudio, 0 ),
		
		//	verify these mimes
		SoyMediaFormatMeta( SoyMediaFormat::Png,			{"png"},	"image/png",	'xxxx', SoyMediaMetaFlags::IsImage, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Jpeg,			{"jpg","jpeg"},	"image/jpeg",	'xxxx', SoyMediaMetaFlags::IsImage, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Bmp,			{"bmp"},	"image/bmp",	'xxxx', SoyMediaMetaFlags::IsImage, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Tga,			{"tga"},	"image/tga",	'xxxx', SoyMediaMetaFlags::IsImage, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Psd,			{"psd"},	"image/Psd",	'xxxx', SoyMediaMetaFlags::IsImage, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Gif,			{"gif"},	"image/gif",	'xxxx', SoyMediaMetaFlags::IsImage, 0 ),

		
		SoyMediaFormatMeta( SoyMediaFormat::Text,			{"txt"},		"text/plain",	'xxxx', SoyMediaMetaFlags::IsText, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Json,			{"js","json"},	"application/javascript",	'xxxx', SoyMediaMetaFlags::IsText, 0 ),
		SoyMediaFormatMeta(SoyMediaFormat::Html,			{"htm","html","xhtml"},	"text/html",	'xxxx', SoyMediaMetaFlags::IsText, 0),
		SoyMediaFormatMeta(SoyMediaFormat::Css,				{"css"},	"text/css",	'xxxx', SoyMediaMetaFlags::IsText, 0),
		SoyMediaFormatMeta( SoyMediaFormat::ClosedCaption,	{},		"text/plain",	'xxxx', SoyMediaMetaFlags::IsText, 0 ),
		SoyMediaFormatMeta(SoyMediaFormat::Subtitle,		{"srt"},	"text/plain",	'xxxx', SoyMediaMetaFlags::IsText, 0),
		SoyMediaFormatMeta(SoyMediaFormat::Svg,				{"svg"},	"image/svg+xml",	'xxxx', SoyMediaMetaFlags::IsText, 0),

		SoyMediaFormatMeta( SoyMediaFormat::QuicktimeTimecode,	{"qt","mov"},	"application/quicktimetimecode",	'tmcd', SoyMediaMetaFlags::None, 0 ),

		//	pixel formats
		//	gr: these fourcc's are from mediafoundation
		SoyMediaFormatMeta( SoyMediaFormat::Greyscale,			{},	"application/Greyscale",		'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::GreyscaleAlpha,		{},	"application/GreyscaleAlpha",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::RGB,				{},	"application/RGB",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::RGBA,				{},	"application/RGBA",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::BGRA,				{},	"application/BGRA",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::BGR,				{},	"application/BGR",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::ARGB,				{},	"application/ARGB",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::KinectDepth,		{},	"application/KinectDepth",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::FreenectDepth10bit,	{},	"application/FreenectDepth10bit",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::FreenectDepth11bit,	{},	"application/FreenectDepth11bit",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Depth16mm		,	{},	"application/Depth16mm",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Luma,			{},	"application/Luma",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Yuv_8_88,		{},	"application/Yuv_8_88",	{'NV12','VIDS'}, SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Yuv_8_8_8,		{},	"application/Yuv_8_8_8",	'I420', SoyMediaMetaFlags::None, 0 ),
		//	gr: 'yuv2' lowercase is fourcc for 420 non planar on OSX
		//		YUV2 uppercase is fourcc for 422 on windows (I think)
		SoyMediaFormatMeta( SoyMediaFormat::YYuv_8888,		{},	"application/YYuv_8888",	{'YUY2','IYUV','Y42T','UYVY'}, SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Uvy_8_88,		{},	"application/Uvy_844",		{'2uvy'}, SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Yuv_8_88,		{},	"application/Yuv_844_Video",	{'yuv2','yuvs'}, SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Yuv_8_88,		{},	"application/Yuv_844",		{'yuv2','yuvs'}, SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::ChromaUV_8_8,		{},	"application/ChromaUV_8_8",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::ChromaUV_88,		{},	"application/ChromaUV_88",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::ChromaU_8,			{},	"application/ChromaU_8",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::ChromaV_8,			{},	"application/ChromaV_8",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Palettised_RGB_8,	{},	"application/Palettised_RGB_8",	'xxxx', SoyMediaMetaFlags::None, 0 ),
		SoyMediaFormatMeta( SoyMediaFormat::Palettised_RGBA_8,	{},	"application/Palettised_RGBA_8",	'xxxx', SoyMediaMetaFlags::None, 0 ),
	};

	static Array<SoyMediaFormatMeta> FormatMap( _FormatMap );
	return FormatMap;
}

const SoyMediaFormatMeta& SoyMediaFormat::GetFormatMeta(SoyMediaFormat::Type Format)
{
	auto& Metas = GetFormatMap();
	auto* Meta = Metas.Find( Format );
	if ( !Meta )
	{
		std::stringstream Error;
		Error << "Missing meta for " << Format;
		throw Soy::AssertException( Error.str() );
	}
	return *Meta;
}


const SoyMediaFormatMeta& SoyMediaFormat::GetFormatMetaFromMime(const std::string& Mime)
{
	auto& Metas = GetFormatMap();
	for ( int m=0;	m<Metas.GetSize();	m++ )
	{
		auto& Meta = Metas[m];
		
		if ( !Meta.mMimes.Find( Mime ) )
			continue;
		
		return Meta;
	}
	
	std::stringstream Error;
	Error << "No formats found matching mime " << Mime;
	throw Soy::AssertException( Error.str() );
}


const SoyMediaFormatMeta& SoyMediaFormat::GetFormatMetaFromExtension(const std::string& Extension)
{
	auto& Metas = GetFormatMap();
	for ( int m=0;	m<Metas.GetSize();	m++ )
	{
		auto& Meta = Metas[m];

		bool Match = false;
		for ( int e=0;	!Match && e<Meta.mFileExtensions.GetSize();	e++ )
		{
			auto FormatExt = Meta.mFileExtensions[e];
			if ( !Soy::StringMatches(FormatExt, Extension, false) )
				continue;
			Match = true;
		}
		if ( !Match )
			continue;
		
		return Meta;
	}
	
	std::stringstream Error;
	Error << "No formats found matching extension " << Extension;
	throw Soy::AssertException( Error.str() );
}

void SoyMediaFormat::GetFormatMetas(ArrayBridge<const SoyMediaFormatMeta*>&& MetaMatches,uint32_t Fourcc,size_t Size)
{
	auto& Metas = GetFormatMap();
	
	auto FourccSwapped = Fourcc;
	Soy::EndianSwap( FourccSwapped );
	
	for ( int m=0;	m<Metas.GetSize();	m++ )
	{
		auto& Meta = Metas[m];
		if ( !Meta.mFourccs.Find( Fourcc ) )
		{
			if ( !Meta.mFourccs.Find( FourccSwapped ) )
				continue;
			else
				std::Debug << "Warning: Detected reversed fourcc.(" << Soy::TFourcc(FourccSwapped) << ") todo: Fix endianness at source." << std::endl;
		}
	
		//	match size
		if ( Meta.mSubtypeSize != Size )
			continue;

		MetaMatches.PushBack( &Meta );
	}
}


SoyPixelsFormat::Type SoyMediaFormat::GetPixelFormat(SoyMediaFormat::Type MediaFormat)
{
	if ( MediaFormat == SoyMediaFormat::Invalid )
		return SoyPixelsFormat::Invalid;
	
	if ( MediaFormat >= SoyMediaFormat::NotPixels )
		return SoyPixelsFormat::Invalid;
	
	return static_cast<SoyPixelsFormat::Type>( MediaFormat );
}


SoyMediaFormat::Type SoyMediaFormat::FromPixelFormat(SoyPixelsFormat::Type PixelFormat)
{
	return static_cast<SoyMediaFormat::Type>( PixelFormat );
}



bool SoyMediaFormat::IsVideo(SoyMediaFormat::Type Format)
{
	if ( IsPixels(Format) )
		return true;
	if ( IsH264(Format) )
		return true;
	if ( IsImage(Format) )
		return true;
	
	auto& Meta = GetFormatMeta( Format );
	return Meta.Is( SoyMediaMetaFlags::IsVideo );
}

bool SoyMediaFormat::IsImage(SoyMediaFormat::Type Format)
{
	auto& Meta = GetFormatMeta( Format );
	return Meta.Is( SoyMediaMetaFlags::IsImage );
}

bool SoyMediaFormat::IsH264(SoyMediaFormat::Type Format)
{
	auto& Meta = GetFormatMeta( Format );
	return Meta.Is( SoyMediaMetaFlags::IsH264 );
}

bool SoyMediaFormat::IsH264(Soy::TFourcc Fourcc)
{
	if ( Fourcc == "avcc" )	return true;
	if ( Fourcc == "avc1" )	return true;
	if ( Fourcc == "1cva" )	return true;

	return false;
}

bool SoyMediaFormat::IsAudio(SoyMediaFormat::Type Format)
{
	auto& Meta = GetFormatMeta( Format );
	return Meta.Is( SoyMediaMetaFlags::IsAudio );
}


bool SoyMediaFormat::IsText(SoyMediaFormat::Type Format)
{
	auto& Meta = GetFormatMeta( Format );
	return Meta.Is( SoyMediaMetaFlags::IsText );

}

std::string SoyMediaFormat::ToMime(SoyMediaFormat::Type Format)
{
	auto& Meta = GetFormatMeta( Format );
	return Meta.mMimes[0];
	
}

SoyMediaFormat::Type SoyMediaFormat::FromMime(const std::string& Mime)
{
	auto& Meta = GetFormatMetaFromMime( Mime );
	return Meta.mFormat;
}

SoyMediaFormat::Type SoyMediaFormat::FromExtension(const std::string& Extension)
{
	auto& Meta = GetFormatMetaFromExtension( Extension );
	return Meta.mFormat;
}

uint32 SoyMediaFormat::ToFourcc(SoyMediaFormat::Type Format)
{
	auto& Meta = GetFormatMeta( Format );
	return Meta.mFourccs[0];
}


SoyMediaFormat::Type SoyMediaFormat::FromFourcc(uint32 Fourcc,size_t H264LengthSize)
{
	BufferArray<const SoyMediaFormatMeta*,10> Metas;
	GetFormatMetas( GetArrayBridge(Metas), Fourcc, H264LengthSize );
	
	if ( Metas.IsEmpty() )
	{
		std::Debug << "Unknown fourcc type: " << Soy::TFourcc(Fourcc) << " (" << H264LengthSize << ")" << std::endl;
		return SoyMediaFormat::Invalid;
	}

	//	multiple found
	if ( Metas.GetSize() > 1 )
		std::Debug << "Warning found multiple metas for fourcc " << Soy::TFourcc(Fourcc) << " returning first" << std::endl;

	return Metas[0]->mFormat;

}
