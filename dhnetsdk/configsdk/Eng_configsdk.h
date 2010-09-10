
#ifndef CONFIGSDK_H
#define CONFIGSDK_H

#ifdef WIN32

#ifdef CONFIGSDK_EXPORTS
#define CLIENT_API  __declspec(dllexport) 
#else
#define CLIENT_API  __declspec(dllimport)
#endif

#define CALLBACK __stdcall
#define CALL_METHOD  __stdcall  //__cdecl

#else	//linux

#define CLIENT_API	extern "C"
#define CALL_METHOD
#define CALLBACK

#endif


#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************
 ** Constant Definition 
 ***********************************************************************/

#define MAX_CHANNEL_NUM			8
#define MAX_VIDEO_CHANNEL_NUM	256					// Max 64 channels
#define MAX_CHANNELNAME_LEN		32					// Max channel name length
#define MAX_VIDEOSTREAM_NUM		3					// Max video stream amount
#define MAX_VIDEO_COVER_NUM		16					// Max privacy mask zone amount
#define WEEK_DAY_NUM			7					// The days in one week
#define MAX_REC_TSECT			6					// Record period amount
#define MAX_WATERMARK_LEN		4096				// Max length of digital watermark data
#define MAX_MOTION_ROW			32					// Row amount of the motion detect zone
#define MAX_MOTION_COL			32					// Column amount of the motion detect zone
#define MAX_IMAGESIZE_NUM		256					// Max resolution amount supported
#define MAX_FPS_NUM				1024				// Max frame rate amount supported
#define MAX_QUALITY_NUM			32					// Max quality amount supported
#define MAX_ADDRESS_LEN			256					// Max address length
#define MAX_USERNAME_LEN		64					// Max user name length
#define MAX_PASSWORD_LEN		64					// Max password length
#define MAX_DIRECTORY_LEN		256					// The name string length of the folder
#define MAX_NAS_TIME_SECTION	2					// The network storage period amount
#define MAX_NAME_LEN			32					//The general name string length
#define MAX_DECPRO_LIST_SIZE	100					// Max decoder protocol list amount


/************************************************************************
 ** Configuration Command
 ***********************************************************************/

#define CFG_CMD_ENCODE			"Encode"			// Video channel property setup (corresponding to CFG_ENCODE_INFO)
#define CFG_CMD_RECORD			"Record"			// Schedule record setup
#define CFG_CMD_ALARMINPUT		"Alarm"				// External input alarm setup
#define CFG_CMD_NETALARMINPUT	"NetAlarm"			// Network alarm setup
#define CFG_CMD_MOTIONDETECT	"MotionDetect"		// Motion detect alarm setup
#define CFG_CMD_VIDEOLOST		"LossDetect"		// Video loss alarm setup
#define CFG_CMD_VIDEOBLIND		"BlindDetect"		// Camera masking alarm setup
#define CFG_CMD_STORAGENOEXIST	"StorageNotExist"	// No storage device alarm setup
#define CFG_CMD_STORAGEFAILURE	"StorageFailure"	// The storage device access error alarm setup
#define CFG_CMD_STORAGELOWSAPCE	"StorageLowSpace"	// The insufficient storage space alarm setup
#define CFG_CMD_NETABORT		"NetAbort"			// Network disconnection alarm setup	
#define CFG_CMD_IPCONFLICT		"IPConflict"		// IP conflict alarm setup
#define CFG_CMD_SNAPCAPINFO		"SnapInfo"			// Snapshot competence search
#define CFG_CMD_NAS				"NAS"				// NAS setup
#define CFG_CMD_PTZ				"Ptz"				// PTZ setup
#define CFG_CMD_WATERMARK		"WaterMark"			// Video watermark setup


/************************************************************************
 ** Configuration Information (There is competence information for each structure to distinguish different device models and application versions. )
 ***********************************************************************/

//-----------------------------Video Channel Property-------------------------------

