#ifndef __UVCCAMERALIBRARY_H__
#define __UVCCAMERALIBRARY_H__

#include "UVCUX.h"

#ifdef UVCCAMERALIBRARY_EXPORTS
#define UVCCAMERAAPI __declspec(dllexport)
#else
#define UVCCAMERAAPI __declspec(dllimport)
#endif
#pragma once

#define MAX_DEVICE_COUNT 128
#define MAX_USB_DESCRIPTOR_LENGTH 256

typedef struct {
	long Min;
	long Max;
	long Default;
}uvc_ranges_t;

const uvc_ranges_t DEFAULT_RANGES = {
		0,
		0,
		0
};

class UVCCAMERAAPI UVCCameraLibrary
{
public:
	UVCCameraLibrary();
	~UVCCameraLibrary();

	//static method to list video connected capture devices
	static void listDevices(char** devices, int& nDevices);

	//connect to device with device name
	bool connectDevice(char* deviceName);

	//disconnect device
	void disconnectDevice();

	//camera control functions
	//pan,tilt
	HRESULT movePanOneLeft(int pan);
	HRESULT movePanOneRight(int pan);
	HRESULT moveTiltOneTop(int tilt);
	HRESULT moveTiltOneBottom(int tilt);
	//zoom
	HRESULT zoomOneIn(int zoom);
	HRESULT zoomOneOut(int zoom);
	//focus
	HRESULT focusOneIn(int focus);
	HRESULT focusOneOut(int focus);
	//home
	HRESULT moveHome();

	//set auto/manual of focus
	HRESULT setAutoFocus(bool af);
	//stop moving, zooming, focusing
	HRESULT stopMoving();
	HRESULT stopZooming();
	HRESULT stopFocusing();

	//get focus status(Auto/Manual)
	bool getAutoFocus();

	//get camera properties
	long getPan();
	long getTilt();
	long getZoom();
	long getFocus();

	/*OSD menu tool*/
	HRESULT osdMenuOpenClose();
	HRESULT osdMenuEnter();
	HRESULT osdMenuBack();
	HRESULT osdMenuUp();
	HRESULT osdMenuDown();
	HRESULT osdMenuLeft();
	HRESULT osdMenuRight();

	//This function must be called after connection to use other functions for osd menu
	HRESULT checkOSDMenu();//check if the uvc camera supports osd menu
private:

	//base directshow filter
	IBaseFilter* pDeviceFilter = NULL;

	// to select a video input device
	ICreateDevEnum* pCreateDevEnum = NULL;
	IEnumMoniker* pEnumMoniker = NULL;
	IMoniker* pMoniker = NULL;
	ULONG nFetched = 0;
	void getEnumMoniker();

	//change the property of camera according to @prop
	HRESULT moveCamera(long prop, int step);
	HRESULT moveCameraPan(int pan);
	HRESULT moveCameraTilt(int tilt);
	HRESULT moveCameraZoom(int zoom);
	HRESULT moveCameraFocus(int focus);
	//stop change of the property
	HRESULT stopControling(long prop);

	//get Auto/Manual status of property
	bool getAuto(long prop);
	HRESULT setAuto(long prop, bool autoEnabled);

	//get value of the property
	long getVal(long prop);

	//the range of the pan, tilt, zoom
	uvc_ranges_t panConRange = DEFAULT_RANGES;
	uvc_ranges_t panAbsRange = DEFAULT_RANGES;

	uvc_ranges_t tiltConRange = DEFAULT_RANGES;
	uvc_ranges_t tiltAbsRange = DEFAULT_RANGES;

	uvc_ranges_t zoomConRange = DEFAULT_RANGES;
	uvc_ranges_t zoomAbsRange = DEFAULT_RANGES;

	uvc_ranges_t focusConRange = DEFAULT_RANGES;
	uvc_ranges_t focusAbsRange = DEFAULT_RANGES;


	//uvc controllability
	bool uvcEnabled = true;

	//continuous movement capability
	bool panConEnabled = true;
	bool panAbsEnabled = true;

	bool tiltConEnabled = true;
	bool tiltAbsEnabled = true;

	bool zoomConEnabled = true;
	bool zoomAbsEnabled = true;

	bool focusConEnabled = true;
	bool focusAbsEnabled = true;

	//get ptz control ranges
	bool getPropRanges(long prop, uvc_ranges_t& range, IAMCameraControl* pCamControl);//returns true if the device is continuous control available
	void getPanRanges(IAMCameraControl* pCamControl);
	void getTiltRanges(IAMCameraControl* pCamControl);
	void getZoomRanges(IAMCameraControl* pCamControl);
	void getFocusRanges(IAMCameraControl* pCamControl);
	HRESULT getControlRanges();


	//get pid and vid
	void getCameraInfo(TCHAR* devPath);
	ULONG productId = 0;
	ULONG vendorId = 0;

	/*OSD Menu*/
	UVCXU *uvcxu;
	ULONG ulUvcRedSize = 0;
	ULONG ulUvcGreenSize = 0;
	ULONG ulUvcBlueSize = 0;
	ULONG ulUvc2dSize = 0;

};



#endif
