#pragma once

#define EXPORT __declspec(dllexport)

#include "Storage3D.hpp"

#include <chrono>
#include <fstream>
#include <thread>

// enumerate with debug levels
enum class DebugLeveli
{
	NoDebug,
	OnlyInfo,
	OnlyErrors,
	Info_and_Errors,
	doExceptions
};

// enumerate with result on debug places
enum class DebugResult
{
	NoErrors,
	MissingFile,
	NoStorage
};

// you can set debug level separately, or using this function
EXPORT void setOBJ3D_DebugLevel(DebugLeveli param);
// and get current debug level
EXPORT uint16_t getOBJ3D_DebugLevel();

// using this function you can check and use all available CPU threads
EXPORT void initializeOBJ3D_MultiThreading();
// or set single if you want do everything on single
EXPORT void setOBJ3D_SingleThreading();
// if you would get current threads count use this function
EXPORT uint32_t getOBJ3D_ThreadsCount();

// use this function if you want catch result of loading functions
EXPORT void catchOBJ3D_DebugResult(DebugResult result);

class EXPORT OBJ3D_Loader
{
private:
	std::string filePath = "";
public:
	DebugResult setFilePath(std::string newPath);
	OBJ3D_Loader();
	OBJ3D_Loader(std::string initialFilePath);
	DebugResult loadMaterial(OBJ3D_Buffer* dataStorage);
	DebugResult loadOBJ(OBJ3D_Buffer* dataStorage);
};