// Image quality 
typedef enum tagCFG_IMAGE_QUALITY
{
	IMAGE_QUALITY_Q10 = 1,							// Image quality 10%
	IMAGE_QUALITY_Q30,								// Image quality 30%
	IMAGE_QUALITY_Q50,								// Image quality 50%
	IMAGE_QUALITY_Q60,								// Image quality 60%
	IMAGE_QUALITY_Q80,								// Image quality 80%
	IMAGE_QUALITY_Q100,								// Image quality 100%
} CFG_IMAGE_QUALITY;

// Video compression format 
typedef enum tagCFG_VIDEO_COMPRESSION
{
	VIDEO_FORMAT_MPEG4,								// MPEG4
	VIDEO_FORMAT_MS_MPEG4,							// MS-MPEG4
	VIDEO_FORMAT_MPEG2,								// MPEG2
	VIDEO_FORMAT_MPEG1,								// MPEG1
	VIDEO_FORMAT_H263,								// H.263
	VIDEO_FORMAT_MJPG,								// MJPG
	VIDEO_FORMAT_FCC_MPEG4,							// FCC-MPEG4
	VIDEO_FORMAT_H264,								// H.264
} CFG_VIDEO_COMPRESSION;

// Bit stream control mode 
typedef enum tagCFG_BITRATE_CONTROL
{
	BITRATE_CBR,									// Constant bit rate (CBR)
	BITRATE_VBR,									// Variable bit rate (VBR)
} CFG_BITRATE_CONTROL;

// Resolution enumeration
typedef enum tagCFG_CAPTURE_SIZE
{
	IMAGE_SIZE_D1,								// 704*576(PAL)  704*480(NTSC)
	IMAGE_SIZE_HD1,								// 352*576(PAL)  352*480(NTSC)
	IMAGE_SIZE_BCIF,							// 704*288(PAL)  704*240(NTSC)
	IMAGE_SIZE_CIF,								// 352*288(PAL)  352*240(NTSC)
	IMAGE_SIZE_QCIF,							// 176*144(PAL)  176*120(NTSC)
	IMAGE_SIZE_VGA,								// 640*480
	IMAGE_SIZE_QVGA,							// 320*240
	IMAGE_SIZE_SVCD,							// 480*480
	IMAGE_SIZE_QQVGA,							// 160*128
	IMAGE_SIZE_SVGA,							// 800*592
	IMAGE_SIZE_XVGA,							// 1024*768
	IMAGE_SIZE_WXGA,							// 1280*800
	IMAGE_SIZE_SXGA,							// 1280*1024  
	IMAGE_SIZE_WSXGA,							// 1600*1024  
	IMAGE_SIZE_UXGA,							// 1600*1200
	IMAGE_SIZE_WUXGA,							// 1920*1200
	IMAGE_SIZE_LTF,								// 240*192
	IMAGE_SIZE_720,								// 1280*720
	IMAGE_SIZE_1080,							// 1920*1080
	IMAGE_SIZE_1_3M,							// 1280*960
	IMAGE_SIZE_NR  
} CFG_CAPTURE_SIZE;

// Video format
typedef struct tagCFG_VIDEO_FORMAT
{
	// Competence 
	bool				abCompression;
	bool				abWidth;
	bool				abHeight;
	bool				abBitRateControl;
	bool				abBitRate;
	bool				abFrameRate;
	bool				abIFrameInterval;
	bool				abImageQuality;
	bool				abFrameType;

	// Information
	CFG_VIDEO_COMPRESSION emCompression;			// Video compression mode 
	int					nWidth;						// Video width
	int					nHeight;					// Video height
	CFG_BITRATE_CONTROL	emBitRateControl;			// Bit rate control mode 
	int					nBitRate;					// Video bit rate (kbps)
	int					nFrameRate;					// Video frame rate
	int					nIFrameInterval;			// I frame interval(1-100). For example, 50 means there is I frame in each 49 B frame or P frame.
	CFG_IMAGE_QUALITY	emImageQuality;				// Video quality 
	int					nFrameType;					// Sniffer mode£¬0£­YWAV£¬1£­"PS"
} CFG_VIDEO_FORMAT;

