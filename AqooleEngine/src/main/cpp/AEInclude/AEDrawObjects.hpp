//Copyright 2022 Shigeoka Kodai
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//        limitations under the License.
//
//          Copyright Joe Coder 2004 - 2006.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef _AE_DRAW_OBJECT
#define _AE_DRAW_OBJECT

#define _USE_MATH_DEFINES
#ifdef __ANDROID__
#include <vulkan_wrapper.h>
#endif
#include "glm/glm.hpp"
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <thread>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <cstring>
#include <limits>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/foreach.hpp"
#include "boost/lexical_cast.hpp"
#include "regex"
#include "mutex"
#include "tiny_gltf.h"

/*
prototypes
*/
struct Vertex3D;
struct Vertex3DTexture;
struct Vertex3DObj;
struct Vertex2D;
struct Vertex3DTEST;
struct GltfMaterial;
class AEComputePipeline;
class AELogicalDevice;
class AECommandPool;
class AEDeviceQueue;
class AECommandBuffer;
class AEDescriptorSetLayout;
class AEDescriptorPool;
class AEBufferBase;
class AEDescriptorSet;
class AEBufferUtilOnGPU;
class AEBufferUniform;
class AESemaphore;
class AEFence;
class AEEvent;

namespace AEDrawObject
{
    void AddOffsetToIndex(std::vector<uint32_t> &indices, uint32_t offset);
    void LoadModel();
    //for private
    void Split(std::vector<std::string> &fields, std::string& oneLine, const char delimiter);
    void GetInAngleBrackets(std::string &output, std::string const& oneLine);
    std::string GetRootDirName(std::string &modelPath);
}

class AETorus
{
    private:
    std::vector<Vertex3D> mVertices;
    std::vector<std::array<uint32_t, 3>> mIndices;
    void Debug();
    public:
    AETorus(uint32_t circle, uint32_t circleNum, float irad, float orad);
    ~AETorus();
    //iterator
    void GetIndices(std::vector<uint32_t> &indices);
    void GetVertices(std::vector<Vertex3D> &vertices);
    std::vector<Vertex3D>& GetVertexAddress(){return mVertices;}
    uint32_t GetVertexSize(){return mVertices.size();}
    uint32_t GetIndexSize(){return mIndices.size() * 3;}
};

class AECoordinates
{
    private:
    std::vector<Vertex3D> mXZ;
    std::vector<uint32_t> mIndices;
    //functions
    void CreateVertex(Vertex3D const& v);
    public:
    AECoordinates(glm::vec3 center, float halfWidth, float interval);
    ~AECoordinates();
    //iterator
    uint32_t GetVertexSize()const{return mXZ.size();}
    std::vector<Vertex3D>const& GetVertexAddress()const{return mXZ;}
    uint32_t GetIndexSize()const{return mIndices.size();}
    std::vector<uint32_t>const& GetIndexAddress()const{return mIndices;}
};

