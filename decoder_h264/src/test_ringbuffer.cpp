// RST_CSDK_Demo.cpp : Defines the entry point for the console application
//

#include <stdlib.h>
#include <stdio.h>
#include "../RST_CSDK/RST_CSDK.h"
#include "H264Decoder.h"
#include <fstream>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <string.h>
#endif // WIN32
#include <iostream>
#include <pthread.h>
#include "opencv2/opencv.hpp"
#include "ring_buffer.h"

unsigned char buf[700000];//should be max enough
bool decodeflag=false;
unsigned char framedata[700000000];
int framelen=1280*720*3;
pthread_mutex_t frameLocker;
int decodelen=0;
circular_buffer <unsigned char*>circle(1280*720*300);

void *UpdateFrame(void *args)
{
	// 初始化SDK
	int nRet = RSTC_Initialize();
	if (nRet != E_CSDK_OK)
	{
		printf("initialize error, %d\n", nRet);
		return 0;
	}

	H_SERVER hServer = NULL;

	do 
	{
		// 登录
		nRet = RSTC_Login("192.168.1.101", 7178, "admin", "", &hServer);
		if (nRet != E_CSDK_OK)
		{
			printf("login error, %d\n", nRet);
			break;
		}

		// 获取信息
		nRet = RSTC_Fetch(hServer);
		if (nRet != E_CSDK_OK)
		{
			printf("fetch error, %d\n", nRet);
			break;
		}

		// 获取设备列表
		int nOffset = 0;
		int nCount = 200;
		H_RES hPUArray[200] = { NULL };
		nRet = RSTC_GetPUList(hServer, hPUArray, &nCount, nOffset);
		if (nRet != E_CSDK_OK)
		{
			printf("get pu list error, %d\n", nRet);
			break;
		}

		H_RES hCameraRes = NULL;

		for (int i = 0; i < nCount; i++)
		{
			H_RES hPU = hPUArray[i];

			char pszName[MAX_STR_LEN] = { 0 };
			RSTC_COMMON_GetName(hPU, pszName);

			char pszPUID[MAX_STR_LEN] = { 0 };
			RSTC_COMMON_GetPUID(hPU, pszPUID);
			printf("pu Name:%s, PUID:%s\n", pszName, pszPUID);

			// 获取设备子资源
			H_RES hChildResArray[100] = { NULL };
			int nChildCount = 100;
			int nChildnOffset = 0;
			nRet = RSTC_COMMON_GetChildren(hPU, hChildResArray, &nChildCount, nChildnOffset);
			if (nRet == E_CSDK_OK)
			{
				for (int j=0; j< nChildCount; j++)
				{
					H_RES hRes = hChildResArray[j];

					RSTC_COMMON_GetName(hRes, pszName);
					
					unsigned int uiResType = 0;
					RSTC_COMMON_GetResType(hRes, &uiResType);

					unsigned int uiResChannel = 0;
					RSTC_COMMON_GetResChannel(hRes, &uiResChannel);

					printf("pu Name:%s, type:%u, channel:%u\n", pszName, uiResType, uiResChannel);

					if (uiResType == RST_RES_TYPE_CAM)
					{
						hCameraRes = hRes;
						break;
					}
				}
			}

			// 这里只是示例，实际需要根据具体业务来确定所需获取视频的摄像头资源
			if (hCameraRes != NULL)
			{
				break;
			}
		}

		if (hCameraRes == NULL)
		{
			printf("no camera\n");
			break;
		}

		printf("get one camera res\n");

		// 开启流
		nRet = RSTC_StartStream(hCameraRes);
		if (nRet != E_CSDK_OK)
		{
			printf("start stream error, %d\n", nRet);
			break;
		}

#ifdef WIN32
		FILE *pFile = fopen("c:\\teststream.h264", "wb+");
#else
		FILE *pFile = fopen("./outputFrame.h264", "w");
		
#endif // WIN32

		// 接收数据
		unsigned char *pFrame = new unsigned char[3 * 1280 * 720];
		unsigned int uiFrameLen = 3 * 1280 * 720;
		int uiFrameType = 0;
		int uiKeyFrame = 0;
		unsigned int uiTimestamp = 0;
		unsigned int uiAlg = 0;
		while (true)
		{
			uiFrameLen = 3 * 1280 * 720;
			time_t t0=clock();
			nRet = RSTC_RecvFrame(hCameraRes, pFrame, &uiFrameLen, &uiFrameType, &uiKeyFrame, &uiTimestamp, &uiAlg);
			if (nRet == E_CSDK_OK)
			{
				//printf("11111111\n");
				//pthread_mutex_lock(&frameLocker);
				if(!circle.full())
				{
				    circle.put(pFrame);
				    //printf("uiFrameLen=%d\n",uiFrameLen);
				}
				else
				{
				   printf("#############circle is full#################\n");
				}
				//circle.put(pFrame);	
				//memcpy(&framedata,pFrame,uiFrameLen);
				//pthread_mutex_unlock(&frameLocker);
			}
			else if (nRet == E_CSDK_WOULD_BLOCK)
			{
				// 表示没有收到数据，需要继续接收，可以等一会
#ifdef WIN32
				Sleep(5);
#else
				usleep(5*1000);
				//printf("22222222\n");
#endif // WIN32

			}
			else
			{
				printf("recv fram error, %d\n", nRet);
				break;
			}
			time_t t1=clock();
			decodelen=uiFrameLen;
			//printf("receive cost %f s\n",(t1-t0));
		}

		if (pFile != NULL)
		{
			fclose(pFile);
			pFile = NULL;
		}
		
		// 停流
		RSTC_StopStream(hCameraRes);
	} while (false);

	if (hServer != NULL)
	{
		// 退出登录
		RSTC_Logout(hServer);
		hServer = NULL;
	}

	RSTC_Terminate();

    return 0;
}



int main()
{
   pthread_t data_thread;
   pthread_create(&data_thread,NULL,UpdateFrame,NULL);
   pthread_mutex_init(&frameLocker,NULL);
   H264Decoder decoder;
   unsigned char *currentdata;
   int copytime=0;
   while(1)
   {
	//currentdata=&framedata[0];
	if(!circle.empty())
	{
	    currentdata=circle.get();
            decoder.decode(currentdata,decodelen);
            cv::Mat image=decoder.getMat();
        	if(image.empty()){
        	    //printf("recieved empty frame\n");
        	    continue;
        	}
        	copytime+=1;
        	cv::imshow("decode",image);
        	cv::waitKey(1);
	}
   }
   return 0;
}
