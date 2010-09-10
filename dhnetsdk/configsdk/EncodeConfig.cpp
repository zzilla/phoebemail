
#include "StdAfx.h"
#include "EncodeConfig.h"
#include "json/json.h"
#include "Utils_StrParser.h"

#define YWAVSTR "DHAV"

BOOL Encode_Common_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;

	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_ENCODE_INFO))
	{
		return bRet;
	}

	Json::Reader reader;
	static Json::Value value;

	CFG_ENCODE_INFO stuEncodeInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);

	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["Encode"]["Chn"].type() != Json::nullValue)
		{
			stuEncodeInfo.nChannelID = value["Encode"]["Chn"].asInt();
		}

		if (value["Encode"]["Name"].type() != Json::nullValue)
		{
			int nNameLen = value["Encode"]["Name"].asString().size();
			nNameLen = nNameLen>MAX_CHANNELNAME_LEN?MAX_CHANNELNAME_LEN:nNameLen;
			strncpy(stuEncodeInfo.szChnName, value["Encode"]["Name"].asString().c_str(), nNameLen);
		}

		int i = 0;
		for (i = 0; i < MAX_VIDEOSTREAM_NUM; i++)
		{
			//主码流
			if (value["Encode"]["MainFormat"][i]["AudioEn"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].abAudioEnable = true;
				stuEncodeInfo.stuMainStream[i].bAudioEnable = value["Encode"]["MainFormat"][i]["AudioEn"].asInt();
			}

			if (value["Encode"]["MainFormat"][i]["VideoEn"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].abVideoEnable = true;
				stuEncodeInfo.stuMainStream[i].bVideoEnable = value["Encode"]["MainFormat"][i]["VideoEn"].asInt();
			}

			if (value["Encode"]["MainFormat"][i]["Video"]["Compression"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.abCompression = true;
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.emCompression = CompressionStrtoInt((char *)value["Encode"]["MainFormat"][i]["Video"]["Compression"].asString().c_str());
			}

			if (value["Encode"]["MainFormat"][i]["Video"]["Width"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.abWidth = true;
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.nWidth = value["Encode"]["MainFormat"][i]["Video"]["Width"].asInt();
			}

			if (value["Encode"]["MainFormat"][i]["Video"]["Height"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.abHeight = true;
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.nHeight = value["Encode"]["MainFormat"][i]["Video"]["Height"].asInt();
			}

			if (value["Encode"]["MainFormat"][i]["Video"]["BitRateControl"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.abBitRateControl = true;
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.emBitRateControl = BitRateControlStrtoInt((char *)value["Encode"]["MainFormat"][i]["Video"]["BitRateControl"].asString().c_str());
			}

			if (value["Encode"]["MainFormat"][i]["Video"]["BitRate"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.abBitRate = true;
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.nBitRate = value["Encode"]["MainFormat"][i]["Video"]["BitRate"].asInt();
			}

			if (value["Encode"]["MainFormat"][i]["Video"]["FPS"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.abFrameRate = true;
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.nFrameRate = value["Encode"]["MainFormat"][i]["Video"]["FPS"].asInt();
			}

			if (value["Encode"]["MainFormat"][i]["Video"]["GOP"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.abIFrameInterval = true;
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.nIFrameInterval = value["Encode"]["MainFormat"][i]["Video"]["GOP"].asInt();
			}

			if (value["Encode"]["MainFormat"][i]["Video"]["Quality"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.abImageQuality = true;
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.emImageQuality = (CFG_IMAGE_QUALITY)value["Encode"]["MainFormat"][i]["Video"]["Quality"].asInt();
			}
			
			if (value["Encode"]["MainFormat"][i]["Video"]["Pack"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.abFrameType = true;
				stuEncodeInfo.stuMainStream[i].stuVideoFormat.nFrameType = FrameTypeStrtoInt((char *)value["Encode"]["MainFormat"][i]["Video"]["Pack"].asString().c_str());
			}

			//辅码流
			if (value["Encode"]["ExtraFormat"][i]["AudioEn"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].abAudioEnable = true;
				stuEncodeInfo.stuExtraStream[i].bAudioEnable = value["Encode"]["ExtraFormat"][i]["AudioEn"].asInt();
			}

			if (value["Encode"]["ExtraFormat"][i]["VideoEn"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].abVideoEnable = true;
				stuEncodeInfo.stuExtraStream[i].bVideoEnable = value["Encode"]["ExtraFormat"][i]["VideoEn"].asInt();
			}

			if (value["Encode"]["ExtraFormat"][i]["Video"]["Compression"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.abCompression = true;
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.emCompression = CompressionStrtoInt((char *)value["Encode"]["ExtraFormat"][i]["Video"]["Compression"].asString().c_str());
			}

			if (value["Encode"]["ExtraFormat"][i]["Video"]["Width"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.abWidth = true;
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.nWidth = value["Encode"]["ExtraFormat"][i]["Video"]["Width"].asInt();
			}

			if (value["Encode"]["ExtraFormat"][i]["Video"]["Height"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.abHeight = true;
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.nHeight = value["Encode"]["ExtraFormat"][i]["Video"]["Height"].asInt();
			}

			if (value["Encode"]["ExtraFormat"][i]["Video"]["BitRateControl"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.abBitRateControl = true;
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.emBitRateControl = BitRateControlStrtoInt((char *)value["Encode"]["ExtraFormat"][i]["Video"]["BitRateControl"].asString().c_str());
			}

			if (value["Encode"]["ExtraFormat"][i]["Video"]["BitRate"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.abBitRate = true;
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.nBitRate = value["Encode"]["ExtraFormat"][i]["Video"]["BitRate"].asInt();
			}

			if (value["Encode"]["ExtraFormat"][i]["Video"]["FPS"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.abFrameRate = true;
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.nFrameRate = value["Encode"]["ExtraFormat"][i]["Video"]["FPS"].asInt();
			}

			if (value["Encode"]["ExtraFormat"][i]["Video"]["GOP"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.abIFrameInterval = true;
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.nIFrameInterval = value["Encode"]["ExtraFormat"][i]["Video"]["GOP"].asInt();
			}

			if (value["Encode"]["ExtraFormat"][i]["Video"]["Quality"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.abImageQuality = true;
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.emImageQuality = (CFG_IMAGE_QUALITY)value["Encode"]["ExtraFormat"][i]["Video"]["Quality"].asInt();
			}
			
			if (value["Encode"]["ExtraFormat"][i]["Video"]["Pack"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.abFrameType = true;
				stuEncodeInfo.stuExtraStream[i].stuVideoFormat.nFrameType = FrameTypeStrtoInt((char *)value["Encode"]["ExtraFormat"][i]["Video"]["Pack"].asString().c_str());
			}

			//抓图	
			if (value["Encode"]["SnapFormat"][i]["TimingEn"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuSnapFormat[i].abSnapEnable = true;
				stuEncodeInfo.stuSnapFormat[i].bSnapEnable = value["Encode"]["SnapFormat"][i]["TimingEn"].asInt();
			}

			if (value["Encode"]["SnapFormat"][i]["Video"]["Compression"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuSnapFormat[i].stuVideoFormat.abCompression = true;
				stuEncodeInfo.stuSnapFormat[i].stuVideoFormat.emCompression = CompressionStrtoInt((char *)value["Encode"]["SnapFormat"][i]["Video"]["Compression"].asString().c_str());
			}

			if (value["Encode"]["SnapFormat"][i]["Video"]["Width"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuSnapFormat[i].stuVideoFormat.abWidth = true;
				stuEncodeInfo.stuSnapFormat[i].stuVideoFormat.nWidth = value["Encode"]["SnapFormat"][i]["Video"]["Width"].asInt();
			}

			if (value["Encode"]["SnapFormat"][i]["Video"]["Height"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuSnapFormat[i].stuVideoFormat.abHeight = true;
				stuEncodeInfo.stuSnapFormat[i].stuVideoFormat.nHeight = value["Encode"]["SnapFormat"][i]["Video"]["Height"].asInt();
			}

			if (value["Encode"]["SnapFormat"][i]["Video"]["Quality"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuSnapFormat[i].stuVideoFormat.abImageQuality = true;
				stuEncodeInfo.stuSnapFormat[i].stuVideoFormat.emImageQuality = (CFG_IMAGE_QUALITY)value["Encode"]["SnapFormat"][i]["Video"]["Quality"].asInt();
			}

		}
		
		// 通道标题	
		if (value["Encode"]["ChnTitle"]["FrontColor"]["Red"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuFrontColor.nRed = value["Encode"]["ChnTitle"]["FrontColor"]["Red"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["FrontColor"]["Green"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuFrontColor.nGreen = value["Encode"]["ChnTitle"]["FrontColor"]["Green"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["FrontColor"]["Blue"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuFrontColor.nBlue = value["Encode"]["ChnTitle"]["FrontColor"]["Blue"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["FrontColor"]["Alpha"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuFrontColor.nAlpha = value["Encode"]["ChnTitle"]["FrontColor"]["Alpha"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["BackColor"]["Red"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuBackColor.nRed = value["Encode"]["ChnTitle"]["BackColor"]["Red"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["BackColor"]["Green"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuBackColor.nGreen = value["Encode"]["ChnTitle"]["BackColor"]["Green"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["BackColor"]["Blue"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuBackColor.nBlue = value["Encode"]["ChnTitle"]["BackColor"]["Blue"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["BackColor"]["Alpha"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuBackColor.nAlpha = value["Encode"]["ChnTitle"]["BackColor"]["Alpha"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["Rect"]["Left"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuRect.nLeft = value["Encode"]["ChnTitle"]["Rect"]["Left"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["Rect"]["Top"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuRect.nTop = value["Encode"]["ChnTitle"]["Rect"]["Top"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["Rect"]["Right"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuRect.nRight = value["Encode"]["ChnTitle"]["Rect"]["Right"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["Rect"]["Bottom"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.stuRect.nBottom = value["Encode"]["ChnTitle"]["Rect"]["Bottom"].asInt();
		}

		if (value["Encode"]["ChnTitle"]["Show"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuChnTitle.abShowEnable = true;
			stuEncodeInfo.stuChnTitle.bShowEnable = value["Encode"]["ChnTitle"]["Show"].asInt();
		}

		// 时间标题	
		if (value["Encode"]["TimeTitle"]["FrontColor"]["Red"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuFrontColor.nRed = value["Encode"]["TimeTitle"]["FrontColor"]["Red"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["FrontColor"]["Green"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuFrontColor.nGreen = value["Encode"]["TimeTitle"]["FrontColor"]["Green"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["FrontColor"]["Blue"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuFrontColor.nBlue = value["Encode"]["TimeTitle"]["FrontColor"]["Blue"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["FrontColor"]["Alpha"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuFrontColor.nAlpha = value["Encode"]["TimeTitle"]["FrontColor"]["Alpha"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["BackColor"]["Red"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuBackColor.nRed = value["Encode"]["TimeTitle"]["BackColor"]["Red"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["BackColor"]["Green"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuBackColor.nGreen = value["Encode"]["TimeTitle"]["BackColor"]["Green"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["BackColor"]["Blue"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuBackColor.nBlue = value["Encode"]["TimeTitle"]["BackColor"]["Blue"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["BackColor"]["Alpha"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuBackColor.nAlpha = value["Encode"]["TimeTitle"]["BackColor"]["Alpha"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["Rect"]["Left"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuRect.nLeft = value["Encode"]["TimeTitle"]["Rect"]["Left"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["Rect"]["Top"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuRect.nTop = value["Encode"]["TimeTitle"]["Rect"]["Top"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["Rect"]["Right"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuRect.nRight = value["Encode"]["TimeTitle"]["Rect"]["Right"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["Rect"]["Bottom"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.stuRect.nBottom = value["Encode"]["TimeTitle"]["Rect"]["Bottom"].asInt();
		}

		if (value["Encode"]["TimeTitle"]["Show"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuTimeTitle.abShowEnable = true;
			stuEncodeInfo.stuTimeTitle.bShowEnable = value["Encode"]["TimeTitle"]["Show"].asInt();
		}

		// 图像颜色
		if (value["Encode"]["VideoColor"]["Brightness"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuVideoColor.nBrightness = value["Encode"]["VideoColor"]["Brightness"].asInt();
		}

		if (value["Encode"]["VideoColor"]["Contrast"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuVideoColor.nContrast = value["Encode"]["VideoColor"]["Contrast"].asInt();
		}
			
		if (value["Encode"]["VideoColor"]["Saturation"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuVideoColor.nSaturation = value["Encode"]["VideoColor"]["Saturation"].asInt();
		}
	
		if (value["Encode"]["VideoColor"]["Hue"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuVideoColor.nHue = value["Encode"]["VideoColor"]["Hue"].asInt();
		}

		if (value["Encode"]["VideoColor"]["GainEn"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuVideoColor.bGainEn = value["Encode"]["VideoColor"]["GainEn"].asInt();
		}
	
		if (value["Encode"]["VideoColor"]["Gain"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuVideoColor.nGain = value["Encode"]["VideoColor"]["Gain"].asInt();
		}

		//区域遮挡
		if (value["Encode"]["CoverType"].type() != Json::nullValue)
		{
			stuEncodeInfo.dwCoverAbilityMask = value["Encode"]["CoverType"].asInt();
		}
		if (value["Encode"]["CoverEn"].type() != Json::nullValue)
		{
			stuEncodeInfo.dwCoverEnableMask = value["Encode"]["CoverEn"].asInt();
		}
		
		if (value["Encode"]["Covers"]["TotalBlocks"].type() != Json::nullValue)
		{
			stuEncodeInfo.stuVideoCover.nTotalBlocks = value["Encode"]["Covers"]["TotalBlocks"].asInt();
		}

		for (i = 0; i < stuEncodeInfo.stuVideoCover.nTotalBlocks; i++)
		{
			if (value["Encode"]["CoverBlock"][i]["BlockType"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].abBlockType = true;
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].nBlockType = value["Encode"]["CoverBlock"][i]["BlockType"].asInt();
				stuEncodeInfo.stuVideoCover.nCurBlocks++;
			}

			if (value["Encode"]["CoverBlock"][i]["Color"]["Red"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].stuColor.nRed = value["Encode"]["CoverBlock"][i]["Color"]["Red"].asInt();
			}

			if (value["Encode"]["CoverBlock"][i]["Color"]["Green"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].stuColor.nGreen = value["Encode"]["CoverBlock"][i]["Color"]["Green"].asInt();
			}

			if (value["Encode"]["CoverBlock"][i]["Color"]["Blue"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].stuColor.nBlue = value["Encode"]["CoverBlock"][i]["Color"]["Blue"].asInt();
			}

			if (value["Encode"]["CoverBlock"][i]["Color"]["Alpha"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].stuColor.nAlpha = value["Encode"]["CoverBlock"][i]["Color"]["Alpha"].asInt();
			}

			if (value["Encode"]["CoverBlock"][i]["Rect"]["Left"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].stuRect.nLeft = value["Encode"]["CoverBlock"][i]["Rect"]["Left"].asInt();
			}

			if (value["Encode"]["CoverBlock"][i]["Rect"]["Top"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].stuRect.nTop = value["Encode"]["CoverBlock"][i]["Rect"]["Top"].asInt();
			}

			if (value["Encode"]["CoverBlock"][i]["Rect"]["Right"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].stuRect.nRight = value["Encode"]["CoverBlock"][i]["Rect"]["Right"].asInt();
			}

			if (value["Encode"]["CoverBlock"][i]["Rect"]["Bottom"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].stuRect.nBottom = value["Encode"]["CoverBlock"][i]["Rect"]["Bottom"].asInt();
			}

			if (value["Encode"]["CoverBlock"][i]["PreviewBlend"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].abPreviewBlend = true;
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].nPreviewBlend = value["Encode"]["CoverBlock"][i]["PreviewBlend"].asInt();
			}

			if (value["Encode"]["CoverBlock"][i]["EncodeBlend"].type() != Json::nullValue)
			{
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].abEncodeBlend = true;
				stuEncodeInfo.stuVideoCover.stuCoverBlock[i].nEncodeBlend = value["Encode"]["CoverBlock"][i]["EncodeBlend"].asInt();
			}
		}

		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_ENCODE_INFO);
		}
		memcpy(lpOutBuffer, &stuEncodeInfo, sizeof(CFG_ENCODE_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	

	return bRet;
}

BOOL Encode_Common_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_ENCODE_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_ENCODE_INFO *pEncodeInfo = NULL;
	pEncodeInfo = (CFG_ENCODE_INFO *)lpInBuffer;
	char szValue[128] = {0};
	
	bRet = TRUE;
	
	value["Encode"]["Chn"] = pEncodeInfo->nChannelID;
	value["Encode"]["Name"] = pEncodeInfo->szChnName;

	int i = 0;
	for (i = 0; i < MAX_VIDEOSTREAM_NUM; i++)
	{
		//主码流
		if (pEncodeInfo->stuMainStream[i].abAudioEnable)
		{
			value["Encode"]["MainFormat"][i]["AudioEn"] = pEncodeInfo->stuMainStream[i].bAudioEnable;
		}
		
		if (pEncodeInfo->stuMainStream[i].abVideoEnable)
		{
			value["Encode"]["MainFormat"][i]["VideoEn"] = pEncodeInfo->stuMainStream[i].bVideoEnable;
		}
		
		if (pEncodeInfo->stuMainStream[i].stuVideoFormat.abCompression)
		{
			memset(szValue, 0, 128);
			CompressionInttoStr(pEncodeInfo->stuMainStream[i].stuVideoFormat.emCompression, szValue);
			value["Encode"]["MainFormat"][i]["Video"]["Compression"] = szValue;
		}
		
		if (pEncodeInfo->stuMainStream[i].stuVideoFormat.abWidth)
		{
			value["Encode"]["MainFormat"][i]["Video"]["Width"] = pEncodeInfo->stuMainStream[i].stuVideoFormat.nWidth;
		}
		
		if (pEncodeInfo->stuMainStream[i].stuVideoFormat.abHeight)
		{
			value["Encode"]["MainFormat"][i]["Video"]["Height"] = pEncodeInfo->stuMainStream[i].stuVideoFormat.nHeight;
		}
		
		if (pEncodeInfo->stuMainStream[i].stuVideoFormat.abBitRateControl)
		{
			memset(szValue, 0, 128);
			BitRateControlInttoStr(pEncodeInfo->stuMainStream[i].stuVideoFormat.emBitRateControl, szValue);
			value["Encode"]["MainFormat"][i]["Video"]["BitRateControl"] = szValue;
		}
		
		if (pEncodeInfo->stuMainStream[i].stuVideoFormat.abBitRate)
		{
			value["Encode"]["MainFormat"][i]["Video"]["BitRate"] = pEncodeInfo->stuMainStream[i].stuVideoFormat.nBitRate;
		}
		
		if (pEncodeInfo->stuMainStream[i].stuVideoFormat.abFrameRate)
		{
			value["Encode"]["MainFormat"][i]["Video"]["FPS"] = pEncodeInfo->stuMainStream[i].stuVideoFormat.nFrameRate;
		}
		
		if (pEncodeInfo->stuMainStream[i].stuVideoFormat.abIFrameInterval)
		{
			value["Encode"]["MainFormat"][i]["Video"]["GOP"] = pEncodeInfo->stuMainStream[i].stuVideoFormat.nIFrameInterval;
		}
		
		if (pEncodeInfo->stuMainStream[i].stuVideoFormat.abImageQuality)
		{
			value["Encode"]["MainFormat"][i]["Video"]["Quality"] = (int)pEncodeInfo->stuMainStream[i].stuVideoFormat.emImageQuality;
		}
		
		if (pEncodeInfo->stuMainStream[i].stuVideoFormat.abFrameType)
		{
			memset(szValue, 0, 128);
			FrameTypeInttoStr(pEncodeInfo->stuMainStream[i].stuVideoFormat.nFrameType, szValue);
			value["Encode"]["MainFormat"][i]["Video"]["Pack"] = szValue;
		}
		
		//辅码流
		if (pEncodeInfo->stuExtraStream[i].abAudioEnable)
		{
			value["Encode"]["ExtraFormat"][i]["AudioEn"] = pEncodeInfo->stuExtraStream[i].bAudioEnable;
		}
		
		if (pEncodeInfo->stuExtraStream[i].abVideoEnable)
		{
			value["Encode"]["ExtraFormat"][i]["VideoEn"] = pEncodeInfo->stuExtraStream[i].bVideoEnable;
		}
		
		if (pEncodeInfo->stuExtraStream[i].stuVideoFormat.abCompression)
		{
			memset(szValue, 0, 128);
			CompressionInttoStr(pEncodeInfo->stuExtraStream[i].stuVideoFormat.emCompression, szValue);
			value["Encode"]["ExtraFormat"][i]["Video"]["Compression"] = szValue;
		}
		
		if (pEncodeInfo->stuExtraStream[i].stuVideoFormat.abWidth)
		{
			value["Encode"]["ExtraFormat"][i]["Video"]["Width"] = pEncodeInfo->stuExtraStream[i].stuVideoFormat.nWidth;
		}
		
		if (pEncodeInfo->stuExtraStream[i].stuVideoFormat.abHeight)
		{
			value["Encode"]["ExtraFormat"][i]["Video"]["Height"] = pEncodeInfo->stuExtraStream[i].stuVideoFormat.nHeight;
		}
		
		if (pEncodeInfo->stuExtraStream[i].stuVideoFormat.abBitRateControl)
		{
			memset(szValue, 0, 128);
			BitRateControlInttoStr(pEncodeInfo->stuExtraStream[i].stuVideoFormat.emBitRateControl, szValue);
			value["Encode"]["ExtraFormat"][i]["Video"]["BitRateControl"] = szValue;
		}
		
		if (pEncodeInfo->stuExtraStream[i].stuVideoFormat.abBitRate)
		{
			value["Encode"]["ExtraFormat"][i]["Video"]["BitRate"] = pEncodeInfo->stuExtraStream[i].stuVideoFormat.nBitRate;
		}
		
		if (pEncodeInfo->stuExtraStream[i].stuVideoFormat.abFrameRate)
		{
			value["Encode"]["ExtraFormat"][i]["Video"]["FPS"] = pEncodeInfo->stuExtraStream[i].stuVideoFormat.nFrameRate;
		}
		
		if (pEncodeInfo->stuExtraStream[i].stuVideoFormat.abIFrameInterval)
		{
			value["Encode"]["ExtraFormat"][i]["Video"]["GOP"] = pEncodeInfo->stuExtraStream[i].stuVideoFormat.nIFrameInterval;
		}
		
		if (pEncodeInfo->stuExtraStream[i].stuVideoFormat.abImageQuality)
		{
			value["Encode"]["ExtraFormat"][i]["Video"]["Quality"] = (int)pEncodeInfo->stuExtraStream[i].stuVideoFormat.emImageQuality;
		}
		
		if (pEncodeInfo->stuExtraStream[i].stuVideoFormat.abFrameType)
		{
			memset(szValue, 0, 128);
			FrameTypeInttoStr(pEncodeInfo->stuExtraStream[i].stuVideoFormat.nFrameType, szValue);
			value["Encode"]["ExtraFormat"][i]["Video"]["Pack"] = szValue;
		}
		
		//抓图	
		if (pEncodeInfo->stuSnapFormat[i].abSnapEnable)
		{
			value["Encode"]["SnapFormat"][i]["TimingEn"] = pEncodeInfo->stuSnapFormat[i].bSnapEnable;
		}
		
		if (pEncodeInfo->stuSnapFormat[i].stuVideoFormat.abCompression)
		{
			memset(szValue, 0, 128);
			CompressionInttoStr(pEncodeInfo->stuSnapFormat[i].stuVideoFormat.emCompression, szValue);
			value["Encode"]["SnapFormat"][i]["Video"]["Compression"] = szValue;
		}
		
		if (pEncodeInfo->stuSnapFormat[i].stuVideoFormat.abWidth)
		{
			value["Encode"]["SnapFormat"][i]["Video"]["Width"] = pEncodeInfo->stuSnapFormat[i].stuVideoFormat.nWidth;
		}
		
		if (pEncodeInfo->stuSnapFormat[i].stuVideoFormat.abHeight)
		{
			value["Encode"]["SnapFormat"][i]["Video"]["Height"] = pEncodeInfo->stuSnapFormat[i].stuVideoFormat.nHeight;
		}
		
		if (pEncodeInfo->stuSnapFormat[i].stuVideoFormat.abImageQuality)
		{
			value["Encode"]["SnapFormat"][i]["Video"]["Quality"] = (int)pEncodeInfo->stuSnapFormat[i].stuVideoFormat.emImageQuality;
		}	
	}
	
	// 通道标题		
	value["Encode"]["ChnTitle"]["FrontColor"]["Red"] = pEncodeInfo->stuChnTitle.stuFrontColor.nRed;
	value["Encode"]["ChnTitle"]["FrontColor"]["Green"] = pEncodeInfo->stuChnTitle.stuFrontColor.nGreen;
	value["Encode"]["ChnTitle"]["FrontColor"]["Blue"] = pEncodeInfo->stuChnTitle.stuFrontColor.nBlue;
	value["Encode"]["ChnTitle"]["FrontColor"]["Alpha"] = pEncodeInfo->stuChnTitle.stuFrontColor.nAlpha;
	
	value["Encode"]["ChnTitle"]["BackColor"]["Red"] = pEncodeInfo->stuChnTitle.stuBackColor.nRed;
	value["Encode"]["ChnTitle"]["BackColor"]["Green"] = pEncodeInfo->stuChnTitle.stuBackColor.nGreen;
	value["Encode"]["ChnTitle"]["BackColor"]["Blue"] = pEncodeInfo->stuChnTitle.stuBackColor.nBlue;
	value["Encode"]["ChnTitle"]["BackColor"]["Alpha"] = pEncodeInfo->stuChnTitle.stuBackColor.nAlpha;
	
	value["Encode"]["ChnTitle"]["Rect"]["Left"] = pEncodeInfo->stuChnTitle.stuRect.nLeft;
	value["Encode"]["ChnTitle"]["Rect"]["Top"] = pEncodeInfo->stuChnTitle.stuRect.nTop;
	value["Encode"]["ChnTitle"]["Rect"]["Right"] = pEncodeInfo->stuChnTitle.stuRect.nRight; 
	value["Encode"]["ChnTitle"]["Rect"]["Bottom"] = pEncodeInfo->stuChnTitle.stuRect.nBottom;
	
	if (pEncodeInfo->stuChnTitle.abShowEnable)
	{
		value["Encode"]["ChnTitle"]["Show"] = pEncodeInfo->stuChnTitle.bShowEnable;
	}
	
	// 时间标题	
	value["Encode"]["TimeTitle"]["FrontColor"]["Red"] = pEncodeInfo->stuTimeTitle.stuFrontColor.nRed;
	value["Encode"]["TimeTitle"]["FrontColor"]["Green"] = pEncodeInfo->stuTimeTitle.stuFrontColor.nGreen;
	value["Encode"]["TimeTitle"]["FrontColor"]["Blue"] = pEncodeInfo->stuTimeTitle.stuFrontColor.nBlue;
	value["Encode"]["TimeTitle"]["FrontColor"]["Alpha"] = pEncodeInfo->stuTimeTitle.stuFrontColor.nAlpha;
	
	value["Encode"]["TimeTitle"]["BackColor"]["Red"] = pEncodeInfo->stuTimeTitle.stuBackColor.nRed;
	value["Encode"]["TimeTitle"]["BackColor"]["Green"] = pEncodeInfo->stuTimeTitle.stuBackColor.nGreen;
	value["Encode"]["TimeTitle"]["BackColor"]["Blue"] = pEncodeInfo->stuTimeTitle.stuBackColor.nBlue;
	value["Encode"]["TimeTitle"]["BackColor"]["Alpha"] = pEncodeInfo->stuTimeTitle.stuBackColor.nAlpha;
	
	value["Encode"]["TimeTitle"]["Rect"]["Left"] = pEncodeInfo->stuTimeTitle.stuRect.nLeft;
	value["Encode"]["TimeTitle"]["Rect"]["Top"] = pEncodeInfo->stuTimeTitle.stuRect.nTop;
	value["Encode"]["TimeTitle"]["Rect"]["Right"] = pEncodeInfo->stuTimeTitle.stuRect.nRight; 
	value["Encode"]["TimeTitle"]["Rect"]["Bottom"] = pEncodeInfo->stuTimeTitle.stuRect.nBottom;
	
	if (pEncodeInfo->stuTimeTitle.abShowEnable)
	{
		value["Encode"]["TimeTitle"]["Show"] = pEncodeInfo->stuTimeTitle.bShowEnable;
	}
	
	// 图像颜色

	value["Encode"]["VideoColor"]["Brightness"] = pEncodeInfo->stuVideoColor.nBrightness;
	value["Encode"]["VideoColor"]["Contrast"] = pEncodeInfo->stuVideoColor.nContrast;
	value["Encode"]["VideoColor"]["Saturation"] = pEncodeInfo->stuVideoColor.nSaturation;
	value["Encode"]["VideoColor"]["Hue"] = pEncodeInfo->stuVideoColor.nHue;
	value["Encode"]["VideoColor"]["GainEn"] = pEncodeInfo->stuVideoColor.bGainEn;
	value["Encode"]["VideoColor"]["Gain"] = pEncodeInfo->stuVideoColor.nGain;
	
	//区域遮挡

	value["Encode"]["CoverType"] = (int)pEncodeInfo->dwCoverAbilityMask;
	value["Encode"]["CoverEn"] = (int)pEncodeInfo->dwCoverEnableMask;

	for (i = 0; i < pEncodeInfo->stuVideoCover.nCurBlocks; i++)
	{
		if (pEncodeInfo->stuVideoCover.stuCoverBlock[i].abBlockType)
		{
			value["Encode"]["CoverBlock"][i]["BlockType"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].nBlockType;
		}
		
		value["Encode"]["CoverBlock"][i]["Color"]["Red"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].stuColor.nRed;
		value["Encode"]["CoverBlock"][i]["Color"]["Green"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].stuColor.nGreen;
		value["Encode"]["CoverBlock"][i]["Color"]["Blue"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].stuColor.nBlue;
		value["Encode"]["CoverBlock"][i]["Color"]["Alpha"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].stuColor.nAlpha;
		
		value["Encode"]["CoverBlock"][i]["Rect"]["Left"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].stuRect.nLeft;
		value["Encode"]["CoverBlock"][i]["Rect"]["Top"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].stuRect.nTop;
		value["Encode"]["CoverBlock"][i]["Rect"]["Right"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].stuRect.nRight;
		value["Encode"]["CoverBlock"][i]["Rect"]["Bottom"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].stuRect.nBottom;
		
		if (pEncodeInfo->stuVideoCover.stuCoverBlock[i].abPreviewBlend)
		{
			value["Encode"]["CoverBlock"][i]["PreviewBlend"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].nPreviewBlend;
		}
		
		if (pEncodeInfo->stuVideoCover.stuCoverBlock[i].abEncodeBlend)
		{
			value["Encode"]["CoverBlock"][i]["EncodeBlend"] = pEncodeInfo->stuVideoCover.stuCoverBlock[i].nEncodeBlend;
		}
	}

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Encode_WaterMark_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;

	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_WATERMARK_INFO))
	{
		return bRet;
	}

	Json::Reader reader;
	static Json::Value value;

	CFG_WATERMARK_INFO stuWaterMarkInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);

	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["WaterMark"]["Chn"].type() != Json::nullValue)
		{
			stuWaterMarkInfo.nChannelID = value["WaterMark"]["Chn"].asInt();
		}

		if (value["WaterMark"]["En"].type() != Json::nullValue)
		{
			stuWaterMarkInfo.bEnable = value["WaterMark"]["En"].asInt();
		}

		if (value["WaterMark"]["Stream"].type() != Json::nullValue)
		{
			stuWaterMarkInfo.nStreamType = value["WaterMark"]["Stream"].asInt();
		}

		if (value["WaterMark"]["Key"].type() != Json::nullValue)
		{
			stuWaterMarkInfo.nDataType = value["WaterMark"]["Key"].asInt();
		}
		
		if (value["WaterMark"]["String"].type() != Json::nullValue)
		{
			int nStringLen = value["WaterMark"]["String"].asString().size();
			nStringLen = nStringLen>MAX_WATERMARK_LEN?MAX_WATERMARK_LEN:nStringLen;
			strncpy(stuWaterMarkInfo.pData, value["WaterMark"]["String"].asString().c_str(), nStringLen);
		}

		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_WATERMARK_INFO);
		}
		memcpy(lpOutBuffer, &stuWaterMarkInfo, sizeof(CFG_WATERMARK_INFO));
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}

BOOL Encode_WaterMark_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_WATERMARK_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_WATERMARK_INFO *pWaterMarkInfo = NULL;
	pWaterMarkInfo = (CFG_WATERMARK_INFO *)lpInBuffer;

	bRet = TRUE;
	
//	if (pWaterMarkInfo->abChannelID)
	{
		value["WaterMark"]["Chn"] = pWaterMarkInfo->nChannelID;
	}

	value["WaterMark"]["En"] = pWaterMarkInfo->bEnable;

	value["WaterMark"]["Stream"] = pWaterMarkInfo->nStreamType;
	
	value["WaterMark"]["Key"] = pWaterMarkInfo->nDataType;

	value["WaterMark"]["String"] = pWaterMarkInfo->pData;
	
	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Encode_SnapCap_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_SNAPCAPINFO_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_SNAPCAPINFO_INFO stuSnapCapInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["SnapInfo"]["Chn"].type() != Json::nullValue)
		{
			stuSnapCapInfo.nChannelID = value["SnapInfo"]["Chn"].asInt();
		}
		
		int i = 0;
		CStrParse parser;
		parser.setSpliter(":");
		bool bSuccess = true;

		if (value["SnapInfo"]["Size"].type() != Json::nullValue)
		{
			bSuccess = parser.Parse((char *)value["SnapInfo"]["Size"].asString().c_str());
			if (!bSuccess) 
			{
				return 0;
			}
			
			for(i = 0; i < parser.Size(); i++)
			{
				stuSnapCapInfo.dwIMageSizeNum = parser.Size();
				stuSnapCapInfo.emIMageSizeList[i] = (CFG_CAPTURE_SIZE)parser.getValue(i);
			}
		}

		if (value["SnapInfo"]["FPS"].type() != Json::nullValue)
		{
			bSuccess = parser.Parse((char *)value["SnapInfo"]["FPS"].asString().c_str());
			if (!bSuccess) 
			{
				return 0;
			}
			
			for(i = 0; i < parser.Size(); i++)
			{
				stuSnapCapInfo.dwFramesPerSecNum = parser.Size();
				stuSnapCapInfo.nFramesPerSecList[i] = parser.getValue(i);
			}
		}

		if (value["SnapInfo"]["Mode"].type() != Json::nullValue)
		{
			bSuccess = parser.Parse((char *)value["SnapInfo"]["Mode"].asString().c_str());
			if (!bSuccess) 
			{
				return 0;
			}
			
			for(i = 0; i < parser.Size(); i++)
			{
				stuSnapCapInfo.dwMode |= (0x01<<parser.getValue(i));
			}
		}

		if (value["SnapInfo"]["Format"].type() != Json::nullValue)
		{
			bSuccess = parser.Parse((char *)value["SnapInfo"]["Format"].asString().c_str());
			if (!bSuccess) 
			{
				return 0;
			}
			
			for(i = 0; i < parser.Size(); i++)
			{
				stuSnapCapInfo.dwFormat |= (0x01<<parser.getValue(i));
			}
		}

		if (value["SnapInfo"]["Quality"].type() != Json::nullValue)
		{
			bSuccess = parser.Parse((char *)value["SnapInfo"]["Quality"].asString().c_str());
			if (!bSuccess) 
			{
				return 0;
			}
			
			for(i = 0; i < parser.Size(); i++)
			{
				stuSnapCapInfo.dwQualityMun = parser.Size();
				stuSnapCapInfo.emQualityList[i] = (CFG_IMAGE_QUALITY)parser.getValue(i);
			}
		}

		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_SNAPCAPINFO_INFO);
		}
		memcpy(lpOutBuffer, &stuSnapCapInfo, sizeof(CFG_SNAPCAPINFO_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Encode_SnapCap_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	return FALSE;
}


CFG_VIDEO_COMPRESSION CompressionStrtoInt(char *szCompression)
{
	CFG_VIDEO_COMPRESSION emCompression = VIDEO_FORMAT_MPEG4;

	if (_stricmp(szCompression, "MPEG4") == 0)
	{
		emCompression = VIDEO_FORMAT_MPEG4;
	}
	else if (_stricmp(szCompression, "MS-MPEG4") == 0)
	{
		emCompression = VIDEO_FORMAT_MS_MPEG4;
	}
	else if (_stricmp(szCompression, "MPEG2") == 0)
	{
		emCompression = VIDEO_FORMAT_MPEG2;
	}
	else if (_stricmp(szCompression, "MPEG1") == 0)
	{
		emCompression = VIDEO_FORMAT_MPEG1;
	}
	else if (_stricmp(szCompression, "H.263") == 0)
	{
		emCompression = VIDEO_FORMAT_H263;
	}
	else if (_stricmp(szCompression, "MJPG") == 0)
	{
		emCompression = VIDEO_FORMAT_MJPG;
	}
	else if (_stricmp(szCompression, "FCC-MPEG4") == 0)
	{
		emCompression = VIDEO_FORMAT_FCC_MPEG4;
	}
	else if (_stricmp(szCompression, "H.264") == 0)
	{
		emCompression = VIDEO_FORMAT_H264;
	}	

	return emCompression;	
}

BOOL CompressionInttoStr(CFG_VIDEO_COMPRESSION emCompression, char *szValue)
{
	if (szValue == NULL)
	{
		return -1;
	}

	switch(emCompression)
	{
	case VIDEO_FORMAT_MPEG4:
		strcpy(szValue, "MPEG4");
		break;
	case VIDEO_FORMAT_MS_MPEG4:
		strcpy(szValue, "MS-MPEG4");
		break;
	case VIDEO_FORMAT_MPEG2:
		strcpy(szValue, "MPEG2");
	    break;
	case VIDEO_FORMAT_MPEG1:
		strcpy(szValue, "MPEG1");
	    break;
	case VIDEO_FORMAT_H263:
		strcpy(szValue, "H.263");
		break;
	case VIDEO_FORMAT_MJPG:
		strcpy(szValue, "MJPG");
		break;
	case VIDEO_FORMAT_FCC_MPEG4:
		strcpy(szValue, "FCC-MPEG4");
	    break;
	case VIDEO_FORMAT_H264:
		strcpy(szValue, "H.264");
	    break;
	default:
		strcpy(szValue, "H.264");
	    break;
	}
 	
	return 0;
}

CFG_BITRATE_CONTROL BitRateControlStrtoInt(char *szBitRateControl)
{
	CFG_BITRATE_CONTROL emBitRateControl = BITRATE_CBR;
	
	if (_stricmp(szBitRateControl, "CBR") == 0)
	{
		emBitRateControl = BITRATE_CBR;
	}
	else if (_stricmp(szBitRateControl, "VBR") == 0)
	{
		emBitRateControl = BITRATE_VBR;
	}
	
	return emBitRateControl;	
}

BOOL BitRateControlInttoStr(CFG_BITRATE_CONTROL emBitRateControl, char *szValue)
{
	if (szValue == NULL)
	{
		return -1;
	}

	switch(emBitRateControl)
	{
	case BITRATE_CBR:
		strcpy(szValue, "CBR");
		break;
	case BITRATE_VBR:
		strcpy(szValue, "VBR");
		break;
	default:
		strcpy(szValue, "CBR");
	    break;
	}
 	
	return 0;

}

int FrameTypeStrtoInt(char *szFrameType)
{
	int nFrameType = 0;
	
	if (_stricmp(szFrameType, YWAVSTR) == 0)
	{
		szFrameType = 0;
	}
	else if (_stricmp(szFrameType, "PS") == 0)
	{
		nFrameType = 1;
	}
	
	return nFrameType;

}

BOOL FrameTypeInttoStr(int nFrameType, char *szValue)
{
	if (szValue == NULL)
	{
		return -1;
	}

	switch(nFrameType)
	{
	case 0:
		strcpy(szValue, YWAVSTR);
		break;
	case 1:
		strcpy(szValue, "PS");
		break;
	default:
		strcpy(szValue, YWAVSTR);
	    break;
	}
 	
	return 0;
}
