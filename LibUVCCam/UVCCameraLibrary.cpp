#include "stdafx.h"
#include "UVCCameraLibrary.h"


/********************************************************Implementation of UVCCameraLibrary class************************************************/

/*
* Constructor of the class
*/
UVCCameraLibrary::UVCCameraLibrary()
{
	// initialize COM
	uvcxu = new UVCXU();
	if (!SUCCEEDED(CoInitialize(0)))
		printf("Coinitialization failed!\n");
}

/*
* Destructor of the class
*/
UVCCameraLibrary::~UVCCameraLibrary()
{
	// release directshow class instances
	if (pEnumMoniker != NULL)
		pEnumMoniker->Release();
	if (pCreateDevEnum != NULL)
		pCreateDevEnum->Release();
	delete uvcxu;
	disconnectDevice();
	// finalize COM
	CoUninitialize();
}

/*
* static function
* List connected devices
* @cameraNames : (out) name list of connected cameras
* @nDevices : (out) the number of connected cameras
*/
void UVCCameraLibrary::listDevices(char** cameraNames, int& nDevices)
{
	nDevices = 0;
	IBaseFilter* pDeviceFilter = NULL;
	// to select a video input device
	ICreateDevEnum* pCreateDevEnum = NULL;
	IEnumMoniker* pEnumMoniker = NULL;
	IMoniker* pMoniker = NULL;
	ULONG nFetched = 0;
	// initialize COM
	if (!SUCCEEDED(CoInitialize(0)))
	{
		printf("Coinitialization failed - list device\n");
		return;
	}
	// Create CreateDevEnum to list device
	if (!SUCCEEDED(CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (PVOID*)&pCreateDevEnum)))
	{
		printf("cocreateinstance failed - list device\n");
		return;
	}

	// Create EnumMoniker to list VideoInputDevice 
	// CLSID_VideoInputDeviceCategory: allows to enum only video input devices
	pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
		&pEnumMoniker, 0);
	if (pEnumMoniker == NULL) {
		// this will be shown if there is no capture device
		printf("no device\n");
		return;
	}

	// reset EnumMoniker
	pEnumMoniker->Reset();

	while (pEnumMoniker->Next(1, &pMoniker, &nFetched) == S_OK) {

		//real name of the camera without suffix
		char cameraRealNames[MAX_DEVICE_COUNT][MAX_USB_DESCRIPTOR_LENGTH];

		IPropertyBag* pPropertyBag;
		//unique name with suffix 
		TCHAR devname[MAX_USB_DESCRIPTOR_LENGTH];

		// bind to IPropertyBag
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
			(void**)&pPropertyBag);

		VARIANT var;
		// get FriendlyName
		var.vt = VT_BSTR;
		if (SUCCEEDED(pPropertyBag->Read(L"FriendlyName", &var, 0)))//device name
		{
			/*
			* "FriendlyName"  The name of the device.VT_BSTR
			* "Description"   A description of the device.VT_BSTR
			* "DevicePath"    A unique string that identifies the device. (Video capture devices only.)   VT_BSTR
			* "WaveInID"  The identifier for an audio capture device. (Audio capture devices only.)   VT_I4
			*/
			WideCharToMultiByte(CP_ACP, 0,
				var.bstrVal, -1, devname, sizeof(devname), 0, 0);
			VariantClear(&var);
			int nSameNamedDevices = 0;
			for (int j = 0; j < nDevices; j++)
			{
				if (strcmp(cameraRealNames[j], devname) == 0)
					nSameNamedDevices++;
			}
			strcpy_s(cameraRealNames[nDevices], devname);
			//if there are the same type of cameras 
			//need to add some suffixes to identify the camera
			//first camera has no prefix
			//suffix [name] #[index] (e.g. PTZOptics Camera #1)
			if (nSameNamedDevices > 0)
				sprintf_s(devname, "%s #%d", devname, nSameNamedDevices);
			strcpy_s(cameraNames[nDevices], sizeof(devname), (TCHAR*)devname);

			// release
			pMoniker->Release();
			pPropertyBag->Release();

			nDevices++;
		}
		else
		{
			printf("Unable to get the device name\n");
		}
		
	}
	// release
	pEnumMoniker->Release();
	pCreateDevEnum->Release();

	// finalize COM
	CoUninitialize();
}

