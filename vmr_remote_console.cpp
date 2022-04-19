// vmr_remote_console.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef __cplusplus
#ifndef STRICT
#define STRICT
#endif
#endif


#include <windows.h>
#include <stdio.h>
#include <iostream>
#include "VoicemeeterRemote.h"
#include <string>
#include "SerialPort.hpp"
#include <sstream>
#include <vector>
#include <algorithm>
#include <thread>
#include "serial.h"


#define DATA_LENGTH 255

const char* portName = "\\\\.\\COM18";

char strips[8][14] = {
	"Strip[0].gain",
	"Strip[1].gain",
	"Strip[2].gain",
	"Strip[3].gain",
	"Strip[4].gain",
	"Strip[5].gain",
	"Strip[6].gain",
	"Strip[7].gain"
};

//Declare a global object
SerialPort* arduino;

using std::string;
using std::vector;
using std::cout;
using std::to_string;
using std::cerr;
using std::endl;
using std::exception;

/*******************************************************************************/
/**                           GET VOICEMEETER DIRECTORY                       **/
/*******************************************************************************/

static char uninstDirKey[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

#define INSTALLER_UNINST_KEY	"VB:Voicemeeter {17359A74-1236-5467}"


void RemoveNameInPath(char* szPath)
{
	long ll;
	ll = (long)strlen(szPath);
	while ((ll > 0) && (szPath[ll] != '\\')) ll--;
	if (szPath[ll] == '\\') szPath[ll] = 0;
}

#ifndef KEY_WOW64_32KEY
#define KEY_WOW64_32KEY 0x0200
#endif

BOOL __cdecl RegistryGetVoicemeeterFolder(char* szDir)
{
	char szKey[256];
	char sss[1024];
	DWORD nnsize = 1024;
	HKEY hkResult;
	LONG rep;
	DWORD pptype = REG_SZ;
	sss[0] = 0;

	// build Voicemeeter uninstallation key
	strcpy(szKey, uninstDirKey);
	strcat(szKey, "\\");
	strcat(szKey, INSTALLER_UNINST_KEY);

	// open key
	rep = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hkResult);
	if (rep != ERROR_SUCCESS)
	{
		// if not present we consider running in 64bit mode and force to read 32bit registry
		rep = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ | KEY_WOW64_32KEY, &hkResult);
	}
	if (rep != ERROR_SUCCESS) return FALSE;
	// read uninstall profram path
	rep = RegQueryValueEx(hkResult, "UninstallString", 0, &pptype, (unsigned char*)sss, &nnsize);
	RegCloseKey(hkResult);

	if (pptype != REG_SZ) return FALSE;
	if (rep != ERROR_SUCCESS) return FALSE;
	// remove name to get the path only
	RemoveNameInPath(sss);
	if (nnsize > 512) nnsize = 512;
	strncpy(szDir, sss, nnsize);

	return TRUE;
}


/*******************************************************************************/
/**                                GET DLL INTERFACE                          **/
/*******************************************************************************/

static HMODULE G_H_Module = NULL;
static T_VBVMR_INTERFACE iVMR;

//if we directly link source code (for development only)
#ifdef VBUSE_LOCALLIB