// Video encode parameter
typedef struct tagCFG_VIDEOENC_OPT
{
	// Competence
	bool				abVideoEnable;
	bool				abAudioEnable;
	bool				abSnapEnable;

	// Information
	BOOL				bVideoEnable;				// Video enable
	CFG_VIDEO_FORMAT	stuVideoFormat;				// Video format
	BOOL				bAudioEnable;				// Audio enable
	BOOL				bSnapEnable;				// Schedule snapshot enable
} CFG_VIDEOENC_OPT;

// RGBA information
typedef struct tagCFG_RGBA
{
	int					nRed;
	int					nGreen;
	int					nBlue;
	int					nAlpha;
} CFG_RGBA;

// Zone information
typedef struct tagCFG_RECT
{
	int					nLeft;
	int					nTop;
    int					nRight;
    int					nBottom;				
} CFG_RECT;

// Privacy mask information
typedef struct tagCFG_COVER_INFO
{
	// Competence
	bool				abBlockType;
	bool				abEncodeBlend;
	bool				abPreviewBlend;

	// Information
	CFG_RECT			stuRect;					// The position (coordinates) of the mask zone
	CFG_RGBA			stuColor;					// The mask color
	int					nBlockType;					// The mask mode £»0£­black block£¬1£­Mosaic
	int					nEncodeBlend;				// Encode-level privacy mask£»1£­enable£¬0£­disable
	int					nPreviewBlend;				// Preview mask£»1£­enable£¬0£­disable
} CFG_COVER_INFO;

// Multiple-zone mask configuration
typedef struct tagCFG_VIDEO_COVER
{
	int                 nTotalBlocks;						// The supported privacy mask zone amount
	int					nCurBlocks;							// The zone amount already set
	CFG_COVER_INFO		stuCoverBlock[MAX_VIDEO_COVER_NUM];	// The mask zone	
} CFG_VIDEO_COVER;

// OSD information
typedef struct tagCFG_OSD_INFO
{
	//Capcity 
	bool				abShowEnable;

	// Information
	CFG_RGBA			stuFrontColor;				// Front color
	CFG_RGBA			stuBackColor;				// Background color
	CFG_RECT			stuRect;					// Rectangle zone
	BOOL				bShowEnable;				// Display enbale
} CFG_OSD_INFO;

// Video color property
typedef struct tagCFG_COLOR_INFO
{
	int					nBrightness;				// Brgihtness(0-100)
	int					nContrast;					// Contrast(0-100)
	int					nSaturation;				// Saturation (0-100)
	int					nHue;						//Hue (0-100)
	int					nGain;						// Gain(0-100)
	BOOL				bGainEn;					// Gain enable
} CFG_COLOR_INFO;

// Video channel property information
typedef struct tagCFG_ENCODE_INFO
{
	int                 nChannelID;							// Channel number(Begins with 0)
	char				szChnName[MAX_CHANNELNAME_LEN];		// Channel name
	CFG_VIDEOENC_OPT	stuMainStream[MAX_VIDEOSTREAM_NUM];	// Main stream£¬0£­General record£¬1-Motion detect£¬2£­alarm record
	CFG_VIDEOENC_OPT	stuExtraStream[MAX_VIDEOSTREAM_NUM];// Extra stream£¬0£­Extra stream 1£¬1£­Extra stream 2£¬2£­Extra stream 3
	CFG_VIDEOENC_OPT	stuSnapFormat[MAX_VIDEOSTREAM_NUM];	// Snapshot£¬0£­General snapshot£¬1£­Motion detect snapshot£¬2£­alarm snapshot
	DWORD				dwCoverAbilityMask;					// The subnet mask of the privacy mask competence. Use the bit to represent. There are local preview, record and network monitor.
	DWORD				dwCoverEnableMask;					// The subnet mask of the privacy mask enable.Use the bit to represent. There are local preview, record and network monitor.
	CFG_VIDEO_COVER		stuVideoCover;						// Privacy mask 
	CFG_OSD_INFO		stuChnTitle;						// Channel title
	CFG_OSD_INFO		stuTimeTitle;						// Time title
	CFG_COLOR_INFO		stuVideoColor;						// Video color
} CFG_ENCODE_INFO;