/*
* get moniker enum
* one moniker corresponds to one camera
*/
void UVCCameraLibrary::getEnumMoniker()
{
	// Create CreateDevEnum to list device
	if (!SUCCEEDED(CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (PVOID*)&pCreateDevEnum)))
	{
		printf("cocreateinstance failed - get enum moniker\n");
		return;
	}

	// Create EnumMoniker to list VideoInputDevice 
	pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
		&pEnumMoniker, 0);
	if (pEnumMoniker == NULL) {
		// this will be shown if there is no capture device
		printf("no device\n");
		return;
	}

	// reset EnumMoniker
	pEnumMoniker->Reset();
}

/*
* connect camera
* @deviceName : (in) camera name defined in function listDevices
* @return : true when connected successfully false if failes
*/
bool UVCCameraLibrary::connectDevice(char* deviceName)
{
	getEnumMoniker();

	if (pEnumMoniker == NULL)
		return false;

	int nDevices = 0;
	while (pEnumMoniker->Next(1, &pMoniker, &nFetched) == S_OK) {

		//we need real name without suffix to connect to camera
		char cameraRealNames[MAX_DEVICE_COUNT][MAX_USB_DESCRIPTOR_LENGTH];

		IPropertyBag* pPropertyBag;
		TCHAR devname[MAX_USB_DESCRIPTOR_LENGTH];

		// bind to IPropertyBag
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
			(void**)&pPropertyBag);

		VARIANT var;
		// get FriendlyName
		var.vt = VT_BSTR;
		if (SUCCEEDED(pPropertyBag->Read(L"FriendlyName", &var, 0)))
		{
			WideCharToMultiByte(CP_ACP, 0,
				var.bstrVal, -1, devname, sizeof(devname), 0, 0);
			VariantClear(&var);
			int nSameNamedDevices = 0;
			for (int j = 0; j < nDevices; j++)
			{
				if (strcmp(cameraRealNames[j], devname) == 0)
					nSameNamedDevices++;
			}
			strcpy_s(cameraRealNames[nDevices], devname);

			if (nSameNamedDevices > 0)
				sprintf_s(devname, "%s #%d", devname, nSameNamedDevices);

			//printf("devname %s , devicename %s\n", devname, deviceName);

			if (strcmp(devname, deviceName) == 0)
			{
				pMoniker->BindToObject(0, 0, IID_IBaseFilter,
					(void**)&pDeviceFilter);
				if (pDeviceFilter != NULL)
				{
					//this is the camera which we are going to connect
					TCHAR devPath[MAX_USB_DESCRIPTOR_LENGTH];
					VARIANT var1;

					// get Device Path
					var1.vt = VT_BSTR;
					//virtual cameras do not have devicepath information and it occurs error when read devicepath
					//need to check the return value of read function
					if (SUCCEEDED(pPropertyBag->Read(L"DevicePath", &var1, 0)))
					{
						WideCharToMultiByte(CP_ACP, 0,
							var1.bstrVal, -1, devPath, sizeof(devPath), 0, 0);
						VariantClear(&var1);
						printf("device path %s\n", devPath);
						//get product id
						getCameraInfo(devPath);
					}

					//get control ranges
					getControlRanges();

					return true;
				}
			}

			// release
			pMoniker->Release();
			pPropertyBag->Release();

			nDevices++;
		}
		else
		{
			printf("Unable to get the device name\n");
		}

	}
	return false;
}

void UVCCameraLibrary::disconnectDevice()
{
	vendorId = 0;
	productId = 0;
	uvcEnabled = true;
	panConEnabled = true;
	tiltConEnabled = true;
	zoomConEnabled = true;
	focusConEnabled = true;

	//release directshow filter
	if (pDeviceFilter != NULL)
		pDeviceFilter->Release();
	pDeviceFilter = NULL;
}

int UVCCameraLibrary::preProcessConVal(int propVal, uvc_ranges_t defaultRange)
{
	if (abs(propVal) > defaultRange.Max)
		propVal = defaultRange.Max * ((propVal > 0) ? 1 : -1);
	else if (abs(propVal) < defaultRange.Min)
		propVal = defaultRange.Min * ((propVal > 0) ? 1 : -1);
	return propVal;
}
int UVCCameraLibrary::preProcessAbsVal(int propVal, uvc_ranges_t defaultRange)
{
	if (propVal > defaultRange.Max)
		propVal = defaultRange.Max;
	else if (propVal < defaultRange.Min)
		propVal = defaultRange.Min;
	return propVal;
}