long InitializeDLLInterfaces(void)
{
	iVMR.VBVMR_Login = VBVMR_Login;
	iVMR.VBVMR_Logout = VBVMR_Logout;
	iVMR.VBVMR_RunVoicemeeter = VBVMR_RunVoicemeeter;
	iVMR.VBVMR_GetVoicemeeterType = VBVMR_GetVoicemeeterType;
	iVMR.VBVMR_GetVoicemeeterVersion = VBVMR_GetVoicemeeterVersion;
	iVMR.VBVMR_IsParametersDirty = VBVMR_IsParametersDirty;
	iVMR.VBVMR_GetParameterFloat = VBVMR_GetParameterFloat;
	iVMR.VBVMR_GetParameterStringA = VBVMR_GetParameterStringA;
	iVMR.VBVMR_GetParameterStringW = VBVMR_GetParameterStringW;

	iVMR.VBVMR_GetLevel = VBVMR_GetLevel;
	iVMR.VBVMR_GetMidiMessage = VBVMR_GetMidiMessage;
	iVMR.VBVMR_SetParameterFloat = VBVMR_SetParameterFloat;
	iVMR.VBVMR_SetParameters = VBVMR_SetParameters;
	iVMR.VBVMR_SetParametersW = VBVMR_SetParametersW;
	iVMR.VBVMR_SetParameterStringA = VBVMR_SetParameterStringA;
	iVMR.VBVMR_SetParameterStringW = VBVMR_SetParameterStringW;

	iVMR.VBVMR_Output_GetDeviceNumber = VBVMR_Output_GetDeviceNumber;
	iVMR.VBVMR_Output_GetDeviceDescA = VBVMR_Output_GetDeviceDescA;
	iVMR.VBVMR_Output_GetDeviceDescW = VBVMR_Output_GetDeviceDescW;
	iVMR.VBVMR_Input_GetDeviceNumber = VBVMR_Input_GetDeviceNumber;
	iVMR.VBVMR_Input_GetDeviceDescA = VBVMR_Input_GetDeviceDescA;
	iVMR.VBVMR_Input_GetDeviceDescW = VBVMR_Input_GetDeviceDescW;

#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
	iVMR.VBVMR_AudioCallbackRegister = VBVMR_AudioCallbackRegister;
	iVMR.VBVMR_AudioCallbackStart = VBVMR_AudioCallbackStart;
	iVMR.VBVMR_AudioCallbackStop = VBVMR_AudioCallbackStop;
	iVMR.VBVMR_AudioCallbackUnregister = VBVMR_AudioCallbackUnregister;
#endif
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
	iVMR.VBVMR_MacroButton_IsDirty = VBVMR_MacroButton_IsDirty;
	iVMR.VBVMR_MacroButton_GetStatus = VBVMR_MacroButton_GetStatus;
	iVMR.VBVMR_MacroButton_SetStatus = VBVMR_MacroButton_SetStatus;

#endif
	return 0;
}

//Dynamic link to DLL in 'C' (regular use)
#else

