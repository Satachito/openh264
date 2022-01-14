#include	<unistd.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<fcntl.h>

#include	<iostream>
#include	<string>
#include	<vector>
#include	<map>
#include	<stack>
#include	<numeric>
#include	<ctime>
#include	<sstream>
#include	<random>
#include	<algorithm>

using namespace std;

#include	<filesystem>
using namespace filesystem;

typedef	unsigned char	UI1;
typedef	unsigned short	UI2;
typedef	unsigned int	UI4;
typedef	unsigned long	UI8;

//	UNIX UNDER ZERO ERROR
template	< typename I >	I
_X( I $, const string& _, const string& file, int line ) {
	if ( $ < 0 ) {
		cerr << file + ':' + to_string( line ) + ':' + strerror( errno ) + ':' + _ << endl;
#ifdef DEBUG
		__builtin_trap();
#endif
		throw file + ':' + to_string( line ) + ':' + strerror( errno ) + ':' + _;
	}
	return $;
}
#define X( $ ) _X( $, #$, __FILE__, __LINE__ )

//	ASSERTION
inline void
_A( bool $, const string& _, const string& file, int line ) {
	if ( !$ ) {
		cerr << file + ':' + to_string( line ) + ':' + _ << endl;
#ifdef DEBUG
		__builtin_trap();
#endif
		throw file + ':' + to_string( line ) + ':' + _;
	}
}
#define A( $ ) _A( $, #$, __FILE__, __LINE__ )

//	NULL EXCEPTION
template	< typename T >	T*
_N( T* $, const string& _, const string& file, int line ) {
	if ( ! $ ) {
		cerr << file + ':' + to_string( line ) + ':' + _ << endl;
#ifdef DEBUG
		__builtin_trap();
#endif
		throw file + ':' + to_string( line ) + ':' + _;
	}
	return $;
}
#define	N( $ ) _N( $, #$, __FILE__, __LINE__ )

namespace JP {

	typedef	int	UNICODE;

	inline	UNICODE
	Unicode( istream& p ) {
		unsigned char	c;
		p >> c;
		A( !p.eof() );
		if ( c < 0x80 ) return c;

		c <<= 1;
		auto	wNumCont = 0;
		while ( c & 0x80 ) {
			wNumCont++;
			c <<= 1;
		}
		UNICODE	v = c >> ( wNumCont + 1 );

		while ( wNumCont-- ) {
			p >> c;
			A( !p.eof() );
			v = ( v << 6 ) | ( c & 0x3f );
		}
		
		return v;
	}
	struct
	UnicodeStream {
		UNICODE	buffer = 0;
		bool	Unreaded = false;
		void
		Unread( UNICODE p ) {
			buffer = p;
			Unreaded = true;
		}
		UNICODE
		Read() {
			if ( Unreaded ) {
				Unreaded = false;
				return buffer;
			}
			return Unicode( cin );	//	UC
		}
	};
	struct
	PreProcessor: UnicodeStream {
		UNICODE
		Read() {
		RESET:
			auto v = UnicodeStream::Read();
			if ( v == '/' ) {
				auto w = UnicodeStream::Read();
				if ( w == '/' ) {
					while ( UnicodeStream::Read() != '\n' ) {}
					goto RESET;	//	return Read();
				} else {
					Unread( w );
					return v;
				}
			}
			return v;
		}
	};
	
	template	< typename I >	I
	UniformRandomInt( I l = 0, I h = 1 ) {
		static	std::mt19937_64 sMT( (std::random_device())() );
		return	std::uniform_int_distribution< I >( l, h - 1 )( sMT );
	}

	inline	UI2
	Swap2( UI2 $ ) {
		return $ >> 8 | $ << 8;
	}
	inline	UI4
	Swap4( UI4 $ ) {
		return $ >> 24 | ( $ >> 8 & 0x0000ff00 ) | ( $ << 8 & 0x00ff0000 ) | $ << 24;
	}
	inline	UI8
	Swap8( UI8 $ ) {
		return
			$ >> 56
		|	( $ >> 40 & 0x000000000000ff00L )
		|	( $ >> 24 & 0x0000000000ff0000L )
		|	( $ >>	8 & 0x00000000ff000000L )
		|	( $ <<	8 & 0x000000ff00000000L )
		|	( $ << 24 & 0x0000ff0000000000L )
		|	( $ << 40 & 0x00ff000000000000L )
		|	$ << 56
		;
	}

