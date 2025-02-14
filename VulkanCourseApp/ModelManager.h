#pragma once

#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "MeshModel.h"
#include "TextureManager.h"
#include "DeviceManager.h"

class ModelManager
{
public:
	ModelManager();

	ModelManager(DeviceManager* mainDevice, CommandPoolManager* commandPoolManager, TextureManager* textureManager);

	int createMeshModel(std::string modelFile, VkSampler* textureSampler);

	void setModel(int modelId, glm::mat4 newModel) {
		modelList[modelId].setModel(newModel);
	}

	void destroyModel(int i) {
		modelList[i].destroyMeshModel();
	}

	int getModelListSize() {
		return modelList.size();
	}

	std::vector<MeshModel>* getModelList() {
		return &modelList;
	}

	~ModelManager();

private:
	DeviceManager* mainDevice;
	CommandPoolManager* commandPoolManager;
	TextureManager* textureManager;

	std::vector<MeshModel> modelList;
};

