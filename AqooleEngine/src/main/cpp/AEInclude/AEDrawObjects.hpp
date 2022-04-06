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
#ifndef _AE_DRAW_OBJECT
#define _AE_DRAW_OBJECT

#define _USE_MATH_DEFINES
#include "glm/glm.hpp"
#include <vector>
#include <array>
#include <cmath>
#include "string"
#include <sstream>
#include <fstream>
#include <random>
#include <thread>
#ifndef __ANDROID__
#include <cstring>
#include <limits>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#endif

/*
prototypes
*/
struct Vertex3D;
struct Vertex3DTexture;
struct Vertex3DObj;
struct Vertex2D;
struct Vertex3DTEST;

namespace AEDrawObject
{
    void AddOffsetToIndex(std::vector<uint32_t> &indices, uint32_t offset);
    void LoadModel();
    //for private
    void Split(std::vector<std::string> &fields, std::string const& oneLine, const char delimiter);
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
    void ReadMtlFile();
    void CalcTangent();
    public:
    AEDrawObjectBaseObjFile(const char* filePath);
    virtual ~AEDrawObjectBaseObjFile();
    //iterator
    uint32_t GetVertexSize(){return mVertices.size();}
    std::vector<Vertex3DObj>const& GetVertexAddress()const{return mVertices;}
    uint32_t GetTextureCount(){return mTextureFiles.size();}
    std::string& GetTexturePath(uint32_t index){return mTextureFiles[index];}
    uint32_t GetOffset(uint32_t index){return mOffsets[index];}
    //scale
    void Scale(float multiple);
};

#ifndef __ANDROID__
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
        std::vector<std::unique_ptr<SkeletonNode>> children;
    };
    struct JointWeight
    {
        std::vector<uint32_t> jointIndices;
        std::vector<float> weights; 
    };
    std::vector<Vertex3DObj> mVertices;
    std::vector<std::string> mMaterials;
    std::vector<uint32_t> mOffsets;
    std::string mMatFileName;
    std::vector<std::string> mTextureFiles;
    std::vector<Geometry> mGeometries;
    std::vector<glm::vec3> mPositions;
    std::vector<glm::vec3> mNormals;
    std::vector<uint32_t> mPositionIndices;
    std::vector<uint32_t> mNormalsIndices;
    std::unique_ptr<AEDrawObjectBaseCollada::SkeletonNode> mRoot;
    std::vector<std::string> mSkinJointsArray;
    std::vector<AEDrawObjectBaseCollada::JointWeight> mJointWeights;
    std::vector<glm::mat4> mInverseMatrices;
    //functions
    void ProcessGeometry(std::ifstream &file);
    void MakeVertices();
    void ReadSkeletonNode(boost::property_tree::ptree::const_iterator nowNode,
        std::unique_ptr<AEDrawObjectBaseCollada::SkeletonNode>& skeletonNode);
    void DebugRootNode();
    void GetVertexWeights(std::vector<float> &vertexWeights, std::string const& weightString);
    public:
    AEDrawObjectBaseCollada(const char* filePath);
    virtual ~AEDrawObjectBaseCollada();
    //iterator
    uint32_t GetVertexSize(){return mVertices.size();}
    std::vector<Vertex3DObj>& GetVertexAddress(){return mVertices;}
    uint32_t GetIndexSize(){return mIndices.size();}
    std::vector<uint32_t>& GetIndexAddress(){return mIndices;}
    //scale
    void Scale(float scale);
};
#endif

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