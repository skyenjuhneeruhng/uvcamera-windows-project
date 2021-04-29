// LibraryTester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <UVCCameraLibrary.h>
#include <Windows.h>

int main()
{
	char **deviceNames;
	deviceNames = new char*[256];
	for (int i = 0; i < 256; i++)
	{
		deviceNames[i] = new char[256];
	}

	int devices = 0;
	
	UVCCameraLibrary::listDevices(deviceNames, devices);
	printf("the number of devices = %d\n", devices);
	for (int i = 0; i < devices; i++)
	{
		printf("%s\n", deviceNames[i]);
	}
	UVCCameraLibrary lib;
	int camIndex = 0;
	printf("Enter the camera index to test\n");
	scanf_s("%d", &camIndex);//does not work if you put strings not related input format
	printf("Entered index is %d\n", camIndex);
	printf("Selected camera name %s\n", deviceNames[((camIndex<devices)?camIndex:0)]);
	if (lib.connectDevice(deviceNames[((camIndex < devices) ? camIndex : 0)]))
	{
		uvc_ranges_t panRange = lib.getPanCtrlRanges();
		printf("Min %d, Max %d, Default %d\n", panRange.Min, panRange.Max, panRange.Default);
		uvc_ranges_t tiltRange = lib.getTiltCtrlRanges();
		printf("Min %d, Max %d, Default %d\n", tiltRange.Min, tiltRange.Max, tiltRange.Default);
		uvc_ranges_t zoomRange = lib.getZoomCtrlRanges();
		printf("Min %d, Max %d, Default %d\n", zoomRange.Min, zoomRange.Max, zoomRange.Default);
		uvc_ranges_t focusRange = lib.getFocusCtrlRanges();
		printf("Min %d, Max %d, Default %d\n", focusRange.Min, focusRange.Max, focusRange.Default);
	}
	else
	{
		printf("Can not connect device");
	}
	while (true);
	
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
