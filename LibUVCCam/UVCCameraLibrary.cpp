#include "stdafx.h"
#include "UVCCameraLibrary.h"

/**********************************************Implementation of UVCXU class*********************************************************/
UVCXU::UVCXU()
{
	SAFE_RELEASE(m_pKsControl);
	m_pKsControl = NULL;
	m_dwNodeId = 0;
	UvcXuGudi = { 0 };
}

UVCXU::~UVCXU()
{
	SAFE_RELEASE(m_pKsControl);
}



HRESULT
UVCXU::QueryUvcXuInterface(
	IBaseFilter* pDeviceFilter,
	GUID extensionGuid,
	DWORD FirstNodeId)
{
	HRESULT hr = S_OK;
	IKsTopologyInfo* pIksTopologyInfo = NULL;
	DWORD numberOfNodes;
	KSP_NODE ExtensionProp;

	if (pDeviceFilter == NULL)
		return E_POINTER;


	hr = pDeviceFilter->QueryInterface(__uuidof(IKsTopologyInfo), (void**)&pIksTopologyInfo);
	if (FAILED(hr))
		return hr;


	hr = pIksTopologyInfo->get_NumNodes(&numberOfNodes);
	if (FAILED(hr) || numberOfNodes == 0)
	{
		SAFE_RELEASE(pIksTopologyInfo);
		return E_FAIL;
	}

	DWORD i = 0, j = 0;
	GUID nodeGuid;
	if (FirstNodeId < numberOfNodes)
		i = FirstNodeId;

	for (; i < numberOfNodes; i++)
	{
		if (SUCCEEDED(pIksTopologyInfo->get_NodeType(i, &nodeGuid)))
		{
			if (IsEqualGUID(KSNODETYPE_DEV_SPECIFIC, nodeGuid))
			{
#if _DEBUG
				printf("found one xu node, NodeId = %d\n", i);
#endif
				SAFE_RELEASE(m_pKsControl);
				m_pKsControl = NULL;
				hr = pIksTopologyInfo->CreateNodeInstance(i, IID_IKsControl, (void**)&m_pKsControl);
				if (SUCCEEDED(hr))
				{
					ULONG ulBytesReturned;
					ExtensionProp.Property.Set = extensionGuid;
					ExtensionProp.Property.Id = KSPROPERTY_EXTENSION_UNIT_INFO;
					ExtensionProp.Property.Flags = KSPROPERTY_TYPE_SETSUPPORT | KSPROPERTY_TYPE_TOPOLOGY;
					ExtensionProp.NodeId = i;
					ExtensionProp.Reserved = 0;

					hr = m_pKsControl->KsProperty(
						(PKSPROPERTY)&ExtensionProp,
						sizeof(ExtensionProp),
						NULL,
						0,
						&ulBytesReturned);

					if (hr == S_OK)
					{
#if _DEBUG						
						printf("CreateNodeInstance NodeId = %d\n", i);
#endif
						m_dwNodeId = i;
						UvcXuGudi = extensionGuid;
						return hr;
					}
				}
				else
				{
#if _DEBUG
					printf("CreateNodeInstance failed - 0x%x, NodeId = %d\n", hr, i);
#endif
				}
			}
		}

	}

	if (i == numberOfNodes)
	{	// Did not find the node
		SAFE_RELEASE(m_pKsControl);
		m_pKsControl = NULL;
		hr = E_FAIL;
	}

	SAFE_RELEASE(pIksTopologyInfo);
	return hr;
}

HRESULT
UVCXU::FinalConstruct()
{
	if (m_pKsControl == NULL) return E_FAIL;
	return S_OK;
}

HRESULT
UVCXU::get_InfoSize(
	ULONG* pulSize)
{
	HRESULT hr = S_OK;
	ULONG ulBytesReturned;
	KSP_NODE ExtensionProp;

	if (!pulSize || m_pKsControl == NULL) return E_POINTER;

	ExtensionProp.Property.Set = UvcXuGudi;
	ExtensionProp.Property.Id = KSPROPERTY_EXTENSION_UNIT_INFO;
	ExtensionProp.Property.Flags = KSPROPERTY_TYPE_GET |
		KSPROPERTY_TYPE_TOPOLOGY;
	ExtensionProp.NodeId = m_dwNodeId;

	hr = m_pKsControl->KsProperty(
		(PKSPROPERTY)&ExtensionProp,
		sizeof(ExtensionProp),
		NULL,
		0,
		&ulBytesReturned);

	if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA))
	{
		*pulSize = ulBytesReturned;
		hr = S_OK;
	}

	return hr;
}