HRESULT UVCCameraLibrary::moveCameraPan(int pan)
{
	HRESULT hr = E_FAIL;
	if (panConEnabled)
	{
		int panVal = preProcessConVal(pan, panConRange);
		hr = moveCamera(KSPROPERTY_CAMERACONTROL_PAN_RELATIVE, panVal);
	}
	else if (panAbsEnabled)
	{
		long panVal = getPan() + pan;
		panVal = preProcessAbsVal(panVal, panAbsRange);
		hr = moveCamera(KSPROPERTY_CAMERACONTROL_PAN, panVal);
	}
	return hr;
}

HRESULT UVCCameraLibrary::moveCameraTilt(int tilt)
{
	HRESULT hr = E_FAIL;
	if (tiltConEnabled)
	{
		int tiltVal = preProcessConVal(tilt, tiltConRange);
		hr = moveCamera(KSPROPERTY_CAMERACONTROL_TILT_RELATIVE, tiltVal);
	}
	else if (tiltAbsEnabled)
	{
		long tiltVal = getTilt() + tilt;
		tiltVal = preProcessAbsVal(tiltVal, tiltAbsRange);
		hr = moveCamera(KSPROPERTY_CAMERACONTROL_TILT, tiltVal);
	}
	return hr;
}
HRESULT UVCCameraLibrary::moveCameraZoom(int zoom)
{
	HRESULT hr = E_FAIL;
	if (zoomConEnabled)
	{
		int zoomVal = preProcessConVal(zoom, zoomConRange);
		hr = moveCamera(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE, zoomVal);
	}
	else if (zoomAbsEnabled)
	{
		long zoomVal = getZoom() + zoom;
		zoomVal = preProcessAbsVal(zoomVal, zoomAbsRange);
		hr = moveCamera(KSPROPERTY_CAMERACONTROL_ZOOM, zoomVal);
	}
	return hr;
}
HRESULT UVCCameraLibrary::moveCameraFocus(int focus)
{
	HRESULT hr = E_FAIL;
	if (focusConEnabled)
	{
		int focusVal = preProcessConVal(focus, focusConRange);
		hr = moveCamera(KSPROPERTY_CAMERACONTROL_FOCUS_RELATIVE, focusVal);
	}
	else if (focusAbsEnabled)
	{
		long focusVal = getFocus() + focus;
		focusVal = preProcessAbsVal(focusVal, focusAbsRange);
		hr = moveCamera(KSPROPERTY_CAMERACONTROL_FOCUS, focusVal);
	}
	return hr;
}
/*
* move pan to left one step
* @pan: (in) step of the panning
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::movePanOneLeft(int pan)
{
	return moveCameraPan(-pan);
}
/*
* move pan to right one step
* @pan: (in) step of the panning
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::movePanOneRight(int pan)
{
	return moveCameraPan(pan);
}
/*
* move tilt to top one step
* @tilt: (in) step of the tilting
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::moveTiltOneTop(int tilt)
{
	return moveCameraTilt(tilt);
}
/*
* move tilt to bottom one step
* @tilt: (in) step of the tilting
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::moveTiltOneBottom(int tilt)
{
	return moveCameraTilt(-tilt);
}

/*
* zoom in one step
* @tilt: (in) step of the zooming
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::zoomOneIn(int zoom)
{
	return moveCameraZoom(zoom);
}
/*
* zoom out one step
* @tilt: (in) step of the zooming
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::zoomOneOut(int zoom)
{
	return moveCameraZoom(-zoom);
}
/*
* focus in one step
* does work if focus mode is set as Auto
* @tilt: (in) step of the focusing
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::focusOneIn(int focus)
{
	return moveCameraFocus(focus);
}
/*
* focus out one step
* does work if focus mode is set as Auto
* @tilt: (in) step of the focusing
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::focusOneOut(int focus)
{
	return moveCameraFocus(-focus);
}
/*
* set fucus mode
* @af: if true set as Auto otherwise set as Manual
* @return: if success returns S_OK
*/
HRESULT UVCCameraLibrary::setAutoFocus(bool af)
{
	return setAuto(KSPROPERTY_CAMERACONTROL_FOCUS, af);
}

