#include <iostream>
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CompressedImage.h>
#include <sensor_msgs/image_encodings.h>
#include <diagnostic_msgs/DiagnosticArray.h>

using namespace cv;
using namespace std;

#define __app_name__ "decoder_h264_node"
#define MAX_PACK_SIZE 5120
#define MAX_FRAME_LENTH 64
#define MAX_DEV 1

typedef  unsigned int       DWORD;
typedef  unsigned short     WORD;
typedef  unsigned short     USHORT;
typedef  short              SHORT;
typedef  int                LONG;
typedef  unsigned char      BYTE;
typedef  unsigned int       UINT;
typedef  void*              LPVOID;
typedef  void*              HANDLE;
typedef  unsigned int*      LPDWORD; 
typedef  unsigned long long UINT64;
typedef  signed long long   INT64;

typedef struct tagCacheBuffder
{
	BYTE* pCacheBuffer;
	int	  nCacheBufLenth;//最大包长度为5120
	long  lTimeStamp;
	DWORD  dwFrameLenth;

}CacheBuffder;

class decoder_h264_node
{
private:
    ros::Publisher heartbeat_pub_;
    ros::Subscriber getstream_sub_;

    int m_mFrameCacheLenth;
    CacheBuffder m_pFrameCache[MAX_FRAME_LENTH];

public:
    decoder_h264_node();
    ~decoder_h264_node();
    void update();
    void pub_heartbeat(int level, string message, string hardware_id);

    void getstream_callback(const sensor_msgs::Image& msg);
    void create_display_thread();

    //void receive_real_data(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* dwUser);
    bool get_h246_fromps(BYTE* pBuffer, int nBufLenth, BYTE** pH264, int& nH264Lenth, bool& bVideo);
};
