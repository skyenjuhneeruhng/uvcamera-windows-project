#include "stdafx.h"
#include "UVCUX.h"
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