/*
1st base class
*/
class AEDrawObjectBase
{
    protected:
    std::vector<uint32_t> mIndices;
    //function
    void AddIndex(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
    void AddIndex(uint32_t a, uint32_t b, uint32_t c);
    void CalcNormal();
    public:
    AEDrawObjectBase();
    virtual ~AEDrawObjectBase();
    uint32_t GetIndexBufferSize();
    //iterator
    uint32_t GetIndexSize()const{return mIndices.size();}
    std::vector<uint32_t>const& GetIndexAddress()const{return mIndices;}
    std::vector<uint32_t>& GetIndexAddressNotConst(){return mIndices;}
};

/*
2nd base class
*/
class AEDrawObjectBase3D : public AEDrawObjectBase
{
    protected:
    std::vector<Vertex3D> mVertices;
    //functions
    void AddVertex(glm::vec3 const& v, glm::vec3 const& color);
    void AddVertex(glm::vec3 const& v, glm::vec3 const& color, glm::vec3 const& normal);
    void AddVertex(glm::vec3 const& v, glm::vec3 const& color, Vertex3D &reuse);
    void CalcNormal();
    public:
    AEDrawObjectBase3D();
    virtual ~AEDrawObjectBase3D();
    uint32_t GetVertexBufferSize();
    //iterator
    uint32_t GetVertexSize()const{return mVertices.size();}
    std::vector<Vertex3D>const& GetVertexAddress()const{return mVertices;}
};

class AEDrawObjectBaseTexture : public AEDrawObjectBase
{
    protected:
    std::vector<Vertex3DTexture> mVertices;
    //functions
    void AddVertex(glm::vec3 const& v, glm::vec3 const& color, glm::vec3 const& normal,
        glm::vec2 const& texCoord, Vertex3DTexture &reuse);
    public:
    AEDrawObjectBaseTexture();
    virtual ~AEDrawObjectBaseTexture();
};

class AEDrawObjectBaseObjFile : public AEDrawObjectBase
{
    protected:
    std::vector<Vertex3DObj> mVertices;
    std::vector<std::string> mMaterials;
    std::vector<uint32_t> mOffsets;
    std::string mMatFileName;
    std::vector<std::string> mTextureFiles;
    //functions
    void AddVertex(glm::vec3 const& normal, glm::vec2 const& texCoord, Vertex3DTexture &reuse);
#ifndef __ANDROID__
    void ReadMtlFile();
#else
    void ReadMtlFile(android_app* app);
#endif
    void CalcTangent();
    public:
#ifndef __ANDROID__
    AEDrawObjectBaseObjFile(const char* filePath);
#else
    AEDrawObjectBaseObjFile(const char* filePath, android_app* app, bool isReverseY);
#endif
    virtual ~AEDrawObjectBaseObjFile();
    //iterator
    uint32_t GetVertexSize(){return mVertices.size();}
    uint32_t GetVertexBufferSize();
    std::vector<Vertex3DObj>const& GetVertexAddress()const{return mVertices;}
    uint32_t GetTextureCount(){return mTextureFiles.size();}
    std::string& GetTexturePath(uint32_t index){return mTextureFiles[index];}
    uint32_t GetOffset(uint32_t index){return mOffsets[index];}
    std::vector<uint32_t>const& GetOffsetAll(){return mOffsets;}
    //scale
    void Scale(float multiple);
};

class AEDrawObjectBaseCollada : public AEDrawObjectBase
{
protected:
    struct Geometry
    {
        std::string geometryId;
        std::string sourceID;
    };
    struct SkeletonNode
    {
        glm::mat4 matrix;
        std::string sidName;
        std::string id;
        std::vector<std::unique_ptr<SkeletonNode>> children;
        int jointNo;
    };
    struct JointWeight
    {
        std::vector<uint32_t> jointIndices;
        std::vector<float> weights; 
    };
    struct AnimationMatrix
    {
        std::string id;
        std::vector<glm::mat4> matrixList;
        std::string target;
    };
    struct JointMapper
    {
        std::string jointName;
        int animNo;
        std::string nodeId;
        std::vector<std::pair<uint32_t, float>> indexWeight;
        glm::mat4 controllerMatrix;
    };
    struct TextureMap
    {
        std::string textureImage;
        std::string texcoord;
    };
    struct TextureImage
    {
        std::string imageId;
        std::string fileName;
    };
    std::vector<Vertex3DObj> mVertices;
    std::vector<std::string> mMaterials;
    std::vector<uint32_t> mOffsets;
    std::string mMatFileName;
    std::vector<TextureImage> mTextureFiles;
    std::vector<Geometry> mGeometries;
    std::vector<std::vector<glm::vec3>> mPositions;
    std::vector<std::vector<glm::vec3>> mNormals;
    std::vector<std::vector<glm::vec2>> mMaps;
    std::vector<std::vector<uint32_t>> mPositionIndices;
    std::vector<std::vector<uint32_t>> mNormalsIndices;
    std::vector<std::vector<uint32_t>> mMapIndices;
    std::unique_ptr<AEDrawObjectBaseCollada::SkeletonNode> mRoot;
    std::vector<JointMapper> mSkinJointsArray;
    std::vector<AEDrawObjectBaseCollada::JointWeight> mJointWeights;
    std::vector<glm::mat4> mInverseMatrices;
    std::vector<AnimationMatrix> mAnimationMatrices;
    glm::mat4 mGlobalInverseMatrix;
    std::unique_ptr<AEComputePipeline> mComputePipeline;
    std::vector<glm::mat4> mAnimationTransforms;
    std::vector<glm::mat4> mAnimationTransformsNext;
    std::vector<uint32_t> mVerteces2;
    uint32_t mComputeLocalSize;
    std::vector<uint32_t> mJoints;
    std::vector<float> mWeights;
    std::vector<std::unique_ptr<AEDescriptorSet>> mDSs;
    std::vector<std::unique_ptr<AEBufferUtilOnGPU>> mBuffers;
    std::vector<std::unique_ptr<AEBufferUniform>> mUniformBuffers;
    std::vector<glm::vec3> mAnimationPositionDebug;
    std::vector<std::vector<uint32_t>> mInfluenceCountList;
    std::vector<std::unique_ptr<AEBufferUtilOnGPU>> mInfluenceCountBuffers;
    std::vector<std::vector<uint32_t>> mJointOffsetList;
    std::vector<std::unique_ptr<AEBufferUtilOnGPU>> mJointOffsetBuffers;
    std::vector<std::vector<uint32_t>> mSerialPositionIndices;
    std::vector<float> mAnimationTime;
    std::vector<std::unique_ptr<AEBufferUtilOnGPU>> mBasePositionBuffers;
    std::vector<TextureMap> mTextureMap;
    std::vector<glm::mat4> mBindShapeMatrices;
    float mScale;
    //functions
    void MakeVertices();
    void ReadSkeletonNode(boost::property_tree::ptree::const_iterator nowNode,
        std::unique_ptr<AEDrawObjectBaseCollada::SkeletonNode>& skeletonNode);
    void GetVertexWeights(std::vector<float> &vertexWeights, std::string& weightString);
    void ReadAnimation(const boost::property_tree::ptree::value_type& node);
    void ReadEffect(const boost::property_tree::ptree::value_type& node);
    void ReadController(const boost::property_tree::ptree::value_type& node);
    void ReadAnimationNode(const boost::property_tree::ptree::value_type& node);
    void SkeletonJointNo(SkeletonNode* node);
    void SkeletonAnimation(SkeletonNode* node, glm::mat4 parentBindPoseMatrix, glm::mat4 parentAnimationMatrix, glm::mat4 ibp, std::vector<glm::vec3>& tmpPositions);
    void DebugPosition(uint32_t index, std::vector<glm::vec3> const& debug);
    void DebugPositionObj(uint32_t index, std::vector<Vertex3DObj> const& debug);
    void CheckJointAndWeight();
public:
    AEDrawObjectBaseCollada(const char* filePath, android_app* app, AELogicalDevice* device, std::vector<const char*> &shaderPaths,
                            AECommandPool* commandPool, AEDeviceQueue* queue);
    virtual ~AEDrawObjectBaseCollada();
    //iterator
    uint32_t GetVertexSize(){return mVertices.size();}
    std::vector<Vertex3DObj>& GetVertexAddress(){return mVertices;}
    uint32_t GetIndexSize(){return mIndices.size();}
    std::vector<uint32_t>& GetIndexAddress(){return mIndices;}
    uint32_t GetVertexBufferSize();
    uint32_t GetTextureCount(){return mTextureFiles.size();}
    std::string& GetTexturePath(uint32_t index){
        for(uint32_t i = 0; i < mTextureMap.size(); i++){
            if(mTextureMap[index].textureImage == mTextureFiles[i].imageId){
                return mTextureFiles[i].fileName;
            }
        }
        return mTextureFiles[index].fileName;
    }
    uint32_t GetOffset(uint32_t index){return mOffsets[index];}
    std::vector<uint32_t>const& GetOffsetAll(){return mOffsets;}
    std::vector<uint32_t>const& GetMapIndexAddress(uint32_t index)const{return mMapIndices[index];}
    std::vector<glm::vec2>const& GetMapsAddress()const{return mMaps[0];}
    std::vector<uint32_t>const& GetEachMapIndices(uint32_t index)const{return mMapIndices[index];}
    std::vector<std::vector<uint32_t>>const& GetMapIndices()const{return mMapIndices;}
    std::vector<glm::vec3>const& GetPositions(uint32_t index)const{return mPositions[index];}
    uint32_t GetGeometrySize()const{return mPositions.size();}
    uint32_t GetMaterialSize(){return mPositionIndices.size();}
    std::vector<float>const& GetKeyFrames()const{return mAnimationTime;}
    float GetMaxKeyFrame(){return mAnimationTime[mAnimationTime.size() - 1];}
    void SetGeometrySize(std::vector<uint32_t> &geometries){
        uint32_t offset = 0;
        for(uint32_t i = 0; i < mPositions.size(); i++){
            offset += mPositionIndices[i].size();
            geometries.emplace_back(offset);
        }
    }
    void MakeAnimation()
    {
        Animation();
        MakeVertices();
    }
    //scale
    void Scale(float scale);
    //animation
    void Animation();
    void AnimationPrepare(android_app* app, AELogicalDevice* device, std::vector<const char*>& shaders,
                          AEBufferBase* buffer[], AEDeviceQueue* queue, AECommandPool* commandPool, AEDescriptorPool* descriptorPool);
    void AnimationDispatch(AELogicalDevice* device, AECommandBuffer* command, AEDeviceQueue* queue, AECommandPool* commandPool,
                           uint32_t animationNum, AEFence* fence, VkSemaphore *waitSemaphore, VkSemaphore* signalSemaphore,
                           double time, AEEvent* event);
    void AnimationDispatchJoint(SkeletonNode* node, glm::mat4 parentBindPoseMatrix, glm::mat4 parentAnimationMatrix, uint32_t animationNum,
                                std::vector<glm::mat4> &targetTransform);
    void RecordCommand(AELogicalDevice* device, AECommandBuffer* commandBuffer);
    AEBufferUtilOnGPU* GetBuffer(uint32_t index){return mBuffers[index].get();}
    void Debug(AEDeviceQueue* queue, AECommandPool* commandPool);
    void DebugWeights(AEDeviceQueue* queue, AECommandPool* commandPool);
};

class AEDrawObjectBaseGltf : public AEDrawObjectBase
{
protected:
    struct GltfTexture{
        std::string filename;
        uint32_t width;
        uint32_t height;
        size_t size;
        const void* data;
    };
    struct Joint{
        std::string nodename;
        int nodeid;
        std::vector<int> children;
        int jointNo;
        glm::mat4 ibm;
        std::vector<float> keyFrames;
        std::vector<glm::mat4> animationTransform;
        std::vector<std::vector<float>> morphWeights;
    };
    struct Morph{
        std::string name;
        std::vector<glm::vec3> data3;
    };
    struct Geometry{
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<uint32_t> indices;
        std::vector<glm::vec2> texCoords;
        std::vector<uint32_t> influences;
        std::vector<uint32_t> jointOffsets;
        std::vector<float> weights;
        std::vector<std::vector<Morph>> morphTargets;
    };
    struct GeometryBuffers{
        std::unique_ptr<AEBufferUtilOnGPU> positionBuffer;
        std::unique_ptr<AEBufferUtilOnGPU> indexBuffer;
        std::unique_ptr<AEBufferUtilOnGPU> texCoodBuffer;
        std::unique_ptr<AEBufferUtilOnGPU> influenceCountBuffer;
        std::unique_ptr<AEBufferUtilOnGPU> jointOffsetBuffer;
        std::unique_ptr<AEBufferUtilOnGPU> weightBuffer;
        std::unique_ptr<AEBufferUtilOnGPU> morphTargetsBuffer;
        std::unique_ptr<AEBufferUtilOnGPU> morphWeightsBuffer;
    };
    struct UniformBuffers{
        std::unique_ptr<AEBufferUniform> animationUniform;
        std::unique_ptr<AEBufferUniform> material;
    };
    float mScale;
    std::vector<Vertex3DObj> mVertices;
    std::vector<GltfTexture> mTextures;
    std::vector<Joint> mJoints;
    std::vector<uint32_t> mJointList;
    std::unique_ptr<AEComputePipeline> mComputePipeline;
    std::unique_ptr<AEComputePipeline> mComputePipelineMorph;
    std::vector<std::unique_ptr<AEBufferUtilOnGPU>> mBuffers;
    std::vector<glm::mat4> mAnimationTransforms;
    std::vector<glm::mat4> mAnimationTransformsNext;
    UniformBuffers mUniforms;
    std::vector<float> mAnimationTime;
    std::vector<std::unique_ptr<AEDescriptorSet>> mDSs;
    std::vector<Geometry> mGeometries;
    std::vector<GeometryBuffers> mGeoBuffers;
    std::vector<float> mMorphWeight;
    uint32_t mRoot;
    uint32_t mMorphNode;
    void ReadMesh(const tinygltf::Model& model);
    void ReadTexture(const tinygltf::Model& model);
    void ReadNode(const tinygltf::Model& model);
    void ReadMaterial(const tinygltf::Model& model, AELogicalDevice* device);
    void MakeVertices();
    void ReadBuffer(const tinygltf::Model& model, uint32_t accId, size_t componentSize, void* dstBuf);
    glm::mat4 t2m(const glm::vec3& translate);
    glm::mat4 s2m(const glm::vec3& scale);
    glm::mat4 r2m(const glm::vec4& rotate);
    uint32_t joint2node(uint32_t jointNum);
    uint32_t joint2node(uint32_t jointNum, std::vector<Joint> const& joints);
    void InputMorphData(const tinygltf::Model &model, Morph& m, uint32_t accId);
    void MakeAnimation();
    bool hasKeyFrames(float keyframe, std::vector<float>const& keyFrames, uint32_t &index);
    void PrepareAnimationMatrices(AEDrawObjectBaseGltf::Joint& joint, glm::mat4 parentBindPoseMatrix, glm::mat4 parentAnimationMatrix, uint32_t keyframe,
                                  std::vector<glm::mat4> &targetTransform);
public:
    AEDrawObjectBaseGltf(const char* filePath, android_app* app, AELogicalDevice* device, float scale = 1.0f);
    uint32_t GetTextureWidth(uint32_t index){return mTextures[index].width;};
    uint32_t GetTextureHeight(uint32_t index){return mTextures[index].height;};
    size_t GetTextureSize(uint32_t index){return mTextures[index].size;};
    const void* GetTextureData(uint32_t index){return mTextures[index].data;}
    uint32_t GetVertexSize(){return mVertices.size();}
    std::vector<Vertex3DObj>& GetVertexAddress(){return mVertices;}
    uint32_t GetIndexSize(uint32_t index){return mGeometries[index].indices.size();}
    std::vector<uint32_t>& GetIndexAddress(uint32_t index){return mGeometries[index].indices;}
    VkDeviceSize GetIndexBufferSize(uint32_t index){return mGeometries[index].indices.size() * sizeof(uint32_t);};
    uint32_t GetVertexBufferSize();
    std::vector<float>& GetKeyFrames(){return mAnimationTime;}
    AEBufferUniform* GetMaterialUniform(){return mUniforms.material.get();}
    float GetMaxKeyframe(){return mAnimationTime[mAnimationTime.size() - 1];}
    void AnimationPrepare(android_app* app, AELogicalDevice* device, std::vector<const char*>& shaders,
                          AEBufferBase* buffer[], AEDeviceQueue* queue, AECommandPool* commandPool, AEDescriptorPool* descriptorPool);
    void AnimationDispatch(AELogicalDevice* device, AECommandBuffer* command, AEDeviceQueue* queue, AECommandPool* commandPool,
            uint32_t animationNum, AEFence* fence, VkSemaphore *waitSemaphore, VkSemaphore* signalSemaphore, double time, AEEvent* event);
    void AnimationPrepareMorph(android_app* app, AELogicalDevice* device, std::vector<const char*>& shaders,
                               AEBufferBase* buffer[], AEDeviceQueue* queue, AECommandPool* commandPool, AEDescriptorPool* descriptorPool);
    void AnimationDispatchMorph(AELogicalDevice* device, AECommandBuffer* command, AEDeviceQueue* queue, AECommandPool* commandPool,
                           uint32_t animationNum, AEFence* fence, VkSemaphore *waitSemaphore, VkSemaphore* signalSemaphore, double time, AEEvent* event);
    void OutputPosition(uint32_t frameNum, AEBufferUtilOnGPU* buffer, AEDeviceQueue* const queue, AECommandPool* const commandpool);
    void DebugBuffer(AEDeviceQueue* const queue, AECommandPool* const commandpool);
    uint32_t GetGeometrySize()const{return mGeometries.size();}
};

/*
3rd objects
*/
/*
3D objects class not texture
*/
class AECube : public AEDrawObjectBase3D
{
    private:
    float mLen;
    glm::vec3 mColor;
    //function
    public:
    AECube(float len, glm::vec3 const& min, glm::vec3 const& color);
    ~AECube();
    void Update(glm::vec3 min);
};

class AEPyramid : public AEDrawObjectBase3D
{
    private:
    public:
    AEPyramid(glm::vec3 const& min, float len);
    ~AEPyramid();
};

/*
3D objects using textures
*/
class AECubeTexture : public AEDrawObjectBaseTexture
{
    private:
    std::vector<Vertex3DTexture> mVerticesTexture;
    public:
    AECubeTexture(glm::vec3 const& min, float len);
    ~AECubeTexture();
    //iterator
    std::vector<Vertex3DTexture>& GetVertexTexture(){return mVerticesTexture;}
    uint32_t GetVertexTextureSize(){return mVerticesTexture.size();}
};

class AEPlane : public AEDrawObjectBase3D
{
    private:
    //functions
    void SetElement(float x, float y, float z, Vertex3D& oneVertex);
    public:
    AEPlane(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 color);
    ~AEPlane();
};

class AESphere : public AEDrawObjectBase3D
{
    private:
    void CalcVertex(Vertex3D& v, float r, float theta, float phi);
    public:
    AESphere(glm::vec3 origin, float radius);
    ~AESphere();
};

class AEWaterSurface : public AEDrawObjectBase3D
{
    protected:
    std::mt19937 mEngine;
    std::uniform_int_distribution<> mRand;
    float mTop;
    float mRight;
    float mSeaBase;
    float mSpeed;
    float mFreq;
    float mAmp;
    float mDz;
    AELogicalDevice* mDevice;
    std::vector<std::unique_ptr<AEDescriptorSet>> mDSs;
    std::unique_ptr<AEComputePipeline> mComputePipeline;
    std::unique_ptr<AEBufferUniform> mWaveUniformBuffer;
    void CalcSeaLevel(uint32_t mod, float time, uint32_t threadNum);
    void Gerstner(uint32_t index, glm::vec3 waveVector, float amp, float freq, float speed, float steep, float time);
    void OneWave(glm::vec3& pos, glm::vec3 origin, float speed, float freq, float amp, float time);
    void SyntheticWave(glm::vec3& pos, glm::vec3 origin, float speed, float freq, float amp, float time);
    void FindExistVertex(std::vector<uint32_t>& ret, uint32_t mod, glm::vec3 pos, uint32_t threadNum);
    uint32_t FindExistVertexMultiThreads(uint32_t index, glm::vec3 pos);
    uint32_t FindClosestPoint(glm::vec3 pos);
    float Wave(float x);
    public:
    AEWaterSurface(float seaBase, float leftX, float rightX, float topZ,
                   float bottomZ, glm::vec3 color, float poolbottom = 0.0f, bool surfaceOnly = false, float inLength = 0.04);
    AEWaterSurface(float seaBase, float leftX, float rightX, float topZ, float bottomZ, glm::vec3 color, glm::vec3 cameraPos);
    ~AEWaterSurface();
    void SeaLevel(float time);
    //get
    float GetWaveSpeed(){return mSpeed;}
    float GetWaveFreq(){return mFreq;}
    float GetWaveAmp(){return mAmp;}
    float GetWaveDz(){return mDz;}
    //set
    void SetWaveSpeed(float speed){mSpeed = speed;}
    void SetWaveFreq(float freq){mFreq = freq;}
    void SetWaveAmp(float amp){mAmp = amp;}
    void SetWaveDz(float dz){mDz = dz;}
    void WavePrepare(android_app *app, AELogicalDevice *device, std::vector<const char *> &shaders,
                     AEBufferBase **buffer, AEDeviceQueue *queue, AECommandPool *commandPool,
                     AEDescriptorPool *descriptorPool);
    void DispatchWave(AELogicalDevice* device, AECommandBuffer* command, AEDeviceQueue* queue, AECommandPool* commandPool,
                      AEFence* fence, VkSemaphore *waitSemaphore, VkSemaphore* signalSemaphore,
                      double time, AEEvent* event);
};

#endif