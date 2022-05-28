#include "pch.h"
#include "OBJ3D.hpp"
#include <Windows.h>
#include <iostream>
#include <filesystem>

// operator for equal operation of two IndexInfo values
inline bool operator==(IndexInfo value0, IndexInfo value1)
{
	if (value0.normalVectorIndex == value1.normalVectorIndex)
	{
		if (value0.positionVertexIndex == value1.positionVertexIndex)
		{
			if (value0.textureCoordsVectorIndex == value1.textureCoordsVectorIndex)
			{
				return true;
			}
		}
	}
	return false;
}


// variable that specifies debug level which library would use
static DebugLeveli DebugLevel = DebugLeveli::doExceptions;
static uint32_t threadsCount = 1;


// debug level functions
void setOBJ3D_DebugLevel(DebugLeveli param)
{
	DebugLevel = param;
}
uint16_t getOBJ3D_DebugLevel()
{
	return uint32_t(DebugLevel);
}
void catchOBJ3D_DebugResult(DebugResult result)
{
	if (DebugLevel == DebugLeveli::NoDebug)
	{
		return;
	}
	else if (DebugLevel == DebugLeveli::OnlyErrors || DebugLevel == DebugLeveli::Info_and_Errors || DebugLevel == DebugLeveli::doExceptions)
	{
		if (result == DebugResult::MissingFile)
		{
			if (DebugLevel == DebugLeveli::OnlyErrors || DebugLevel == DebugLeveli::Info_and_Errors)
			{
				std::cerr << "[OBJ3D ERROR] There is missing files. Loading aborted." << std::endl;
			}
			else if (DebugLevel == DebugLeveli::doExceptions)
			{
				std::string errorText = "There is missing files. Loading aborted.";
				MessageBoxA(NULL, &errorText[0], "[OBJ3D ERROR] MISSING FILE", MB_OK);
				throw;
			}
		}
		else if (result == DebugResult::NoStorage)
		{
			if (DebugLevel == DebugLeveli::OnlyErrors || DebugLevel == DebugLeveli::Info_and_Errors)
			{
				std::cerr << "[OBJ3D ERROR] Loader has no storage! Loading aborted." << std::endl;
			}
			else if (DebugLevel == DebugLeveli::doExceptions)
			{
				std::string errorText = "Loader has no storage! Loading aborted.";
				MessageBoxA(NULL, &errorText[0], "[OBJ3D ERROR] MISSING STORAGE", MB_OK);
				throw;
			}
		}
	}
	if (DebugLevel == DebugLeveli::Info_and_Errors || DebugLevel == DebugLeveli::OnlyInfo)
	{
		if (result == DebugResult::NoErrors)
		{
			std::cout << "[OBJ3D INFO] There is no errors" << std::endl;
		}
	}
}

// threading functions
void initializeOBJ3D_MultiThreading()
{
	threadsCount = std::thread::hardware_concurrency();
}
void setOBJ3D_SingleThreading()
{
	threadsCount = 1;
}
uint32_t getOBJ3D_ThreadsCount()
{
	return threadsCount;
}

// implicit OBJ3D methods
inline void getStringToChar(std::string text, uint32_t& pointer, char endChar, std::string& buffer)
{
	std::string cache;

	if (text[pointer] == endChar)
	{
		pointer++;
	}
	while (pointer < text.length() && text[pointer] != endChar)
	{
		cache += text[pointer];
		if (text[pointer] == '\n')
		{
			break;
		}
		pointer++;
	}
	if (cache[cache.length() - 1] == '\n')
	{
		cache.erase(cache.length() - 1, 1);
	}
	if (cache[cache.length() - 1] == endChar)
	{
		cache.erase(cache.length() - 1, 1);
	}
	buffer = cache;
}
bool checkValueIsAlreadyExists(std::vector<IndexInfo>& whereSearch, IndexInfo valueToFind)
{
	for (uint32_t i = 0; i < whereSearch.size(); i++)
	{
		if (whereSearch[i] == valueToFind)
		{
			return true;
		}
	}
	return false;
}
void getElements(std::vector<IndexInfo>& elementsFromFile, std::vector<uint32_t>& dst, std::vector<IndexInfo>& uniqueVectors)
{
	for (uint32_t i = 0; i < elementsFromFile.size(); i++)
	{
		for (uint32_t ID = 0; ID < uniqueVectors.size(); ID++)
		{
			if (elementsFromFile[i] == uniqueVectors[ID])
			{
				dst.push_back(ID);
				break;
			}
		}
	}
}