HRESULT
UVCXU::get_Info(
	ULONG ulSize,
	BYTE pInfo[])
{
	HRESULT hr = S_OK;
	KSP_NODE ExtensionProp;
	ULONG ulBytesReturned;

	if (m_pKsControl == NULL) return E_POINTER;

	ExtensionProp.Property.Set = UvcXuGudi;
	ExtensionProp.Property.Id = KSPROPERTY_EXTENSION_UNIT_INFO;
	ExtensionProp.Property.Flags = KSPROPERTY_TYPE_GET |
		KSPROPERTY_TYPE_TOPOLOGY;
	ExtensionProp.NodeId = m_dwNodeId;

	hr = m_pKsControl->KsProperty(
		(PKSPROPERTY)&ExtensionProp,
		sizeof(ExtensionProp),
		(PVOID)pInfo,
		ulSize,
		&ulBytesReturned);

	return hr;
}


HRESULT
UVCXU::get_PropertySize(
	ULONG PropertyId,
	ULONG* pulSize)
{
	HRESULT hr = S_OK;
	ULONG ulBytesReturned;
	KSP_NODE ExtensionProp;

	if (!pulSize || m_pKsControl == NULL) return E_POINTER;

	ExtensionProp.Property.Set = UvcXuGudi;
	ExtensionProp.Property.Id = PropertyId;
	ExtensionProp.Property.Flags = KSPROPERTY_TYPE_GET |
		KSPROPERTY_TYPE_TOPOLOGY;
	ExtensionProp.NodeId = m_dwNodeId;

	hr = m_pKsControl->KsProperty(
		(PKSPROPERTY)&ExtensionProp,
		sizeof(ExtensionProp),
		NULL,
		0,
		&ulBytesReturned);

	if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA))
	{
		*pulSize = ulBytesReturned;
		hr = S_OK;
	}

	return hr;
}

HRESULT
UVCXU::get_Property(
	ULONG PropertyId,
	ULONG ulSize,
	BYTE pValue[])
{
	HRESULT hr = S_OK;
	KSP_NODE ExtensionProp;
	ULONG ulBytesReturned;

	if (m_pKsControl == NULL) return E_POINTER;

	ExtensionProp.Property.Set = UvcXuGudi;
	ExtensionProp.Property.Id = PropertyId;
	ExtensionProp.Property.Flags = KSPROPERTY_TYPE_GET |
		KSPROPERTY_TYPE_TOPOLOGY;
	ExtensionProp.NodeId = m_dwNodeId;
	ExtensionProp.Reserved = 0;

	hr = m_pKsControl->KsProperty(
		(PKSPROPERTY)&ExtensionProp,
		sizeof(ExtensionProp),
		(PVOID)pValue,
		ulSize,
		&ulBytesReturned);

	if (ulSize != ulBytesReturned)
		return E_FAIL;

	return hr;
}

HRESULT
UVCXU::put_Property(
	ULONG PropertyId,
	ULONG ulSize,
	BYTE pValue[])
{
	HRESULT hr = S_OK;
	KSP_NODE ExtensionProp;
	ULONG ulBytesReturned;

	if (m_pKsControl == NULL) return E_POINTER;

	ExtensionProp.Property.Set = UvcXuGudi;
	ExtensionProp.Property.Id = PropertyId;
	ExtensionProp.Property.Flags = KSPROPERTY_TYPE_SET |
		KSPROPERTY_TYPE_TOPOLOGY;
	ExtensionProp.NodeId = m_dwNodeId;
	ExtensionProp.Reserved = 0;

	hr = m_pKsControl->KsProperty(
		(PKSPROPERTY)&ExtensionProp,
		sizeof(ExtensionProp),
		(PVOID)pValue,
		ulSize,
		&ulBytesReturned);

	return hr;
}

typedef struct {
	KSPROPERTY_MEMBERSHEADER    MembersHeader;
	const VOID* Members;
} KSPROPERTY_MEMBERSLIST, * PKSPROPERTY_MEMBERSLIST;