// Watermark configuration
typedef struct tagCFG_WATERMARK_INFO 
{
	int                 nChannelID;					// Channel number(Begins with 0)
	BOOL				bEnable;					// Enable
	int					nStreamType;				// Bit stream type (1¡«n)£¬0-all stremas
	int					nDataType;					// Data type£¬1£­character£¬2£­image
	char				pData[MAX_WATERMARK_LEN];	// String watermark data
} CFG_WATERMARK_INFO;

//-------------------------------Record Configuration---------------------------------

// Period information
typedef struct tagCFG_TIME_SECTION 
{
	DWORD				dwRecordMask;						// Record subnet mask. The bit represents motion detect reocrd, alarm record, schedule record. Bit3~Bit15 is reserved. Bit 16 motion detect snapshot, Bit 17 alarm snapshot, Bit18 schedule snapshot
	int					nBeginHour;
	int					nBeginMin;
	int					nBeginSec;
	int					nHourEnd;
	int					nEndMin;
	int					nEndSec;	
} CFG_TIME_SECTION;

// Schedule record configuration information
typedef struct tagCFG_RECORD_INFO
{
	int                 nChannelID;					// The channel number(Begins with 0)
	CFG_TIME_SECTION	stuTimeSection[WEEK_DAY_NUM][MAX_REC_TSECT]; // Time table
	int					nPreRecTime;				// Pre-record time.The value ranges from 0 to 300. This function is disable when the value is 0.
	BOOL				bRedundancyEn;				// Record redundancy enbale button
	int					nStreamType;				// 0£­main stream£¬1£­extra stream 1£¬2£­extra stream 2£¬3£­extra stream 3
} CFG_RECORD_INFO;

//-------------------------------Alarm Configuration---------------------------------

// PTZ activation type
typedef enum tagCFG_LINK_TYPE
{
	LINK_TYPE_NONE,									// No activation
	LINK_TYPE_PRESET,								// Activate the preset
	LINK_TYPE_TOUR,									// Activate the tour
	LINK_TYPE_PATTERN,								// Activate the pattern
} CFG_LINK_TYPE;

// PTZ activation information
typedef struct tagCFG_PTZ_LINK
{
	CFG_LINK_TYPE		emType;						// Activation type
	int					nValue;						// The activation value is the corresponding preset number, tour number and etc.
} CFG_PTZ_LINK;