// OBJ3D methods
OBJ3D_Loader::OBJ3D_Loader() {}
OBJ3D_Loader::OBJ3D_Loader(std::string initialFilePath)
{
	DebugResult result = OBJ3D_Loader::setFilePath(initialFilePath);
	if (result == DebugResult::MissingFile)
	{
		if (DebugLevel == DebugLeveli::OnlyErrors || DebugLevel == DebugLeveli::Info_and_Errors)
		{
			std::cerr << "[OBJ3D ERROR] Couldn't find file: " << initialFilePath << std::endl;
		}
		else if (DebugLevel == DebugLeveli::doExceptions)
		{
			std::string errorText = "Couldn't find file: " + initialFilePath;
			MessageBoxA(NULL, &errorText[0], "[OBJ3D ERROR] MISSING FILE", MB_OK);
			throw;
		}
	}
}
DebugResult OBJ3D_Loader::setFilePath(std::string newPath)
{
	if (!std::filesystem::exists(newPath + ".obj") || !std::filesystem::exists(newPath + ".mtl"))
	{
		return DebugResult::MissingFile;
	}
	OBJ3D_Loader::filePath = newPath;
	return DebugResult::NoErrors;
}

DebugResult OBJ3D_Loader::loadMaterial(OBJ3D_Buffer* dataStorage)
{
	// check file path is not empty
	if (OBJ3D_Loader::filePath == "")
	{
		return DebugResult::MissingFile;
	}
	// check user gave pointer to storage
	if (dataStorage == nullptr)
	{
		return DebugResult::NoStorage;
	}

	// load .mtl
	std::string fileBuffer = "";
	std::ifstream MTL_file;
	MTL_file.open(filePath + ".mtl");
	std::getline(MTL_file, fileBuffer, '\0');
	fileBuffer += '\0';
	MTL_file.close();

	std::string pointerInFileBufferCache;
	uint32_t pointerInFileBuffer = 0;
	int32_t materialID = -1;

	while (pointerInFileBuffer < fileBuffer.length())
	{
		getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
		pointerInFileBuffer++;
		if (pointerInFileBufferCache == "Count:")
		{
			// get materials count
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->setMaterialsCount(std::stoul(pointerInFileBufferCache));
		}
		else if (pointerInFileBufferCache == "newmtl")
		{
			// catch declaration of a new material
			materialID++;
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].Name = pointerInFileBufferCache;
		}
		else if (pointerInFileBufferCache == "Ns")
		{
			// get specular intensity component
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].specularWeight = std::stof(pointerInFileBufferCache);
		}
		else if (pointerInFileBufferCache == "Ka")
		{
			// get ambient color component
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			dataStorage->materials[materialID].ambientColor.x = std::stof(pointerInFileBufferCache);
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			dataStorage->materials[materialID].ambientColor.y = std::stof(pointerInFileBufferCache);
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].ambientColor.z = std::stof(pointerInFileBufferCache);
		}
		else if (pointerInFileBufferCache == "Kd")
		{
			// get diffuse color component
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			dataStorage->materials[materialID].albedoColor.x = std::stof(pointerInFileBufferCache);
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			dataStorage->materials[materialID].albedoColor.y = std::stof(pointerInFileBufferCache);
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].albedoColor.z = std::stof(pointerInFileBufferCache);
		}
		else if (pointerInFileBufferCache == "Ks")
		{
			// get specular color component
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			dataStorage->materials[materialID].specularColor.x = std::stof(pointerInFileBufferCache);
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			dataStorage->materials[materialID].specularColor.y = std::stof(pointerInFileBufferCache);
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].specularColor.z = std::stof(pointerInFileBufferCache);
		}
		else if (pointerInFileBufferCache == "Ke")
		{
			// get emission color component
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			dataStorage->materials[materialID].emission.x = std::stof(pointerInFileBufferCache);
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			dataStorage->materials[materialID].emission.y = std::stof(pointerInFileBufferCache);
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].emission.z = std::stof(pointerInFileBufferCache);
		}
		else if (pointerInFileBufferCache == "Ni")
		{
			// get refraction component
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].refractionIntensity = std::stof(pointerInFileBufferCache);
		}
		else if (pointerInFileBufferCache == "d")
		{
			// get alpha of material component
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].materialAlpha = std::stof(pointerInFileBufferCache);
		}
		else if (pointerInFileBufferCache == "illum")
		{
			// get illumination model of material
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].illuminationModel = std::stoul(pointerInFileBufferCache);
		}
		else if (pointerInFileBufferCache == "map_Bump")
		{
			// get path to normal map
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			if (pointerInFileBufferCache == "-bm")
			{
				// get normal map strength
				getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
				dataStorage->materials[materialID].NormalMapStrength = std::stof(pointerInFileBufferCache);
				getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			}
			dataStorage->materials[materialID].NormalMapName = pointerInFileBufferCache;
		}
		else if (pointerInFileBufferCache == "map_Kd")
		{
			// get path to diffuse map
			getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
			dataStorage->materials[materialID].AlbedoMapName = pointerInFileBufferCache;
		}

	}
	return DebugResult::NoErrors;

}
DebugResult OBJ3D_Loader::loadOBJ(OBJ3D_Buffer* dataStorage)
{
	std::cerr << ".obj" << std::endl;
	// check file path is not empty
	if (OBJ3D_Loader::filePath == "")
	{
		return DebugResult::MissingFile;
	}
	// check user gave pointer to storage
	if (dataStorage == nullptr)
	{
		return DebugResult::NoStorage;
	}

	std::cout << ".obj" << std::endl;

	// declare vectors where data will be stored
	std::vector<OBJ3D_vec3> allVerticies;
	std::vector<OBJ3D_vec3> allNormalVector;
	std::vector<OBJ3D_vec2> allTextureCoords;
	std::vector<std::string> materialInfoPerVertex;
	std::vector<IndexInfo> allIndicies;
	std::vector<IndexInfo> uniqueVerticies;

	// this variable is specifying how many vectors have X material
	uint32_t verticesCountPerMaterial = 0;

	// here are declared variables which library use later
	std::string fileBuffer;
	std::string pointerInFileBufferCache = "";
	uint32_t pointerInFileBuffer = 0;
	std::ifstream OBJ_File;
	OBJ3D_vec3 cacheVector;
	OBJ3D_vec2 coordsVectorCache;
	IndexInfo indexInfoCache;


	// open file and start read it
	OBJ_File.open(OBJ3D_Loader::filePath + ".obj");
	std::cout << ".obj" << std::endl;
	while (!OBJ_File.eof())
	{
		// download content from file
		std::getline(OBJ_File, fileBuffer, '\n');
		pointerInFileBuffer = 0;

		// process cache downloaded from file
		while (pointerInFileBuffer < fileBuffer.length())
		{
			getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
			pointerInFileBuffer++;
			if (pointerInFileBufferCache == "v")
			{
				verticesCountPerMaterial++;
				getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
				cacheVector.x = std::stof(pointerInFileBufferCache);
				getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
				cacheVector.y = std::stof(pointerInFileBufferCache);
				getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
				cacheVector.z = std::stof(pointerInFileBufferCache);
				allVerticies.push_back(cacheVector);
			}
			else if (pointerInFileBufferCache == "vt")
			{
				getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
				coordsVectorCache.x = std::stof(pointerInFileBufferCache);
				getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
				coordsVectorCache.y = std::stof(pointerInFileBufferCache);
				allTextureCoords.push_back(coordsVectorCache);
			}
			else if (pointerInFileBufferCache == "vn")
			{
				getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
				cacheVector.x = std::stof(pointerInFileBufferCache);
				getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
				cacheVector.y = std::stof(pointerInFileBufferCache);
				getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
				cacheVector.z = std::stof(pointerInFileBufferCache);
				allNormalVector.push_back(cacheVector);
			}
			if (pointerInFileBufferCache == "usemtl")
			{
				getStringToChar(fileBuffer, pointerInFileBuffer, '\n', pointerInFileBufferCache);
				for (uint32_t i = 0; i < verticesCountPerMaterial; i++)
				{
					materialInfoPerVertex.push_back(pointerInFileBufferCache);
				}
				verticesCountPerMaterial = 0;
			}
			if (pointerInFileBufferCache == "f")
			{
				for (uint8_t i = 0; i < 3; i++)
				{
					getStringToChar(fileBuffer, pointerInFileBuffer, '/', pointerInFileBufferCache);
					indexInfoCache.positionVertexIndex = std::stoul(pointerInFileBufferCache) - 1;
					getStringToChar(fileBuffer, pointerInFileBuffer, '/', pointerInFileBufferCache);
					indexInfoCache.textureCoordsVectorIndex = std::stoul(pointerInFileBufferCache) - 1;
					pointerInFileBuffer++;
					getStringToChar(fileBuffer, pointerInFileBuffer, ' ', pointerInFileBufferCache);
					indexInfoCache.normalVectorIndex = std::stoul(pointerInFileBufferCache) - 1;

					if (!checkValueIsAlreadyExists(uniqueVerticies, indexInfoCache))
					{
						uniqueVerticies.push_back(indexInfoCache);
					}
					allIndicies.push_back(indexInfoCache);
				}
			}
		}
	}

	OBJ_File.close();

	// get elements
	std::vector<uint32_t> generatedElements;
	getElements(allIndicies, generatedElements, uniqueVerticies);


	// copy everything to storage
	dataStorage->setMeshElementsData(generatedElements.data());

	std::vector<VertexInfo> mesh;
	mesh.reserve(uniqueVerticies.size());

	for (uint32_t i = 0; i < uniqueVerticies.size(); i++)
	{
		mesh[i].materialName = materialInfoPerVertex[uniqueVerticies[i].positionVertexIndex];
		mesh[i].normalVector = allNormalVector[uniqueVerticies[i].normalVectorIndex];
		mesh[i].position = allVerticies[uniqueVerticies[i].textureCoordsVectorIndex];
		mesh[i].textureCoordination = allTextureCoords[uniqueVerticies[i].textureCoordsVectorIndex];
	}

	
	dataStorage->setMeshData(mesh.size(), mesh.data());

	// report info about loaded model
	if (DebugLevel == DebugLeveli::OnlyInfo || DebugLevel == DebugLeveli::Info_and_Errors)
	{
		std::cout << '\n' << "========================" << std::endl;
		std::cout << "File: " << OBJ3D_Loader::filePath << ".obj" << std::endl;
		std::cout << "Verticies: " << uniqueVerticies.size() << std::endl;
		std::cout << "Triangles: " << allIndicies.size() / 3 << std::endl;
		std::cout << "-------------" << std::endl;
		std::cout << "File readed in: " << 0.0f << std::endl;
		std::cout << "Elements generated in: " << 0.0f << std::endl;
		std::cout << "Copied to storage in: " << 0.0f << std::endl;
	}


	return DebugResult::NoErrors;
}