typedef struct {
	KSIDENTIFIER                    PropTypeSet;
	ULONG                           MembersListCount;
	_Field_size_(MembersListCount)
		const KSPROPERTY_MEMBERSLIST* MembersList;
} KSPROPERTY_VALUES, * PKSPROPERTY_VALUES;

HRESULT
UVCXU::get_PropertyRange(
	ULONG PropertyId,
	ULONG ulSize,
	BYTE pMin[],
	BYTE pMax[],
	BYTE pSteppingDelta[],
	BYTE pDefault[])
{
	if (m_pKsControl == NULL) return E_POINTER;

	// IHV may add code here, current stub just returns S_OK
	HRESULT hr = S_OK;
	return hr;
}

/************************************************************End of UVCXU class******************************************************************/

/********************************************************Implementation of UVCCameraLibrary class************************************************/

/*
* Constructor of the class
*/
UVCCameraLibrary::UVCCameraLibrary()
{
	// initialize COM
	CoInitialize(0);
}

/*
* Destructor of the class
*/
UVCCameraLibrary::~UVCCameraLibrary()
{
	// release directshow class instances
	if(pEnumMoniker != NULL)
		pEnumMoniker->Release();
	if(pCreateDevEnum != NULL)
		pCreateDevEnum->Release();
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
void UVCCameraLibrary::listDevices(char **cameraNames , int &nDevices)
{
	nDevices = 0;
	IBaseFilter *pDeviceFilter = NULL;
	// to select a video input device
	ICreateDevEnum *pCreateDevEnum = NULL;
	IEnumMoniker *pEnumMoniker = NULL;
	IMoniker *pMoniker = NULL;
	ULONG nFetched = 0;
	// initialize COM
	CoInitialize(0);
	// Create CreateDevEnum to list device
	CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (PVOID *)&pCreateDevEnum);

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
	
	while (pEnumMoniker->Next(1, &pMoniker, &nFetched) == S_OK) {

		//real name of the camera without suffix
		char cameraRealNames[256][256];
		
		IPropertyBag *pPropertyBag;
		//unique name with suffix 
		TCHAR devname[256];

		// bind to IPropertyBag
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
			(void **)&pPropertyBag);

		VARIANT var;

		// get FriendlyName
		var.vt = VT_BSTR;
		pPropertyBag->Read(L"FriendlyName", &var, 0);//device name
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
		if(nSameNamedDevices > 0)
			sprintf_s(devname, "%s #%d", devname, nSameNamedDevices);
		strcpy_s(cameraNames[nDevices] , sizeof(devname) , (TCHAR*)devname);

		// release
		pMoniker->Release();
		pPropertyBag->Release();

		nDevices++;
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
	CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (PVOID *)&pCreateDevEnum);

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
bool UVCCameraLibrary::connectDevice(char *deviceName)
{
	getEnumMoniker();

	if (pEnumMoniker == NULL)
		return false;

	int nDevices = 0;
	while (pEnumMoniker->Next(1, &pMoniker, &nFetched) == S_OK) {
		
		//we need real name without suffix to connect to camera
		char cameraRealNames[256][256];

		IPropertyBag *pPropertyBag;
		TCHAR devname[256];

		// bind to IPropertyBag
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
			(void **)&pPropertyBag);

		VARIANT var;

		// get FriendlyName
		var.vt = VT_BSTR;
		pPropertyBag->Read(L"FriendlyName", &var, 0);
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

		if(nSameNamedDevices > 0)
			sprintf_s(devname, "%s #%d", devname, nSameNamedDevices);

		//printf("devname %s , devicename %s\n", devname, deviceName);

		if (strcmp(devname, deviceName) == 0)
		{
			pMoniker->BindToObject(0, 0, IID_IBaseFilter,
				(void**)&pDeviceFilter);
			if (pDeviceFilter != NULL)
			{
				//this is the camera which we are going to connect
				TCHAR devPath[256];
				VARIANT var1;

				// get FriendlyName
				var1.vt = VT_BSTR;
				pPropertyBag->Read(L"DevicePath", &var1, 0);
				WideCharToMultiByte(CP_ACP, 0,
					var1.bstrVal, -1, devPath, sizeof(devPath), 0, 0);
				printf("device path %s", devPath);
				VariantClear(&var1);

				//get product id
				//TODO

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
	return false;
}



void UVCCameraLibrary::disconnectDevice()
{
	//release directshow filter
	if(pDeviceFilter != NULL)
	    pDeviceFilter->Release();
	pDeviceFilter = NULL;
}

/*
* move pan to left one step
* @pan: (in) step of the panning
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::movePanOneLeft(int pan)
{
	return moveCamera(KSPROPERTY_CAMERACONTROL_PAN_RELATIVE , - pan);

	//for check if the camera supports absolute movement
	//PTZ Pro 2 does not support absolute movement
	//long zoomVal = getZoom();
	//long panVal = getPan();
	//long tiltVal = getTilt();
	//long focusVal = getFocus();
	//return moveTo(panVal + pan, tiltVal, zoomVal, focusVal);
}
/*
* move pan to right one step
* @pan: (in) step of the panning
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::movePanOneRight(int pan)
{
	return moveCamera(KSPROPERTY_CAMERACONTROL_PAN_RELATIVE, pan);
}
/*
* move tilt to top one step
* @tilt: (in) step of the tilting
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::moveTiltOneTop(int tilt)
{
	return moveCamera(KSPROPERTY_CAMERACONTROL_TILT_RELATIVE , tilt);
}
/*
* move tilt to bottom one step
* @tilt: (in) step of the tilting
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::moveTiltOneBottom(int tilt)
{
	return moveCamera(KSPROPERTY_CAMERACONTROL_TILT_RELATIVE, -tilt);
}
/*HRESULT angleUpLeft(int pan, int tilt)
{
	HRESULT hr;
	return hr;
}
HRESULT angleUpRight(int pan, int tilt)
{
	HRESULT hr;
	return hr;
}
HRESULT angleDownLeft(int pan, int tilt)
{
	HRESULT hr;
	return hr;
}
HRESULT anglueDownRight(int pan, int tilt)
{
	HRESULT hr;
	return hr;
}*/
/*
* zoom in one step
* @tilt: (in) step of the zooming
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::zoomOneIn(int zoom)
{
	//continuous zooming in
	//return moveCamera(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE , zoom);
	//absolute zooming in once
	/*
	* the following code does not support continuous zooming
	* then how can we solve that problem?
	* what if we zoom to the end and then stop zooming when we release the button?
	*/
	//long zoomVal = getZoom();
	//long panVal = getPan();
	//long tiltVal = getTilt();
	//long focusVal = getFocus();
	//return moveTo(panVal, tiltVal, zoomVal + zoom, focusVal);

	long panVal = getPan();
	long tiltVal = getTilt();
	long focusVal = getFocus();
	return moveTo(panVal, tiltVal, zoomRange.Max, focusVal);
}
/*
* zoom out one step
* @tilt: (in) step of the zooming
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::zoomOneOut(int zoom)
{
	//continous zooming out
	//return moveCamera(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE, -zoom);
	//absolute zooming out once
	/*
	* the following code does not support continuous zooming
	* then how can we solve that problem?
	* what if we zoom to the end and then stop zooming when we release the button?
	*/
	//long zoomVal = getZoom();
	//long panVal = getPan();
	//long tiltVal = getTilt();
	//long focusVal = getFocus();
	//return moveTo(panVal, tiltVal, zoomVal - zoom, focusVal);

	long panVal = getPan();
	long tiltVal = getTilt();
	long focusVal = getFocus();
	return moveTo(panVal, tiltVal, zoomRange.Min, focusVal);
}
/*
* focus in one step
* does work if focus mode is set as Auto
* @tilt: (in) step of the focusing
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::focusOneIn(int focus)
{
	//continuous focusing in
	//return moveCamera(KSPROPERTY_CAMERACONTROL_FOCUS_RELATIVE , focus);
	//absolute focus in to the end - the same result as continuous focusing
	long panVal = getPan();
	long tiltVal = getTilt();
	long zoomVal = getZoom();
	return moveTo(panVal, tiltVal, zoomVal, focusRange.Max);
}
/*
* focus out one step
* does work if focus mode is set as Auto
* @tilt: (in) step of the focusing
* @return: HRESULT structure if success returns S_OK
*/
HRESULT UVCCameraLibrary::focusOneOut(int focus)
{
	//continuous focusing out
	//return moveCamera(KSPROPERTY_CAMERACONTROL_FOCUS_RELATIVE, -focus);
	//absolute focus in to the end - the same result as continuous focusing
	long panVal = getPan();
	long tiltVal = getTilt();
	long zoomVal = getZoom();
	return moveTo(panVal, tiltVal, zoomVal, focusRange.Min);
}
/*
* set fucus mode
* @af: if true set as Auto otherwise set as Manual
* @return: if success returns S_OK
*/
HRESULT UVCCameraLibrary::setAutoFocus(bool af)
{
	stopFocusing();
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;
	IAMCameraControl *pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");

	}
	else
	{
		long Min, Max, Step, Default, Flags, Val;

		// Get the range and default values 
		hr = pCameraControl->GetRange(CameraControl_Focus, &Min, &Max, &Step, &Default, &Flags);
		if (SUCCEEDED(hr))
		{
			hr = pCameraControl->Get(CameraControl_Focus, &Val, &Flags);
			if(af)
				hr = pCameraControl->Set(CameraControl_Focus, Val , CameraControl_Flags_Auto);
			else
				hr = pCameraControl->Set(CameraControl_Focus, Val, CameraControl_Flags_Manual);
		}
		else
		{
			printf("This device does not support PTZControl\n");
		}

	}
	if(pCameraControl != NULL)
	    pCameraControl->Release();
	return hr;
}