/*
* change the property of the camera
* @prop: (in) property e.g. KSPROPERTY_CAMERACONTROL_PAN_RELATIVE, KSPROPERTY_CAMERACONTROL_TILT_RELATIVE, KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE ...
* Use KSPROPERTIES for continuous movement
* @return: if success returns S_OK
*/
HRESULT UVCCameraLibrary::moveCamera(long prop, int step)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;
	IAMCameraControl* pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");
	}
	else
	{
		hr = pCameraControl->Set(prop, step, KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}
//move home
HRESULT UVCCameraLibrary::moveHome()
{
	HRESULT hr = E_FAIL;
	hr = moveAbsPTZ(panAbsRange.Default, tiltAbsRange.Default, zoomAbsRange.Default);
	hr = setAutoFocus(true);
	return hr;
}

HRESULT UVCCameraLibrary::moveAbsPTZ(int pan, int tilt, int zoom)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;
	IAMCameraControl* pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");

	}
	else
	{
		//zoom should be the first since the digital ptz does not work before zooming
		if (zoomAbsEnabled)
			hr = pCameraControl->Set(KSPROPERTY_CAMERACONTROL_ZOOM, preProcessAbsVal(zoom, zoomAbsRange), KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);	

		if (panAbsEnabled)
			hr = pCameraControl->Set(KSPROPERTY_CAMERACONTROL_PAN, preProcessAbsVal(pan, panAbsRange), KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);

		if(tiltAbsEnabled)
			hr = pCameraControl->Set(KSPROPERTY_CAMERACONTROL_TILT, preProcessAbsVal(tilt, tiltAbsRange), KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}
bool UVCCameraLibrary::getAutoFocus()
{
	return getAuto(KSPROPERTY_CAMERACONTROL_FOCUS);
}
long UVCCameraLibrary::getPan()
{
	return getVal(KSPROPERTY_CAMERACONTROL_PAN);
}
long UVCCameraLibrary::getTilt()
{
	return getVal(KSPROPERTY_CAMERACONTROL_TILT);
}
long UVCCameraLibrary::getZoom()
{
	return getVal(KSPROPERTY_CAMERACONTROL_ZOOM);
}
long UVCCameraLibrary::getFocus()
{
	return getVal(KSPROPERTY_CAMERACONTROL_FOCUS);
}

HRESULT UVCCameraLibrary::setAuto(long prop, bool autoEnabled)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return hr;
	IAMCameraControl* pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");
	}
	else
	{
		long Val, Flags;
		hr = pCameraControl->Get(prop, &Val, &Flags);
		if (SUCCEEDED(hr))
		{
			if (autoEnabled)
				hr = pCameraControl->Set(prop, Val, KSPROPERTY_CAMERACONTROL_FLAGS_AUTO);
			else
				hr = pCameraControl->Set(prop, Val, KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);
		}
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}

bool UVCCameraLibrary::getAuto(long prop)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return false;

	bool autoEnabled = false;
	IAMCameraControl* pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");
	}
	else
	{
		long Flags, Val;

		// Get the range and default values 
		hr = pCameraControl->Get(prop, &Val, &Flags);
		if (SUCCEEDED(hr))
		{
			if (Flags == KSPROPERTY_CAMERACONTROL_FLAGS_AUTO)
				autoEnabled = true;
		}
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return autoEnabled;

}
long UVCCameraLibrary::getVal(long prop)
{
	HRESULT hr = E_FAIL;

	long propVal = 0;

	if (pDeviceFilter == NULL)
		return propVal;

	IAMCameraControl* pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");
	}
	else
	{
		long Flags, Val;
		hr = pCameraControl->Get(prop, &Val, &Flags);
		if (SUCCEEDED(hr))
		{
			propVal = Val;
		}
	}

	if (pCameraControl != NULL)
		pCameraControl->Release();

	return propVal;
}

HRESULT UVCCameraLibrary::stopMoving()
{
	stopControling(KSPROPERTY_CAMERACONTROL_PAN_RELATIVE);
	return stopControling(KSPROPERTY_CAMERACONTROL_TILT_RELATIVE);
}
HRESULT UVCCameraLibrary::stopZooming()
{
	return stopControling(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE);
}
HRESULT UVCCameraLibrary::stopFocusing()
{
	return stopControling(KSPROPERTY_CAMERACONTROL_FOCUS_RELATIVE);
}

