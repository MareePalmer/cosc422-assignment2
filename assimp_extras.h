// ----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)


void get_bounding_box_for_node (const aiScene* scene, aiNode* nd, aiVector3D* min,  aiVector3D* max, 
	aiMatrix4x4 trafo)
{
	aiMatrix4x4 prev = trafo;
	unsigned int n = 0, t;

	aiMultiplyMatrix4(&trafo, &nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n)
	{
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t)
		{

			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp, &trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(scene, nd->mChildren[n],min,max,trafo);
	}
	trafo = prev;
}

// ----------------------------------------------------------------------------
void get_bounding_box (const aiScene *scene, aiVector3D* min, aiVector3D* max)
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene, scene->mRootNode, min, max, trafo);
}


// ----------------------------------------------------------------------------
void printSceneInfo(const aiScene* scene)
{
	if(scene != NULL)
	{
		cout << "======================= Scene Data ========================" << endl;
		cout << "Number of animations = " << scene->mNumAnimations << endl;
		cout << "Number of cameras = " << scene->mNumCameras << endl;
		cout << "Number of lights = " << scene->mNumLights << endl;
		cout << "Number of materials = " << scene->mNumMaterials << endl;
		cout << "Number of meshes = " << scene->mNumMeshes << endl;
		cout << "Number of textures = " << scene->mNumTextures << endl;
	}
	else
		cout << "*********** ERROR: Empty scene ****************" << endl;
}

// ----------------------------------------------------------------------------
void printMeshInfo(const aiScene* scene)
{
	int numMesh = scene->mNumMeshes;
	int matIndx;
	aiMaterial* mtl;
	aiColor4D diffuse;
	cout << "======================= Mesh Data =============================" << endl;
	cout << "Number of meshes = " << numMesh << endl;
	
	for (int n = 0; n < numMesh; ++n)
	{
		aiMesh* mesh = scene->mMeshes[n];
		matIndx = mesh->mMaterialIndex;
		cout << "Mesh index " << n << ": nverts = " << mesh->mNumVertices << "  nfaces =  " <<
			mesh->mNumFaces << "  nbones = " << mesh->mNumBones << "  Material index = " << matIndx << endl;
		mtl = scene->mMaterials[matIndx];
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))  //Get material colour if available
			cout << "         Material colour: " << diffuse.r << "  " << diffuse.g << "  " << diffuse.b << endl;
		if (mesh->HasTextureCoords(0))
			cout << "Mesh has texture coordinates." << endl;
		else
			cout << "Mesh does not have texture coordinates."<<endl;
		if (mesh->HasVertexColors(0))
			cout << "Mesh has vertex colors." << endl;
		else
			cout << "Mesh does not have vertex colors." << endl;
		if (mesh->HasNormals())
			cout << "Mesh has vertex normals." << endl;
		else
			cout << "Mesh does not have vertex normals." << endl;
	}

}

// ----------------------------------------------------------------------------
void printTreeInfo(const aiNode* node)
{
	int numMesh = node->mNumMeshes;
	const char * parentName;
	float* mat = new float[16];
	if (node->mParent != NULL) parentName = (node->mParent->mName).C_Str();
	else parentName = "NO PARENT";
	cout << "==================== Node Data ===========================" << endl;
	cout << "Node Name: " << (node->mName).C_Str() << "  Parent: " << parentName <<
		"  nchild = " << node->mNumChildren << "  nmesh = " << numMesh << endl;
	if (numMesh > 0)
	{
		cout << "Mesh indices: ";
		for (int n = 0; n < numMesh; n++) cout << node->mMeshes[n] << " ";
		cout << endl;
	}
	cout << "Transformation:  " ;
	mat = (float *)&(node->mTransformation.a1);
	for (int n = 0; n < 16; ++n) cout << mat[n] << " " ;
	cout << endl;

	for (int n = 0; n < node->mNumChildren; n++)
		printTreeInfo(node->mChildren[n]);
}

// ----------------------------------------------------------------------------
void printBoneInfo(const aiScene* scene)
{
		float* mat = new float[16];
		cout << "==================== Bone Data ===========================" << endl;
		int nd = scene->mNumMeshes;
		for (int n = 0; n < scene->mNumMeshes; ++n)
		{
			aiMesh* mesh = scene->mMeshes[n];
			if(mesh->HasBones())
			{
				for(int i = 0; i < mesh->mNumBones; i++)
				{
					aiBone* bone = mesh->mBones[i];
					cout << "Bone Name: " << (bone->mName).C_Str() << "   Mesh: " << n << "  nweights = " 
						<< bone->mNumWeights << endl;
					mat = &(bone->mOffsetMatrix.a1);
					cout << "     Offset matrix: ";
					for (int k = 0; k < 16; k++) cout << mat[k] << " " ;
					cout << endl;
					cout << "      Vertex ids: " << (bone->mWeights[0]).mVertexId << "  " 
						<< (bone->mWeights[bone->mNumWeights-1]).mVertexId << endl;
				}
			}
		}
}

// ----------------------------------------------------------------------------
void printAnimInfo(const aiScene* scene)
{
	float* pos = new float[3];
	float* quat = new float[4];
	if(scene != NULL)
	{
		cout << "==================== Animation Data ===========================" << endl;
		cout << "Number of animations = " << scene->mNumAnimations << endl;

		for (int n = 0; n < scene->mNumAnimations; ++n)
		{
			aiAnimation* anim = scene->mAnimations[n];
			cout << " --- Anim " << n << ":  Name = " << (anim->mName).C_Str() << 
				"  nchanls = " << anim->mNumChannels << " nticks = " << anim->mTicksPerSecond <<
				"  duration (ticks) = " << anim->mDuration << endl;
			for(int i = 0; i < anim->mNumChannels; i++)
			{
				aiNodeAnim* ndAnim = anim->mChannels[i];
				cout << "     Channel " << i << ": nodeName = " << (ndAnim->mNodeName).C_Str()  << " nposkeys = " << ndAnim->mNumPositionKeys << "  nrotKeys = " <<
					ndAnim->mNumRotationKeys << " nsclKeys = " << ndAnim->mNumScalingKeys << endl;
				for(int k = 0; k < ndAnim->mNumPositionKeys; k++)
				{
					aiVectorKey posKey = ndAnim->mPositionKeys[k];    //Note: Does not return a pointer
					pos = (float*)&posKey.mValue;
					cout <<  "        posKey " << k << ":  Time = " << posKey.mTime << " Value = " << pos[0] << " " << pos[1] << " " << pos[2] << endl;
				}
				for(int k = 0; k < ndAnim->mNumRotationKeys; k++)
				{
					aiQuatKey rotnKey = ndAnim->mRotationKeys[k];    //Note: Does not return a pointer
					quat = (float*)&rotnKey.mValue;
					cout <<  "        rotnKey " << k << ":  Time = " << rotnKey.mTime << " Value = " << quat[0] << " " << 
						quat[1] << " " << quat[2] << " " << quat[3] <<endl;
				}
			}
		}
	}
}