/*
* change the property of the camera
* @prop: (in) property e.g. KSPROPERTY_CAMERACONTROL_PAN_RELATIVE, KSPROPERTY_CAMERACONTROL_TILT_RELATIVE, KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE ...
* Use KSPROPERTIES for continuous movement
* @return: if success returns S_OK
*/
HRESULT UVCCameraLibrary::moveCamera(KSPROPERTY_VIDCAP_CAMERACONTROL prop, int step)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;
	IAMCameraControl *pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");

	}
	else
	{
		long Min, Max, Step, Default, Flags, Val;

		// Get the range and default values 
		hr = pCameraControl->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);
		printf("Min %d , Max %d , Step %d", Min, Max , Step);
		if (SUCCEEDED(hr))
		{
			hr = pCameraControl->Set(prop, step, CameraControl_Flags_Manual);
		}
		else
		{
			printf("This device does not support PTZControl\n");
		}
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}
/*
* move to absolute position
* @pan: pan
* @tilt: tilt
* @zoom: zoom
* @focus: focus
* @return: if success returns S_OK
* must add min values from pan, tilt, zoom
* the range of the pan, tilt, zoom values are like this -1 t0 1
* but the available properties are like 0 to 255
*/
HRESULT UVCCameraLibrary::moveTo(int pan, int tilt, int zoom, int focus)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;
	hr = stopMoving();
	hr = stopZooming();
	hr = stopFocusing();
	IAMCameraControl *pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");

	}
	else
	{
		pan += panRange.Min;
		if (pan < panRange.Min)
			pan = panRange.Min;
		if (pan > panRange.Max)
			pan = panRange.Max;
		tilt += tiltRange.Min;
		if (tilt < tiltRange.Min)
			tilt = tiltRange.Min;
		if (tilt > tiltRange.Max)
			tilt = tiltRange.Max;
		zoom += zoomRange.Min;
		if (zoom < zoomRange.Min)
			zoom = zoomRange.Min;
		if (zoom > zoomRange.Max)
			zoom = zoomRange.Max;

		long Val, Flags;
		hr = pCameraControl->Get(CameraControl_Focus, &Val, &Flags);
		long focusFlag = Flags;
		long focusVal = Val;
		focus += focusRange.Min;
		if (focus < focusRange.Min)
			focus = focusRange.Min;
		if (focus > focusRange.Max)
			focus = focusRange.Max;

		if (SUCCEEDED(hr))
		{
			//use CameraControl_Pan, Tilt, Zoom for absolute movement
			hr = pCameraControl->Set(CameraControl_Pan, pan, CameraControl_Flags_Manual);

			hr = pCameraControl->Set(CameraControl_Tilt, tilt, CameraControl_Flags_Manual);

			hr = pCameraControl->Set(CameraControl_Zoom, zoom, CameraControl_Flags_Manual);

			if (focusFlag != CameraControl_Flags_Auto)
				hr = pCameraControl->Set(CameraControl_Focus, focus, CameraControl_Flags_Manual);
		}
		else
		{
			printf("This device does not support PTZControl\n");
		}
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}
//move home
HRESULT UVCCameraLibrary::moveHome()
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;
	hr = stopMoving();
	hr = stopZooming();
	hr = stopFocusing();
	IAMCameraControl *pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");

	}
	else
	{
		long Flags, Val;

		hr = pCameraControl->Get(CameraControl_Focus, &Val, &Flags);
		long focusFlag = Flags;
		long focusVal = Val;
		if (SUCCEEDED(hr))
		{
			hr = pCameraControl->Set(CameraControl_Pan, panRange.Default, CameraControl_Flags_Manual);

			hr = pCameraControl->Set(CameraControl_Tilt, tiltRange.Default, CameraControl_Flags_Manual);

			hr = pCameraControl->Set(CameraControl_Zoom, zoomRange.Default, CameraControl_Flags_Manual);
			if (focusFlag != CameraControl_Flags_Auto)
				hr = pCameraControl->Set(CameraControl_Focus, Val, CameraControl_Flags_Auto);
		}
		else
		{
			printf("This device does not support PTZControl\n");
		}
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}