HRESULT UVCCameraLibrary::stopControling(long prop)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;

	IAMCameraControl* pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");
	}
	else
	{

		hr = pCameraControl->Set(prop, 0, KSPROPERTY_CAMERACONTROL_FLAGS_RELATIVE);
		if (FAILED(hr))
		{
			printf("Stop Controlling: This device does not support PTZControl\n");
		}
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}
/*OSD menu tool*/
HRESULT UVCCameraLibrary::osdMenuOpenClose()
{
	int nPos = 0;
	return uvcxu->put_Property(UVC_1702C_XU_PLUG_CTRL_OPEN_CLOSE, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuEnter()
{
	int nPos = 0;
	return uvcxu->put_Property(UVC_1702C_XU_PLUG_CTRL_OK, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuBack()
{
	int nPos = 0;
	return uvcxu->put_Property(UVC_1702C_XU_PLUG_CTRL_BACK, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuUp()
{
	int nPos = 0;
	return uvcxu->put_Property(UVC_1702C_XU_PLUG_CTRL_UP, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuDown()
{
	int nPos = 0;
	return uvcxu->put_Property(UVC_1702C_XU_PLUG_CTRL_DOWN, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuLeft()
{
	int nPos = 0;
	return uvcxu->put_Property(UVC_1702C_XU_PLUG_CTRL_LEFT, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuRight()
{
	int nPos = 0;
	return uvcxu->put_Property(UVC_1702C_XU_PLUG_CTRL_RIGHT, ulUvcRedSize, (BYTE*)&nPos);
}

HRESULT UVCCameraLibrary::checkOSDMenu()
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;

	hr = uvcxu->QueryUvcXuInterface(pDeviceFilter, PROPSETID_XU_PLUG_IN_1700U, 0);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	hr = E_FAIL;

	if (uvcxu->get_PropertySize(UVC_1702C_XU_PLUG_CTRL_OPEN_CLOSE, &ulUvcRedSize) == S_OK)
	{
		BYTE* pbPropertyValue;
		pbPropertyValue = new BYTE[ulUvcRedSize];
		if (pbPropertyValue)
		{
			if (uvcxu->get_Property(UVC_1702C_XU_PLUG_CTRL_OPEN_CLOSE, ulUvcRedSize, (BYTE*)pbPropertyValue) == S_OK)
			{
				hr = S_OK;
			}
		}
		delete []pbPropertyValue;
	}

	return hr;
}

bool UVCCameraLibrary::getPropRanges(long prop, uvc_ranges_t& range, IAMCameraControl* pCamControl)
{
	HRESULT hr = E_FAIL;
	bool enabled = true;
	long Min, Max, Step, Default, Flags;

	// Get the range and default values 
	hr = pCamControl->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);
	if (FAILED(hr))
	{
		enabled = false;
	}
	else
	{
		range.Min = Min;
		range.Max = Max;
		range.Default = Default;
	}
	return enabled;
}

void UVCCameraLibrary::getPanRanges(IAMCameraControl* pCamControl)
{
	panConEnabled = getPropRanges(KSPROPERTY_CAMERACONTROL_PAN_RELATIVE, panConRange, pCamControl);
	printf("pan continuous movement enabled: %s\n", panConEnabled ? "true" : "false");
	printf("Pan - Min %d, Max %d, Default %d\n", panConRange.Min, panConRange.Max, panConRange.Default);
	panAbsEnabled = getPropRanges(KSPROPERTY_CAMERACONTROL_PAN, panAbsRange, pCamControl);
	printf("pan absolute movement enabled: %s\n", panAbsEnabled ? "true" : "false");
	printf("Pan - Min %d, Max %d, Default %d\n", panAbsRange.Min, panAbsRange.Max, panAbsRange.Default);
}
void UVCCameraLibrary::getTiltRanges(IAMCameraControl* pCamControl)
{
	tiltConEnabled = getPropRanges(KSPROPERTY_CAMERACONTROL_TILT_RELATIVE, tiltConRange, pCamControl);
	printf("tilt continuous movement enabled: %s\n", tiltConEnabled ? "true" : "false");
	printf("Tilt - Min %d, Max %d, Default %d\n", tiltConRange.Min, tiltConRange.Max, tiltConRange.Default);
	tiltAbsEnabled = getPropRanges(KSPROPERTY_CAMERACONTROL_TILT, tiltAbsRange, pCamControl);
	printf("tilt absolute movement enabled: %s\n", tiltAbsEnabled ? "true" : "false");
	printf("Tilt - Min %d, Max %d, Default %d\n", tiltAbsRange.Min, tiltAbsRange.Max, tiltAbsRange.Default);
}
void UVCCameraLibrary::getZoomRanges(IAMCameraControl* pCamControl)
{
	zoomConEnabled = getPropRanges(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE, zoomConRange, pCamControl);
	printf("zoom continuous movement enabled: %s\n", zoomConEnabled ? "true" : "false");
	printf("Zoom - Min %d, Max %d, Default %d\n", zoomConRange.Min, zoomConRange.Max, zoomConRange.Default);
	zoomAbsEnabled = getPropRanges(KSPROPERTY_CAMERACONTROL_ZOOM, zoomAbsRange, pCamControl);
	printf("zoom absolute movement enabled: %s\n", zoomAbsEnabled ? "true" : "false");
	printf("Zoom - Min %d, Max %d, Default %d\n", zoomAbsRange.Min, zoomAbsRange.Max, zoomAbsRange.Default);
}
void UVCCameraLibrary::getFocusRanges(IAMCameraControl* pCamControl)
{
	focusConEnabled = getPropRanges(KSPROPERTY_CAMERACONTROL_FOCUS_RELATIVE, focusConRange, pCamControl);
	printf("focus continuous movement enabled: %s\n", focusConEnabled ? "true" : "false");
	printf("Focus - Min %d, Max %d, Default %d\n", focusConRange.Min, focusConRange.Max, focusConRange.Default);
	focusAbsEnabled = getPropRanges(KSPROPERTY_CAMERACONTROL_FOCUS, focusAbsRange, pCamControl);
	printf("focus absolute movement enabled: %s\n", focusAbsEnabled ? "true" : "false");
	printf("Focus - Min %d, Max %d, Default %d\n", focusAbsRange.Min, focusAbsRange.Max, focusAbsRange.Default);
}

uvc_ranges_t UVCCameraLibrary::getPanCtrlRanges()
{
	if (panConEnabled)
		return panConRange;

	if (panAbsEnabled)
	{
		uvc_ranges_t ctrlRange = {
			1,
			20,
			7
		};

		return ctrlRange;
	}

	return DEFAULT_RANGES;
}
uvc_ranges_t UVCCameraLibrary::getTiltCtrlRanges()
{
	if (tiltConEnabled)
		return tiltConRange;

	if (tiltAbsEnabled)
	{
		uvc_ranges_t ctrlRange = {
			1,
			20,
			7
		};

		return ctrlRange;
	}

	return DEFAULT_RANGES;
}
uvc_ranges_t UVCCameraLibrary::getZoomCtrlRanges()
{
	if (zoomConEnabled)
		return zoomConRange;

	if (zoomAbsEnabled)
	{
		uvc_ranges_t ctrlRange = {
			1,
			20,
			7
		};

		return ctrlRange;
	}

	return DEFAULT_RANGES;
}
uvc_ranges_t UVCCameraLibrary::getFocusCtrlRanges()
{
	if (focusConEnabled)
		return focusConRange;

	if (focusAbsEnabled)
	{
		uvc_ranges_t ctrlRange = {
			1,
			20,
			7
		};

		return ctrlRange;
	}

	return DEFAULT_RANGES;
}


HRESULT UVCCameraLibrary::getControlRanges()
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;
	IAMCameraControl* pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");
		uvcEnabled = false;
	}
	else
	{
		getPanRanges(pCameraControl);
		getTiltRanges(pCameraControl);
		getZoomRanges(pCameraControl);
		getFocusRanges(pCameraControl);
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}
// Constructs a product ID from the Windows DevicePath. on a USB device the
// devicePath contains product id and vendor id. This seems to work for firewire
// as well.
// Example of device path:
// "\\?\usb#vid_0408&pid_2010&mi_00#7&258e7aaf&0&0000#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\global"
void UVCCameraLibrary::getCameraInfo(TCHAR* devicePath)
{
	ULONG   ven, dev;
	ven = dev = 0;

	if (sscanf_s(devicePath,
		"\\\\?\\usb#vid_%x&pid_%x",
		&ven, &dev) != 2)
	{
		printf("Failed to get camera information\n");
	}

	vendorId = ven;
	productId = dev;
	
	printf("vid_%x pid_%x\n", ven, dev);
}
/********************************************************End of UVCCameraLibrary class************************************************************/