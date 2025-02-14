#include "ModelManager.h"

ModelManager::ModelManager()
{
}

ModelManager::ModelManager(DeviceManager* mainDevice, CommandPoolManager* commandPoolManager, TextureManager* textureManager)
{
	this->mainDevice = mainDevice;
	this->commandPoolManager = commandPoolManager;
	this->textureManager = textureManager;
}

int ModelManager::createMeshModel(std::string modelFile, VkSampler* textureSampler)
{
	// Import mode "scene"
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if (!scene) {
		throw std::runtime_error("Failed to load model! (" + modelFile + ")");
	}

	// Get vector of all materials with 1:1 ID placement
	std::vector<std::string> textureNames = MeshModel::LoadMaterials(scene);

	// Conversion from the materials list IDs to our Descriptor Array IDs
	std::vector<int> matToTex(textureNames.size());

	// Loop over textureNames and create textures for them
	for (size_t i = 0; i < textureNames.size(); i++) {
		// If material had no texture, set '0' to indicate no texture, texture 0 will be reserved for a default texture
		if (textureNames[i].empty()) {
			matToTex[i] = 0;
		}
		else {
			// Otherwise, create texture and set value to index of new texture
			matToTex[i] = textureManager->createTexture(textureNames[i], textureSampler);
		}
	}

	// Load in all our meshes
	std::vector<Mesh> modelMeshes = MeshModel::LoadNode(mainDevice->getPhysicalDevice(), mainDevice->getLogicalDevice(), mainDevice->getGraphicsQueue(),
		*commandPoolManager->getGraphicsCommandPool(), scene->mRootNode, scene, matToTex);

	// Create mesh model and add to list
	MeshModel meshModel = MeshModel(modelMeshes);
	modelList.push_back(meshModel);

	return modelList.size() - 1;
}

ModelManager::~ModelManager()
{
}