bool UVCCameraLibrary::getAutoFocus()
{
	return getAuto(CameraControl_Focus);
}
long UVCCameraLibrary::getPan()
{
	return getVal(CameraControl_Pan);
}
long UVCCameraLibrary::getTilt()
{
	return getVal(CameraControl_Tilt);
}
long UVCCameraLibrary::getZoom()
{
	return getVal(CameraControl_Zoom);
}
long UVCCameraLibrary::getFocus()
{
	return getVal(CameraControl_Focus);
}

bool UVCCameraLibrary::getAuto(CameraControlProperty prop)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;
	IAMCameraControl *pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		if (pCameraControl != NULL)
			pCameraControl->Release();
		printf("This device does not support IAMCameraControl\n");
		return false;

	}
	else
	{
		long Flags, Val;

		// Get the range and default values 
		hr = pCameraControl->Get(prop, &Val, &Flags);
		if (pCameraControl != NULL)
			pCameraControl->Release();
		if (SUCCEEDED(hr))
		{
			if (Flags == CameraControl_Flags_Auto)
				return true;
			else
				return false;
		}
		else
		{
			printf("This device does not support PTZControl\n");
			return false;
		}
	}
	
}
long UVCCameraLibrary::getVal(CameraControlProperty prop)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;
	IAMCameraControl *pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		if (pCameraControl != NULL)
			pCameraControl->Release();
		printf("This device does not support IAMCameraControl\n");
		return 0;

	}
	else
	{
		long Min, Max, Step, Default, Flags, Val;

		if (prop == CameraControl_Zoom)
			printf("Zoom ...");
		else
			printf("No zoom ...");
		// Get the range and default values 
		hr = pCameraControl->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);
		printf("Min %d , Max %d , Step %d", Min, Max, Step);
		hr = pCameraControl->Get(prop, &Val, &Flags);
		printf("required value is %li \n", Val);
		if (pCameraControl != NULL)
			pCameraControl->Release();
		if (SUCCEEDED(hr))
		{
			return Val - Min;
		}
		else
		{
			printf("This device does not support PTZControl\n");
			return 0;
		}
	}
}

