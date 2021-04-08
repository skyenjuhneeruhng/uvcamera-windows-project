#include <windows.h>
#include <iostream>
#include <strmif.h>
#include <vidcap.h>
#include <dshow.h>
#include <ks.h>
#include <ksproxy.h>
#include <ksmedia.h>
#include <uuids.h>
#include <stdio.h>

DEFINE_GUIDSTRUCT("A29E7641-DE04-47E3-8B2B-F4341AFF003B", PROPSETID_XU_H264);
#define PROPSETID_XU_H264 DEFINE_GUIDNAMED(PROPSETID_XU_H264)

DEFINE_GUIDSTRUCT("3C16A808-701C-49C7-AF98-2EC6C125B963", PROPSETID_XU_PLUG_IN);
#define PROPSETID_XU_PLUG_IN DEFINE_GUIDNAMED(PROPSETID_XU_PLUG_IN)

//DEFINE_GUIDSTRUCT("A7A6A934-3A4A-4351-85F4-042EF2CDCAAB", PROPSETID_XU_PLUG_IN_1700U);
//#define PROPSETID_XU_PLUG_IN_1700U DEFINE_GUIDNAMED(PROPSETID_XU_PLUG_IN_1700U)
DEFINE_GUIDSTRUCT("661DCD8C-ADB0-B344-8CB4-D335115FC18C", PROPSETID_XU_PLUG_IN_1700U);
#define PROPSETID_XU_PLUG_IN_1700U DEFINE_GUIDNAMED(PROPSETID_XU_PLUG_IN_1700U)

//some camera pid
#define USB_PID_V61U (0x0605)
#define USB_PID_V60U (0x0606)
#define USB_PID_1702C_84 (0x0735)  
#define USB_PID_1702C_120 (0x0737) //PTZ Pro 2
#define USB_PID_1700U_120 (0x1000)


#ifndef ASSERT
#define ASSERT(_x_) ((void)0)
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }
#endif

typedef enum {
	UVC_XU_PLUG_IN_FLIP_CTRL = 1,
	UVC_XU_PLUG_IN_OSD_RESTTING_CTRL,
	UVC_XU_PLUG_IN_SAVE_STTING_CTRL,
	UVC_XU_PLUG_IN_PRESET_CTRL,
	UVC_XU_PLUG_IN_SLEEP_CTRL,
	UVC_XU_PLUG_IN_RESET_PAN_TILT,
	UVC_XU_PLUG_IN_RED,
	UVC_XU_PLUG_IN_GREEN,
	UVC_XU_PLUG_IN_BLUE,
	UVC_XU_PLUG_IN_2D,
}uvc_xu_plug_in_id_t;


typedef enum {
	UVC_1700U_XU_PLUG_IN_LDC = 3,
	UVC_1700U_XU_PLUG_IN_FOCUSA,
	UVC_1700U_XU_PLUG_IN_ZOOMLIMT,
	UVC_1700U_XU_PLUG_STATUS = 8,
	UVC_1700U_XU_PLUG_INFO,

}uvc_1700u_xu_plug_in_id_t;


typedef enum {
	UVC_1702C_XU_PLUG_CTRL_UP = 1,
	UVC_1702C_XU_PLUG_CTRL_DOWN,
	UVC_1702C_XU_PLUG_CTRL_LEFT,
	UVC_1702C_XU_PLUG_CTRL_RIGHT,
	UVC_1702C_XU_PLUG_CTRL_OK,
	UVC_1702C_XU_PLUG_CTRL_BACK,
	UVC_1702C_XU_PLUG_CTRL_OPEN_CLOSE,

}uvc_17002c_xu_plug_in_id_t;


class UVCXU// : public IKsNodeControl
{
protected:
	GUID    UvcXuGudi;
	ULONG m_dwNodeId;
	IKsControl* m_pKsControl = NULL;

	//IKsPropertySet *m_pIKsPropertySet = NULL;

public:

	UVCXU();
	virtual ~UVCXU();

	HRESULT
		QueryUvcXuInterface(
			IBaseFilter* pDeviceFilter,
			GUID extensionGuid,
			DWORD FirstNodeId);

	HRESULT FinalConstruct();

	HRESULT get_Info(
		ULONG ulSize,
		BYTE pInfo[]);
	HRESULT get_InfoSize(
		ULONG* pulSize);
	HRESULT get_PropertySize(
		ULONG PropertyId,
		ULONG* pulSize);
	HRESULT get_Property(
		ULONG PropertyId,
		ULONG ulSize,
		BYTE pValue[]);
	HRESULT put_Property(
		ULONG PropertyId,
		ULONG ulSize,
		BYTE pValue[]);
	HRESULT get_PropertyRange(
		ULONG PropertyId,
		ULONG ulSize,
		BYTE pMin[],
		BYTE pMax[],
		BYTE pSteppingDelta[],
		BYTE pDefault[]);
};