long InitializeDLLInterfaces(void)
{
	char szDllName[1024];
	memset(&iVMR, 0, sizeof(T_VBVMR_INTERFACE));

	//get folder where is installed Voicemeeter
	if (RegistryGetVoicemeeterFolder(szDllName) == FALSE)
	{
		// voicemeeter not installed
		return -100;
	}
	//use right dll according O/S type
	if (sizeof(void*) == 8) strcat(szDllName, "\\VoicemeeterRemote64.dll");
	else strcat(szDllName, "\\VoicemeeterRemote.dll");

	// Load Dll
	G_H_Module = LoadLibrary(szDllName);
	if (G_H_Module == NULL) return -101;

	// Get function pointers
	iVMR.VBVMR_Login = (T_VBVMR_Login)GetProcAddress(G_H_Module, "VBVMR_Login");
	iVMR.VBVMR_Logout = (T_VBVMR_Logout)GetProcAddress(G_H_Module, "VBVMR_Logout");
	iVMR.VBVMR_RunVoicemeeter = (T_VBVMR_RunVoicemeeter)GetProcAddress(G_H_Module, "VBVMR_RunVoicemeeter");
	iVMR.VBVMR_GetVoicemeeterType = (T_VBVMR_GetVoicemeeterType)GetProcAddress(G_H_Module, "VBVMR_GetVoicemeeterType");
	iVMR.VBVMR_GetVoicemeeterVersion = (T_VBVMR_GetVoicemeeterVersion)GetProcAddress(G_H_Module, "VBVMR_GetVoicemeeterVersion");

	iVMR.VBVMR_IsParametersDirty = (T_VBVMR_IsParametersDirty)GetProcAddress(G_H_Module, "VBVMR_IsParametersDirty");
	iVMR.VBVMR_GetParameterFloat = (T_VBVMR_GetParameterFloat)GetProcAddress(G_H_Module, "VBVMR_GetParameterFloat");
	iVMR.VBVMR_GetParameterStringA = (T_VBVMR_GetParameterStringA)GetProcAddress(G_H_Module, "VBVMR_GetParameterStringA");
	iVMR.VBVMR_GetParameterStringW = (T_VBVMR_GetParameterStringW)GetProcAddress(G_H_Module, "VBVMR_GetParameterStringW");
	iVMR.VBVMR_GetLevel = (T_VBVMR_GetLevel)GetProcAddress(G_H_Module, "VBVMR_GetLevel");
	iVMR.VBVMR_GetMidiMessage = (T_VBVMR_GetMidiMessage)GetProcAddress(G_H_Module, "VBVMR_GetMidiMessage");

	iVMR.VBVMR_SetParameterFloat = (T_VBVMR_SetParameterFloat)GetProcAddress(G_H_Module, "VBVMR_SetParameterFloat");
	iVMR.VBVMR_SetParameters = (T_VBVMR_SetParameters)GetProcAddress(G_H_Module, "VBVMR_SetParameters");
	iVMR.VBVMR_SetParametersW = (T_VBVMR_SetParametersW)GetProcAddress(G_H_Module, "VBVMR_SetParametersW");
	iVMR.VBVMR_SetParameterStringA = (T_VBVMR_SetParameterStringA)GetProcAddress(G_H_Module, "VBVMR_SetParameterStringA");
	iVMR.VBVMR_SetParameterStringW = (T_VBVMR_SetParameterStringW)GetProcAddress(G_H_Module, "VBVMR_SetParameterStringW");

	iVMR.VBVMR_Output_GetDeviceNumber = (T_VBVMR_Output_GetDeviceNumber)GetProcAddress(G_H_Module, "VBVMR_Output_GetDeviceNumber");
	iVMR.VBVMR_Output_GetDeviceDescA = (T_VBVMR_Output_GetDeviceDescA)GetProcAddress(G_H_Module, "VBVMR_Output_GetDeviceDescA");
	iVMR.VBVMR_Output_GetDeviceDescW = (T_VBVMR_Output_GetDeviceDescW)GetProcAddress(G_H_Module, "VBVMR_Output_GetDeviceDescW");
	iVMR.VBVMR_Input_GetDeviceNumber = (T_VBVMR_Input_GetDeviceNumber)GetProcAddress(G_H_Module, "VBVMR_Input_GetDeviceNumber");
	iVMR.VBVMR_Input_GetDeviceDescA = (T_VBVMR_Input_GetDeviceDescA)GetProcAddress(G_H_Module, "VBVMR_Input_GetDeviceDescA");
	iVMR.VBVMR_Input_GetDeviceDescW = (T_VBVMR_Input_GetDeviceDescW)GetProcAddress(G_H_Module, "VBVMR_Input_GetDeviceDescW");

#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
	iVMR.VBVMR_AudioCallbackRegister = (T_VBVMR_AudioCallbackRegister)GetProcAddress(G_H_Module, "VBVMR_AudioCallbackRegister");
	iVMR.VBVMR_AudioCallbackStart = (T_VBVMR_AudioCallbackStart)GetProcAddress(G_H_Module, "VBVMR_AudioCallbackStart");
	iVMR.VBVMR_AudioCallbackStop = (T_VBVMR_AudioCallbackStop)GetProcAddress(G_H_Module, "VBVMR_AudioCallbackStop");
	iVMR.VBVMR_AudioCallbackUnregister = (T_VBVMR_AudioCallbackUnregister)GetProcAddress(G_H_Module, "VBVMR_AudioCallbackUnregister");
#endif
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
	iVMR.VBVMR_MacroButton_IsDirty = (T_VBVMR_MacroButton_IsDirty)GetProcAddress(G_H_Module, "VBVMR_MacroButton_IsDirty");
	iVMR.VBVMR_MacroButton_GetStatus = (T_VBVMR_MacroButton_GetStatus)GetProcAddress(G_H_Module, "VBVMR_MacroButton_GetStatus");
	iVMR.VBVMR_MacroButton_SetStatus = (T_VBVMR_MacroButton_SetStatus)GetProcAddress(G_H_Module, "VBVMR_MacroButton_SetStatus");
#endif

	// check pointers are valid
	if (iVMR.VBVMR_Login == NULL) return -1;
	if (iVMR.VBVMR_Logout == NULL) return -2;
	if (iVMR.VBVMR_RunVoicemeeter == NULL) return -2;
	if (iVMR.VBVMR_GetVoicemeeterType == NULL) return -3;
	if (iVMR.VBVMR_GetVoicemeeterVersion == NULL) return -4;
	if (iVMR.VBVMR_IsParametersDirty == NULL) return -5;
	if (iVMR.VBVMR_GetParameterFloat == NULL) return -6;
	if (iVMR.VBVMR_GetParameterStringA == NULL) return -7;
	if (iVMR.VBVMR_GetParameterStringW == NULL) return -8;
	if (iVMR.VBVMR_GetLevel == NULL) return -9;
	if (iVMR.VBVMR_SetParameterFloat == NULL) return -10;
	if (iVMR.VBVMR_SetParameters == NULL) return -11;
	if (iVMR.VBVMR_SetParametersW == NULL) return -12;
	if (iVMR.VBVMR_SetParameterStringA == NULL) return -13;
	if (iVMR.VBVMR_SetParameterStringW == NULL) return -14;
	if (iVMR.VBVMR_GetMidiMessage == NULL) return -15;

	if (iVMR.VBVMR_Output_GetDeviceNumber == NULL) return -30;
	if (iVMR.VBVMR_Output_GetDeviceDescA == NULL) return -31;
	if (iVMR.VBVMR_Output_GetDeviceDescW == NULL) return -32;
	if (iVMR.VBVMR_Input_GetDeviceNumber == NULL) return -33;
	if (iVMR.VBVMR_Input_GetDeviceDescA == NULL) return -34;
	if (iVMR.VBVMR_Input_GetDeviceDescW == NULL) return -35;

#ifdef VMR_INCLUDE_AUDIO_PROCESSING_EXAMPLE
	if (iVMR.VBVMR_AudioCallbackRegister == NULL) return -40;
	if (iVMR.VBVMR_AudioCallbackStart == NULL) return -41;
	if (iVMR.VBVMR_AudioCallbackStop == NULL) return -42;
	if (iVMR.VBVMR_AudioCallbackUnregister == NULL) return -43;
#endif
#ifdef	VMR_INCLUDE_MACROBUTTONS_REMOTING
	if (iVMR.VBVMR_MacroButton_IsDirty == NULL) return -50;
	if (iVMR.VBVMR_MacroButton_GetStatus == NULL) return -51;
	if (iVMR.VBVMR_MacroButton_SetStatus == NULL) return -52;
#endif
	return 0;
}