// Alarm activation information
typedef struct tagCFG_ALARM_MSG_HANDLE
{
	//Competence 
	bool				abRecordMask;
	bool				abRecordEnable;
	bool				abRecordLatch;
	bool				abAlarmOutMask;
	bool				abAlarmOutEn;
	bool				abAlarmOutLatch;	
	bool				abExAlarmOutMask;
	bool				abExAlarmOutEn;
	bool				abPtzLinkEn;
	bool				abTourMask;
	bool				abTourEnable;
	bool				abSnapshot;
	bool				abSnapshotEn;
	bool				abSnapshotPeriod;
	bool				abSnapshotTimes;
	bool				abTipEnable;
	bool				abMailEnable;
	bool				abMessageEnable;
	bool				abBeepEnable;
	bool				abVoiceEnable;
	bool				abMatrixMask;
	bool				abMatrixEnable;
	bool				abEventLatch;
	bool				abLogEnable;
	bool				abDelay;
	bool				abVideoMessageEn;
	bool				abMMSEnable;
	bool				abMessageToNetEn;

	//Information
	int					nChannelCount;								// The video channel of the device
	int					nAlarmOutCount;								// The alarm output amount of the device
	DWORD				dwRecordMask[MAX_CHANNEL_NUM];				// Subnet mask of the recording channel(use the bit to represent)
	BOOL				bRecordEnable;								// Record enable
	int					nRecordLatch;								// Record delay time(s)
	DWORD				dwAlarmOutMask[MAX_CHANNEL_NUM];			// Subnet mask of alarm output channel 
	BOOL				bAlarmOutEn;								// Alarm output enable
	int					nAlarmOutLatch;								//Alarm output delay time (s)
	DWORD				dwExAlarmOutMask[MAX_CHANNEL_NUM];			// Subnet mask of extension alarm output channel 
	BOOL				bExAlarmOutEn;								// Extension alarm output enable
	CFG_PTZ_LINK		stuPtzLink[MAX_VIDEO_CHANNEL_NUM];			// PTZ activation item
	BOOL				bPtzLinkEn;									// PTZ activation enable
	DWORD				dwTourMask[MAX_CHANNEL_NUM];				// Subnet mask of tour channel 
	BOOL				bTourEnable;								// Tour enable
	DWORD				dwSnapshot[MAX_CHANNEL_NUM];				// Snapshot channel subnet mask
	BOOL				bSnapshotEn;								// Snapshot enable
	int					nSnapshotPeriod;							// Snapshot period(s)
	int					nSnapshotTimes;								// Snapshot times
	BOOL				bTipEnable;									// Local prompt dialogue box
	BOOL				bMailEnable;								// Send out emali. The image is sent out as the attachment. 
	BOOL				bMessageEnable;								// Upload to the alarm server 
	BOOL				bBeepEnable;								// Buzzer
	BOOL				bVoiceEnable;								// Audio prompt
	DWORD				dwMatrixMask[MAX_CHANNEL_NUM];				// Subnet mask of the activated video channel
	BOOL				bMatrixEnable;								// Activate the video matrix
	int					nEventLatch;								// Activation delay time (s)£¬0£­15
	BOOL				bLogEnable;									// Record log or not
	int					nDelay;										// Delay first and then becomes valid when set. Unit is second.
	BOOL				bVideoMessageEn;							// Overlay the prompt character to the video. The overlay character includes the event type, channel number. The unit is second.
	BOOL				bMMSEnable;									// Enable MMS
	BOOL				bMessageToNetEn;							// Send the message to the network enable
} CFG_ALARM_MSG_HANDLE;

// External alarm setup
typedef struct tagCFG_ALARMIN_INFO
{
	int					nChannelID;									// Alarm channel number (Begins with 0)
	BOOL				bEnable;									// Enable
	char				szChnName[MAX_CHANNELNAME_LEN];				// Alarm channel name
	int					nAlarmType;									// Alarm device type£¬0£ºNO£¬1£ºNC
	CFG_ALARM_MSG_HANDLE stuEventHandler;							// Alarm activation
	CFG_TIME_SECTION	stuTimeSection[WEEK_DAY_NUM][MAX_REC_TSECT];// Event responding period
} CFG_ALARMIN_INFO;

// Network input alarm setup
typedef struct tagCFG_NETALARMIN_INFO 
{
	int					nChannelID;									// Alarm channel number (Begins with 0)
	BOOL				bEnable;									// Enable
	char				szChnName[MAX_CHANNELNAME_LEN];				// Alarm channel name
	int					nAlarmType;									// Alarm device type£¬0£ºNC£¬1£ºNO
	CFG_ALARM_MSG_HANDLE stuEventHandler;							// Alarm activation
	CFG_TIME_SECTION	stuTimeSection[WEEK_DAY_NUM][MAX_REC_TSECT];// Event responding period
} CFG_NETALARMIN_INFO;

