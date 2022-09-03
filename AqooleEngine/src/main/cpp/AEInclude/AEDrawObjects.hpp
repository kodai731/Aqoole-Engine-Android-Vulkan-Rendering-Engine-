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
#include "string"
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


/*
prototypes
*/
struct Vertex3D;
struct Vertex3DTexture;
struct Vertex3DObj;
struct Vertex2D;
struct Vertex3DTEST;
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

namespace AEDrawObject
{
    void AddOffsetToIndex(std::vector<uint32_t> &indices, uint32_t offset);
    void LoadModel();
    //for private
    void Split(std::vector<std::string> &fields, std::string& oneLine, const char delimiter);
    void GetInAngleBrackets(std::string &output, std::string const& oneLine);
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
        std::vector<float> timeList;
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
    std::vector<Vertex3DObj> mVertices;
    std::vector<std::string> mMaterials;
    std::vector<uint32_t> mOffsets;
    std::string mMatFileName;
    std::vector<std::string> mTextureFiles;
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
    std::unique_ptr<AEDescriptorSet> mDs;
    std::vector<std::unique_ptr<AEBufferUtilOnGPU>> mBuffers;
    std::vector<std::unique_ptr<AEBufferUniform>> mUniformBuffers;
    std::vector<glm::vec3> mAnimationPositionDebug;
    std::vector<glm::vec3> mZeroData;
    std::vector<uint32_t> mInfluenceCountList;
    std::vector<uint32_t> mJointOffsetList;
    //functions
    void ProcessGeometry(std::ifstream &file);
    void MakeVertices();
    void ReadSkeletonNode(boost::property_tree::ptree::const_iterator nowNode,
        std::unique_ptr<AEDrawObjectBaseCollada::SkeletonNode>& skeletonNode);
    void DebugRootNode();
    void GetVertexWeights(std::vector<float> &vertexWeights, std::string& weightString);
    void ReadAnimation(const boost::property_tree::ptree::value_type& node);
    void SkeletonJointNo(SkeletonNode* node);
    void SkeletonAnimation(SkeletonNode* node, glm::mat4 parentBindPoseMatrix, glm::mat4 parentAnimationMatrix, glm::mat4 ibp, std::vector<glm::vec3>& tmpPositions);
    void DebugPosition(uint32_t index, std::vector<glm::vec3> const& debug);
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
    std::string& GetTexturePath(uint32_t index){return mTextureFiles[index];}
    uint32_t GetOffset(uint32_t index){return mOffsets[index];}
    std::vector<uint32_t>const& GetOffsetAll(){return mOffsets;}
    std::vector<uint32_t>const& GetMapIndexAddress(uint32_t index)const{return mMapIndices[index];}
    std::vector<glm::vec2>const& GetMapsAddress()const{return mMaps[0];}
    std::vector<uint32_t>const& GetEachMapIndices(uint32_t index)const{return mMapIndices[index];}
    std::vector<std::vector<uint32_t>>const& GetMapIndices()const{return mMapIndices;}
    std::vector<glm::vec3>const& GetPositions(uint32_t index)const{return mPositions[index];}
    uint32_t GetMaterialSize(){return mPositionIndices.size();}
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
                          AEDeviceQueue* queue, AECommandPool* commandPool, AEDescriptorPool* descriptorPool);
    void AnimationDispatch(AELogicalDevice* device, AECommandBuffer* command, AEDeviceQueue* queue, AECommandPool* commandPool,
                           uint32_t animationNum, AEFence* fence, VkSemaphore *waitSemaphore, VkSemaphore* signalSemaphore,
                           double time);
    void AnimationDispatchJoint(SkeletonNode* node, glm::mat4 parentBindPoseMatrix, glm::mat4 parentAnimationMatrix, uint32_t animationNum,
                                std::vector<glm::mat4> &targetTransform);
    void RecordCommand(AELogicalDevice* device, AECommandBuffer* commandBuffer);
    AEBufferUtilOnGPU* GetBuffer(uint32_t index){return mBuffers[index].get();}
    void Debug(AEDeviceQueue* queue, AECommandPool* commandPool);
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
    //function
    public:
    AECube(float len, glm::vec3 const& min, glm::vec3 const& color);
    ~AECube();
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

#ifdef __RAY_TRACING__PC
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
    void CalcSeaLevel(uint32_t mod, float time, uint32_t threadNum);
    void Gerstner(uint32_t index, glm::vec3 waveVector, float amp, float freq, float speed, float steep, float time);
    void OneWave(glm::vec3& pos, glm::vec3 origin, float speed, float freq, float amp, float time);
    void SyntheticWave(glm::vec3& pos, glm::vec3 origin, float speed, float freq, float amp, float time);
    void FindExistVertex(std::vector<uint32_t>& ret, uint32_t mod, glm::vec3 pos, uint32_t threadNum);
    uint32_t FindExistVertexMultiThreads(uint32_t index, glm::vec3 pos);
    uint32_t FindClosestPoint(glm::vec3 pos);
    float Wave(float x);
    public:
    AEWaterSurface(float seaBase, float leftX, float rightX, float topZ, float bottomZ, glm::vec3 color);
    AEWaterSurface(float seaBase, float leftX, float rightX, float topZ, float bottomZ, glm::vec3 color, glm::vec3 cameraPos);
    ~AEWaterSurface();
    void SeaLevel(float time);
    //get
    float* GetWaveSpeed(){return &mSpeed;}
    float* GetWaveFreq(){return &mFreq;}
    float* GetWaveAmp(){return &mAmp;}
    float* GetWaveDz(){return &mDz;}
    //set
    void SetWaveSpeed(float speed){mSpeed = speed;}
    void SetWaveFreq(float freq){mFreq = freq;}
    void SetWaveAmp(float amp){mAmp = amp;}
    
};
#endif

#endif