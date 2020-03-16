#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>

#include "ros/ros.h"
#include "image_transport/image_transport.h"
#include "cv_bridge/cv_bridge.h"
#include "sensor_msgs/image_encodings.h"
#include "sensor_msgs/Image.h"
#include <opencv2/opencv.hpp>

#include <decoder_h264/decoder_h264_node.hpp>
#include <decoder_h264/H264Decoder.h>

using namespace std;

Mat g_frame;
int g_lPort = 0;
char* g_rgbBuffer = NULL;
unsigned char* g_yuvBuffer = NULL;
unsigned int g_len = 0;
std::string g_error_info_str;

long lUserID;
long lRealPlayHandle;

int flag = 0;
cv::Mat g_result_pic;
int g_get_image = 0;
H264Decoder h264_decoder;

void *display_image(void* args)
{
    while(true)
    {
        if(g_get_image == 1)
        {
            try{
                g_get_image = 0;
                cv::imshow("img", g_result_pic);
                cv::waitKey(30);
            }
            catch(const std::exception& e){
                std::cerr << e.what() << '\n';
            }
        }
    }
}

 void decoder_h264_node::getstream_callback(const sensor_msgs::Image& msg)
 {
    vector<unsigned char> vc;
    vc = msg.data;
    //for (int n=0;n<4;n++)
	//	            printf("  0x%02x\t%02x\n",vc[n],vc[n]);
    //std::cout << "strlen(pBuf):" << vc.size() << std::endl;

    unsigned char* pBuffer = &vc.at(0);
    int dwBufSize = vc.size();

    //for (int n=0;n<4;n++)
	//	printf("  0x%02x\t%02x\n",vc[n],vc[n]);
    //std::cout << "strlen(pBuf):" << vc.size() << std::endl;
    for (int n=0;n<4;n++)
        //printf("  %02x ", pBuffer[n]);
        printf("  0x%02x\t%02x\n",pBuffer+n,pBuffer[n]);
    std::cout << "strlen(pBuffer):" << dwBufSize << std::endl;

    //PS流数据解析处理		
    int nI = 0;
    int nCacheSize = 0;
    nI = m_mFrameCacheLenth;

    //直接--提取H264数据
    bool bVideo = false;
    bool bPatialData = false;

    bPatialData = get_h246_fromps(pBuffer,dwBufSize, &m_pFrameCache[nI].pCacheBuffer, 
        m_pFrameCache[nI].nCacheBufLenth, bVideo);

    if (bVideo)
    {
        if (bPatialData)//部分包数据
        {
            //缓存数据
            m_pFrameCache[nI].lTimeStamp = clock();
            m_mFrameCacheLenth++;
        } 
        else//包头
        {
            int i = 0;
            if(m_mFrameCacheLenth>0)
            {
                long lH264DataLenth = m_mFrameCacheLenth*MAX_PACK_SIZE;
                BYTE* pH264Nal =  NULL;
                pH264Nal = new BYTE[lH264DataLenth];
                memset(pH264Nal, 0x00, lH264DataLenth);
                BYTE* pTempBuffer = pH264Nal;
                int nTempBufLenth = 0;

                for (i=0; i < m_mFrameCacheLenth; i++)
                {
                    if(m_pFrameCache[i].pCacheBuffer!=NULL&&m_pFrameCache[i].nCacheBufLenth>0)
                    {
                        memcpy(pH264Nal+nTempBufLenth, m_pFrameCache[i].pCacheBuffer, 
                            m_pFrameCache[i].nCacheBufLenth);
                        nTempBufLenth += m_pFrameCache[i].nCacheBufLenth;
                    }
                    if (m_pFrameCache[i].pCacheBuffer)
                    {
                        delete [](m_pFrameCache[i].pCacheBuffer);
                        m_pFrameCache[i].pCacheBuffer = NULL;
                    }	
                    m_pFrameCache[i].nCacheBufLenth = 0; 
                }
                
                if (pH264Nal && nTempBufLenth>0)
                {
                    bool bIsKeyFrame = false;
                    //查找是否为关键帧
                    if(pH264Nal[4]==0x67)
                    {
                        bIsKeyFrame = true;
                    }
                    long lTimeStamp = clock();

                    try
                    {
                        if(pH264Nal[4] == 0x67)
                        {
                            printf("nTempBufLenth = %d ",nTempBufLenth);
                            h264_decoder.decode(pH264Nal, nTempBufLenth);
                            g_result_pic = h264_decoder.getMat();
                            g_get_image = 1;
                        }
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }

                if (pH264Nal)
                {
                    delete []pH264Nal;
                    pH264Nal = NULL;
                }
                // 缓存数据个数 清0
                m_mFrameCacheLenth = 0;
            }
        }
    }
 }

decoder_h264_node::decoder_h264_node()
{ 
    ros::NodeHandle private_node_handle("~");
    ros::NodeHandle node_handle_;

    heartbeat_pub_ = node_handle_.advertise<diagnostic_msgs::DiagnosticArray>("/decoder_h264/heartbeat", 1);
    getstream_sub_ = node_handle_.subscribe("/yida/internal/visible/image_proc", 1000, &decoder_h264_node::getstream_callback, this);

    for (int i = 0; i < MAX_DEV+1; i++)
	{
		if(i < MAX_DEV)
		{
			for(int nI=0; nI < MAX_FRAME_LENTH; nI++)
			{
				memset(&m_pFrameCache[nI], 0x00, sizeof(CacheBuffder));
			}
			m_mFrameCacheLenth = 0;
		}
	}

    create_display_thread();
}

decoder_h264_node::~decoder_h264_node()
{

}

void decoder_h264_node::create_display_thread()
{
    pthread_t ros_thread_display = 0;
    pthread_create(&ros_thread_display,NULL,display_image,NULL);
}

//从PS流中取H264数据
//返回值：是否为数据包 
bool decoder_h264_node::get_h246_fromps(BYTE* pBuffer, int nBufLenth, BYTE** pH264, int& nH264Lenth, bool& bVideo)
{
	if (!pBuffer || nBufLenth<=0)
	{
		return false;
	}

	BYTE* pH264Buffer = NULL;
	int nHerderLen = 0;
	if( pBuffer
		&& pBuffer[0]==0x00
		&& pBuffer[1]==0x00 
		&& pBuffer[2]==0x01
		&& pBuffer[3]==0xE0) //E==视频数据(此处E0标识为视频)
	{   
		bVideo = true;
		nHerderLen = 9 + (int)pBuffer[8];//9个为固定的数据包头长度，pBuffer[8]为填充头部分的长度
		pH264Buffer = pBuffer+nHerderLen;
		if (*pH264 == NULL)
		{
			*pH264 = new BYTE[nBufLenth];
		}
		if (*pH264&&pH264Buffer&&(nBufLenth-nHerderLen)>0)
		{	
			memcpy(*pH264, pH264Buffer, (nBufLenth-nHerderLen));
		}	
		nH264Lenth = nBufLenth-nHerderLen;
		return true;
	}	
	else if(pBuffer 
		&& pBuffer[0]==0x00
		&& pBuffer[1]==0x00
		&& pBuffer[2]==0x01
		&& pBuffer[3]==0xC0) //C==音频数据？
	{
		*pH264 = NULL;
		nH264Lenth = 0;
		bVideo = false;
	}
	else if(pBuffer 
		&& pBuffer[0]==0x00
		&& pBuffer[1]==0x00
		&& pBuffer[2]==0x01
		&& pBuffer[3]==0xBA) //视频流数据包 包头
	{
		bVideo = true;
		*pH264 = NULL;
		nH264Lenth = 0;
		return false;
	}
	return false;
}

void decoder_h264_node::update()
{
    //if(g_frame.empty())
    /*if(g_len == 0)
    {
        int level = 2;
        std::string message = g_error_info_str;
        std::string hardware_id = "visible_camera";
        pub_heartbeat(level, message, hardware_id);
    }
    else
    {
        int level = 0;
        std::string message = "visible camera is ok!";
        std::string hardware_id = "visible_camera_getstream";
        pub_heartbeat(level, message, hardware_id);
    }*/
}

void decoder_h264_node::pub_heartbeat(int level, string message, string hardware_id)
{
    diagnostic_msgs::DiagnosticArray log;
    log.header.stamp = ros::Time::now();
    
    diagnostic_msgs::DiagnosticStatus s;
    s.name = __app_name__;         // 这里写节点的名字
    s.level = level;               // 0 = OK, 1 = Warn, 2 = Error
    if (!message.empty())
    {
        s.message = message;       // 问题描述
    }
    s.hardware_id = hardware_id;   // 硬件信息
    log.status.push_back(s);

    heartbeat_pub_.publish(log);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, __app_name__);
    decoder_h264_node hsdkcap;
    ros::Rate rate(40);

    while (ros::ok())
    {
        hsdkcap.update();
        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}
