﻿// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#pragma warning(disable:4996)
// define types
typedef unsigned char _BOOL8;

// define functions
typedef _BOOL8(__fastcall * _loadCPK)(__int64 *cpkid, __int64 unk, __int64 baseAddr);

_loadCPK loadCPK = NULL;

// variables
DWORD LOAD_CPK_ADDRESS = 0x644000;
__int64 DATA_PATH = 0x141588BC0;
__int64 IS_IN_GAME_MENU = 0x1411063D0;

uintptr_t BASE_ADDRESS = (uintptr_t)GetModuleHandle(NULL);

const int IGNORE_CPKS_SIZE = 18;

const char* ignoreCpks[] = {
	"data000.cpk", "data001.cpk", "data002.cpk", "data003.cpk",
	"data004.cpk", "data005.cpk", "data006.cpk", "data008.cpk",
	"data009.cpk", "data010.cpk", "data012.cpk", "data013.cpk",
	"data014.cpk", "data015.cpk", "data016.cpk", "data018.cpk",
	"data019.cpk", "data100.cpk"
};

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

bool value_exists_in_array(const char * value, const char* arr[], int size) {
	for (int i = 0; i < size; i++) {
		if (strcmp(arr[i], value) == 0) {
			return true;
		}
	}

	return false;
}

// where the magic happens
_BOOL8 MountCpk(const char* cpkName) {
	if(loadCPK == NULL)
		loadCPK = (_loadCPK)((uintptr_t)GetModuleHandle(NULL) + LOAD_CPK_ADDRESS);

	return loadCPK((__int64*)&cpkName, 0x0, BASE_ADDRESS);
}

// Load cpks
void LoadCpks() {
	char* data = ((char*)DATA_PATH);

	// Don't do anything until the game allocates the data path to DATA_PATH (0x141588BC0)
	while (strlen(data) == 0)
	{

	}

	data = ((char*)DATA_PATH);

	// Don't do anything until the player loads a save
	while (*(int*)IS_IN_GAME_MENU == 0) {
	}

	// Get all files in /data
	for (recursive_directory_iterator y(data), end; y != end; ++y) {
		// If path is not a directory
		if (!is_directory(y->path()))
		{
			// Get file extension
			std::string ext = y->path().extension().string();
			const char* extension = ext.c_str();

			// File path string
			std::string path = y->path().string();

			// Replace the absolute data path with an empty string
			replace(path, data, "");
			
			// Get final cpk file
			const char* cpkFile = path.c_str();

			// If the file is a cpk
			if (strcmp(extension, ".cpk") == 0) {
				// To prevent loading the game's cpks we will check if it's in the ignoredCpks array.
				if (!value_exists_in_array(cpkFile, ignoreCpks, IGNORE_CPKS_SIZE)) {
					// Replace the extension with an empty string
					replace(path, extension, "");

					// Mount cpk
					if (!MountCpk(cpkFile)) {
						cout << "File: " << cpkFile << " could not be loaded." << endl;
					}
					else cout << cpkFile << " was successfully loaded." << endl;
				}
			}
		}
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, 0, (unsigned long(__stdcall*)(void*))LoadCpks, NULL, 0, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