	inline vector< UI1 >
	Read( int fd ) {
		vector< UI1 >	$;
		char	buffer[ 1024 * 1024 ];
		ssize_t	numRead;
		while ( ( numRead = X( read( fd, buffer, sizeof( buffer ) ) ) ) ) $.insert( $.end(), buffer, buffer + numRead );
		return $;
	}

	inline vector< UI1 >
	In() {
		return Read( 0 );
	}

	inline void
	Write( int fd, const void* $, UI8 _ ) {
		A( write( fd, $, _ ) == _ );
	}

	inline void
	Write( int fd, const vector< UI1 >& $ ) {
		Write( fd, &$[ 0 ], $.size() );
	}

	inline void
	Out( const void* $, UI8 _ ) {
		Write( 1, $, _ );
	}

	inline void
	Out( const vector< UI1 >& $ ) {
		Write( 1, &$[ 0 ], $.size() );
	}

	inline void
	Err( const void* $, UI8 _ ) {
		Write( 2, $, _ );
	}

	inline void
	Err( const vector< UI1 >& $ ) {
		Write( 2, &$[ 0 ], $.size() );
	}

	inline UI8
	FileSize( const string& path ) {
		struct stat $;
		X( stat( path.c_str(), &$ ) );
		return $.st_size;
	}

	inline vector< UI1 >
	GetFileContent( const string& path ) {
		vector< UI1 >	$( FileSize( path ) );
		int fd = open( path.c_str(), O_RDONLY );
		A( fd > 2 );
		A( read( fd, &$[ 0 ], $.size() ) == $.size() );
		close( fd );
		return $;
	}