#endif

int initVoicemeeter()
{
	int rep = InitializeDLLInterfaces();
	if (rep < 0)
	{
		if (rep == -100)
		{
			cout << "Voicemeeter is not installed\n";
		}
		else
		{
			cout << "Error loading Voicemeeter dll\n";
		}
		return FALSE;
	}
	rep = iVMR.VBVMR_Login();
	if (rep != 0)
	{
		cout << "Error logging into Voicemeeter\n";
		if (rep == 1) 
		{
			cout << "Attempting to launch Voicemeeter\n";
			int attempts = 0;
			int isRunning = 1;
			while (attempts < 5 && isRunning != 0) 
			{
				isRunning = iVMR.VBVMR_RunVoicemeeter(3);
				attempts++;
			}
			if (isRunning == 0) 
			{
				return TRUE;
			}
		}
		return FALSE;
	}
	return TRUE;
}

void waitForUpdate()
{
	while (!iVMR.VBVMR_IsParametersDirty())
	{
		Sleep(10);
	}
	while (iVMR.VBVMR_IsParametersDirty())
	{
		Sleep(10);
	}
	Sleep(10);
}

string getVersion()
{	
	long ver;
	string verString;
	iVMR.VBVMR_GetVoicemeeterVersion(&ver);
	verString += to_string(ver >> 24);
	verString += to_string(ver >> 16);
	verString += to_string(ver >> 8);
	verString += to_string(ver);
	return to_string(ver);
}