// Motion detect alarm setup
typedef struct tagCFG_MOTION_INFO 
{
	int					nChannelID;									// Alarm channel number (Begins with 0)
	BOOL				bEnable;									// Enable
	int					nSenseLevel;								// Sensitivity 1¡«6
	int					nMotionRow;									// The rows of the motion detect zone 
	int					nMotionCol;									// The columns of the motion detect zone
	BYTE				byRegion[MAX_MOTION_ROW][MAX_MOTION_COL];	// Motion detect£¬Max 32*32 zones
	CFG_ALARM_MSG_HANDLE stuEventHandler;							// Alarm activation
	CFG_TIME_SECTION	stuTimeSection[WEEK_DAY_NUM][MAX_REC_TSECT];// Event responding period
} CFG_MOTION_INFO;

// Video loss alarm setup
typedef struct tagCFG_VIDEOLOST_INFO 
{
	int					nChannelID;									// Alarm channel number (Begins with 0)
	BOOL				bEnable;									// Enable
	CFG_ALARM_MSG_HANDLE stuEventHandler;							// Alarm activation
	CFG_TIME_SECTION	stuTimeSection[WEEK_DAY_NUM][MAX_REC_TSECT];// Event responding period
} CFG_VIDEOLOST_INFO;

// Camera masking alarm setup
typedef struct tagCFG_SHELTER_INFO 
{
	int					nChannelID;									// Alarm channel number (Begins with 0)
	BOOL				bEnable;									// Enable
	int					nSenseLevel;								// Sensitivity
	CFG_ALARM_MSG_HANDLE stuEventHandler;							// Alarm activation
	CFG_TIME_SECTION	stuTimeSection[WEEK_DAY_NUM][MAX_REC_TSECT];// Event responding period
} CFG_SHELTER_INFO;

// No storage device alarm setup
typedef struct tagCFG_STORAGENOEXIST_INFO 
{
	BOOL				bEnable;						// Enable
	CFG_ALARM_MSG_HANDLE stuEventHandler;				// Alarm activation
} CFG_STORAGENOEXIST_INFO;

// Storage device access error alarm setup
typedef struct tagCFG_STORAGEFAILURE_INFO 
{
	BOOL				bEnable;						// Enable
	CFG_ALARM_MSG_HANDLE stuEventHandler;				// Alarm activation
} CFG_STORAGEFAILURE_INFO;

// Insufficient storage space alarm setup
typedef struct tagCFG_STORAGELOWSAPCE_INFO 
{
	BOOL				bEnable;						// Enable
	int					nLowerLimit;					// The HDD free space threshold. Percentage (0~99)
	CFG_ALARM_MSG_HANDLE stuEventHandler;				// Alarm activation
} CFG_STORAGELOWSAPCE_INFO;

//Network disconnection alarm setup
typedef struct tagCFG_NETABORT_INFO 
{
	BOOL				bEnable;						// Enable
	CFG_ALARM_MSG_HANDLE stuEventHandler;				// Alarm activation
} CFG_NETABORT_INFO;

// IP conflict alarm setup
typedef struct tagCFG_IPCONFLICT_INFO 
{
	BOOL				bEnable;						// Enable
	CFG_ALARM_MSG_HANDLE stuEventHandler;				// Alarm activation
} CFG_IPCONFLICT_INFO;

//-------------------------------Snapshot Configuration Competence ---------------------------------
// Snapshot configuration competence 
typedef struct tagCFG_SNAPCAPINFO_INFO  
{
	int					nChannelID;						// Snapshot channel number(Begin with 0)
	DWORD				dwIMageSizeNum;					// The  supported resolution information
	CFG_CAPTURE_SIZE	emIMageSizeList[MAX_IMAGESIZE_NUM];
	DWORD				dwFramesPerSecNum;				// The supported frame rate information
	int					nFramesPerSecList[MAX_FPS_NUM];
	DWORD				dwQualityMun;					// The supported image quality information
	CFG_IMAGE_QUALITY	emQualityList[MAX_QUALITY_NUM];

	DWORD               dwMode;							// Mode,use the bit to represent £ºThe first bit£ºSchedule. The second bit£ºManual.
	DWORD               dwFormat;						// Image format mode,use the bit to represent£ºThe first bit£ºbmp£»The second bit£ºjpg.
} CFG_SNAPCAPINFO_INFO;