	inline void
	SetFileContent( const string& path, const UI1* $, UI8 _ ) {
		int fd = creat( path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
		A( write( fd, $, _ ) == _ );
		close( fd );
	}

	inline void
	SetFileContent( const string& path, const vector< UI1 >& $ ) {
		SetFileContent( path, &$[ 0 ], $.size() );
	}

	inline vector< string >
	GetLines( istream& $ = cin ) {
		vector< string >	_;
		string				s;
		while ( !$.eof() ) {
			getline( $, s );
			if ( s.size() ) _.emplace_back( s );
		}
		return _;
	}
}
using namespace JP;


#include "codec_def.h"
#include "codec_app_def.h"
#include "codec_api.h"
#include "typedefs.h"
#include "measure_time.h"
#include "d3d9_utils.h"


int32_t readBit (uint8_t* pBufPtr, int32_t& curBit) {
	int nIndex = curBit / 8;
	int nOffset = curBit % 8 + 1;

	curBit++;
	return (pBufPtr[nIndex] >> (8 - nOffset)) & 0x01;
}

int32_t readBits (uint8_t* pBufPtr, int32_t& n, int32_t& curBit) {
	int r = 0;
	int i;
	for (i = 0; i < n; i++) {
		r |= (readBit (pBufPtr, curBit) << (n - i - 1));
	}
	return r;
}

int32_t bsGetUe (uint8_t* pBufPtr, int32_t& curBit) {
	int r = 0;
	int i = 0;
	while ((readBit (pBufPtr, curBit) == 0) && (i < 32)) {
		i++;
	}
	r = readBits (pBufPtr, i, curBit);
	r += (1 << i) - 1;
	return r;
}

int32_t readFirstMbInSlice (uint8_t* pSliceNalPtr) {
	int32_t curBit = 0;
	int32_t firstMBInSlice = bsGetUe (pSliceNalPtr + 1, curBit);
	return firstMBInSlice;
}

int32_t readPicture (uint8_t* pBuf, const int32_t& iFileSize, const int32_t& bufPos, uint8_t*& pSpsBuf,
										int32_t& sps_byte_count) {
	int32_t bytes_available = iFileSize - bufPos;
	if (bytes_available < 4) {
		return bytes_available;
	}
	uint8_t* ptr = pBuf + bufPos;
	int32_t read_bytes = 0;
	int32_t sps_count = 0;
	int32_t pps_count = 0;
	int32_t non_idr_pict_count = 0;
	int32_t idr_pict_count = 0;
	int32_t nal_deliminator = 0;
	pSpsBuf = NULL;
	sps_byte_count = 0;
	while (read_bytes < bytes_available - 4) {
		bool has4ByteStartCode = ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 0 && ptr[3] == 1;
		bool has3ByteStartCode = false;
		if (!has4ByteStartCode) {
			has3ByteStartCode = ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 1;
		}
		if (has4ByteStartCode || has3ByteStartCode) {
			int32_t byteOffset = has4ByteStartCode ? 4 : 3;
			uint8_t nal_unit_type = has4ByteStartCode ? (ptr[4] & 0x1F) : (ptr[3] & 0x1F);
			if (nal_unit_type == 1) {
				int32_t firstMBInSlice = readFirstMbInSlice (ptr + byteOffset);
				if (++non_idr_pict_count >= 1 && idr_pict_count >= 1 && firstMBInSlice == 0) {
					return read_bytes;
				}
				if (non_idr_pict_count >= 2 && firstMBInSlice == 0) {
					return read_bytes;
				}
			} else if (nal_unit_type == 5) {
				int32_t firstMBInSlice = readFirstMbInSlice (ptr + byteOffset);
				if (++idr_pict_count >= 1 && non_idr_pict_count >= 1 && firstMBInSlice == 0) {
					return read_bytes;
				}
				if (idr_pict_count >= 2 && firstMBInSlice == 0) {
					return read_bytes;
				}
			} else if (nal_unit_type == 7) {
				pSpsBuf = ptr + (has4ByteStartCode ? 4 : 3);
				if ((++sps_count >= 1) && (non_idr_pict_count >= 1 || idr_pict_count >= 1)) {
					return read_bytes;
				}
				if (sps_count == 2) {
					return read_bytes;
				}
			} else if (nal_unit_type == 8) {
				if (++pps_count == 1 && sps_count == 1) {
					sps_byte_count = int32_t (ptr - pSpsBuf);
				}
				if (pps_count >= 1 && (non_idr_pict_count >= 1 || idr_pict_count >= 1)) {
					return read_bytes;
				}
			} else if (nal_unit_type == 9) {
				if (++nal_deliminator == 2) {
					return read_bytes;
				}
			}
			if (read_bytes >= bytes_available - 4) {
				return bytes_available;
			}
			read_bytes += 4;
			ptr += 4;
		} else {
			++ptr;
			++read_bytes;
		}
	}
	return bytes_available;
}

void
Write2File (FILE* pFp, unsigned char* pData[3], int iStride[2], int iWidth, int iHeight) {
  int   i;
  unsigned char*  pPtr = NULL;

  pPtr = pData[0];
  for (i = 0; i < iHeight; i++) {
    fwrite (pPtr, 1, iWidth, pFp);
    pPtr += iStride[0];
  }

  iHeight = iHeight / 2;
  iWidth = iWidth / 2;
  pPtr = pData[1];
  for (i = 0; i < iHeight; i++) {
    fwrite (pPtr, 1, iWidth, pFp);
    pPtr += iStride[1];
  }

  pPtr = pData[2];
  for (i = 0; i < iHeight; i++) {
    fwrite (pPtr, 1, iWidth, pFp);
    pPtr += iStride[1];
  }
}

void
Process( void* pDst[3], SBufferInfo* pInfo, FILE* pFp ) {

  int iStride[2];
  int iWidth = pInfo->UsrData.sSystemBuffer.iWidth;
  int iHeight = pInfo->UsrData.sSystemBuffer.iHeight;
  iStride[0] = pInfo->UsrData.sSystemBuffer.iStride[0];
  iStride[1] = pInfo->UsrData.sSystemBuffer.iStride[1];

  Write2File (pFp, (unsigned char**)pDst, iStride, iWidth, iHeight);
}

void
FlushFrames(
	ISVCDecoder* pDecoder
,	int64_t& iTotal
,	FILE* pYuvFile
,	int32_t& iFrameCount
,	unsigned long long& uiTimeStamp
,	int32_t& iWidth
,	int32_t& iHeight
,	int32_t& iLastWidth
,	int32_t iLastHeight
) {
	uint8_t* pData[3] = { NULL };
	uint8_t* pDst[3] = { NULL };
	SBufferInfo sDstBufInfo;
	int32_t num_of_frames_in_buffer = 0;
	pDecoder->GetOption (DECODER_OPTION_NUM_OF_FRAMES_REMAINING_IN_BUFFER, &num_of_frames_in_buffer);
	for (int32_t i = 0; i < num_of_frames_in_buffer; ++i) {
		int64_t iStart = WelsTime();
		pData[0] = NULL;
		pData[1] = NULL;
		pData[2] = NULL;
		memset (&sDstBufInfo, 0, sizeof (SBufferInfo));
		sDstBufInfo.uiInBsTimeStamp = uiTimeStamp;
		pDecoder->FlushFrame (pData, &sDstBufInfo);
		if (sDstBufInfo.iBufferStatus == 1) {
			pDst[0] = sDstBufInfo.pDst[0];
			pDst[1] = sDstBufInfo.pDst[1];
			pDst[2] = sDstBufInfo.pDst[2];
		}
		int64_t iEnd = WelsTime();
		iTotal += iEnd - iStart;
		if (sDstBufInfo.iBufferStatus == 1) {
			Process ((void**)pDst, &sDstBufInfo, pYuvFile);
			iWidth = sDstBufInfo.UsrData.sSystemBuffer.iWidth;
			iHeight = sDstBufInfo.UsrData.sSystemBuffer.iHeight;
			++iFrameCount;
		}
	}
}
void
H264DecodeInstance(
	ISVCDecoder* pDecoder
,	const char* kpH264FileName
,	const char* kpOuputFileName
,	int32_t& iWidth
,	int32_t& iHeight
,	int32_t iErrorConMethod
) {
	FILE* pH264File	= NULL;
	FILE* pYuvFile		= NULL;

	unsigned long long uiTimeStamp = 0;
	int64_t iStart = 0, iEnd = 0, iTotal = 0;
	int32_t iSliceSize;
	int32_t iSliceIndex = 0;
	uint8_t* pBuf = NULL;
	uint8_t uiStartCode[4] = {0, 0, 0, 1};

	uint8_t* pData[3] = {NULL};
	uint8_t* pDst[3] = {NULL};
	SBufferInfo sDstBufInfo;

	int32_t iBufPos = 0;
	int32_t iFileSize;
	int32_t iLastWidth = 0, iLastHeight = 0;
	int32_t iFrameCount = 0;
	int32_t iEndOfStreamFlag = 0;
	pDecoder->SetOption (DECODER_OPTION_ERROR_CON_IDC, &iErrorConMethod);
	double dElapsed = 0;
	uint8_t uLastSpsBuf[32];
	int32_t iLastSpsByteCount = 0;

	int32_t iThreadCount = 1;
	pDecoder->GetOption (DECODER_OPTION_NUM_OF_THREADS, &iThreadCount);

	if (kpH264FileName) {
		pH264File = fopen (kpH264FileName, "rb");
		if (pH264File == NULL) {
			fprintf (stderr, "Can not open h264 source file, check its legal path related please..\n");
			return;
		}
		fprintf (stderr, "H264 source file name: %s..\n", kpH264FileName);
	} else {
		fprintf (stderr, "Can not find any h264 bitstream file to read..\n");
		fprintf (stderr, "----------------decoder return------------------------\n");
		return;
	}

	if (kpOuputFileName) {
		pYuvFile = fopen (kpOuputFileName, "wb");
		if (pYuvFile == NULL) {
			fprintf (stderr, "Can not open yuv file to output result of decoding..\n");
			// any options
			//return; // can let decoder work in quiet mode, no writing any output
		} else
			fprintf (stderr, "Sequence output file name: %s..\n", kpOuputFileName);
	} else {
		fprintf (stderr, "Can not find any output file to write..\n");
		// any options
	}

	printf ("------------------------------------------------------\n");

	fseek (pH264File, 0L, SEEK_END);
	iFileSize = (int32_t) ftell (pH264File);
	if (iFileSize <= 4) {
		fprintf (stderr, "Current Bit Stream File is too small, read error!!!!\n");
		goto label_exit;
	}
	fseek (pH264File, 0L, SEEK_SET);

	pBuf = new uint8_t[iFileSize + 4];
	if (pBuf == NULL) {
		fprintf (stderr, "new buffer failed!\n");
		goto label_exit;
	}

	if (fread (pBuf, 1, iFileSize, pH264File) != (uint32_t)iFileSize) {
		fprintf (stderr, "Unable to read whole file\n");
		goto label_exit;
	}

	memcpy (pBuf + iFileSize, &uiStartCode[0], 4); //confirmed_safe_unsafe_usage

	while (true) {

		if (iBufPos >= iFileSize) {
			iEndOfStreamFlag = true;
			if (iEndOfStreamFlag)
				pDecoder->SetOption (DECODER_OPTION_END_OF_STREAM, (void*)&iEndOfStreamFlag);
			break;
		}
// Read length from file if needed
		if (iThreadCount >= 1) {
			uint8_t* uSpsPtr = NULL;
			int32_t iSpsByteCount = 0;
			iSliceSize = readPicture (pBuf, iFileSize, iBufPos, uSpsPtr, iSpsByteCount);
			if (iLastSpsByteCount > 0 && iSpsByteCount > 0) {
				if (iSpsByteCount != iLastSpsByteCount || memcmp (uSpsPtr, uLastSpsBuf, iLastSpsByteCount) != 0) {
					//whenever new sequence is different from preceding sequence. All pending frames must be flushed out before the new sequence can start to decode.
					FlushFrames (pDecoder, iTotal, pYuvFile, iFrameCount, uiTimeStamp, iWidth, iHeight, iLastWidth,
											iLastHeight);
				}
			}
			if (iSpsByteCount > 0 && uSpsPtr != NULL) {
				if (iSpsByteCount > 32) iSpsByteCount = 32;
				iLastSpsByteCount = iSpsByteCount;
				memcpy (uLastSpsBuf, uSpsPtr, iSpsByteCount);
			}
		} else {
			int i = 0;
			for (i = 0; i < iFileSize; i++) {
				if ((pBuf[iBufPos + i] == 0 && pBuf[iBufPos + i + 1] == 0 && pBuf[iBufPos + i + 2] == 0 && pBuf[iBufPos + i + 3] == 1
						&& i > 0) || (pBuf[iBufPos + i] == 0 && pBuf[iBufPos + i + 1] == 0 && pBuf[iBufPos + i + 2] == 1 && i > 0)) {
					break;
				}
			}
			iSliceSize = i;
		}
		if (iSliceSize < 4) { //too small size, no effective data, ignore
			iBufPos += iSliceSize;
			continue;
		}

//for coverage test purpose
		int32_t iEndOfStreamFlag;
		pDecoder->GetOption (DECODER_OPTION_END_OF_STREAM, &iEndOfStreamFlag);
		int32_t iCurIdrPicId;
		pDecoder->GetOption (DECODER_OPTION_IDR_PIC_ID, &iCurIdrPicId);
		int32_t iFrameNum;
		pDecoder->GetOption (DECODER_OPTION_FRAME_NUM, &iFrameNum);
		int32_t bCurAuContainLtrMarkSeFlag;
		pDecoder->GetOption (DECODER_OPTION_LTR_MARKING_FLAG, &bCurAuContainLtrMarkSeFlag);
		int32_t iFrameNumOfAuMarkedLtr;
		pDecoder->GetOption (DECODER_OPTION_LTR_MARKED_FRAME_NUM, &iFrameNumOfAuMarkedLtr);
		int32_t iFeedbackVclNalInAu;
		pDecoder->GetOption (DECODER_OPTION_VCL_NAL, &iFeedbackVclNalInAu);
		int32_t iFeedbackTidInAu;
		pDecoder->GetOption (DECODER_OPTION_TEMPORAL_ID, &iFeedbackTidInAu);
//~end for

		iStart = WelsTime();
		pData[0] = NULL;
		pData[1] = NULL;
		pData[2] = NULL;
		uiTimeStamp ++;
		memset (&sDstBufInfo, 0, sizeof (SBufferInfo));
		sDstBufInfo.uiInBsTimeStamp = uiTimeStamp;
			pDecoder->DecodeFrameNoDelay (pBuf + iBufPos, iSliceSize, pData, &sDstBufInfo);

		if (sDstBufInfo.iBufferStatus == 1) {
			pDst[0] = sDstBufInfo.pDst[0];
			pDst[1] = sDstBufInfo.pDst[1];
			pDst[2] = sDstBufInfo.pDst[2];
		}
		iEnd		= WelsTime();
		iTotal += iEnd - iStart;
		if (sDstBufInfo.iBufferStatus == 1) {
			Process ((void**)pDst, &sDstBufInfo, pYuvFile);
			iWidth	= sDstBufInfo.UsrData.sSystemBuffer.iWidth;
			iHeight = sDstBufInfo.UsrData.sSystemBuffer.iHeight;

			++ iFrameCount;
		}

		iBufPos += iSliceSize;
		++ iSliceIndex;
	}
	FlushFrames (pDecoder, iTotal, pYuvFile, iFrameCount, uiTimeStamp, iWidth, iHeight, iLastWidth,
							iLastHeight);
	dElapsed = iTotal / 1e6;
	fprintf (stderr, "-------------------------------------------------------\n");
	fprintf (stderr, "iWidth:\t\t%d\nheight:\t\t%d\nFrames:\t\t%d\ndecode time:\t%f sec\nFPS:\t\t%f fps\n",
					iWidth, iHeight, iFrameCount, dElapsed, (iFrameCount * 1.0) / dElapsed);
	fprintf (stderr, "-------------------------------------------------------\n");

	// coverity scan uninitial
label_exit:
	if (pBuf) {
		delete[] pBuf;
		pBuf = NULL;
	}
	if (pH264File) {
		fclose (pH264File);
		pH264File = NULL;
	}
	if (pYuvFile) {
		fclose (pYuvFile);
		pYuvFile = NULL;
	}
}

int32_t
main (int32_t iArgC, char* pArgV[]) {

	ISVCDecoder* pDecoder = NULL;

	SDecodingParam sDecParam = {0};
	sDecParam.sVideoProperty.size = sizeof (sDecParam.sVideoProperty);
	sDecParam.eEcActiveIdc = ERROR_CON_SLICE_MV_COPY_CROSS_IDR_FREEZE_RES_CHANGE;
	sDecParam.uiTargetDqLayer = (uint8_t) - 1;
	sDecParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
	sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
	
	int iLevelSetting = (int) WELS_LOG_WARNING;

	auto strInputFile = string( pArgV[1] );
	auto strOutputFile = string( pArgV[2] );

	if (WelsCreateDecoder (&pDecoder)	|| (NULL == pDecoder)) {
		printf ("Create Decoder failed.\n");
		return 1;
	}
	if (iLevelSetting >= 0) {
		pDecoder->SetOption (DECODER_OPTION_TRACE_LEVEL, &iLevelSetting);
	}

	int32_t iThreadCount = 0;
	pDecoder->SetOption (DECODER_OPTION_NUM_OF_THREADS, &iThreadCount);

	if (pDecoder->Initialize (&sDecParam)) {
		printf ("Decoder initialization failed.\n");
		return 1;
	}

	int32_t iWidth = 0;
	int32_t iHeight = 0;

	H264DecodeInstance(
		pDecoder
	,	strInputFile.c_str()
	,	!strOutputFile.empty() ? strOutputFile.c_str() : NULL
	,	iWidth
	,	iHeight
	,	(int32_t)sDecParam.eEcActiveIdc
	);

	if ( pDecoder ) {
		pDecoder->Uninitialize();
		WelsDestroyDecoder( pDecoder );
	}

	return 0;
}