long getType()
{
	long type;
	iVMR.VBVMR_GetVoicemeeterType(&type);
	return type;
}

int setParameters(char* param)
{
	int ret = iVMR.VBVMR_SetParameters(param);
	waitForUpdate();
	return ret;
}

int setParameterFloat(char* param, float val)
{
	int ret = iVMR.VBVMR_SetParameterFloat(param, val);
	//waitForUpdate();
	return ret;
}

int initArduino()
{
	arduino = new SerialPort(portName);
	return arduino->isConnected();
}

void enumerate_ports()
{
	vector<serial::PortInfo> devices_found = serial::list_ports();

	vector<serial::PortInfo>::iterator iter = devices_found.begin();

	while (iter != devices_found.end())
	{
		serial::PortInfo device = *iter++;

		printf("(%s, %s, %s)\n", device.port.c_str(), device.description.c_str(),
			device.hardware_id.c_str());
	}
}

string getData() 
{
	char receivedString[DATA_LENGTH];
	int hasRead = arduino->readSerialPort(receivedString, DATA_LENGTH);
	string dataString = receivedString;
	int dataSize = dataString.size();
	if (dataSize >= 2)
	{
		dataString.erase(dataString.size() - 2);
	}
	return dataString;
}

int sendData(const char* message)
{	
	bool hasWritten = arduino->writeSerialPort(message, DATA_LENGTH);
	return hasWritten;
}


/*******************************************************************************/
/**                                    MAIN                                   **/
/*******************************************************************************/

int main()
{
	cout << "Voicemeeter remote\n";
	if (initVoicemeeter())
	{
		cout << "Communication established with Voicemeeter\n";
		if (initArduino())
		{
			//thread t1(getData);
			//t1.join();

			cout << "Communication established with Arduino\n";

			vector<string>dataVect;

			enumerate_ports();

			while (1)
			{	
				string dataString = getData();
				int dataSize = dataString.size();

				if (dataSize != 0)
				{	
					//get size of datastring, ignoring commas
					string dCopy = dataString;
					dCopy.erase(remove(dCopy.begin(), dCopy.end(), ','), dCopy.end());	
					//dCopy.erase(remove(dCopy.begin(), dCopy.end(), '<'), dCopy.end());
					//dCopy.erase(remove(dCopy.begin(), dCopy.end(), '>'), dCopy.end());
					int dCopySize = dCopy.size();
					dataVect.resize(dCopySize);

					int vectIndex = 0;
					int stringIndex = 0;
					string dataBuff;

					//fill vector with datastring
					while (stringIndex < dataSize)
					{
						if (dataString[stringIndex] != ',' && dataString[stringIndex] != '<' && dataString[stringIndex] != '>')
						{
							dataBuff += dataString[stringIndex];
						}
						else
						{
							dataVect[vectIndex] = dataBuff;
							dataBuff.clear();
							vectIndex++;
						}
						stringIndex++;
					}

					for (int i = 0; i < dCopySize; i++)
					{
						cout << dataVect[i];
					}

					cout << "\n";

					try 
					{
						//setParameterFloat(strips[0], stoi(dataVect[8]));
						setParameterFloat(strips[1], stoi(dataVect[9]));
						//setParameterFloat(strips[2], stoi(dataVect[10]));
						//setParameterFloat(strips[3], stoi(dataVect[11]));
						//setParameterFloat(strips[4], stoi(dataVect[12]));
						//setParameterFloat(strips[5], stoi(dataVect[13]));
					}
					catch (...) 
					{
						cout << "Error setting parameters\n";
					}
					waitForUpdate();
				}
				else
				{
					Sleep(1);
				}
			}
		}
	}
	else
	{
		cout << "Error establishing connection with Voicemeeter\n";
	}
	arduino->closeSerial();
	iVMR.VBVMR_Logout();
}


//char param1[14] = "Strip[0].gain";
//float val = 5;
//setParameterFloat(param1, val);

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