HRESULT UVCCameraLibrary::stopMoving()
{
	stopControling(KSPROPERTY_CAMERACONTROL_PAN_RELATIVE);
	return stopControling(KSPROPERTY_CAMERACONTROL_TILT_RELATIVE);
}
HRESULT UVCCameraLibrary::stopZooming()
{
	//return stopControling(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE);
	/*
	long panVal = getPan();
	long tiltVal = getTilt();
	long focusVal = getFocus();
	long zoomVal = getZoom();
	return moveTo(panVal, tiltVal, focusVal, zoomVal);
	*/
	long zoomVal = getZoom();
	printf("current zoom : %d", zoomVal);
	return stopAbsControling(CameraControl_Zoom);
}
HRESULT UVCCameraLibrary::stopFocusing()
{
	//return stopControling(KSPROPERTY_CAMERACONTROL_FOCUS_RELATIVE);
	/*
	long panVal = getPan();
	long tiltVal = getTilt();
	long focusVal = getFocus();
	long zoomVal = getZoom();
	return moveTo(panVal, tiltVal, focusVal, zoomVal);
	*/
	return stopAbsControling(CameraControl_Focus);
}

HRESULT UVCCameraLibrary::stopControling(KSPROPERTY_VIDCAP_CAMERACONTROL prop)
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;

	IAMCameraControl *pCameraControl = 0;
	hr = pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	if (FAILED(hr))
	{
		// The device does not support IAMCameraControl
		printf("This device does not support IAMCameraControl\n");

	}
	else
	{
		long Min, Max, Step, Default, Flags, Val;

		// Get the range and default values 
		hr = pCameraControl->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);
		printf("Min %d , Max %d , Step %d", Min, Max, Step);
		if (SUCCEEDED(hr))
		{
			hr = pCameraControl->Set(prop, 0, KSPROPERTY_CAMERACONTROL_FLAGS_RELATIVE);
		}
		else
		{
			printf("Stop Controlling: This device does not support PTZControl\n");
		}
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}