//-------------------------------NAS Configuration---------------------------------
// NAS configuration
typedef struct tagCFG_CHANNEL_TIME_SECTION 
{
	CFG_TIME_SECTION stuTimeSection[WEEK_DAY_NUM][MAX_NAS_TIME_SECTION];//Storage period
} CFG_CHANNEL_TIME_SECTION;

typedef struct tagCFG_NAS_INFO
{
	BOOL						bEnable;									// Enable button
	int							nVersion;									// NAS version 0=Old FTP£¬1=NAS
	int							nProtocol;									// Protocol type 0=FTP 1=SMB
	char						szAddress[MAX_ADDRESS_LEN];					// IP address or network name
	int							nPort;										// Port number
	char						szUserName[MAX_USERNAME_LEN];				// Account name
	char						szPassword[MAX_PASSWORD_LEN];				// Password 
	char						szDirectory[MAX_DIRECTORY_LEN];				// Share folder name
	int							nFileLen;									// File length
	int							nInterval;									// Adjacent file interval
	CFG_CHANNEL_TIME_SECTION	stuChnTime[MAX_VIDEO_CHANNEL_NUM];			// Storage period
} CFG_NAS_INFO;

//------------------------------PTZ Configuration--------------------------------------------
// Protocol name
typedef struct tagCFG_PRONAME
{
	char				name[MAX_NAME_LEN];			// Protocol name
} CFG_PRONAME;

// COM basic property
typedef struct tagCFG_COMM_PROP
{
	BYTE				byDataBit;					// Data bit£»0£º5£¬1£º6£¬2£º7£¬3£º8
	BYTE				byStopBit;					// Stop bit£»0£º1-bit£¬1£º1.5-bit£¬2£º2-bit
	BYTE				byParity;					// Parity£»0£ºNone£¬1£ºOdd£»2£ºEven
	BYTE				byBaudRate;					// Baud rate£»0£º300£¬1£º600£¬2£º1200£¬3£º2400£¬4£º4800£¬													// 5£º9600£¬6£º19200£¬7£º38400£¬8£º57600£¬9£º115200
} CFG_COMM_PROP;

// PTZ configuration
typedef struct tagCFG_PTZ_INFO
{
	//Competence
	bool				abMartixID;
	bool				abCamID;
	bool				abPTZType;

	// Information
	int					nChannelID;								// Channel name(Begins with 0)	
	CFG_PRONAME         stuDecProName[MAX_DECPRO_LIST_SIZE];	// Protocol name list (Read-only)
	int					nProName;								// Protocl type. The subscript of the Protocl Name List
	int					nDecoderAddress;						// Decoder address£»0 - 255
	CFG_COMM_PROP		struComm;
	int					nMartixID;								// PTZ number
	int					nPTZType;								// PTZ type 0-compatible£¬local PTZ 1-Remote network PTZ
	int					nCamID;									// Camera ID
} CFG_PTZ_INFO;
  

/************************************************************************
 ** Interface Definition 
 ***********************************************************************/

// Parse the latest searched configuration information
CLIENT_API BOOL  CALL_METHOD CLIENT_ParseData(char* szCommand, char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, void* pReserved);

// Constitute the configuration information to be set.
CLIENT_API BOOL  CALL_METHOD CLIENT_PacketData(char* szCommand, LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize);


#ifdef __cplusplus
}
#endif

#endif // CONFIGSDK_H