HRESULT UVCCameraLibrary::stopAbsControling(CameraControlProperty prop)
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
		long Min, Max, Step, Default, Flags, Val;

		// Get the range and default values 
		hr = pCameraControl->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);
		printf("Min %d , Max %d , Step %d", Min, Max, Step);
		if (SUCCEEDED(hr))
		{
			hr = pCameraControl->Set(prop, 0, KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE);
		}
		else
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
	return uvcxu.put_Property(UVC_1702C_XU_PLUG_CTRL_OPEN_CLOSE, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuEnter()
{
	int nPos = 0;
	return uvcxu.put_Property(UVC_1702C_XU_PLUG_CTRL_OK, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuBack()
{
	int nPos = 0;
	return uvcxu.put_Property(UVC_1702C_XU_PLUG_CTRL_BACK, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuUp()
{
	int nPos = 0;
	return uvcxu.put_Property(UVC_1702C_XU_PLUG_CTRL_UP, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuDown()
{
	int nPos = 0;
	return uvcxu.put_Property(UVC_1702C_XU_PLUG_CTRL_DOWN, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuLeft()
{
	int nPos = 0;
	return uvcxu.put_Property(UVC_1702C_XU_PLUG_CTRL_LEFT, ulUvcRedSize, (BYTE*)&nPos);
}
HRESULT UVCCameraLibrary::osdMenuRight()
{
	int nPos = 0;
	return uvcxu.put_Property(UVC_1702C_XU_PLUG_CTRL_RIGHT, ulUvcRedSize, (BYTE*)&nPos);
}

HRESULT UVCCameraLibrary::checkOSDMenu()
{
	HRESULT hr = E_FAIL;

	if (pDeviceFilter == NULL)
		return E_FAIL;

	hr = uvcxu.QueryUvcXuInterface(pDeviceFilter, PROPSETID_XU_PLUG_IN_1700U, 0);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	hr = E_FAIL;

	if (uvcxu.get_PropertySize(UVC_1702C_XU_PLUG_CTRL_OPEN_CLOSE, &ulUvcRedSize) == S_OK)
	{
		BYTE* pbPropertyValue;
		pbPropertyValue = new BYTE[ulUvcRedSize];
		if (pbPropertyValue)
		{
			if (uvcxu.get_Property(UVC_1702C_XU_PLUG_CTRL_OPEN_CLOSE, ulUvcRedSize, (BYTE*)pbPropertyValue) == S_OK)
			{
				hr = S_OK;
			}
		}
		delete pbPropertyValue;
	}

	return hr;
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

	}
	else
	{
		long Min, Max, Step, Default, Flags, Val;

		// Get the range and default values 
		hr = pCameraControl->GetRange(CameraControl_Pan, &Min, &Max, &Step, &Default, &Flags);
		panRange.Min = Min;
		panRange.Max = Max;
		panRange.Default = Default;
		hr = pCameraControl->GetRange(CameraControl_Tilt, &Min, &Max, &Step, &Default, &Flags);
		tiltRange.Min = Min;
		tiltRange.Max = Max;
		tiltRange.Default = Default;
		hr = pCameraControl->GetRange(CameraControl_Zoom, &Min, &Max, &Step, &Default, &Flags);
		zoomRange.Min = Min;
		zoomRange.Max = Max;
		zoomRange.Default = Default;
		hr = pCameraControl->GetRange(CameraControl_Focus, &Min, &Max, &Step, &Default, &Flags);
		focusRange.Min = Min;
		focusRange.Max = Max;
		focusRange.Default = Default;

		if (SUCCEEDED(hr))
		{

		}
		else
		{
			printf("This device does not support PTZControl\n");
		}
	}
	if (pCameraControl != NULL)
		pCameraControl->Release();
	return hr;
}
/********************************************************End of UVCCameraLibrary class************************************************************/