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
//
//        limitations under the License.
//          Copyright Joe Coder 2004 - 2006.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TINYGLTF_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#endif
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "AEDrawObjects.hpp"
#include "AEUBO.hpp"
#include "AEMatrix.hpp"
#include "AEDevice.hpp"
#include "AEPipeline.hpp"
#include "descriptorSet.hpp"
#include "AEBuffer.hpp"
#include "AEDeviceQueue.hpp"
#include "AECommand.hpp"
#include "AESyncObjects.hpp"


//=====================================================================
//namespace AE draw object
//=====================================================================
void AEDrawObject::AddOffsetToIndex(std::vector<uint32_t> &indices, uint32_t offset)
{
    uint32_t size = indices.size();
    for(uint32_t i = 0; i < size; i++)
        indices[i] += offset;
}

/*
load model
*/
// void AEDrawObject::LoadModel(const char* filePath)
// {
//     std::ifstream objData(filePath, std::ios::in);
//     std::string oneLine;
//     std::vector<std::string> fields;
//     while(std::getline(objData, oneLine))
//     {

//     }
// }

/*
split
*/
void AEDrawObject::Split(std::vector<std::string> &fields, std::string& oneLine, const char delimiter)
{
    oneLine = std::regex_replace(oneLine, std::regex("\n"), " ");
    std::istringstream stream(oneLine);
    std::string field;
    fields.clear();
    //format oneline
    while (std::getline(stream, field, delimiter))
        fields.push_back(field);
    //check the head is EOL or not
    while(strcmp(fields[0].c_str(), "") == 0)
        fields.erase(fields.begin());
}

/*
get in angle brackets
*/
void AEDrawObject::GetInAngleBrackets(std::string &output, std::string const& oneLine)
{
    int begin = oneLine.find_first_of('<');
    int end = oneLine.find_first_of('>');
    if(begin >= 0 && end >= 0)
        output = oneLine.substr(begin + 1, end - begin - 1);
}

/*
 * get root dir name
 */
std::string AEDrawObject::GetRootDirName(std::string &modelPath)
{
    std::vector<std::string> fields;
    Split(fields, modelPath, '/');
    return fields[0] + std::string("/");
}


//=====================================================================
//Torus
//=====================================================================
/*
constructor
*/
AETorus::AETorus(uint32_t circlePoints, uint32_t circleNum, float irad, float orad)
{
    //Torus = OP + PQ
    //OP = {x | orad * (cos, sin)}
    //PQ = {x | irad * (cos, sin)}
    //vertices
    glm::vec3 OP;
    glm::vec3 PQ;
    glm::vec3 naxis;
    Vertex3D element;
    float oneTheta = (2.0f * M_PI) / (float)circleNum;
    float onePhi = (2.0f * M_PI) / (float)circlePoints;
    const float diameter = 2.0f * orad;
    const float zDepth = 2.0f * irad;
    glm::mat3 rotateTheta;
    glm::mat3 rotatePhi;
    for(uint32_t i = 0; i < circleNum; i++)
    {
        float theta = oneTheta * (float)i;
        float cTheta = cos(theta);
        float sTheta = sin(theta);
        glm::mat3 rotateTheta(cTheta, sTheta, 0.0f, -sTheta, cTheta, 0.0f, 0.0f, 0.0f, 1.0f);
        OP = rotateTheta * glm::vec3(1.0f, 0.0f, 0.0f);
        OP *= orad;
        naxis = glm::cross(OP, glm::vec3(0.0f, 0.0f, 1.0f));
        naxis = glm::normalize(naxis);
        for(uint32_t j = 0; j < circlePoints; j++)
        {
            float phi = onePhi * (float)j;
            float cPhi = cos(phi);
            float sPhi = sin(phi);
            AEMatrix::Rodrigues(rotatePhi, cPhi, sPhi, naxis);
            PQ = rotatePhi * glm::vec3(0.0f, 0.0f, 1.0f);
            PQ *= irad;
            element.pos = glm::vec3(OP + PQ);
            element.color = glm::vec3(glm::smoothstep(-diameter, diameter, element.pos.x),
                glm::smoothstep(-diameter, diameter, element.pos.y), glm::smoothstep(-zDepth, zDepth, element.pos.z));
            element.normal = glm::normalize(element.pos - OP);
            mVertices.push_back(element);
        }
    }
    //indices
    std::array<uint32_t, 3> indices1;
    std::array<uint32_t, 3> indices2;
    for(uint32_t i = 0; i < circleNum; i++)
    {
        for(uint32_t j = 0; j < circlePoints; j++)
        {
            /*choose 4 points
            i * circlePoints + j,      (i + 1) * circlePoints + j
            i * circlePoints + j + 1 , (i + 1) * circlePoints + j + 1
            !!attension!!
            if j = max, not (+ j + 1) but (+ 0)
            if i = max, not (i + 1) but (i = 0)
            */
            uint32_t base = i * circlePoints;
            uint32_t basePlus = ((i + 1) % circleNum) * circlePoints;
            indices1 = { base + j, base + ((j + 1) % circlePoints), basePlus + j};
            indices2 = { base + ((j + 1) % circlePoints), basePlus + ((j + 1) % circlePoints), basePlus + j};
            mIndices.push_back(indices1);
            mIndices.push_back(indices2);
        }
    }
    Debug();
}

/*
destructor
*/
AETorus::~AETorus()
{

}

/*
get indices
*/
void AETorus::GetIndices(std::vector<uint32_t> &indices)
{
    indices.clear();
    for(auto indexArray : mIndices)
        for(auto index : indexArray)
            indices.push_back(index);
    return;
}

/*
get vertices
*/
void AETorus::GetVertices(std::vector<Vertex3D> &vertices)
{
    vertices.clear();
    for(auto vertex : mVertices)
        vertices.push_back(vertex);
    return;
}

/*
debug
*/
void AETorus::Debug()
{
    std::ofstream vertexf("./debug/torusVertex.txt", std::ios::out | std::ios::trunc);
    vertexf << "vertex information" << std::endl;
    vertexf << "total size = " << mVertices.size() << std::endl;
    for(auto vertex : mVertices)
    {
        vertexf << "pos = " << vertex.pos.x << " " << vertex.pos.y << " " << vertex.pos.z << std::endl;
        vertexf << "color = " << vertex.color.x << " " << vertex.color.y << " " << vertex.color.z << std::endl;
    }
    vertexf.close();
    std::ofstream indexf("./debug/torusIndex.txt", std::ios::out | std::ios::trunc);
    indexf << "index information" << std::endl;
    indexf << "total size = " << mIndices.size() * 3 << std::endl;;
    for(auto index : mIndices)
    {
        indexf << index[0] << " " << index[1] << " " << index[2] << std::endl;
    }
    indexf.close();
    return;
}

//=====================================================================
//AE coordinates
//=====================================================================
/*
constructor
*/
AECoordinates::AECoordinates(glm::vec3 center, float halfWidth, float interval)
{
    uint32_t half = floorf(halfWidth / interval);
    Vertex3D vx;
    //red
    vx.color = glm::vec3(1.0f, 0.0f, 0.0f);
    Vertex3D vz;
    //blue
    vz.color = glm::vec3(0.0f, 0.0f, 1.0f);
    //left half
    for(uint32_t i = 0; i < half; i++)
    {
        //x-axis
        //positive
        //CAUTION : we can NOT use HUGE_VALF
        vx.pos = glm::vec3(-100.0f, center.y, center.z + (i * interval));
        CreateVertex(vx);
        vx.pos = glm::vec3(100.0f, center.y, center.z + (i * interval));
        //CreateVertex(vx);
        CreateVertex(vx);
        //negative
        vx.pos = glm::vec3(-HUGE_VALF, center.y, center.z - (i * interval));
        CreateVertex(vx);
        vx.pos = glm::vec3(HUGE_VALF, center.y, center.z - (i * interval));
        //CreateVertex(vx);
        CreateVertex(vx);
        //z-axis
        //positive
        vz.pos = glm::vec3(center.x + (i * interval), center.y, -100.0f);
        CreateVertex(vz);
        vz.pos = glm::vec3(center.x + (i * interval), center.y, 100.0f);
        //CreateVertex(vz);
        CreateVertex(vz);
        //negative
        vz.pos = glm::vec3(center.x - (i * interval), center.y, -HUGE_VALF);
        CreateVertex(vz);
        vz.pos = glm::vec3(center.x - (i * interval), center.y, HUGE_VALF);
        //CreateVertex(vz);
        CreateVertex(vz);
    }
}

/*
destructor
*/
AECoordinates::~AECoordinates()
{

}

/*
create vertex
*/
void AECoordinates::CreateVertex(Vertex3D const& pos)
{
    mXZ.push_back(pos);
    mIndices.push_back(mIndices.size());
    return;
}

//=====================================================================
//AE draw object base
//=====================================================================
/*
constructor
*/
AEDrawObjectBase::AEDrawObjectBase()
{

}

/*
destructor
*/
AEDrawObjectBase::~AEDrawObjectBase()
{

}

/*
add index
   
    a--d
    |  |
    b--c

*/
void AEDrawObjectBase::AddIndex(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    mIndices.push_back(a);
    mIndices.push_back(b);
    mIndices.push_back(c);
    mIndices.push_back(c);
    mIndices.push_back(d);
    mIndices.push_back(a);
}

void AEDrawObjectBase::AddIndex(uint32_t a, uint32_t b, uint32_t c)
{
    mIndices.push_back(a);
    mIndices.push_back(b);
    mIndices.push_back(c);
}

/*
calculate normal vector
*/
void AEDrawObjectBase::CalcNormal()
{
    // uint32_t size = mVertices.size();
    // uint32_t totalIndices = mIndices.size();
    // uint32_t polygonSize = totalIndices / 3;
    // glm::vec3 sumVec(0.0f, 0.0f, 0.0f);
    // for(uint32_t i = 0; i < size; i++)
    // {
    //     for(uint32_t j = 0; j < polygonSize; j++)
    //     {
    //         if(i == mIndices[3 * j] || i == mIndices[3 * j + 1] || i == mIndices[3 * j + 2])
    //             sumVec += glm::cross()
    //     }
    // }
}

uint32_t AEDrawObjectBase::GetIndexBufferSize()
{
    return sizeof(uint32_t) * mIndices.size();
}

//=====================================================================
//AE draw object base 3D
//=====================================================================
/*
constructor
*/
AEDrawObjectBase3D::AEDrawObjectBase3D()
    : AEDrawObjectBase()
{

}
/*
destructor
*/
AEDrawObjectBase3D::~AEDrawObjectBase3D()
{

}

/*
add vertex
*/
void AEDrawObjectBase3D::AddVertex(glm::vec3 const& v, glm::vec3 const& color)
{
    Vertex3D v3D;
    v3D.pos = v;
    v3D.color = color;
    mVertices.push_back(v3D);
}

/*
add vertex with normal
*/
void AEDrawObjectBase3D::AddVertex(glm::vec3 const& v, glm::vec3 const& color, glm::vec3 const& normal)
{
    Vertex3D v3D;
    v3D.pos = v;
    v3D.color = color;
    v3D.normal = normal;
    mVertices.push_back(v3D);
}


/*
add vertex v2
*/
void AEDrawObjectBase3D::AddVertex(glm::vec3 const& v, glm::vec3 const& color, Vertex3D &reuse)
{
    reuse.pos = v;
    reuse.color = color;
    mVertices.push_back(reuse);
}

/*
calc normal
*/
void AEDrawObjectBase3D::CalcNormal()
{
    uint32_t primitiveCount = GetIndexSize() / 3;
    uint32_t vertexCount = GetVertexSize();
    std::vector<glm::vec3> totalNormals;
    std::vector<uint32_t> involvedCount;
    //initialize
    for(uint32_t i = 0; i < vertexCount; i++)
    {
        totalNormals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        involvedCount.push_back(0);
    }
    //count up
    for(uint32_t i = 0; i < primitiveCount; i++)
    {
        uint32_t i0 = mIndices[3 * i];
        uint32_t i1 = mIndices[3 * i + 1];
        uint32_t i2 = mIndices[3 * i + 2];
        glm::vec3 v0 = mVertices[i0].pos;
        glm::vec3 v1 = mVertices[i1].pos;
        glm::vec3 v2 = mVertices[i2].pos;
        //normals
        totalNormals[i0] += glm::normalize(glm::cross(v1 - v0,v2 - v0));
        totalNormals[i1] += glm::normalize(glm::cross(v2 - v1, v0 - v1));
        totalNormals[i2] += glm::normalize(glm::cross(v0 - v2, v1 - v2));
        //counts
        involvedCount[i0] += 1;
        involvedCount[i1] += 1;
        involvedCount[i2] += 1;
    }
    //calc
    for(uint32_t i = 0; i < vertexCount; i++)
    {
        totalNormals[i] = glm::normalize((float)(1.0f / involvedCount[i]) * totalNormals[i]);
        mVertices[i].normal = totalNormals[i];
    }
}

uint32_t AEDrawObjectBase3D::GetVertexBufferSize()
{
    return sizeof(Vertex3D) * mVertices.size();
}

//=====================================================================
//AE draw object base texture
//=====================================================================
/*
constructor
*/
AEDrawObjectBaseTexture::AEDrawObjectBaseTexture()
    : AEDrawObjectBase()
{

}

/*
destructor
*/
AEDrawObjectBaseTexture::~AEDrawObjectBaseTexture()
{

}

/*
add vertexTexture
*/
void AEDrawObjectBaseTexture::AddVertex(glm::vec3 const& v, glm::vec3 const& color, glm::vec3 const& normal,
    glm::vec2 const& texCoord, Vertex3DTexture &reuse)
{
    reuse.pos = v;
    reuse.color = color;
    reuse.normal = normal;
    reuse.texcoord = texCoord;
    mVertices.push_back(reuse);
}

//=====================================================================
//AE object base obj file
//=====================================================================
/*
constructor
*/
#ifndef __ANDROID__
AEDrawObjectBaseObjFile::AEDrawObjectBaseObjFile(const char* filePath)
    : AEDrawObjectBase()
{
    std::ifstream objData(filePath, std::ios::in);
    std::string oneLine;
    std::vector<std::string> fields;
    std::vector<std::string> lineFields;
    Vertex3DObj tmpV;
    std::vector<glm::vec3> localVertices;
    std::vector<glm::vec2> localTexCoords;
    std::vector<glm::vec3> localNormals;
    int indices[4];
    int totalF = 0;
    uint32_t offset = 0;
    while(std::getline(objData, oneLine))
    {
        AEDrawObject::Split(fields, oneLine, ' ');
        if(fields[0] == "v")
            localVertices.push_back(glm::vec3(std::stof(fields[1]), std::stof(fields[2]), std::stof(fields[3])));
        else if(fields[0] == "vt")
            localTexCoords.push_back(glm::vec2(std::stof(fields[1]), 1.0f - std::stof(fields[2])));
        else if(fields[0] == "vn")
            localNormals.push_back(glm::vec3(std::stof(fields[1]), std::stof(fields[2]), std::stof(fields[3])));
        else if(fields[0] == "f")
        {
            uint32_t fieldsSize = fields.size();
            for(int i = 1; i < fieldsSize; i++)
            {
                AEDrawObject::Split(lineFields, fields[i], '/');
                tmpV.pos = localVertices[std::stoi(lineFields[0]) - 1];
                tmpV.texcoord = localTexCoords[std::stof(lineFields[1]) - 1];
                tmpV.normal = localNormals[std::stof(lineFields[2]) - 1];
                //calc vertex tangent
                // float xn2 = tmpV.normal.x * tmpV.normal.x;
                // float zn2 = tmpV.normal.z * tmpV.normal.z;
                // float xn2zn2 = xn2 + zn2;
                // float xt = sqrt(zn2 / xn2zn2);
                // float zt = sqrt(xn2 / xn2zn2);
                // glm::vec3 tangent;
                // if(glm::dot(tmpV.normal, glm::vec3(0, 1.0f, 0)) != 1.0)
                //     tangent = glm::cross(tmpV.normal, glm::vec3(0.0f, 1.0f, 0.0f));
                // else
                //     tangent = glm::cross(tmpV.normal, glm::vec3(0.0f, -1.0f, 0.0f));
                // tmpV.vertexTangent = glm::vec4(tangent, -1.0f);
                mVertices.push_back(tmpV);
                indices[i - 1] = totalF;
                totalF++;
            }
            if(fieldsSize == 5)
            {
                AddIndex(indices[0], indices[1], indices[2], indices[3]);
                offset += 6;
            }
            else if(fieldsSize == 4)
            {
                AddIndex(indices[0], indices[1], indices[2]);
                offset += 3;
            }
            else
                throw std::runtime_error("obj file is unknown");
        }
        else if(fields[0] == "usemtl")
        {
            mMaterials.push_back(fields[1]);
            mOffsets.push_back(offset);
        }
        else if(fields[0] == "mtllib")
        {
            std::string str(filePath);
            str = str.substr(0, str.find_last_of('/'));
            mMatFileName = str + std::string("/") + fields[1];
        }
    }
    //read material information
    ReadMtlFile();
    //calc tangent
    CalcTangent();
}
#else
AEDrawObjectBaseObjFile::AEDrawObjectBaseObjFile(const char* filePath, android_app* app, bool isReverseY)
        : AEDrawObjectBase()
{
    float signY = 1.0f;
    if(isReverseY)
        signY = -1.0f;
    AAsset* file = AAssetManager_open(app->activity->assetManager,
                                      filePath, AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(file);
    char* fileContent = new char[fileLength];
    AAsset_read(file, fileContent, fileLength);
    std::string objData(fileContent);
    AAsset_close(file);
    std::string oneLine;
    std::vector<std::string> fields;
    std::vector<std::string> lineFields;
    Vertex3DObj tmpV;
    std::vector<glm::vec3> localVertices;
    std::vector<glm::vec2> localTexCoords;
    std::vector<glm::vec3> localNormals;
    int indices[4];
    int totalF = 0;
    uint32_t offset = 0;
    std::string::size_type pos = 0;
    std::string::size_type lastPos = 0;
    while((pos = objData.find('\n', lastPos)) != std::string::npos)
    {
        oneLine = objData.substr(lastPos, pos - lastPos);
        lastPos = pos + 1;
        AEDrawObject::Split(fields, oneLine, ' ');
        if(fields[0] == "v")
            localVertices.push_back(glm::vec3(std::stof(fields[1]), std::stof(fields[2]) * signY, std::stof(fields[3])));
        else if(fields[0] == "vt")
            localTexCoords.push_back(glm::vec2(std::stof(fields[1]), 1.0f - std::stof(fields[2])));
        else if(fields[0] == "vn")
            localNormals.push_back(glm::vec3(std::stof(fields[1]), std::stof(fields[2]), std::stof(fields[3])));
        else if(fields[0] == "f")
        {
            uint32_t fieldsSize = fields.size();
            for(int i = 1; i < fieldsSize; i++)
            {
                AEDrawObject::Split(lineFields, fields[i], '/');
                tmpV.pos = localVertices[std::stoi(lineFields[0]) - 1];
                tmpV.texcoord = localTexCoords[std::stof(lineFields[1]) - 1];
                tmpV.normal = localNormals[std::stof(lineFields[2]) - 1];
                //calc vertex tangent
                // float xn2 = tmpV.normal.x * tmpV.normal.x;
                // float zn2 = tmpV.normal.z * tmpV.normal.z;
                // float xn2zn2 = xn2 + zn2;
                // float xt = sqrt(zn2 / xn2zn2);
                // float zt = sqrt(xn2 / xn2zn2);
                // glm::vec3 tangent;
                // if(glm::dot(tmpV.normal, glm::vec3(0, 1.0f, 0)) != 1.0)
                //     tangent = glm::cross(tmpV.normal, glm::vec3(0.0f, 1.0f, 0.0f));
                // else
                //     tangent = glm::cross(tmpV.normal, glm::vec3(0.0f, -1.0f, 0.0f));
                // tmpV.vertexTangent = glm::vec4(tangent, -1.0f);
                mVertices.push_back(tmpV);
                indices[i - 1] = totalF;
                totalF++;
            }
            if(fieldsSize == 5)
            {
                AddIndex(indices[0], indices[1], indices[2], indices[3]);
                offset += 6;
            }
            else if(fieldsSize == 4)
            {
                AddIndex(indices[0], indices[1], indices[2]);
                offset += 3;
            }
            else
                throw std::runtime_error("obj file is unknown");
        }
        else if(fields[0] == "usemtl")
        {
            mMaterials.push_back(fields[1]);
            mOffsets.push_back(offset);
        }
        else if(fields[0] == "mtllib")
        {
            std::string str(filePath);
            str = str.substr(0, str.find_last_of('/'));
            mMatFileName = str + std::string("/") + fields[1];
        }
    }
    //read material information
    ReadMtlFile(app);
    //calc tangent
    CalcTangent();
    //delete
    delete[] fileContent;
}
#endif


/*
destructor
*/
AEDrawObjectBaseObjFile::~AEDrawObjectBaseObjFile()
{

}

/*
scale
*/
void AEDrawObjectBaseObjFile::Scale(float multiple)
{
    uint32_t size = mVertices.size();
    for(uint32_t i = 0; i < size; i++)
    {
        mVertices[i].pos *= multiple;
    }
}

/*
read material file
*/
#ifndef __ANDROID__
void AEDrawObjectBaseObjFile::ReadMtlFile()
{
    std::ifstream mtlData(mMatFileName.c_str(), std::ios::in);
    std::string oneLine;
    std::vector<std::string> fields;
    uint32_t nowIndex = 0;
    uint32_t size = mMaterials.size();
    mTextureFiles.resize(size);
    std::string base = mMatFileName.substr(0, mMatFileName.find_last_of('/')) + std::string("/");
    while(std::getline(mtlData, oneLine))
    {
        if(oneLine == "")
            continue;
        AEDrawObject::Split(fields, oneLine, ' ');
        if(fields[0] == "newmtl")
        {
            for(uint32_t i = 0; i < size; i++)
                if(mMaterials[i] == fields[1])
                    nowIndex = i;
        }
        else if(fields[0] == "map_Kd")
            mTextureFiles[nowIndex] = base + fields[1];
    }
}
#else
void AEDrawObjectBaseObjFile::ReadMtlFile(android_app* app)
{
    AAsset* file = AAssetManager_open(app->activity->assetManager,
                                      mMatFileName.c_str(), AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(file);
    char* fileContent = new char[fileLength];
    AAsset_read(file, fileContent, fileLength);
    std::string mtlData(fileContent);
    AAsset_close(file);
    std::string oneLine;
    std::vector<std::string> fields;
    uint32_t nowIndex = 0;
    uint32_t size = mMaterials.size();
    mTextureFiles.resize(size);
    std::string base = mMatFileName.substr(0, mMatFileName.find_last_of('/')) + std::string("/");
    std::string::size_type pos = 0;
    std::string::size_type lastPos = 0;
    while((pos = mtlData.find('\n', lastPos)) != std::string::npos)
    {
        oneLine = mtlData.substr(lastPos, pos - lastPos);
        lastPos = pos + 1;
        if(oneLine == "")
            continue;
        AEDrawObject::Split(fields, oneLine, ' ');
        if(fields[0] == "newmtl")
        {
            for(uint32_t i = 0; i < size; i++)
                if(mMaterials[i] == fields[1])
                    nowIndex = i;
        }
        else if(fields[0] == "map_Kd")
            mTextureFiles[nowIndex] = base + fields[1];
    }
    delete[] fileContent;
}
#endif
/*
calc tangent
*/
void AEDrawObjectBaseObjFile::CalcTangent()
{
    uint32_t vertexCount = GetVertexSize();
    uint32_t triangleCount = GetIndexSize() / 3;
    //tmp storage
    std::unique_ptr<glm::vec3> tangents(new glm::vec3[vertexCount]);
    std::unique_ptr<glm::vec3> bitangents(new glm::vec3[vertexCount]);
    for(uint32_t i = 0; i < vertexCount; i++)
    {
        tangents.get()[i] = glm::vec3(0.0f, 0.0f, 0.0f);
        bitangents.get()[i] = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    //calclate tangents and bitangents for each vertex
    for(uint32_t i = 0; i < triangleCount; i++)
    {
        uint32_t i0 = mIndices[i * 3];
        uint32_t i1 = mIndices[i * 3 + 1];
        uint32_t i2 = mIndices[i * 3 + 2];
        glm::vec3 e1 = mVertices[i1].pos - mVertices[i0].pos;
        glm::vec3 e2 = mVertices[i2].pos - mVertices[i0].pos;
        float x1 = mVertices[i1].texcoord.x - mVertices[i0].texcoord.x;
        float x2 = mVertices[i2].texcoord.x - mVertices[i0].texcoord.x;
        float y1 = mVertices[i1].texcoord.y - mVertices[i0].texcoord.y;
        float y2 = mVertices[i2].texcoord.y - mVertices[i0].texcoord.y;
        float r = 1.0f / (x1 * y2 - x2 * y1);
        glm::vec3 t = (e1 * y2 - e2 * y1) * r;
        glm::vec3 b = (e2 * x1 - e1 * x2) * r;
        tangents.get()[i0] += t;
        tangents.get()[i1] += t;
        tangents.get()[i2] += t;
        bitangents.get()[i0] += b;
        bitangents.get()[i1] += b;
        bitangents.get()[i2] += b;
    }
    //orthorize
    for(uint32_t i = 0; i < vertexCount; i++)
    {
        //Reject : return (a - b * (Dot(a, b) / Dot(b, b))); a, b : vector3
        const glm::vec3& t = tangents.get()[i];
        const glm::vec3& b = bitangents.get()[i];
        const glm::vec3& n = mVertices[i].normal;
        glm::vec3 tangent = glm::normalize(t - n * (glm::dot(t, n) / glm::dot(n, n)));
        float w = (glm::dot(glm::cross(t, b), n) > 0.0f) ? 1.0f : -1.0f;
        mVertices[i].vertexTangent = glm::vec4(tangent, w);
    }
    //delete
    tangents.reset();
    bitangents.reset();
}

uint32_t AEDrawObjectBaseObjFile::GetVertexBufferSize(){return sizeof(Vertex3DObj) * mVertices.size();}

//=====================================================================
//AE collada
//=====================================================================
/*
constructor
*/
AEDrawObjectBaseCollada::AEDrawObjectBaseCollada(const char* filePath, android_app* app, AELogicalDevice* device,
                                                 std::vector<const char*> &shaderPaths, AECommandPool* commandPool, AEDeviceQueue* queue)
    : AEDrawObjectBase()
{
    //using boost
    {
        using namespace boost::property_tree;
        ptree tree;
        AAsset* file = AAssetManager_open(app->activity->assetManager,
                                          filePath, AASSET_MODE_BUFFER);
        size_t fileLength = AAsset_getLength(file);
        char* fileContent = new char[fileLength];
        AAsset_read(file, fileContent, fileLength);
        std::stringbuf strbuf(fileContent);
        std::basic_istream stream(&strbuf);
        read_xml(stream, tree);
        //one 3D obj
        Vertex3DObj one3DObj;
        glm::vec3 oneVec3; 
        glm::vec2 oneVec2;
        std::vector<std::string> fields;
        //read effects
        BOOST_FOREACH(const auto& effect, tree.get_child("COLLADA.library_effects")){
                        ReadEffect(effect);
        }
        //read images
        for(const ptree::value_type& images : tree.get_child("COLLADA.library_images"))
        {
            if (boost::optional<std::string> imageId = images.second.get_optional<std::string>("<xmlattr>.id"))
            {
                TextureImage ti;
                ti.imageId = imageId.get();
                auto imageChild = images.second.get_child("init_from");
                std::string imageName(imageChild.data());
                while(imageName.find("\\") != std::string::npos)
                {
                    imageName.replace(0, imageName.find("\\") + 1, "");
                }
                if(!std::regex_search(imageName.c_str(), std::regex("\\.png")))
                    imageName += std::string(".png");
                ti.fileName = imageName;
                mTextureFiles.emplace_back(ti);
            }
        }
        //read geometry
        Geometry oneGeometry;
        for(const ptree::value_type& geometry : tree.get_child("COLLADA.library_geometries")) 
        {
            if(boost::optional<std::string> geometryId = geometry.second.get_optional<std::string>("<xmlattr>.id"))
            {
                oneGeometry.geometryId = geometryId.get();
                BOOST_FOREACH(const auto &geometryChild, geometry.second.get_child("mesh"))
                {
                    //get source, triangle, vertices
                    auto geometryChildTag = geometryChild.first.data();
                    //source
                    if(strncmp(geometryChildTag, "source", 7) == 0)
                    {
                        auto floatArrayId =  geometryChild.second.get_optional<std::string>(
                            "float_array.<xmlattr>.id");
                        auto floatArrayStr = floatArrayId.get();
                        //positions or normals
                        if(std::regex_search(floatArrayStr.c_str(), std::regex("position", std::regex::icase)) ||
                                std::regex_search(floatArrayStr.c_str(), std::regex("normal", std::regex::icase)))
                        {
                            std::vector<glm::vec3> v;
                            uint32_t targetIndex = 0;
                            if(std::regex_search(floatArrayStr.c_str(), std::regex("position", std::regex::icase)))
                            {
                                targetIndex = mPositions.size();
                                mPositions.emplace_back(v);
                            }
                            else if(std::regex_search(floatArrayStr.c_str(), std::regex("normal", std::regex::icase)))
                            {
                                targetIndex = mNormals.size();
                                mNormals.emplace_back(v);
                            }
                            std::string oneLinePositions = geometryChild.second.get<std::string>("float_array");
                            AEDrawObject::Split(fields, oneLinePositions, ' ');
                            uint32_t size = fields.size() / 3;
                            for(uint32_t i = 0; i < size; i++)
                            {
                                oneVec3.x = std::stof(fields[i * 3]);
                                oneVec3.y = std::stof(fields[i * 3 + 1]);
                                oneVec3.z = std::stof(fields[i * 3 + 2]);
                                if(std::regex_search(floatArrayStr.c_str(), std::regex("position", std::regex::icase)))
                                {
                                    mPositions[targetIndex].push_back(oneVec3);
                                }
                                else if(std::regex_search(floatArrayStr.c_str(), std::regex("normal", std::regex::icase)))
                                {
                                    mNormals[targetIndex].push_back(oneVec3);
                                }
                            }
                        }
                        else if(std::regex_search(floatArrayStr.c_str(), std::regex("map", std::regex::icase)) ||
                                std::regex_search(floatArrayStr.c_str(), std::regex("uv", std::regex::icase)))
                        {
                            std::string oneLinePositions = geometryChild.second.get<std::string>("float_array");
                            AEDrawObject::Split(fields, oneLinePositions, ' ');
                            uint32_t size = fields.size() / 2;
                            uint32_t targetIndex = mMaps.size();
                            std::vector<glm::vec2> v;
                            mMaps.emplace_back(v);
                            for(uint32_t i = 0; i < size; i++)
                            {
                                oneVec2.x = std::stof(fields[i * 2]);
                                oneVec2.y = 1.0f - std::stof(fields[i * 2 + 1]);
                                mMaps[targetIndex].emplace_back(oneVec2);
                            }
                        }
                    }
                    //triangles
                    if(strncmp(geometryChildTag, "triangles", 10) == 0)
                    {
                        std::string data = geometryChild.second.get<std::string>("p");
                        AEDrawObject::Split(fields, data, ' ');
                        uint32_t size = fields.size() / 3;
                        std::vector<uint32_t> indices;
                        mPositionIndices.emplace_back(indices);
                        mNormalsIndices.emplace_back(indices);
                        mMapIndices.emplace_back(indices);
                        for(uint32_t i = 0; i < size; i++)
                        {
                            mPositionIndices[mPositionIndices.size() - 1].emplace_back(std::stoi(fields[i * 3]));
                            mNormalsIndices[mNormalsIndices.size() - 1].emplace_back(std::stoi(fields[i * 3 + 1]));
                            mMapIndices[mMapIndices.size() - 1].emplace_back(std::stoi(fields[i * 3 + 2]));
                        }
                    }
                    //polylist
                    if(std::regex_search(geometryChildTag, std::regex("polylist", std::regex::icase)))
                    {
                        std::string data = geometryChild.second.get<std::string>("p");
                        AEDrawObject::Split(fields, data, ' ');
                        uint32_t vcount = 4;
                        uint32_t size = fields.size() / vcount;
                        std::vector<uint32_t> indices;
                        mPositionIndices.emplace_back(indices);
                        mNormalsIndices.emplace_back(indices);
                        mMapIndices.emplace_back(indices);
                        for(uint32_t i = 0; i < size; i++)
                        {
                            mPositionIndices[mPositionIndices.size() - 1].emplace_back(std::stoi(fields[i * vcount]));
                            mNormalsIndices[mNormalsIndices.size() - 1].emplace_back(std::stoi(fields[i * vcount + 1]));
                            mMapIndices[mMapIndices.size() - 1].emplace_back(std::stoi(fields[i * vcount + 2]));
                        }
                    }
                }
            }
        }
        //read animations
        BOOST_FOREACH(const auto& animation, tree.get_child("COLLADA.library_animations"))
        {
            ReadAnimation(animation);
        }
        //read library_visual_scenes
        ptree visualNodes = tree.get_child("COLLADA.library_visual_scenes.visual_scene");
        auto numChildren = visualNodes.size() - visualNodes.count("<xmlattr>");
        for(ptree::const_iterator obj = visualNodes.begin(); obj != visualNodes.end(); ++obj)
        {
            //skip if first is <xmlattr>
            if(strncmp(obj->first.data(), "<xmlattr>", 10) == 0)
                continue;
            //check type joint
            if(obj->second.get_optional<std::string>("<xmlattr>.type") == boost::none)
                continue;
            //skeleton is node
            auto nodeId = obj->second.get_optional<std::string>("<xmlattr>.id").get();
            if(std::regex_search(obj->second.get_optional<std::string>("<xmlattr>.id").get(), std::regex("model", std::regex::icase)) ||
            std::regex_search(obj->second.get_optional<std::string>("<xmlattr>.id").get(), std::regex("armature", std::regex::icase)))
            {
                ReadSkeletonNode(obj, mRoot);
            }
        }
        //read controllers
        BOOST_FOREACH(const auto& controller, tree.get_child("COLLADA.library_controllers")) {
            ReadController(controller);
        }
    }
}


/*
destructor
*/
AEDrawObjectBaseCollada::~AEDrawObjectBaseCollada()
{

}

/*
scale
*/
void AEDrawObjectBaseCollada::Scale(float scale)
{
    mScale = scale;
    uint32_t size = mVertices.size();
    for(uint32_t i = 0; i < size; i++)
        mVertices[i].pos *= scale;
}

/*
make vertices data
*/
void AEDrawObjectBaseCollada::MakeVertices()
{
//    std::vector<std::vector<uint32_t>> p2n;
//    std::vector<std::vector<uint32_t>> p2m;
//    //initialize
//    std::vector<uint32_t> tmp;
//    for(uint32_t i = 0; i < mPositionIndices[0].size(); i++){
//        p2n.emplace_back(tmp);
//        p2m.emplace_back(tmp);
//    }
//    //corresponding index retrieve
//    for(uint32_t i = 0; i < mPositionIndices[0].size(); i++) {
//        p2n[mPositionIndices[0][i]].emplace_back(mNormalsIndices[0][i]);
//        p2m[mPositionIndices[0][i]].emplace_back(mMapIndices[0][i]);
//    }
//    //vertices
//    Vertex3DObj oneVertex;
//    glm::vec3 aveN(0.0f);
//    glm::vec2 aveM(0.0f);
//    for(uint32_t i = 0; i < mPositions.size(); i++)
//    {
//        uint32_t vertices = mPositions[i].size();
//        for(uint32_t j = 0; j < vertices; j++)
//        {
//            oneVertex.pos = mPositions[i][j];
//            for(auto index : p2n[j]){
//                aveN += mNormals[i][index];
//            }
//            aveN /= (float)p2n[j].size();
//            oneVertex.normal = aveN;
//            for(auto index : p2m[j]){
//                aveM += mMaps[i][index];
//            }
//            aveM /= (float)p2m[j].size();
//            oneVertex.texcoord = aveM;
//            mVertices.emplace_back(oneVertex);
//            aveN = glm::vec3(0.0f);
//            aveM = glm::vec2(0.0f);
//        }
//    }
//    //indices
//    uint32_t offset = 0;
//    for(uint32_t i = 0; i < mPositionIndices.size(); i++)
//    {
//        for(uint32_t j = 0; j < mPositionIndices[i].size(); j++)
//        {
//            mIndices.emplace_back(offset + mPositionIndices[i][j]);
//        }
//        offset += mPositions[i].size();
//    }
    uint32_t index = 0;
    Vertex3DObj oneVertex;
    for(uint32_t i = 0; i < mPositionIndices.size(); i++)
    {
        std::vector<uint32_t> serialP;
        for(uint32_t j = 0; j < mPositionIndices[i].size(); j++)
        {
            oneVertex.pos = mPositions[i][mPositionIndices[i][j]];
            oneVertex.normal = mNormals[i][mNormalsIndices[i][j]];
            oneVertex.texcoord = mMaps[i * 3][mMapIndices[i][j]];
            mVertices.emplace_back(oneVertex);
            mIndices.emplace_back(index);
            serialP.emplace_back(mPositionIndices[i][j]);
            index++;
        }
        mSerialPositionIndices.emplace_back(serialP);
    }
    //check joints and weights
    //CheckJointAndWeight();
}

/*
 *get vertex buffer size
 */
uint32_t AEDrawObjectBaseCollada::GetVertexBufferSize(){return sizeof(Vertex3DObj) * mVertices.size();}

/*
read skeleton node
if node exists
*/
void AEDrawObjectBaseCollada::ReadSkeletonNode(boost::property_tree::ptree::const_iterator nowNode,
    std::unique_ptr<AEDrawObjectBaseCollada::SkeletonNode>& skeletonNode)
{
    //allocate
    skeletonNode.reset(new SkeletonNode);
    //node name
    std::string strrrr = nowNode->second.data();
    auto sidName = nowNode->second.get_optional<std::string>("<xmlattr>.sid");
    if(sidName == boost::none)
        sidName = nowNode->second.get_optional<std::string>("<xmlattr>.id");
    if(sidName.is_initialized())
    {
        skeletonNode->sidName = sidName.get();
        skeletonNode->id = nowNode->second.get_optional<std::string>("<xmlattr>.id")->c_str();
    }
    //joint no
    skeletonNode->jointNo = -1;
    //matrix
    std::vector<std::string> fields;
    std::string matrixString;
    if(nowNode->second.get_optional<std::string>("matrix") != boost::none)
    {
        matrixString = nowNode->second.get_optional<std::string>("matrix").get();
        AEDrawObject::Split(fields, matrixString, ' ');

    }
    else
        matrixString.clear();
    if(matrixString.size() > 0)
    {
        for (uint32_t i = 0; i < 4; i++)
            for (uint32_t j = 0; j < 4; j++)
                skeletonNode->matrix[i][j] = std::stof(fields[(i * 4) + j]);
    }
    else
    {
        skeletonNode->matrix = glm::mat4(1.0f);
    }
    for(boost::property_tree::ptree::const_iterator child = nowNode->second.begin();
        child != nowNode->second.end(); ++child)
    {
        //jump if child.first is <xmlattr>
        if(strncmp(child->first.data(), "node", 10) != 0)
            continue;
        std::unique_ptr<SkeletonNode> node;
        ReadSkeletonNode(child, node);
        skeletonNode->children.push_back(std::move(node));
    }
}

/*
 * read animation data time and matrix
 */
void AEDrawObjectBaseCollada::ReadAnimation(const boost::property_tree::ptree::value_type& node)
{
    using namespace boost::property_tree;
    AnimationMatrix a{};
    std::string first = node.first.data();
    if(node.second.get_optional<std::string>("<xmlattr>.id") == boost::none)
        return;
    if(node.second.get_optional<std::string>("animation") != boost::none){
        BOOST_FOREACH(const auto& animation, node.second.get_child("")){
                        if(node.second.get_optional<std::string>("<xmlattr>.id") == boost::none)
                            continue;
                        ReadAnimationNode(animation);
        }
    } else {
        ReadAnimationNode(node);
    }
}

/*
 * read animation each node
 */
void AEDrawObjectBaseCollada::ReadAnimationNode(const boost::property_tree::ptree::value_type& node)
{
    using namespace boost::property_tree;
    AnimationMatrix a{};
    std::string first = node.first.data();
    if(node.second.get_optional<std::string>("<xmlattr>.id") == boost::none)
        return;
    a.id = node.second.get_optional<std::string>("<xmlattr>.id").get();
    BOOST_FOREACH(const auto& source, node.second.get_child(""))
    {
        auto childId = source.first.data();
        if(strcmp(childId, "<xmlattr>") == 0)
            continue;
        if(strcmp(childId, "source") == 0)
        {
            auto floatId = source.second.get_optional<std::string>("<xmlattr>.id")->c_str();
            if (mAnimationTime.size() == 0 && std::regex_search(source.second.get_optional<std::string>("<xmlattr>.id")->c_str(), std::regex("input")))
            {
                std::string timeListS = source.second.get<std::string>("float_array");
                std::vector<std::string> timeList;
                AEDrawObject::Split(timeList, timeListS, ' ');
                for (uint32_t i = 0; i < timeList.size(); i++) {
                    float time = std::stof(timeList[i]);
                    mAnimationTime.emplace_back(time);
                }
            }
            else if(std::regex_search(source.second.get_optional<std::string>("<xmlattr>.id")->c_str(), std::regex("output")))
            {
                std::string matrixListS = source.second.get<std::string>("float_array");
                std::vector<std::string> matrixList;
                AEDrawObject::Split(matrixList, matrixListS, ' ');
                for (uint32_t i = 0; i < matrixList.size(); i =  i + 16)
                {
                    glm::mat4 m(1.0f);
                    for(uint32_t j = 0; j < 4; j++)
                    {
                        for(uint32_t k = 0; k < 4; k++)
                        {
                            float e = std::stof(matrixList[i + (4 * j) + k]);
                            m[j][k] = e;
                        }
                    }
                    a.matrixList.emplace_back(m);
                }
            }
        }
        if(strcmp(childId, "channel") == 0)
        {
            std::string target = source.second.get_optional<std::string>("<xmlattr>.target")->c_str();
            a.target = target.replace(target.find("/"), target.size() - target.find("/"), "");
        }
    }
    mAnimationMatrices.emplace_back(a);
}

/*
 * read effects
 */
void AEDrawObjectBaseCollada::ReadEffect(const boost::property_tree::ptree::value_type& node)
{
    using namespace boost::property_tree;
    std::string first = node.first.data();
    if(node.second.get_optional<std::string>("<xmlattr>.id") == boost::none)
        return;
    std::string effectId = node.second.get_optional<std::string>("<xmlattr>.id").get();
    TextureMap t = {};
    BOOST_FOREACH(const auto& profile, node.second.get_child("")){
         auto profileId = profile.first.data();
         if(std::regex_search(profileId, std::regex("<xmlattr>", std::regex::icase)))
             continue;
         //profile_COMMON
         if(std::regex_search(profileId, std::regex("profile", std::regex::icase))){
             BOOST_FOREACH(const auto& child, profile.second.get_child("")){
                //newparam
                if(std::regex_search(child.first.data(), std::regex("newparam", std::regex::icase))) {
                    if (child.second.get_optional<std::string>("surface") != boost::none) {
                        const auto &surface = child.second.get_child("surface");
                        if (surface.get_optional<std::string>("<xmlattr>.type") != boost::none) {
                            auto textureName = surface.get<std::string>("init_from");
                            t.textureImage = textureName;
                        }
                    }
                }
                //technique
                if(std::regex_search(child.first.data(), std::regex("technique", std::regex::icase))) {
                    BOOST_FOREACH(const auto& techChild, child.second.get_child("")) {
                        if (techChild.second.get_optional<std::string>("diffuse") !=
                            boost::none) {
                            BOOST_FOREACH(const auto& diffuse, techChild.second.get_child("diffuse")) {
                                auto texcoord = diffuse.second.get_optional<std::string>("<xmlattr>.texcoord").get();
                                t.texcoord = texcoord;
                            }
                        }
                    }
                }
             }
         }
    }
    mTextureMap.emplace_back(t);
}

/*
 * read collada controller
 */
void AEDrawObjectBaseCollada::ReadController(const boost::property_tree::ptree::value_type& node)
{
    using namespace boost::property_tree;
    std::vector<std::string> fields;
    std::vector<std::string> vertexWeights;
    BOOST_FOREACH(const auto& skinNodes, node.second.get_child("skin")) {
        auto firstId = skinNodes.first.data();
        //bsm
        if(std::regex_search(firstId, std::regex("bind_shape_matrix", std::regex::icase))){
            std::string bsmString = skinNodes.second.get_value_optional<std::string>().get();
            AEDrawObject::Split(fields, bsmString, ' ');
            glm::mat4 oneMatrix;
            for (uint32_t i = 0; i * 16 < fields.size(); i++) {
                for (uint32_t j = 0; j < 4; j++)
                    for (uint32_t k = 0; k < 4; k++)
                        oneMatrix[j][k] = std::stof(
                                fields[16 * i + 4 * j + k]);
                mBindShapeMatrices.emplace_back(oneMatrix);
            }
        }
        //source id = skin-joints
        if (strncmp(skinNodes.first.data(), "source", 7) == 0) {
            if (std::regex_search(
                    skinNodes.second.get_optional<std::string>("<xmlattr>.id").get(),
                    std::regex("joint", std::regex::icase))) {
                auto sourceId = skinNodes.second.get_optional<std::string>(
                        "<xmlattr>.id").get();
                for (boost::property_tree::ptree::const_iterator child = skinNodes.second.begin();
                     child != skinNodes.second.end(); ++child) {
                    //find Name_array
                    if (strncmp(child->first.data(), "Name_array", 11) != 0)
                        continue;
                    std::string nameArrayString = child->second.get_value_optional<std::string>().get();
                    AEDrawObject::Split(fields, nameArrayString, ' ');
                    for (auto f: fields) {
                        JointMapper j;
                        j.jointName = f;
                        j.animNo = -1;
                        mSkinJointsArray.emplace_back(j);
                    }
                }
            }
        }
        //source id = matrices
        if (strncmp(skinNodes.first.data(), "source", 7) == 0) {
            if (std::regex_search(
                    skinNodes.second.get_optional<std::string>("<xmlattr>.id").get(),
                    std::regex("matrices", std::regex::icase))) {
                for (boost::property_tree::ptree::const_iterator child = skinNodes.second.begin();
                     child != skinNodes.second.end(); ++child) {
                    //find Name_array
                    if (strncmp(child->first.data(), "float_array", 11) != 0)
                        continue;
                    std::string matricesString = child->second.get_value_optional<std::string>().get();
                    AEDrawObject::Split(fields, matricesString, ' ');
                    uint32_t index = 0;
                    for (uint32_t i = 0; i < fields.size(); i = i + 16) {
                        glm::mat4 m;
                        for (uint32_t j = 0; j < 4; j++) {
                            for (uint32_t k = 0; k < 4; k++) {
                                m[j][k] = std::stof(fields[i + j * 4 + k]);
                            }
                        }
                        mSkinJointsArray[index].controllerMatrix = m;
                        index++;
                    }
                }
            }
        }
        //source id = skin-weights
        if (strncmp(skinNodes.first.data(), "source", 7) == 0) {
            if (std::regex_search(
                    skinNodes.second.get_optional<std::string>("<xmlattr>.id").get(),
                    std::regex("skin-weights", std::regex::icase))) {
                for (boost::property_tree::ptree::const_iterator child = skinNodes.second.begin();
                     child != skinNodes.second.end(); ++child) {
                    //find float_array
                    if (strncmp(child->first.data(), "float_array", 11) != 0)
                        continue;
                    std::string weightString = child->second.get_value_optional<std::string>().get();
                    AEDrawObject::Split(vertexWeights, weightString, ' ');
                }
            }
        }
        //vertex_weights
        if (std::regex_search(skinNodes.first.data(),
                              std::regex("vertex_weights", std::regex::icase))) {
            uint32_t totalVCount = 0;
            auto vcountString = skinNodes.second.get_optional<std::string>("vcount").get();
            std::vector<std::string> influenceCounts;
            AEDrawObject::Split(influenceCounts, vcountString, ' ');
            auto vString = skinNodes.second.get_optional<std::string>("v").get();
            std::vector<std::string> jointWeightList;
            AEDrawObject::Split(jointWeightList, vString, ' ');
            uint32_t nowPointing = 0;
            uint32_t vertexIndex = 0;
            uint32_t jointOffset = 0;
            std::vector<uint32_t> influenceCountList;
            std::vector<uint32_t> jointOffsetList;
            mInfluenceCountList.emplace_back(influenceCountList);
            mJointOffsetList.emplace_back(jointOffsetList);
            uint32_t ilistIndex = mInfluenceCountList.size() - 1;
            uint32_t jlistIndex = mJointOffsetList.size() - 1;
            for (uint32_t i = 0; i < influenceCounts.size(); i++) {
                JointWeight oneJointWeight;
                uint32_t influenceCount = std::stoi(influenceCounts[i]);
                mInfluenceCountList[ilistIndex].emplace_back(influenceCount);
                mJointOffsetList[jlistIndex].emplace_back(totalVCount);
                if (totalVCount < 1024 && totalVCount + influenceCount > 1024) {
                    mComputeLocalSize = totalVCount;
                }
                totalVCount += influenceCount;
                for (uint32_t j = 0; j < influenceCount; j++) {
                    int joint = std::stoi(jointWeightList[nowPointing]);
                    oneJointWeight.jointIndices.push_back(joint);
                    mJoints.emplace_back(joint);
                    nowPointing++;
                    float f = std::stof(
                            vertexWeights[std::stoi(jointWeightList[nowPointing])]);
                    oneJointWeight.weights.push_back(f);
                    mWeights.emplace_back(f);
                    nowPointing++;
                    mVerteces2.emplace_back(vertexIndex);
                }
                //check total weight
                float total = 0.0f;
                for (uint32_t j = 0; j < influenceCount; j++)
                    total += oneJointWeight.weights[j];
                if (!(0.99f < total && total < 1.01f))
                    __android_log_print(ANDROID_LOG_DEBUG, "animation",
                                        "total weight is not 1.0f", 0);
                mJointWeights.push_back(oneJointWeight);
                //mapping each positon to influenced joints
                for (uint32_t k = 0; k < influenceCount; k++) {
                    std::pair<uint32_t, float> p;
                    p = {vertexIndex, oneJointWeight.weights[k]};
                    mSkinJointsArray[oneJointWeight.jointIndices[k]].indexWeight.emplace_back(
                            p);
                }
                vertexIndex++;
            }
            //check values
            if (totalVCount != mVerteces2.size()) {
                __android_log_print(ANDROID_LOG_DEBUG, "animation",
                                    "vcount not equal to VList", 0);
            }
        }
        //inverse matrices
        if (strncmp(skinNodes.first.data(), "joints", 7) == 0) {
            //get inverse matrices id
            std::string inverseMatricesId;
            for (boost::property_tree::ptree::const_iterator child = skinNodes.second.begin();
                 child != skinNodes.second.end(); ++child) {
                //find Name_array
                if (strncmp(child->first.data(), "input", 6) != 0)
                    continue;
                if (strncmp(child->second.get_optional<std::string>(
                                    "<xmlattr>.semantic").get().c_str(),
                            "INV_BIND_MATRIX", 16) == 0)
                    inverseMatricesId = child->second.get_optional<std::string>(
                            "<xmlattr>.source").get();
            }
            //inverse matrices
            BOOST_FOREACH(const auto& innerObj, node.second.get_child("skin")) {
                auto innerObjId = innerObj.first.data();
                if (std::strncmp(innerObj.first.data(), "source", 7) == 0) {
                    auto sourceId = innerObj.second.get_optional<std::string>(
                            "<xmlattr>.id").get();
                    if (sourceId.find(inverseMatricesId.c_str()) != std::string::npos ||
                        inverseMatricesId.find(sourceId.c_str()) != std::string::npos) {
                        for (auto child = innerObj.second.begin();
                             child != innerObj.second.end(); ++child) {
                            //find float_array
                            if (strncmp(child->first.data(), "float_array", 11) != 0)
                                continue;
                            std::string inverseString = child->second.get_value_optional<std::string>().get();
                            AEDrawObject::Split(fields, inverseString, ' ');
                            glm::mat4 oneMatrix;
                            for (uint32_t i = 0; i * 16 < fields.size(); i++) {
                                for (uint32_t j = 0; j < 4; j++)
                                    for (uint32_t k = 0; k < 4; k++)
                                        oneMatrix[j][k] = std::stof(
                                                fields[16 * i + 4 * j + k]);
                                mInverseMatrices.push_back(oneMatrix);
                            }
                        }
                    }
                }
            }
        }
    }
}


/*
get vertex weights
*/
void AEDrawObjectBaseCollada::GetVertexWeights(std::vector<float> &vertexWeights,
    std::string& weightString)
{
    std::vector<std::string> fields;
    AEDrawObject::Split(fields, weightString, ' ');
    vertexWeights.resize(fields.size());
    for(uint32_t i = 0; i < fields.size(); i++)
        vertexWeights[i] = std::stof(fields[i]);
}

/*
 * animation
 *
 */
void AEDrawObjectBaseCollada::Animation()
{
    //mapping skeleton joint to joint array
    SkeletonJointNo(mRoot.get());
    //joint mapper
    for(auto& joint : mSkinJointsArray)
    {
        for(uint32_t i = 0; i < mAnimationMatrices.size(); i++)
        {
            if(joint.nodeId == mAnimationMatrices[i].target)
            {
                joint.animNo = i;
                //check anim <-> joint in logs
                __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", (std::string("animation id = ") + mAnimationMatrices[i].id).c_str(), 0);
                __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", (std::string("animation target = ") + mAnimationMatrices[i].target).c_str(), 0);
                __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", (std::string("joint name = ") + joint.jointName).c_str(), 0);
                __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", (std::string("node id = ") + joint.nodeId).c_str(), 0);
                break;
            }
        }
    }
    //check vertices for each joint
    uint32_t i = 0;
    for(auto joint : mSkinJointsArray){
//        __android_log_print(ANDROID_LOG_DEBUG, "aqoole joint check", (std::string("joint no = ") + std::to_string(i)).c_str(), 0);
//        __android_log_print(ANDROID_LOG_DEBUG, "aqoole joint check", (std::string("joint name = ") + joint.jointName).c_str(), 0);
        std::string index;
        std::string weight;
        for(auto vw : joint.indexWeight){
            index += std::to_string(vw.first);
            index += std::string(" ");
            weight += (std::to_string(vw.second) + std::string(" "));
        }
//        __android_log_print(ANDROID_LOG_DEBUG, "aqoole joint check", (std::string("vertex in joint = ") + index).c_str(), 0);
//        __android_log_print(ANDROID_LOG_DEBUG, "aqoole joint check", (std::string("weight in joint = ") + weight).c_str(), 0);
        i++;
    }
}

/*
 * mapping skeleton joint no
 */
void AEDrawObjectBaseCollada::SkeletonJointNo(SkeletonNode* node)
{
    for(uint32_t i = 0; i < mSkinJointsArray.size(); i++)
    {
        if(node->sidName == mSkinJointsArray[i].jointName)
        {
            node->jointNo = i;
            mSkinJointsArray[i].nodeId = node->id;
            break;
        }
    }
    for(uint32_t i = 0; i < node->children.size(); i++)
    {
        SkeletonJointNo(node->children[i].get());
    }
}

/*
 * skeleton animation
 */
void AEDrawObjectBaseCollada::SkeletonAnimation(SkeletonNode* node, glm::mat4 parentBindPoseMatrix, glm::mat4 parentAnimationMatrix,
                                                glm::mat4 ibp, std::vector<glm::vec3>& tmpPositions)
{
    int jointnum = node->jointNo;
    int animnum = mSkinJointsArray[jointnum].animNo;
    __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation",
                        (std::string("anim num = ") + std::to_string(animnum)).c_str(), 0);
    __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation",
                        (std::string("joint num = ") + std::to_string(jointnum)).c_str(), 0);
    __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", (std::string("node id = ") + node->id).c_str(), 0);
    if(mAnimationMatrices.empty())
    {
        __android_log_print(ANDROID_LOG_ERROR, "aqoole animation", "no animation matrices");
    }
    if(jointnum >= 0 && animnum >= 0) {
        //transpose matrix
        node->matrix = glm::transpose(node->matrix);
        glm::mat4 animationTransform = mAnimationMatrices[animnum].matrixList[4];
        animationTransform = glm::transpose(animationTransform);
//        //rotate
//            glm::mat4 r = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
//            animationTransform = r * animationTransform;
//            node->matrix = r * node->matrix;
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation transform", (std::string("animation name = ") + mAnimationMatrices[animnum].id).c_str(), 0);
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation transform", (std::string("joint name = ") + node->id).c_str(), 0);
        parentAnimationMatrix = parentAnimationMatrix * animationTransform;
        parentBindPoseMatrix = parentBindPoseMatrix * node->matrix;
        glm::mat4 inverseBindPose = glm::inverse(parentBindPoseMatrix);
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation",
                            "node matrix retrieved successfully");
        ibp = ibp * mInverseMatrices[node->jointNo];
        glm::vec4 oldpos;
        glm::vec4 newpos;
        glm::mat4 finalTransform = parentAnimationMatrix * inverseBindPose;
        for (auto indexWeight: mSkinJointsArray[jointnum].indexWeight) {
            oldpos = glm::vec4(mPositions[0][indexWeight.first], 1.0f);
            newpos = indexWeight.second * finalTransform * oldpos;
            tmpPositions[indexWeight.first] += glm::vec3(newpos);
        }
    }
    if(node->children.size() > 0) {
        for (auto &child: node->children)
            SkeletonAnimation(child.get(), parentBindPoseMatrix, parentAnimationMatrix, ibp,
                              tmpPositions);
    }
}

/*
 * animation compute pipeline
 */
void AEDrawObjectBaseCollada::AnimationPrepare(android_app* app, AELogicalDevice* device, std::vector<const char*>& shaders,
                                               AEBufferBase* buffer[], AEDeviceQueue* queue, AECommandPool* commandPool, AEDescriptorPool* descriptorPool) {
    //compute pipeline
    AEDescriptorSetLayout cl(device);
    cl.AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(9, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(10, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(11, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(12, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(13, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(14, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.CreateDescriptorSetLayout();
    mComputePipeline = std::make_unique<AEComputePipeline>(device, shaders, &cl, app);
    //buffers for descriptor set
    VkDeviceSize indicesBufferSize = mVerteces2.size() * sizeof(uint32_t);
    VkDeviceSize jointsBufferSize = mJoints.size() * sizeof(uint32_t);
    VkDeviceSize weightsBufferSize = mWeights.size() * sizeof(float);
    //buffer sizes
    std::vector<VkDeviceSize> positionBufferSizes;
    std::vector<VkDeviceSize> influenceCountSizes;
    std::vector<VkDeviceSize> jointOffsetSizes;
    for(uint32_t i = 0; i < mPositionIndices.size(); i++) {
        //base position buffer
        positionBufferSizes.emplace_back(mPositions[i].size() * sizeof(glm::vec3));
        std::unique_ptr<AEBufferUtilOnGPU> positionBuffer = std::make_unique<AEBufferUtilOnGPU>(device,
                                                                                                positionBufferSizes[i],
                                                                                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        positionBuffer->CreateBuffer();
        positionBuffer->CopyData((void *) mPositions[0].data(), 0, positionBufferSizes[i], queue,
                                 commandPool);
        mBasePositionBuffers.emplace_back(std::move(positionBuffer));
        //influence count buffer
        VkDeviceSize influenceCountListBufferSize =
                mInfluenceCountList[i].size() * sizeof(uint32_t);
        influenceCountSizes.emplace_back(influenceCountListBufferSize);
        std::unique_ptr<AEBufferUtilOnGPU> influenceCountListBuffer = std::make_unique<AEBufferUtilOnGPU>(
                device, influenceCountListBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        influenceCountListBuffer->CreateBuffer();
        influenceCountListBuffer->CopyData((void *) mInfluenceCountList[i].data(), 0,
                                           influenceCountListBufferSize, queue, commandPool);
        mInfluenceCountBuffers.emplace_back(std::move(influenceCountListBuffer));
        //joint offset buffer
        VkDeviceSize jointOffsetSize = mJointOffsetList[i].size() * sizeof(uint32_t);
        jointOffsetSizes.emplace_back(jointOffsetSize);
        std::unique_ptr<AEBufferUtilOnGPU> jointOffsetBuffer = std::make_unique<AEBufferUtilOnGPU>(
                device, jointOffsetSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        jointOffsetBuffer->CreateBuffer();
        jointOffsetBuffer->CopyData((void *) mJointOffsetList[i].data(), 0, jointOffsetSize, queue,
                                    commandPool);
        mJointOffsetBuffers.emplace_back(std::move(jointOffsetBuffer));
    }
    //animation result positions
    if (buffer == nullptr) {
        std::unique_ptr<AEBufferUtilOnGPU> positionResultBuffer = std::make_unique<AEBufferUtilOnGPU>(
                device, positionBufferSizes[0], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        positionResultBuffer->CreateBuffer();
        positionResultBuffer->CopyData((void *) mPositions[0].data(), 0, positionBufferSizes[0], queue,
                                       commandPool);
        mBuffers.emplace_back(std::move(positionResultBuffer));
    } else {
        mBuffers.emplace_back((AEBufferUtilOnGPU*)buffer[0]);
    }
    //joints
    std::unique_ptr<AEBufferUtilOnGPU> jointBuffer = std::make_unique<AEBufferUtilOnGPU>(device, jointsBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    jointBuffer->CreateBuffer();
    jointBuffer->CopyData((void*)mJoints.data(), 0, jointsBufferSize, queue, commandPool);
    mBuffers.emplace_back(std::move(jointBuffer));
    //weights
    std::unique_ptr<AEBufferUtilOnGPU> weightBuffer = std::make_unique<AEBufferUtilOnGPU>(device, weightsBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    weightBuffer->CreateBuffer();
    weightBuffer->CopyData((void*)mWeights.data(), 0, weightsBufferSize, queue, commandPool);
    mBuffers.emplace_back(std::move(weightBuffer));
    //animation mats
    for(uint32_t i = 0; i < mSkinJointsArray.size(); i++){
        mAnimationTransforms.emplace_back(glm::mat4(1.0f));
        mAnimationTransformsNext.emplace_back(glm::mat4(1.0f));
    }
    VkDeviceSize matBufferSize = mAnimationTransforms.size() * sizeof(glm::mat4);
    std::unique_ptr<AEBufferUtilOnGPU> matsBuffer = std::make_unique<AEBufferUtilOnGPU>(device, matBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    matsBuffer->CreateBuffer();
    matsBuffer->CopyData((void*)mAnimationTransforms.data(), 0, matBufferSize, queue, commandPool);
    mBuffers.emplace_back(std::move(matsBuffer));
    //uniform buffer
    VkDeviceSize uniformSize = sizeof(AnimationUniforms);
    std::unique_ptr<AEBufferUniform> uniformBuffer = std::make_unique<AEBufferUniform>(device, uniformSize);
    uniformBuffer->CreateBuffer();
    AnimationUniforms au{};
    au.animNum = 0;
    au.scale = mScale;
    au.time = 0.0f;
    au.vertexSize = mSerialPositionIndices[0].size();
    uniformBuffer->CopyData((void*)&au, uniformSize);
    mUniformBuffers.emplace_back(std::move(uniformBuffer));
    //debug buffer
    VkDeviceSize debugSize = mVertices.size() * sizeof(uint32_t);
    std::vector<uint32_t> debugData;
    for(uint32_t i = 0; i < mVertices.size(); i++)
        debugData.emplace_back(0);
    std::unique_ptr<AEBufferUtilOnGPU> debugBuffer = std::make_unique<AEBufferUtilOnGPU>(device, debugSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    debugBuffer->CreateBuffer();
    debugBuffer->CopyData((void*)debugData.data(), 0, debugSize, queue, commandPool);
    mBuffers.emplace_back(std::move(debugBuffer));
    //debug buffer for VERTEX3DOBJ
    VkDeviceSize debugVSize = mVerteces2.size() * sizeof(Vertex3DObj);
    std::vector<Vertex3DObj> tmpVs;
    Vertex3DObj tmpV = {};
    for(uint32_t i = 0; i < mVertices.size(); i++){
        tmpVs.emplace_back(tmpV);
    }
    std::unique_ptr<AEBufferUtilOnGPU> debugVertexBuffer = std::make_unique<AEBufferUtilOnGPU>(device, debugVSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    debugVertexBuffer->CreateBuffer();
    debugVertexBuffer->CopyData((void*)tmpVs.data(), 0, debugVSize, queue, commandPool);
    mBuffers.emplace_back(std::move(debugVertexBuffer));
    //animation next
    std::unique_ptr<AEBufferUtilOnGPU> matsNextBuffer = std::make_unique<AEBufferUtilOnGPU>(device, matBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    matsNextBuffer->CreateBuffer();
    matsNextBuffer->CopyData((void*)mAnimationTransformsNext.data(), 0, matBufferSize, queue, commandPool);
    mBuffers.emplace_back(std::move(matsNextBuffer));
    //index buffer
    VkDeviceSize indicesListSize = sizeof(uint32_t) * mSerialPositionIndices[0].size();
    std::unique_ptr<AEBufferUtilOnGPU> indicesList = std::make_unique<AEBufferUtilOnGPU>(device, indicesListSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    indicesList->CreateBuffer();
    indicesList->CopyData((void*)mSerialPositionIndices[0].data(), 0, indicesListSize, queue, commandPool);
    mBuffers.emplace_back(std::move(indicesList));
    //animation key frame time
    VkDeviceSize keyFrameSize = sizeof(float) * mAnimationTime.size();
    std::unique_ptr<AEBufferUtilOnGPU> keyFrameBuffer = std::make_unique<AEBufferUtilOnGPU>(device, keyFrameSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    keyFrameBuffer->CreateBuffer();
    keyFrameBuffer->CopyData((void*)mAnimationTime.data(), 0, keyFrameSize, queue, commandPool);
    mBuffers.emplace_back(std::move(keyFrameBuffer));
    //prepare descriptor set
    for(uint32_t i = 0; i < mPositionIndices.size(); i++) {
        std::unique_ptr<AEDescriptorSet> ds = std::make_unique<AEDescriptorSet>(device, &cl, descriptorPool);
        ds->BindDescriptorBuffer(0, mBasePositionBuffers[i]->GetBuffer(), positionBufferSizes[i],
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        if (buffer == nullptr) {
            ds->BindDescriptorBuffer(1, mBuffers[0]->GetBuffer(), positionBufferSizes[0],
                                      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        } else {
            ds->BindDescriptorBuffer(1, mBuffers[0]->GetBuffer(),
                                      sizeof(Vertex3DObj) * mVertices.size(),
                                      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        }
        ds->BindDescriptorBuffer(2, mInfluenceCountBuffers[i]->GetBuffer(), influenceCountSizes[i],
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(3, mBuffers[1]->GetBuffer(), jointsBufferSize,
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(4, mBuffers[2]->GetBuffer(), weightsBufferSize,
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(5, mBuffers[3]->GetBuffer(), matBufferSize,
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(6, mUniformBuffers[0]->GetBuffer(), uniformSize,
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        ds->BindDescriptorBuffer(7, mBuffers[4]->GetBuffer(), debugSize,
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(8, mBuffers[5]->GetBuffer(), debugVSize,
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(9, mBuffers[6]->GetBuffer(), matBufferSize,
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(10, mJointOffsetBuffers[i]->GetBuffer(), jointOffsetSizes[i],
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(11, mBuffers[7]->GetBuffer(), indicesListSize,
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(12, mBuffers[8]->GetBuffer(), keyFrameSize,
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        mDSs.emplace_back(std::move(ds));
    }
}

/*
 *
 * animation dispatch compute entry function
 */
void AEDrawObjectBaseCollada::AnimationDispatch(AELogicalDevice* device, AECommandBuffer* command, AEDeviceQueue* queue, AECommandPool* commandPool,
                                                uint32_t animationNum, AEFence* fence, VkSemaphore *waitSemaphore, VkSemaphore* signalSemaphore,
                                                double time, AEEvent* event)
{
    //command record for each joint
    AECommand::BeginCommand(command);
    //prepare event
    vkCmdResetEvent(*command->GetCommandBuffer(), *event->GetEvent(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    AECommand::BindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline.get());
    //update animation transforms
    AnimationDispatchJoint(mRoot.get(), glm::transpose(mBindShapeMatrices[0]), glm::mat4(1.0f), animationNum, mAnimationTransforms);
    AnimationDispatchJoint(mRoot.get(), glm::transpose(mBindShapeMatrices[0]), glm::mat4(1.0f), (animationNum + 1) % 5, mAnimationTransformsNext);
    //update buffer
    VkDeviceSize matBufferSize = mAnimationTransforms.size() * sizeof(glm::mat4);
    mBuffers[3]->CopyData((void*)mAnimationTransforms.data(), 0, matBufferSize, queue, commandPool);
    mBuffers[6]->CopyData((void*)mAnimationTransformsNext.data(), 0, matBufferSize, queue, commandPool);
    //uniform buffer
    AnimationUniforms au{};
    au.time = (float)time;
    au.scale = mScale;
    au.animNum = animationNum;
    for(uint32_t i = 0; i < 1; i++) {
        au.vertexSize = mSerialPositionIndices[i].size();
        mUniformBuffers[0]->CopyData((void*)&au, sizeof(AnimationUniforms));
        AECommand::BindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE,
                                      mComputePipeline->GetPipelineLayout(),
                                      1, mDSs[i]->GetDescriptorSet());
        //dispatch
        //each work groups
        vkCmdDispatch(*command->GetCommandBuffer(), 5, 1, 1);
    }
    vkCmdSetEvent(*command->GetCommandBuffer(), *event->GetEvent(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    //each local thread
    //vkCmdDispatch(*command->GetCommandBuffer(), 740, 1, 1);
    AECommand::EndCommand(command);
    //submit
    VkSubmitInfo submit_info = {};
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    if(waitSemaphore != nullptr) {
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphore,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 0,
                .pSignalSemaphores = nullptr};
    } else if(signalSemaphore != nullptr){
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphore,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signalSemaphore};
    }else{
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 0,
                .pWaitSemaphores = nullptr,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 0,
                .pSignalSemaphores = nullptr};
    }
    if(fence != nullptr) {
        vkQueueSubmit(queue->GetQueue(0), 1, &submit_info, *fence->GetFence());
    }
    else{
        vkQueueSubmit(queue->GetQueue(0), 1, &submit_info, VK_NULL_HANDLE);
    }
}

/*
 * animation dispatch for each joint
 */
void AEDrawObjectBaseCollada::AnimationDispatchJoint(SkeletonNode* node, glm::mat4 parentBindPoseMatrix, glm::mat4 parentAnimationMatrix, uint32_t animationNum,
                                                     std::vector<glm::mat4> &targetTransform)
{
    //create joint and weight buffer
    int jointnum = node->jointNo;
    int animnum = mSkinJointsArray[node->jointNo].animNo;
    if(jointnum >= 0 && animnum >= 0) {
        //compute matrices and put uniform buffer
        parentAnimationMatrix = parentAnimationMatrix * glm::transpose(mAnimationMatrices[animnum].matrixList[animationNum]);
        parentBindPoseMatrix = parentBindPoseMatrix * glm::transpose(node->matrix);
        glm::mat4 inverseBindPose = glm::inverse(parentBindPoseMatrix);
        glm::mat4 finalTransform = parentAnimationMatrix * inverseBindPose;
        targetTransform[jointnum] = finalTransform;
    }
    //continue to child
    if(node->children.size() > 0) {
        for (auto &child: node->children)
            AnimationDispatchJoint(child.get(), parentBindPoseMatrix, parentAnimationMatrix, animationNum, targetTransform);
    }
}

/*
 * debug
 */
void AEDrawObjectBaseCollada::Debug(AEDeviceQueue* queue, AECommandPool* commandPool)
{
    VkDeviceSize debugSize = mJoints.size() * sizeof(uint32_t);
    std::vector<uint32_t> debugData;
    for(uint32_t i = 0; i < mJoints.size(); i++)
        debugData.emplace_back(0);
    mBuffers[1]->BackData(debugData.data(), 0, debugSize, queue, commandPool);
    //compare buffer and data
    for(uint32_t i = 0; i < mJoints.size(); i++){
        if(mJoints[i] != debugData[i])
            __android_log_print(ANDROID_LOG_DEBUG, "animation", (std::string("data not equal index = ") + std::to_string(i) +
            std::string("data, buffer = ") + std::to_string(mJoints[i]) + std::to_string(debugData[i])).c_str(), 0);
    }
    //debug data
    VkDeviceSize debugVSize = mVertices.size() * sizeof(int);
    std::vector<int> debugVData;
    for(uint32_t i = 0; i < mVertices.size(); i++)
        debugVData.emplace_back(-1);
    mBuffers[4]->BackData(debugVData.data(), 0, debugVSize, queue, commandPool);
    //calc position
    VkDeviceSize positionSize = mVertices.size() * sizeof(Vertex3DObj);
    std::vector<Vertex3DObj> debugPData;
    Vertex3DObj obj = {};
    for(uint32_t i = 0; i < mPositions[0].size(); i++)
        debugPData.emplace_back(obj);
    mBuffers[0]->BackData(debugPData.data(), 0, positionSize, queue, commandPool);
    for(uint32_t i = 0; i < mVertices.size(); i++) {
        if(glm::length(mVertices[i].pos - debugPData[i].pos) > 18.0f)
            DebugPositionObj(i, debugPData);
    }
    DebugWeights(queue, commandPool);
//    //clear pos
//    for(uint32_t i = 0; i < mVertices.size(); i++){
//        mVertices[i].pos = glm::vec3(0.0f);
//    }
    //calc by vertex
//    for(uint32_t i = 0; i < mVerteces2.size(); i++){
//        uint32_t index = mVerteces2[i];
//        mVertices[index].pos += debugVData[i].pos;
//    }
//    std::vector<glm::vec3> tmpPos;
//    for(uint32_t i = 0; i < mPositions[0].size(); i++){
//        tmpPos.emplace_back(glm::vec3(0.0f));
//    }
//    for(uint32_t i = 0; i < mVerteces2.size(); i++){
//        uint32_t index = mVerteces2[i];
//        tmpPos[index] += debugPData[i];
//    }
//    for(uint32_t i = 0; i < mPositionIndices[0].size(); i++){
//        mVertices[i].pos = tmpPos[mPositionIndices[0][i]];
//    }
//    for(uint32_t i = 0; i < mPositionIndices[0].size(); i++){
//        uint index = mPositionIndices[0][i];
//        mVertices[i].pos = debugPData[mPositionIndices[0][i]];
//        if(index == 149){
//            mVertices[i].pos = debugPData[mPositionIndices[0][i]] * 0.1f;
//        }
//    }
//    //debug
//    for(uint32_t i = 0; i < mPositions[0].size(); i++){
//        //debug
//        if(glm::distance(mVertices[i].pos, debugPData[i]) > 20.0f){
//            DebugPosition(i, debugPData);
//        }
//    }
    //debug for animation matrix data
//    std::vector<glm::mat4> debugMats;
//    for(uint32_t i = 0; i < mAnimationTransforms.size(); i++){
//        debugMats.emplace_back(glm::mat4(1.0f));
//    }
//    VkDeviceSize matBufferSize = sizeof(glm::mat4) * mAnimationTransforms.size();
//    mBuffers[3]->BackData((void*)debugMats.data(), 0, matBufferSize, queue, commandPool);
//    for(uint32_t i = 0; i < mAnimationTransforms.size(); i++){
//        if(mAnimationTransforms[i] != debugMats[i]){
//            __android_log_print(ANDROID_LOG_DEBUG, "animation", (std::string("animation matrix not equal") + std::to_string(i)).c_str(), 0);
//        }
//    }
    int breakpoint = 0;
}

/*
 * debug position
 */
void AEDrawObjectBaseCollada::DebugPosition(uint32_t index, std::vector<glm::vec3> const& debug)
{
    std::string log("position not equal at ");
    std::string space(" ");
    std::string endl("\n");
    log += (std::to_string(index) + space + endl);
    log += std::string("data:x = ") + std::to_string(mVertices[index].pos.x) + space +
            std::string("y = ") + std::to_string(mVertices[index].pos.y) + space +
            std::string("z = ") + std::to_string(mVertices[index].pos.z) + space + endl;
    log += std::string("debug:x = ") + std::to_string(debug[index].x) + space +
           std::string("y = ") + std::to_string(debug[index].y) + space +
           std::string("z = ") + std::to_string(debug[index].z) + space;
    __android_log_print(ANDROID_LOG_DEBUG, "animation", log.c_str(), 0);
}

/*
 * debug position vertex3dobj
 */
void AEDrawObjectBaseCollada::DebugPositionObj(uint32_t index, std::vector<Vertex3DObj> const& debug)
{
    std::string log("position not equal at ");
    std::string space(" ");
    std::string endl("\n");
    log += (std::to_string(index) + space + endl);
    uint32_t posIndex = mPositionIndices[0][index];
    log += (std::string("geometry num = ") + std::to_string(posIndex) + endl);
    log += std::string("data:x = ") + std::to_string(mVertices[index].pos.x) + space +
           std::string("y = ") + std::to_string(mVertices[index].pos.y) + space +
           std::string("z = ") + std::to_string(mVertices[index].pos.z) + space + endl;
    log += std::string("debug:x = ") + std::to_string(debug[index].pos.x) + space +
           std::string("y = ") + std::to_string(debug[index].pos.y) + space +
           std::string("z = ") + std::to_string(debug[index].pos.z) + space;
    __android_log_print(ANDROID_LOG_DEBUG, "animation", log.c_str(), 0);
}

/*
 * debug weights
 */
void AEDrawObjectBaseCollada::DebugWeights(AEDeviceQueue* queue, AECommandPool* commandPool)
{
    std::vector<float> tmpW;
    for(uint32_t i = 0; i < mWeights.size(); i++)
        tmpW.emplace_back(0.0f);
    mBuffers[2]->BackData((void*)tmpW.data(), 0, sizeof(float) * mWeights.size(), queue, commandPool);
    //compare
    std::string endl("\n");
    std::string space(" ");
    for(uint32_t i = 0; i < mWeights.size(); i++){
        if(mWeights[i] != tmpW[i]){
            std::string log("weight not equal at ");
            log += std::to_string(i);
            log += endl;
            log += (std::string("data : ") + std::to_string(mWeights[i]) + endl);
            log += (std::string("debug : ") + std::to_string(tmpW[i]));
            __android_log_print(ANDROID_LOG_DEBUG, "animation debug", log.c_str(), 0);
        }
    }
}

/*
 * check joint and weight
 */
void AEDrawObjectBaseCollada::CheckJointAndWeight()
{
    for(uint32_t i = 0; i < mInfluenceCountList.size(); i++){
        for(uint32_t j = 0; j < mInfluenceCountList[i].size(); j++) {
            uint32_t influence = mInfluenceCountList[i][j];
            glm::vec3 v4(0.0f);
            uint32_t offset = mJointOffsetList[i][j];
            for (uint32_t k = 0; k < influence; k++) {
                float w = mWeights[offset + k];
                v4 += w * mPositions[i][j];
            }
            if (glm::length(v4 - mVertices[i].pos) > 0.01f) {
                __android_log_print(ANDROID_LOG_DEBUG, "animation weights",
                                    (std::string("postion not equal geometry at geometry") +
                                     std::to_string(i) + std::string(" position ") + std::to_string(j)).c_str(), 0);
            }
        }
    }
}


//=====================================================================
//AE Draw Object GLTF
//=====================================================================
AEDrawObjectBaseGltf::AEDrawObjectBaseGltf(const char* filePath, android_app* app, AELogicalDevice* device, float scale)
{
    mScale = scale;
    using namespace tinygltf;
    AAsset* file = AAssetManager_open(app->activity->assetManager,
                                      filePath, AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(file);
    unsigned char* fileContent = new unsigned char[fileLength];
    AAsset_read(file, fileContent, fileLength);
    std::stringbuf strbuf((char*)fileContent);
    std::basic_istream stream(&strbuf);
    Model model;
    TinyGLTF loader;
    std::string err, warning;
    std::string baseDir(filePath);
    std::vector<std::string> fields;
    AEDrawObject::Split(fields, baseDir, '/');
    baseDir = fields[0];
    bool ret = loader.LoadBinaryFromMemory(&model, &err, &warning, fileContent, fileLength, baseDir);
    ReadNode(model);
    ReadTexture(model);
    ReadMesh(model);
    ReadMaterial(model, device);
    MakeVertices();
    int breakpoint = 0;
}

/*
 * read buffer from accessor id
 */
void AEDrawObjectBaseGltf::ReadBuffer(const tinygltf::Model& model, uint32_t accId, size_t componentSize, void* dstBuf)
{
    const auto& accs = model.accessors[accId];
    const auto& bufview = model.bufferViews[accs.bufferView];
    const auto& buf = model.buffers[bufview.buffer];
    size_t byteoffset = accs.byteOffset + bufview.byteOffset;
    memcpy(dstBuf, &buf.data[byteoffset], accs.count * componentSize);
}

/*
 * translation to mat4
 */
glm::mat4 AEDrawObjectBaseGltf::t2m(const glm::vec3& translate)
{
    glm::mat4 a(1.0f);
    a[3][0] = translate.x;
    a[3][1] = translate.y;
    a[3][2] = translate.z;
    return a;
}

/*
 * scale to mat4
 */
glm::mat4 AEDrawObjectBaseGltf::s2m(const glm::vec3& scale)
{
    glm::mat4 a(1.0f);
    a[0][0] = scale.x;
    a[1][1] = scale.y;
    a[2][2] = scale.z;
    return a;
}

/*
 * rotate to mat4
 * above :
 * bottom : https://automaticaddison.com/how-to-convert-a-quaternion-to-a-rotation-matrix/
 */
glm::mat4 AEDrawObjectBaseGltf::r2m(const glm::vec4& r)
{
    glm::vec4 rotate = glm::normalize(r);
    float qxx = rotate.x * rotate.x;
    float qxy = rotate.x * rotate.y;
    float qxz = rotate.x * rotate.z;
    float qxw = rotate.x * rotate.w;
    float qyy = rotate.y * rotate.y;
    float qyz = rotate.y * rotate.z;
    float qyw = rotate.y * rotate.w;
    float qzz = rotate.z * rotate.z;
    float qzw = rotate.z * rotate.w;
    glm::mat4 a(1.0f);
    a[0][0] = 1.0f - 2.0f * (qyy + qzz);
    a[0][1] = 2.0f * (qxy + qzw);
    a[0][2] = 2.0f * (qxz - qyw);
    a[1][0] = 2.0f * (qxy - qzw);
    a[1][1] = 1.0f - 2.0f * (qxx + qzz);
    a[1][2] = 2.0f * (qyz + qxw);
    a[2][0] = 2.0f * (qxz + qyw);
    a[2][1] = 2.0f * (qyz - qxw);
    a[2][2] = 1.0f - 2.0f * (qxx + qyy);
    return a;
}



/*
 * read node
 */
void AEDrawObjectBaseGltf::ReadNode(const tinygltf::Model& model)
{
    using namespace tinygltf;
    std::vector<Joint> tmpJoint;
    std::vector<glm::mat4> tmpIbms;
    std::vector<std::vector<glm::mat4>> tmpTranslates;
    std::vector<std::vector<glm::mat4>> tmpScales;
    std::vector<std::vector<glm::mat4>> tmpRotations;
    std::vector<std::vector<float>> tmpInputsT;
    std::vector<std::vector<float>> tmpInputsS;
    std::vector<std::vector<float>> tmpInputsR;
    std::vector<std::vector<float>> tmpInputW;
    std::vector<float> tmpMorphWeights;
    std::vector<float> maxFrames;
    //root node
    mRoot = model.scenes[0].nodes[0];
    //node
    uint32_t index = 0;
    for(auto& node : model.nodes){
        Joint j = {};
        j.jointNo = -1;
        j.nodename = node.name;
        j.nodeid = index;
        for(uint32_t i = 0; i < node.children.size(); i++)
            j.children.emplace_back(node.children[i]);
        j.ibm = glm::mat4(1.0f);
        tmpJoint.emplace_back(j);
        index++;
    }
    //joint
    for(auto& skin : model.skins){
        //joint number
        for(uint32_t i = 0; i < tmpJoint.size(); i++){
            for(uint32_t j = 0; j < skin.joints.size(); j++){
                if(tmpJoint[i].nodeid == skin.joints[j]) {
                    tmpJoint[i].jointNo = j;
                }
            }
        }
        //inverse bind matrix
        if(skin.inverseBindMatrices > -1){
            const auto& ibmAcc = model.accessors[skin.inverseBindMatrices];
            const auto& ibmBufView = model.bufferViews[ibmAcc.bufferView];
            const auto& ibmBuf = model.buffers[ibmBufView.buffer];
            for(uint32_t i = 0; i < ibmAcc.count; i++)
                tmpIbms.emplace_back(glm::mat4(1.0f));
            size_t byteOffset = ibmAcc.byteOffset + ibmBufView.byteOffset;
            memcpy(tmpIbms.data(), &ibmBuf.data[byteOffset], ibmAcc.count * sizeof(glm::mat4));
            for(uint32_t i = 0; i < skin.joints.size(); i++)
                tmpJoint[joint2node(i, tmpJoint)].ibm = tmpIbms[i];
        }
    }
    //read animation
    uint32_t jointSize = tmpJoint.size();
    std::vector<glm::mat4> vm;
    std::vector<float> vf;
    for(uint32_t i = 0; i < jointSize; i++){
        tmpTranslates.emplace_back(vm);
        tmpScales.emplace_back(vm);
        tmpRotations.emplace_back(vm);
        tmpInputsR.emplace_back(vf);
        tmpInputsS.emplace_back(vf);
        tmpInputsT.emplace_back(vf);
        tmpInputW.emplace_back(vf);
    }
    for(auto& animation : model.animations){
        for(auto& channel : animation.channels){
            int targetNode = channel.target_node;
            std::string path = channel.target_path;
            int samplerId = channel.sampler;
            const auto& sampler = animation.samplers[samplerId];
            uint32_t inputId = sampler.input;
            std::vector<float> keyFrames(model.accessors[inputId].count);
            ReadBuffer(model, inputId, sizeof(float), keyFrames.data());
            if(maxFrames.size() < keyFrames.size())
                maxFrames = keyFrames;
            uint32_t outputId = sampler.output;
            std::vector<glm::mat4> outputMats(model.accessors[outputId].count);
            if(path == std::string("translation") || path == std::string("scale")){
                std::vector<glm::vec3> outputs(model.accessors[outputId].count);
                ReadBuffer(model, outputId, sizeof(glm::vec3), outputs.data());
                if(path == std::string("translation")){
                    for(uint32_t i = 0; i < outputs.size(); i++)
                        outputMats[i] = t2m(outputs[i]);
                    tmpTranslates[targetNode] = outputMats;
                    tmpInputsT[targetNode] = keyFrames;
                } else if(path == std::string("scale")){
                    for(uint32_t i = 0; i < outputs.size(); i++)
                        outputMats[i] = s2m(outputs[i]);
                    tmpScales[targetNode] = outputMats;
                    tmpInputsS[targetNode] = keyFrames;
                }
            } else if(path == std::string("rotation")) {
                std::vector<glm::vec4> outputs(model.accessors[outputId].count);
                ReadBuffer(model, outputId, sizeof(glm::vec4), outputs.data());
                for(uint32_t i = 0; i < outputs.size(); i++)
                    outputMats[i] = r2m(outputs[i]);
                tmpRotations[targetNode] = outputMats;
                tmpInputsR[targetNode] = keyFrames;
            } else if(std::regex_search(path, std::regex("weight", std::regex::icase))){
                std::vector<float> tmpWeights(model.accessors[outputId].count);
                ReadBuffer(model, outputId, sizeof(float), tmpWeights.data());
                for(uint32_t i = 0; i < tmpWeights.size(); i++)
                    tmpMorphWeights.emplace_back(tmpWeights[i]);
                tmpInputW[targetNode] = keyFrames;
            } else {
            }
        }
    }
    //make TRS matrix
    uint32_t maxKeyframeCount = maxFrames.size();
    mAnimationTime = maxFrames;
    for(uint32_t i = 0; i < tmpJoint.size(); i++){
        glm::mat4 t;
        glm::mat4 r;
        glm::mat4 s;
        for(uint32_t j = 0; j < maxKeyframeCount; j++){
            float keyFrame = maxFrames[j];
            uint32_t keyFrameIndex = 0;
            //translation
            if(hasKeyFrames(keyFrame, tmpInputsT[i], keyFrameIndex)) {
                if (keyFrameIndex >= tmpTranslates[i].size())
                    throw std::runtime_error("index is greater than output size");
                t = tmpTranslates[i][keyFrameIndex];
            }
            else
                t = glm::mat4(1.0f);
            //scale
            if(hasKeyFrames(keyFrame, tmpInputsS[i], keyFrameIndex)) {
                if(keyFrameIndex >= tmpScales[i].size())
                    throw std::runtime_error("index is greater than output size");
                s = tmpScales[i][keyFrameIndex];
            }
            else
                s = glm::mat4(1.0f);
            //rotate
            if(hasKeyFrames(keyFrame, tmpInputsR[i], keyFrameIndex)) {
                if(keyFrameIndex >= tmpRotations[i].size())
                    throw std::runtime_error("index is greater than output size");
                r = tmpRotations[i][keyFrameIndex];
            }
            else
                r = glm::mat4(1.0f);
            //make TRS matrix and store to joint
            tmpJoint[i].keyFrames.emplace_back(keyFrame);
            tmpJoint[i].animationTransform.emplace_back(t * r * s);
            //make morph weight for each frame
            if(tmpMorphWeights.size() > 0){
                uint32_t targets = tmpMorphWeights.size() / tmpInputW[0].size();
                uint32_t offset = targets * j;
                std::vector<float> weights;
                for(uint32_t k = 0; k < targets; k++)
                    weights.emplace_back(tmpMorphWeights[offset + k]);
                tmpJoint[i].morphWeights.emplace_back(weights);
            }
        }
    }
//    //store joint in order
//    for(uint32_t i = 0; i < tmpJoint.size(); i++) {
//        for (uint32_t j = 0; j < tmpJoint.size(); j++) {
//            if (tmpJoint[j].jointNo == i)
//                mJoints.emplace_back(tmpJoint[j]);
//        }
//    }
    //store in order
    for(uint32_t i = 0; i < tmpJoint.size(); i++)
        mJoints.emplace_back(tmpJoint[i]);
}

/*
 * has keyframes in input
 */
bool AEDrawObjectBaseGltf::hasKeyFrames(float keyframe, std::vector<float>const& keyFrames, uint32_t &index)
{
    if(keyFrames.size() < 1){
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", "input size is zero", 0);
        return false;
    }
    for(uint32_t i = 0; i < keyFrames.size() - 1; i++){
        if(keyframe < keyFrames[i + 1]){
            index = i;
            return true;
        }
    }
    index = keyFrames.size() - 1;
    return true;
}


/*
 * read texture
 */
void AEDrawObjectBaseGltf::ReadTexture(const tinygltf::Model &model)
{
    //image
    for(const auto& image : model.images){
        std::string filename = image.name;
        const auto& imageBufView = model.bufferViews[image.bufferView];
        size_t offsetByte = imageBufView.byteOffset;
        const void* imageSrc = &model.buffers[imageBufView.buffer].data[offsetByte];
        GltfTexture texture;
        texture.filename = filename;
        texture.width = image.width;
        texture.height = image.height;
        texture.size = imageBufView.byteLength;
        texture.data = imageSrc;
        mTextures.emplace_back(texture);
    }
    //material
    for(const auto& material : model.materials){
        for(const auto& value : material.values){
            auto name = value.first;
            if(std::regex_search(name, std::regex("basecolor", std::regex::icase))){

            }
        }
    }
}

/*
 * make vertices
 */
void AEDrawObjectBaseGltf::MakeVertices()
{
    //vertices
    Vertex3DObj v = {};
    for(uint32_t i = 0; i < mGeometries.size(); i++) {
        for (uint32_t j = 0; j < mGeometries[i].positions.size(); j++) {
            v.pos = mGeometries[i].positions[j];
            v.texcoord =mGeometries[i].texCoords[j];
            v.normal = mGeometries[i].normals[j];
            mVertices.emplace_back(v);
        }
    }
    //indices
}

/*
 * vertex buffer size
 */
uint32_t AEDrawObjectBaseGltf::GetVertexBufferSize(){return sizeof(Vertex3DObj) * mVertices.size();}

/*
 * read mesh information
 */
void AEDrawObjectBaseGltf::ReadMesh(const tinygltf::Model &model)
{
    using namespace tinygltf;
    std::vector<glm::uvec4> tmpJoint;
    std::vector<glm::vec4> tmpWeight;
    const uint8_t* jointSrc;
    const glm::vec4* weightSrc;
    for(auto& primitive : model.meshes[0].primitives){
        //each primitive
        Geometry geo = {};
        for(auto& attr : primitive.attributes) {
            std::string attName = attr.first;
            //positions
            if (std::regex_search(attName, std::regex("position", std::regex::icase))) {
                const auto &posAccr = model.accessors[attr.second];
                const auto &posBufView = model.bufferViews[posAccr.bufferView];
                size_t offsetByte = posAccr.byteOffset + posBufView.byteOffset;
                const auto *src = reinterpret_cast<const glm::vec3 *>(&(model.buffers[posBufView.buffer].data[offsetByte]));
                size_t vertexSize = posAccr.count;
                for (uint32_t i = 0; i < vertexSize; i++) {
                    //y axis reverse
                    glm::vec3 pos = src[i];
                    pos.y *= -1.0f;
                    geo.positions.emplace_back(pos);
                }
                //indices
                const auto &indexAccr = model.accessors[primitive.indices];
                const auto &indexBufView = model.bufferViews[indexAccr.bufferView];
                size_t indexOffsetByte = indexAccr.byteOffset + indexBufView.byteOffset;
                const auto *indexSrc = reinterpret_cast<const uint16_t *>(&(model.buffers[indexBufView.buffer].data[indexOffsetByte]));
                for (uint32_t i = 0; i < indexAccr.count; i++)
                    geo.indices.emplace_back((uint32_t) indexSrc[i]);
            }
            //normal
            if (std::regex_search(attName, std::regex("normal", std::regex::icase))) {
                const auto& accr = model.accessors[attr.second];
                std::vector<glm::vec3> tmpNormals(accr.count);
                ReadBuffer(model, attr.second, sizeof(glm::vec3), tmpNormals.data());
                for(uint32_t i = 0; i < tmpNormals.size(); i++)
                    geo.normals.emplace_back(tmpNormals[i]);
            }
            //texture coord
            if (std::regex_search(attName, std::regex("texcoord", std::regex::icase))) {
                const auto &tcAccr = model.accessors[attr.second];
                const auto &tcBufView = model.bufferViews[tcAccr.bufferView];
                size_t offsetByte = tcAccr.byteOffset + tcBufView.byteOffset;
                const auto *tcSrc = reinterpret_cast<const glm::vec2 *>(&model.buffers[tcBufView.buffer].data[offsetByte]);
                for (uint32_t i = 0; i < tcAccr.count; i++)
                    geo.texCoords.emplace_back(tcSrc[i]);
            }
            //joint information
            if(std::regex_search(attName, std::regex("joint", std::regex::icase))){
                const auto& jointAccr = model.accessors[attr.second];
                const auto& jointBufView = model.bufferViews[jointAccr.bufferView];
                size_t offsetByte = jointAccr.byteOffset + jointBufView.byteOffset;
                //for case : acc.componentType = TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE
                //TODO : auto select pointer types with void* ?
                jointSrc = reinterpret_cast<const uint8_t*>(&model.buffers[jointBufView.buffer].data[offsetByte]);
                for(uint32_t i = 0; i < jointAccr.count; i++){
                    uint32_t index = i * 4;
                    glm::uvec4 u(jointSrc[index], jointSrc[index + 1], jointSrc[index + 2], jointSrc[index + 3]);
                    tmpJoint.emplace_back(u);
                }
            }
            //weight information
            if(std::regex_search(attName, std::regex("weight", std::regex::icase))){
                const auto& weightAccr = model.accessors[attr.second];
                const auto& weightBufView = model.bufferViews[weightAccr.bufferView];
                size_t offsetByte = weightAccr.byteOffset + weightBufView.byteOffset;
                weightSrc = reinterpret_cast<const glm::vec4*>(&model.buffers[weightBufView.buffer].data[offsetByte]);
                for(uint32_t i = 0; i < weightAccr.count; i++)
                    tmpWeight.emplace_back(weightSrc[i]);
            }
        }
        //morph target
        for(const auto& target : primitive.targets){
            std::vector<Morph> ms;
            if(target.count("NORMAL") > 0){
                Morph m{};
                m.name = "NORMAL";
                InputMorphData(model, m, target.at(m.name.c_str()));
                ms.emplace_back(m);
            }
            if(target.count("POSITION") > 0){
                Morph m{};
                m.name = "POSITION";
                InputMorphData(model, m, target.at(m.name.c_str()));
                ms.emplace_back(m);
            }
            if(target.count("TANGETNT") > 0) {
                Morph m{};
                m.name = "TANGETNT";
                ms.emplace_back(m);
            }
            if(target.count("TEXCOORD_n") > 0){
                Morph m{};
                m.name = "TEXCOORD_n";
                ms.emplace_back(m);
            }
            if(target.count("COLOR_n") > 0){
                Morph m{};
                m.name = "COLOR_n";
                ms.emplace_back(m);
            }
            geo.morphTargets.emplace_back(ms);
        }
        mGeometries.emplace_back(geo);
    }
    //default morph weight
    for(const auto& weight : model.meshes[0].weights){
        mMorphWeight.emplace_back((float)weight);
    }
    //apply joint information
    uint32_t totalInfluences = 0;
    for(uint32_t i = 0; i < mGeometries[0].positions.size(); i++){
        //__android_log_print(ANDROID_LOG_DEBUG, "aqoole gltf", (std::string("vertex = ") + std::to_string(i)).c_str(), 0);
        //detect influence count
        uint32_t ic = 0;
        float total = 0.0f;
        if(tmpWeight.size() > 0) {
            for (uint32_t j = 0; j < 4; j++) {
                total += tmpWeight[i][j];
                if (total > 0.9999f) {
                    ic += j + 1;
                    break;
                }
            }
        }
        if(mGeometries[0].morphTargets.size() > 0){
            if(i < mGeometries[0].morphTargets.size())
                ic += 1;
        }
        mGeometries[0].influences.emplace_back(ic);
        mGeometries[0].jointOffsets.emplace_back(totalInfluences);
        totalInfluences += ic;
        if(tmpWeight.size() > 0) {
            //store vertex index and weight to node
            for (uint32_t k = 0; k < ic; k++) {
                uint32_t jointNum = tmpJoint[i][k];
                uint32_t nodeId = joint2node(jointNum);
                mJointList.emplace_back(nodeId);
                mGeometries[0].weights.emplace_back(weightSrc[i][k]);
            }
        }
    }
    int breakpoint = 1000;
}

/*
 * read material
 */
void AEDrawObjectBaseGltf::ReadMaterial(const tinygltf::Model& model, AELogicalDevice* device)
{
    using namespace tinygltf;
    GltfMaterial gm = {};
    for(const auto& material : model.materials){
        gm.alphaCutoff = (float)material.alphaCutoff;
    }
    //copy data to uniform buffer
    mUniforms.material = std::make_unique<AEBufferUniform>(device, sizeof(GltfMaterial));
    mUniforms.material->CreateBuffer();
    mUniforms.material->CopyData((void*)&gm, sizeof(GltfMaterial));
    int breakpoint = 1000;
}

/*
 * input morph data
 */
void AEDrawObjectBaseGltf::InputMorphData(const tinygltf::Model &model, Morph& m, uint32_t accId)
{
    if(m.name == std::string("NORMAL") || m.name == std::string("POSITION")){
        uint32_t size = model.accessors[accId].count;
        for(uint32_t i = 0; i < size; i++)
            m.data3.emplace_back(glm::vec3(0.0f));
        ReadBuffer(model, accId, sizeof(glm::vec3), m.data3.data());
    }
}

/*
 * joint num to node id
 */
uint32_t AEDrawObjectBaseGltf::joint2node(uint32_t jointNum)
{
    for(uint32_t i = 0; i < mJoints.size(); i++){
        if(mJoints[i].jointNo == jointNum)
            return mJoints[i].nodeid;
    }
    throw std::runtime_error("failed to convert joint to node");
    return mJoints.size();
}

uint32_t AEDrawObjectBaseGltf::joint2node(uint32_t jointNum, std::vector<Joint> const& joints)
{
    for(uint32_t i = 0; i < joints.size(); i++){
        if(joints[i].jointNo == jointNum)
            return joints[i].nodeid;
    }
    return joints.size();
}


/*
 * animation prepare
 */
void AEDrawObjectBaseGltf::AnimationPrepare(android_app* app, AELogicalDevice* device, std::vector<const char*>& shaders,
                                            AEBufferBase* buffer[], AEDeviceQueue* queue, AECommandPool* commandPool, AEDescriptorPool* descriptorPool)
{
    //compute pipeline
    AEDescriptorSetLayout cl(device);
    cl.AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(9, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(10, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(11, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.CreateDescriptorSetLayout();
    mComputePipeline = std::make_unique<AEComputePipeline>(device, shaders, &cl, app);
    //buffers for descriptor set
    VkDeviceSize indicesBufferSize = mVertices.size() * sizeof(uint32_t);
    VkDeviceSize jointsBufferSize = mJointList.size() * sizeof(uint32_t);
    //buffer sizes
    std::vector<VkDeviceSize> positionBufferSizes;
    std::vector<VkDeviceSize> influenceCountSizes;
    std::vector<VkDeviceSize> jointOffsetSizes;
    std::vector<VkDeviceSize> weightBufferSizes;
    std::vector<VkDeviceSize> animationIndexSizes;
    for(uint32_t i = 0; i < mGeometries.size(); i++) {
        GeometryBuffers geoBuf;
        //base position buffer
        VkDeviceSize positionBS = mGeometries[i].positions.size() * sizeof(glm::vec3);
        if(positionBS > 0) {
            positionBufferSizes.emplace_back(mGeometries[i].positions.size() * sizeof(glm::vec3));
            geoBuf.positionBuffer = std::make_unique<AEBufferUtilOnGPU>(device,
                                                                        positionBufferSizes[i],
                                                                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            geoBuf.positionBuffer->CreateBuffer();
            geoBuf.positionBuffer->CopyData((void *) mGeometries[i].positions.data(), 0,
                                            positionBufferSizes[i], queue,
                                            commandPool);
        }
        //influence count buffer
        VkDeviceSize influenceCountListBufferSize =
                mGeometries[i].influences.size() * sizeof(uint32_t);
        if(influenceCountListBufferSize > 0) {
            influenceCountSizes.emplace_back(influenceCountListBufferSize);
            geoBuf.influenceCountBuffer = std::make_unique<AEBufferUtilOnGPU>(device,
                                                                              influenceCountListBufferSize,
                                                                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            geoBuf.influenceCountBuffer->CreateBuffer();
            geoBuf.influenceCountBuffer->CopyData((void *) mGeometries[i].influences.data(), 0,
                                                  influenceCountListBufferSize, queue, commandPool);
        }
        //joint offset buffer
        VkDeviceSize jointOffsetSize = mGeometries[i].jointOffsets.size() * sizeof(uint32_t);
        if(jointOffsetSize > 0) {
            jointOffsetSizes.emplace_back(jointOffsetSize);
            geoBuf.jointOffsetBuffer = std::make_unique<AEBufferUtilOnGPU>(device, jointOffsetSize,
                                                                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            geoBuf.jointOffsetBuffer->CreateBuffer();
            geoBuf.jointOffsetBuffer->CopyData((void *) mGeometries[i].jointOffsets.data(), 0,
                                               jointOffsetSize, queue,
                                               commandPool);
        }
        //weight buffer
        VkDeviceSize weightBufferSize = mGeometries[i].weights.size() * sizeof(float);
        if(weightBufferSize > 0) {
            weightBufferSizes.emplace_back(weightBufferSize);
            geoBuf.weightBuffer = std::make_unique<AEBufferUtilOnGPU>(device, weightBufferSize,
                                                                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            geoBuf.weightBuffer->CreateBuffer();
            geoBuf.weightBuffer->CopyData((void *) mGeometries[i].weights.data(), 0,
                                          weightBufferSize, queue, commandPool);
        }
        //save at member
        mGeoBuffers.emplace_back(std::move(geoBuf));
    }
    //animation result positions
    if (buffer == nullptr) {
        std::unique_ptr<AEBufferUtilOnGPU> positionResultBuffer = std::make_unique<AEBufferUtilOnGPU>(
                device, positionBufferSizes[0], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        positionResultBuffer->CreateBuffer();
        positionResultBuffer->CopyData((void *) mGeometries[0].positions.data(), 0, positionBufferSizes[0], queue,
                                       commandPool);
        mBuffers.emplace_back(std::move(positionResultBuffer));
    } else {
        mBuffers.emplace_back((AEBufferUtilOnGPU*)buffer[0]);
    }
    //joints
    if(jointsBufferSize > 0) {
        std::unique_ptr<AEBufferUtilOnGPU> jointBuffer = std::make_unique<AEBufferUtilOnGPU>(device,
                                                                                             jointsBufferSize,
                                                                                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        jointBuffer->CreateBuffer();
        jointBuffer->CopyData((void *) mJointList.data(), 0, jointsBufferSize, queue, commandPool);
        mBuffers.emplace_back(std::move(jointBuffer));
    }
    //animation mats
    for(uint32_t i = 0; i < mJoints[0].animationTransform.size(); i++){
        mAnimationTransforms.emplace_back(glm::mat4(1.0f));
        mAnimationTransformsNext.emplace_back(glm::mat4(1.0f));
    }
    VkDeviceSize matBufferSize = mAnimationTransforms.size() * sizeof(glm::mat4);
    std::unique_ptr<AEBufferUtilOnGPU> matsBuffer = std::make_unique<AEBufferUtilOnGPU>(device, matBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    matsBuffer->CreateBuffer();
    matsBuffer->CopyData((void*)mAnimationTransforms.data(), 0, matBufferSize, queue, commandPool);
    mBuffers.emplace_back(std::move(matsBuffer));
    //uniform buffer
    VkDeviceSize uniformSize = sizeof(AnimationUniforms);
    std::unique_ptr<AEBufferUniform> uniformBuffer = std::make_unique<AEBufferUniform>(device, uniformSize);
    uniformBuffer->CreateBuffer();
    AnimationUniforms au{};
    au.animNum = 0;
    au.scale = mScale;
    au.time = 0.0f;
    au.vertexSize = mGeometries[0].positions.size();
    uniformBuffer->CopyData((void*)&au, uniformSize);
    mUniforms.animationUniform = std::move(uniformBuffer);
    //debug buffer
    VkDeviceSize debugSize = mVertices.size() * sizeof(uint32_t);
    std::vector<uint32_t> debugData;
    for(uint32_t i = 0; i < mVertices.size(); i++)
        debugData.emplace_back(0);
    std::unique_ptr<AEBufferUtilOnGPU> debugBuffer = std::make_unique<AEBufferUtilOnGPU>(device, debugSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    debugBuffer->CreateBuffer();
    debugBuffer->CopyData((void*)debugData.data(), 0, debugSize, queue, commandPool);
    mBuffers.emplace_back(std::move(debugBuffer));
    //debug buffer for VERTEX3DOBJ
    VkDeviceSize debugVSize = mVertices.size() * sizeof(Vertex3DObj);
    std::vector<Vertex3DObj> tmpVs;
    Vertex3DObj tmpV = {};
    for(uint32_t i = 0; i < mVertices.size(); i++){
        tmpVs.emplace_back(tmpV);
    }
    std::unique_ptr<AEBufferUtilOnGPU> debugVertexBuffer = std::make_unique<AEBufferUtilOnGPU>(device, debugVSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    debugVertexBuffer->CreateBuffer();
    debugVertexBuffer->CopyData((void*)tmpVs.data(), 0, debugVSize, queue, commandPool);
    mBuffers.emplace_back(std::move(debugVertexBuffer));
    //animation next
    std::unique_ptr<AEBufferUtilOnGPU> matsNextBuffer = std::make_unique<AEBufferUtilOnGPU>(device, matBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    matsNextBuffer->CreateBuffer();
    matsNextBuffer->CopyData((void*)mAnimationTransformsNext.data(), 0, matBufferSize, queue, commandPool);
    mBuffers.emplace_back(std::move(matsNextBuffer));
    //animation key frame time
    if(mAnimationTime.size() == 0)
        mAnimationTime.emplace_back(0.0f);
    VkDeviceSize keyFrameSize = sizeof(float) * mAnimationTime.size();
    std::unique_ptr<AEBufferUtilOnGPU> keyFrameBuffer = std::make_unique<AEBufferUtilOnGPU>(device, keyFrameSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    keyFrameBuffer->CreateBuffer();
    keyFrameBuffer->CopyData((void*)mAnimationTime.data(), 0, keyFrameSize, queue, commandPool);
    mBuffers.emplace_back(std::move(keyFrameBuffer));
    //prepare descriptor set
    for(uint32_t i = 0; i < mGeometries.size(); i++) {
        std::unique_ptr<AEDescriptorSet> ds = std::make_unique<AEDescriptorSet>(device, &cl, descriptorPool);
        ds->BindDescriptorBuffer(0, mGeoBuffers[i].positionBuffer->GetBuffer(), positionBufferSizes[i],
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        if (buffer == nullptr) {
            ds->BindDescriptorBuffer(1, mBuffers[0]->GetBuffer(), positionBufferSizes[0],
                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        } else {
            ds->BindDescriptorBuffer(1, mBuffers[0]->GetBuffer(),
                                     sizeof(Vertex3DObj) * mVertices.size(),
                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        }
        ds->BindDescriptorBuffer(2, mGeoBuffers[i].influenceCountBuffer->GetBuffer(), influenceCountSizes[i],
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(3, mBuffers[1]->GetBuffer(), jointsBufferSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(4, mGeoBuffers[i].weightBuffer->GetBuffer(), weightBufferSizes[i],
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(5, mBuffers[2]->GetBuffer(), matBufferSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(6, mUniforms.animationUniform->GetBuffer(), uniformSize,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        ds->BindDescriptorBuffer(7, mBuffers[3]->GetBuffer(), debugSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(8, mBuffers[4]->GetBuffer(), debugVSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(9, mBuffers[5]->GetBuffer(), matBufferSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(10, mGeoBuffers[i].jointOffsetBuffer->GetBuffer(), jointOffsetSizes[i],
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(11, mBuffers[6]->GetBuffer(), keyFrameSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        mDSs.emplace_back(std::move(ds));
    }
    int breakpoint = 10000;
}

/*
 * prepare animaton matrix
 */
void AEDrawObjectBaseGltf::PrepareAnimationMatrices(AEDrawObjectBaseGltf::Joint& joint, glm::mat4 parentBindPoseMatrix, glm::mat4 parentAnimationMatrix, uint32_t keyframe,
                                                     std::vector<glm::mat4> &targetTransform)
{
    //create joint and weight buffer
    if(joint.animationTransform.size() > 0) {
        //compute matrices and put uniform buffer
        parentAnimationMatrix = parentAnimationMatrix * joint.animationTransform[keyframe];
        parentBindPoseMatrix = joint.ibm * parentBindPoseMatrix;
        glm::mat4 finalTransform = parentAnimationMatrix * joint.ibm;
        targetTransform[joint.nodeid] = finalTransform;
    }
    //continue to child
    if(joint.children.size() > 0) {
        for(uint32_t i = 0; i < joint.children.size(); i++)
            PrepareAnimationMatrices(mJoints[joint.children[i]], parentBindPoseMatrix, parentAnimationMatrix, keyframe, targetTransform);
    }
}

/*
 * make animation
 */
void AEDrawObjectBaseGltf::MakeAnimation()
{

}

/*
 * animation prepare morph
 */
void AEDrawObjectBaseGltf::AnimationPrepareMorph(android_app *app, AELogicalDevice *device,
                                                 std::vector<const char *> &shaders,
                                                 AEBufferBase **buffer, AEDeviceQueue *queue,
                                                 AECommandPool *commandPool,
                                                 AEDescriptorPool *descriptorPool)
{
    //compute pipeline
    AEDescriptorSetLayout cl(device);
    cl.AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.CreateDescriptorSetLayout();
    mComputePipelineMorph = std::make_unique<AEComputePipeline>(device, shaders, &cl, app);
    //buffer sizes
    std::vector<VkDeviceSize> positionBufferSizes;
    std::vector<VkDeviceSize> animationIndexSizes;
    VkDeviceSize targetSize;
    VkDeviceSize weightSize;
    for(uint32_t i = 0; i < mGeometries.size(); i++) {
        GeometryBuffers geoBuf;
        //base position buffer
        VkDeviceSize positionBS = mGeometries[i].positions.size() * sizeof(glm::vec3);
        if(positionBS > 0) {
            positionBufferSizes.emplace_back(mGeometries[i].positions.size() * sizeof(glm::vec3));
            geoBuf.positionBuffer = std::make_unique<AEBufferUtilOnGPU>(device,
                                                                        positionBufferSizes[i],
                                                                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            geoBuf.positionBuffer->CreateBuffer();
            geoBuf.positionBuffer->CopyData((void *) mGeometries[i].positions.data(), 0,
                                            positionBufferSizes[i], queue,
                                            commandPool);
        }
        //morph target
        VkDeviceSize onetargetSize = mGeometries[i].morphTargets[0][1].data3.size() * sizeof(glm::vec3);
        targetSize = mGeometries[i].morphTargets.size() * onetargetSize;
        geoBuf.morphTargetsBuffer = std::make_unique<AEBufferUtilOnGPU>(device, targetSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        geoBuf.morphTargetsBuffer->CreateBuffer();
        for(uint32_t j = 0; j < mGeometries[i].morphTargets.size(); j++){
            //choose position index
            uint32_t positionIndex = 0;
            for(uint32_t k = 0; k < mGeometries[i].morphTargets[j].size(); k++){
                if(mGeometries[i].morphTargets[j][k].name == std::string("POSITION")) {
                    positionIndex = k;
                    break;
                }
            }
            geoBuf.morphTargetsBuffer->CopyData((void*)mGeometries[i].morphTargets[j][positionIndex].data3.data(),
                                                onetargetSize * j, onetargetSize, queue, commandPool);
        }
        //morph weight
        VkDeviceSize oneWeightSize = mJoints[mMorphNode].morphWeights[0].size() * sizeof(float);
        weightSize = mJoints[mMorphNode].morphWeights.size() * oneWeightSize;
        geoBuf.morphWeightsBuffer = std::make_unique<AEBufferUtilOnGPU>(device, weightSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        geoBuf.morphWeightsBuffer->CreateBuffer();
        for(uint32_t j = 0; j < mJoints[mMorphNode].morphWeights.size(); j++){
            geoBuf.morphWeightsBuffer->CopyData((void*)mJoints[mMorphNode].morphWeights[j].data(), j * oneWeightSize, oneWeightSize,
                                                queue, commandPool);
        }
        //save at member
        mGeoBuffers.emplace_back(std::move(geoBuf));
    }
    //animation result positions
    if (buffer == nullptr) {
        std::unique_ptr<AEBufferUtilOnGPU> positionResultBuffer = std::make_unique<AEBufferUtilOnGPU>(
                device, positionBufferSizes[0], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        positionResultBuffer->CreateBuffer();
        positionResultBuffer->CopyData((void *) mGeometries[0].positions.data(), 0, positionBufferSizes[0], queue,
                                       commandPool);
        mBuffers.emplace_back(std::move(positionResultBuffer));
    } else {
        mBuffers.emplace_back((AEBufferUtilOnGPU*)buffer[0]);
    }
    //uniform buffer
    VkDeviceSize uniformSize = sizeof(AnimationUniforms);
    std::unique_ptr<AEBufferUniform> uniformBuffer = std::make_unique<AEBufferUniform>(device, uniformSize);
    uniformBuffer->CreateBuffer();
    AnimationUniforms au{};
    au.animNum = 0;
    au.scale = mScale;
    au.time = 0.0f;
    au.vertexSize = mGeometries[0].positions.size();
    au.morphTargetSize = mGeometries[0].morphTargets[0][1].data3.size();
    au.morphTargetPositionSize = mGeometries[0].morphTargets[0].size();
    uniformBuffer->CopyData((void*)&au, uniformSize);
    mUniforms.animationUniform = std::move(uniformBuffer);
    //debug buffer
    VkDeviceSize debugSize = mVertices.size() * sizeof(uint32_t);
    std::vector<uint32_t> debugData;
    for(uint32_t i = 0; i < mVertices.size(); i++)
        debugData.emplace_back(0);
    std::unique_ptr<AEBufferUtilOnGPU> debugBuffer = std::make_unique<AEBufferUtilOnGPU>(device, debugSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    debugBuffer->CreateBuffer();
    debugBuffer->CopyData((void*)debugData.data(), 0, debugSize, queue, commandPool);
    mBuffers.emplace_back(std::move(debugBuffer));
    //debug buffer for VERTEX3DOBJ
    VkDeviceSize debugVSize = mVertices.size() * sizeof(Vertex3DObj);
    std::vector<Vertex3DObj> tmpVs;
    Vertex3DObj tmpV = {};
    for(uint32_t i = 0; i < mVertices.size(); i++){
        tmpVs.emplace_back(tmpV);
    }
    std::unique_ptr<AEBufferUtilOnGPU> debugVertexBuffer = std::make_unique<AEBufferUtilOnGPU>(device, debugVSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    debugVertexBuffer->CreateBuffer();
    debugVertexBuffer->CopyData((void*)tmpVs.data(), 0, debugVSize, queue, commandPool);
    mBuffers.emplace_back(std::move(debugVertexBuffer));
    //animation key frame time
    if(mAnimationTime.size() == 0)
        mAnimationTime.emplace_back(0.0f);
    VkDeviceSize keyFrameSize = sizeof(float) * mAnimationTime.size();
    std::unique_ptr<AEBufferUtilOnGPU> keyFrameBuffer = std::make_unique<AEBufferUtilOnGPU>(device, keyFrameSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    keyFrameBuffer->CreateBuffer();
    keyFrameBuffer->CopyData((void*)mAnimationTime.data(), 0, keyFrameSize, queue, commandPool);
    mBuffers.emplace_back(std::move(keyFrameBuffer));
    //prepare descriptor set
    for(uint32_t i = 0; i < mGeometries.size(); i++) {
        std::unique_ptr<AEDescriptorSet> ds = std::make_unique<AEDescriptorSet>(device, &cl, descriptorPool);
        ds->BindDescriptorBuffer(0, mGeoBuffers[i].positionBuffer->GetBuffer(), positionBufferSizes[i],
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        if (buffer == nullptr) {
            ds->BindDescriptorBuffer(1, mBuffers[0]->GetBuffer(), positionBufferSizes[0],
                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        } else {
            ds->BindDescriptorBuffer(1, mBuffers[0]->GetBuffer(),
                                     sizeof(Vertex3DObj) * mVertices.size(),
                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        }
        ds->BindDescriptorBuffer(2, mGeoBuffers[i].morphTargetsBuffer->GetBuffer(), targetSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(3, mGeoBuffers[i].morphWeightsBuffer->GetBuffer(), weightSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(4, mUniforms.animationUniform->GetBuffer(), uniformSize,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        ds->BindDescriptorBuffer(5, mBuffers[1]->GetBuffer(), debugSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(6, mBuffers[2]->GetBuffer(), debugVSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(7, mBuffers[3]->GetBuffer(), keyFrameSize,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        mDSs.emplace_back(std::move(ds));
    }
    DebugBuffer(queue, commandPool);
    int breakpoint = 10000;
}

/*
 * animation dispatch
 */
void AEDrawObjectBaseGltf::AnimationDispatch(AELogicalDevice* device, AECommandBuffer* command, AEDeviceQueue* queue, AECommandPool* commandPool,
                                                uint32_t animationNum, AEFence* fence, VkSemaphore *waitSemaphore, VkSemaphore* signalSemaphore,
                                                double time, AEEvent* event)
{
    //command record for each joint
    AECommand::BeginCommand(command);
    //prepare event
    vkCmdResetEvent(*command->GetCommandBuffer(), *event->GetEvent(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    AECommand::BindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline.get());
    //update animation transforms
    PrepareAnimationMatrices(mJoints[mRoot], glm::mat4(1.0f), glm::mat4(1.0f), animationNum, mAnimationTransforms);
    PrepareAnimationMatrices(mJoints[mRoot], glm::mat4(1.0f), glm::mat4(1.0f), (animationNum + 1) % mAnimationTime.size(), mAnimationTransformsNext);
    //update buffer
    VkDeviceSize matBufferSize = mAnimationTransforms.size() * sizeof(glm::mat4);
    mBuffers[2]->CopyData((void*)mAnimationTransforms.data(), 0, matBufferSize, queue, commandPool);
    mBuffers[5]->CopyData((void*)mAnimationTransformsNext.data(), 0, matBufferSize, queue, commandPool);
    //uniform buffer
    AnimationUniforms au{};
    au.time = (float)time;
    au.scale = mScale;
    au.animNum = animationNum;
    au.keyFramesSize = mAnimationTime.size();
    for(uint32_t i = 0; i < mGeometries.size(); i++) {
        au.vertexSize = mGeometries[i].positions.size();
        mUniforms.animationUniform->CopyData((void*)&au, sizeof(AnimationUniforms));
        AECommand::BindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE,
                                      mComputePipeline->GetPipelineLayout(),
                                      1, mDSs[i]->GetDescriptorSet());
        //dispatch
        //each work groups
        uint32_t groups = (mGeometries[0].positions.size() / 1024) + 1;
        vkCmdDispatch(*command->GetCommandBuffer(), groups, 1, 1);
    }
    vkCmdSetEvent(*command->GetCommandBuffer(), *event->GetEvent(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    //each local thread
    //vkCmdDispatch(*command->GetCommandBuffer(), 740, 1, 1);
    AECommand::EndCommand(command);
    //submit
    VkSubmitInfo submit_info = {};
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    if(waitSemaphore != nullptr) {
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphore,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 0,
                .pSignalSemaphores = nullptr};
    } else if(signalSemaphore != nullptr){
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphore,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signalSemaphore};
    }else{
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 0,
                .pWaitSemaphores = nullptr,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 0,
                .pSignalSemaphores = nullptr};
    }
    if(fence != nullptr) {
        vkQueueSubmit(queue->GetQueue(0), 1, &submit_info, *fence->GetFence());
    }
    else{
        vkQueueSubmit(queue->GetQueue(0), 1, &submit_info, VK_NULL_HANDLE);
    }
}

/*
 * animation dispatch morph
 */
void AEDrawObjectBaseGltf::AnimationDispatchMorph(AELogicalDevice* device, AECommandBuffer* command, AEDeviceQueue* queue, AECommandPool* commandPool,
                                             uint32_t animationNum, AEFence* fence, VkSemaphore *waitSemaphore, VkSemaphore* signalSemaphore,
                                             double time, AEEvent* event)
{
    //command record for each joint
    AECommand::BeginCommand(command);
    //prepare event
    vkCmdResetEvent(*command->GetCommandBuffer(), *event->GetEvent(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    AECommand::BindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipelineMorph.get());
    //uniform buffer
    AnimationUniforms au{};
    au.time = (float)time;
    au.scale = mScale;
    au.animNum = animationNum;
    au.keyFramesSize = mAnimationTime.size();
    au.morphTargetSize = mGeometries[0].morphTargets.size();
    au.morphTargetPositionSize = mGeometries[0].morphTargets[0][1].data3.size();
    uint32_t threads = mGeometries[0].positions.size();
    for(uint32_t i = 0; i < mGeometries.size(); i++) {
        au.vertexSize = threads;
        mUniforms.animationUniform->CopyData((void*)&au, sizeof(AnimationUniforms));
        AECommand::BindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE,
                                      mComputePipelineMorph->GetPipelineLayout(),
                                      1, mDSs[i]->GetDescriptorSet());
        //dispatch
        //each work groups
        uint32_t groups = (threads / 1024) + 1;
        vkCmdDispatch(*command->GetCommandBuffer(), groups, 1, 1);
    }
    vkCmdSetEvent(*command->GetCommandBuffer(), *event->GetEvent(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    //each local thread
    //vkCmdDispatch(*command->GetCommandBuffer(), 740, 1, 1);
    AECommand::EndCommand(command);
    //submit
    VkSubmitInfo submit_info = {};
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    if(waitSemaphore != nullptr) {
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphore,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 0,
                .pSignalSemaphores = nullptr};
    } else if(signalSemaphore != nullptr){
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphore,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signalSemaphore};
    }else{
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 0,
                .pWaitSemaphores = nullptr,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 0,
                .pSignalSemaphores = nullptr};
    }
    if(fence != nullptr) {
        vkQueueSubmit(queue->GetQueue(0), 1, &submit_info, *fence->GetFence());
    }
    else{
        vkQueueSubmit(queue->GetQueue(0), 1, &submit_info, VK_NULL_HANDLE);
    }
}

/*
 * output position for debug
 */
void AEDrawObjectBaseGltf::OutputPosition(uint32_t frameNum, AEBufferUtilOnGPU* buffer, AEDeviceQueue* const queue, AECommandPool* const commandpool)
{
    std::string space(" ");
    std::string lb("\n");
    std::vector<Vertex3DObj> data(mVertices.size());
    float invScale = 1.0f / mScale;
    buffer->BackData((void*)data.data(), 0, GetVertexBufferSize(), queue, commandpool);
    __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", (std::string("frame = ") + std::to_string(frameNum) + lb).c_str(), 0);
    for(uint32_t i = 0; i < mVertices.size(); i++){
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", (std::to_string(i) + space + std::to_string(data[i].pos.x * invScale) + space +
        std::to_string(data[i].pos.y * invScale) + space + std::to_string(data[i].pos.z * invScale) + lb).c_str(), 0);
    }
    DebugBuffer(queue, commandpool);
}

/*
 * debug buffer
 */
void AEDrawObjectBaseGltf::DebugBuffer(AEDeviceQueue* const queue, AECommandPool* const commandpool)
{
    //target
    uint32_t toneSize = mGeometries[0].morphTargets[0][1].data3.size();
    uint32_t tsize = toneSize * mGeometries[0].morphTargets.size();
    std::vector<glm::vec3> targets(tsize);
    mGeoBuffers[0].morphTargetsBuffer->BackData((void*)targets.data(), 0, tsize * sizeof(glm::vec3), queue, commandpool);
    //check
    for(uint32_t i = 0; i < mGeometries[0].morphTargets.size(); i++){
        for(uint32_t j = 0; j < toneSize; j++){
            if(mGeometries[0].morphTargets[i][1].data3[j] != targets[toneSize * i + j]){
                __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", "detect conflict %c", 0);
            }
        }
    }
    //weight
    uint32_t onewSize = mJoints[mMorphNode].morphWeights[0].size();
    uint32_t wsize = mJoints[mMorphNode].morphWeights.size() * onewSize;
    std::vector<float> weights(wsize);
    mGeoBuffers[0].morphWeightsBuffer->BackData((void*)weights.data(), 0, wsize * sizeof(float), queue, commandpool);
    //check
    for(uint32_t i = 0; i < mJoints[mMorphNode].morphWeights.size(); i++){
        for(uint32_t j = 0; j < onewSize; j++){
            if(mJoints[mMorphNode].morphWeights[i][j] != weights[onewSize * i + j]){
                __android_log_print(ANDROID_LOG_DEBUG, "aqoole animation", "detect weight conflict %c", 0);
            }
        }
    }
    //vertex index
    std::vector<uint32_t> indices(mVertices.size());
    mBuffers[1]->BackData((void*)indices.data(), 0, mVertices.size() * sizeof(uint32_t), queue, commandpool);
    for(uint32_t i = 0; i < mVertices.size(); i++){
        if(i != indices[i])
            int breakpoint = 9999999;
    }
}

//=====================================================================
//AE cube
//=====================================================================
/*
constructor
*/
AECube::AECube(float len, glm::vec3 const& min, glm::vec3 const& color)
    : AEDrawObjectBase3D()
{
    Vertex3D v3d;
    v3d.color = color;
    mLen = len;
    mColor = color;
    float half = mLen / 2.0f;
    glm::vec3 center = glm::vec3(min.x + half, min.y + half, min.z + half);
    //vertex
    //upper
    v3d.normal = glm::normalize(min - center);
    AddVertex(min, v3d.color, v3d);
    v3d.normal = glm::normalize(glm::vec3(min.x + len, min.y, min.z) - center);
    AddVertex(glm::vec3(min.x + len, min.y, min.z), v3d.color, v3d);
    v3d.normal = glm::normalize(glm::vec3(min.x + len, min.y, min.z + len) - center);
    AddVertex(glm::vec3(min.x + len, min.y, min.z + len), v3d.color, v3d);
    v3d.normal = glm::normalize(glm::vec3(min.x, min.y, min.z + len) - center);
    AddVertex(glm::vec3(min.x, min.y, min.z + len), v3d.color, v3d);
    //floor
    v3d.normal = glm::normalize(glm::vec3(min.x, min.y + len, min.z) - center);
    AddVertex(glm::vec3(min.x, min.y + len, min.z), v3d.color, v3d);
    v3d.normal = glm::normalize(glm::vec3(min.x + len, min.y + len, min.z) - center);
    AddVertex(glm::vec3(min.x + len, min.y + len, min.z), v3d.color, v3d);
    v3d.normal = glm::normalize(glm::vec3(min.x + len, min.y + len, min.z + len) - center);
    AddVertex(glm::vec3(min.x + len, min.y + len, min.z + len), v3d.color, v3d);
    v3d.normal = glm::normalize(glm::vec3(min.x, min.y + len, min.z + len) - center);
    AddVertex(glm::vec3(min.x, min.y + len, min.z + len), v3d.color, v3d);
    //indices
    AddIndex(0, 4, 5, 1);
    AddIndex(1, 5, 6, 2);
    AddIndex(2, 6, 7, 3);
    AddIndex(3, 7, 4, 0);
    AddIndex(3, 0, 1, 2);
    AddIndex(4, 7, 6, 5);
    //normals
    CalcNormal();
}

/*
destructor
*/
AECube::~AECube()
{

}

/*
 * update
 */
void AECube::Update(glm::vec3 min)
{
    Vertex3D v3d;
    float half = mLen / 2.0f;
    glm::vec3 center = glm::vec3(min.x + half, min.y + half, min.z + half);
    //vertex
    //upper
    v3d.normal = glm::normalize(min - center);
    v3d.pos = min;
    v3d.color = mColor;
    mVertices[0] = v3d;
    v3d.pos = glm::vec3(min.x + mLen, min.y, min.z);
    v3d.normal = glm::normalize(glm::vec3(min.x + mLen, min.y, min.z) - center);
    mVertices[1] = v3d;
    v3d.pos = glm::vec3(min.x + mLen, min.y, min.z + mLen);
    v3d.normal = glm::normalize(v3d.pos - center);
    mVertices[2] = v3d;
    v3d.pos = glm::vec3(min.x, min.y, min.z + mLen);
    v3d.normal = glm::normalize(v3d.pos - center);
    mVertices[3] = v3d;
    //floor
    v3d.pos = glm::vec3(min.x, min.y + mLen, min.z);
    v3d.normal = glm::normalize(v3d.pos - center);
    mVertices[4] = v3d;
    v3d.pos = glm::vec3(min.x + mLen, min.y + mLen, min.z);
    v3d.normal = glm::normalize(v3d.pos - center);
    mVertices[5] = v3d;
    v3d.pos = glm::vec3(min.x + mLen, min.y + mLen, min.z + mLen);
    v3d.normal = glm::normalize(v3d.pos - center);
    mVertices[6] = v3d;
    v3d.pos = glm::vec3(min.x, min.y + mLen, min.z + mLen);
    v3d.normal = glm::normalize(v3d.pos - center);
    mVertices[7] = v3d;
}

//=====================================================================
//AE pyramid
//=====================================================================
/*
constructor
*/
AEPyramid::AEPyramid(glm::vec3 const& min, float len)
    : AEDrawObjectBase3D()
{
    Vertex3D v3d;
    glm::vec3 color(1.0f, 0.0f, 0.0f);
    v3d.color = glm::vec3(1.0f, 0.0f, 0.0f);
    float halfSqurt = len / sqrtf(2.0f);
    float half = len / 2.0f;
    //vertices
    AddVertex(glm::vec3(min.x + half, min.y - halfSqurt, min.z + half), 
        glm::vec3(glm::smoothstep(0.0f, len, half), 1.0f, glm::smoothstep(0.0f, len, half)), v3d);
    AddVertex(min, glm::vec3(1.0f, 0.0f, 0.0f), v3d);
    AddVertex(glm::vec3(min.x + len, min.y, min.z), glm::vec3(0.0f, 0.0f, 1.0f), v3d);
    AddVertex(glm::vec3(min.x + len, min.y, min.z + len), glm::vec3(0.5f, 0.5f, 0.5f), v3d);
    AddVertex(glm::vec3(min.x, min.y, min.z + len), glm::vec3(1.0f, 0.0f, 0.5f), v3d);
    //indices
    AddIndex(0, 1, 2);
    AddIndex(0, 2, 3);
    AddIndex(0, 3, 4);
    AddIndex(0, 4, 1);
    AddIndex(4, 1, 2);
    AddIndex(2, 3, 4);
}

/*
destructor
*/
AEPyramid::~AEPyramid()
{

}

//=====================================================================
//AE cube texture
//=====================================================================
/*
constructor
*/
AECubeTexture::AECubeTexture(glm::vec3 const& min, float len)
{
    Vertex3DTexture v3d;
    v3d.color = glm::vec3(1.0f, 1.0f, 1.0f);
    float half = len / 2.0f;
    glm::vec3 center = glm::vec3(min.x + half, min.y + half, min.z + half);
    //vertex
    //upper
    AddVertex(min, v3d.color, glm::normalize(min - center), glm::vec2(1.0f, 0.0f), v3d);
    AddVertex(glm::vec3(min.x + len, min.y, min.z), v3d.color,
        glm::normalize(glm::vec3(min.x + len, min.y, min.z) - center), glm::vec2(0.0f, 0.0f), v3d);
    AddVertex(glm::vec3(min.x + len, min.y, min.z + len), v3d.color,
        glm::normalize(glm::vec3(min.x + len, min.y, min.z + len) - center), glm::vec2(0.0f, 1.0f), v3d);
    AddVertex(glm::vec3(min.x, min.y, min.z + len), v3d.color,
        glm::normalize(glm::vec3(min.x, min.y, min.z + len) - center), glm::vec2(1.0f, 1.0f), v3d);
    //floor
    AddVertex(glm::vec3(min.x, min.y + len, min.z), v3d.color,
        glm::normalize(glm::vec3(min.x, min.y + len, min.z) - center), glm::vec2(1.0f, 1.0f), v3d);
    AddVertex(glm::vec3(min.x + len, min.y + len, min.z), v3d.color,
        glm::normalize(glm::vec3(min.x + len, min.y + len, min.z) - center), glm::vec2(0.0f, 1.0f), v3d);
    AddVertex(glm::vec3(min.x + len, min.y + len, min.z + len), v3d.color,
        glm::normalize(glm::vec3(min.x + len, min.y + len, min.z + len) - center), glm::vec2(1.0f, 1.0f), v3d);
    AddVertex(glm::vec3(min.x, min.y + len, min.z + len), v3d.color,
        glm::normalize(glm::vec3(min.x, min.y + len, min.z + len) - center), glm::vec2(0.0f, 0.0f), v3d);
    //indices
    AddIndex(0, 4, 5, 1);
    AddIndex(1, 5, 6, 2);
    AddIndex(2, 6, 7, 3);
    AddIndex(3, 7, 4, 0);
    AddIndex(3, 0, 1, 2);
    AddIndex(7, 4, 5, 6);
}

/*
destructor
*/
AECubeTexture::~AECubeTexture()
{

}

//=====================================================================
//AE plane
//=====================================================================
/*
constructor
*/
AEPlane::AEPlane(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 color)
    : AEDrawObjectBase3D()
{
    glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v3 - v0));
    AddVertex(v0, color + glm::vec3(0.0f, 0.0f, 0.0f), normal);
    AddVertex(v1, color + glm::vec3(0.0f, 1.0f, 0.0f), normal);
    AddVertex(v2, color + glm::vec3(0.0f, 0.0f, 0.0f), normal);
    AddVertex(v3, color + glm::vec3(1.0f, 0.0f, 0.0f), normal);
    //index
    AddIndex(0, 1, 2, 3);
    //CalcNormal();
}

/*
destructor
*/
AEPlane::~AEPlane()
{

}

/*
set element
*/
void AEPlane::SetElement(float x, float y, float z, Vertex3D &oneVertex)
{
    oneVertex.pos.z = x;
    oneVertex.pos.b = y;
    oneVertex.pos.p = z;
}

//=====================================================================
//AE sphere
//=====================================================================
/*
constructor
*/
AESphere::AESphere(glm::vec3 origin, float radius)
    : AEDrawObjectBase3D()
{

}

/*
destructor
*/
AESphere::~AESphere()
{

}

/*
calc vertex
*/
void AESphere::CalcVertex(Vertex3D& v, float r, float theta, float phi)
{
    float st = sin(theta);
    float sp = sin(phi);
    float ct = cos(theta);
    float cp = cos(phi);
    v.pos.x = r * st * sp;
    v.pos.y = r * ct;
    v.pos.z = r * st * cp;
}

//=====================================================================
//AE water surface
//=====================================================================
/*
constructor
-x ...... +x
-z
: 
: |
: v
z

*/
AEWaterSurface::AEWaterSurface(float seaBase, float leftX, float rightX, float topZ,
                               float bottomZ, glm::vec3 color, float poolbottom, bool surfaceOnly, float inLength)
    : AEDrawObjectBase3D(), mRand(16, 24)
{
    //initialize
    mTop = topZ;
    mRight = rightX;
    mSeaBase = seaBase;
    mSpeed = 2.2f;
    mFreq = 6.0f;
    mAmp = 2.0f;
    mDz = 1.0f;
    float length = inLength;
    //float length = 0.5;
    float y = mSeaBase;
    uint32_t i = 0;
    uint32_t j = 0;
    const float LEFT = leftX;
    const float RIGHT = rightX;
    const float TOP = topZ;
    const float BOTTOM = bottomZ;
    glm::vec3 glassColor(0.1f);
    uint32_t leftTopIndex = 0;
    uint32_t leftBottomIndex = 1;
    uint32_t rightBottomIndex = 2;
    uint32_t rightTopIndex = 3;
    std::vector<uint32_t> rightLane;
    for(float left = LEFT; left < RIGHT; left = left + length)
    {
        uint32_t colIndex = 0;
        for(float top = TOP; top > BOTTOM; top = top - length)
        {
            glm::vec3 leftTop(left, y, top);
            glm::vec3 leftBottom(left, y, top - length);
            glm::vec3 rightBottom(left + length, y, top - length);
            glm::vec3 rightTop(left + length, y, top);
            glm::vec3 normal = glm::normalize(glm::cross(leftBottom - leftTop, rightTop - leftTop));
            if(top == TOP && left == LEFT)
            {
                AddVertex(leftTop, color, normal);
                AddVertex(leftBottom, color, normal);
                AddVertex(rightBottom, color, normal);
                AddVertex(rightTop, color, normal);
                leftTopIndex = i;
                leftBottomIndex = i + 1;
                rightBottomIndex = i + 2;
                rightTopIndex = i + 3;
                j = 4;
                rightLane.push_back(rightTopIndex);
                rightLane.push_back(rightBottomIndex);
            }
            else if(left == LEFT)
            {
                AddVertex(leftBottom, color, normal);
                AddVertex(rightBottom, color, normal);
                //use cached index
                leftTopIndex = leftBottomIndex;
                rightTopIndex = rightBottomIndex;
                leftBottomIndex = i;
                rightBottomIndex = i + 1;
                j = 2;
                rightLane.push_back(rightBottomIndex);
            }
            else if(top == TOP)
            {
                AddVertex(rightBottom, color, normal);
                AddVertex(rightTop, color, normal);
                leftTopIndex = rightLane[0];
                leftBottomIndex = rightLane[1];
                rightBottomIndex = i;
                rightTopIndex = i + 1;
                //rightLane.erase(rightLane.begin(), rightLane.begin() + 1);
                j = 2;
                rightLane[0] = rightTopIndex;
                rightLane[1] = rightBottomIndex;
                colIndex++;
            }
            else
            {
                AddVertex(rightBottom, color, normal);
                leftTopIndex = leftBottomIndex;
                leftBottomIndex = rightLane[colIndex + 1];
                rightTopIndex = rightBottomIndex;
                rightBottomIndex = i;
                j = 1;
                rightLane[colIndex + 1] = rightBottomIndex;
                colIndex++;
            }
            AddIndex(leftTopIndex, leftBottomIndex, rightBottomIndex, rightTopIndex);
            //index
            // uint32_t leftTopIndex = FindExistVertexMultiThreads(i, leftTop);
            // if(leftTopIndex != i)
            //     j--;
            // else
            //     AddVertex(leftTop, color, normal);
            // uint32_t leftBottomIndex = FindExistVertexMultiThreads(i + 1, leftBottom);
            // if(leftBottomIndex != i + 1)
            //     j--;
            // else
            //     AddVertex(leftBottom, color, normal);
            // uint32_t rightBottomIndex = FindExistVertexMultiThreads(i + 2, rightBottom);
            // if(rightBottomIndex != i + 2)
            //     j--;
            // else
            //     AddVertex(rightBottom, color, normal);
            // uint32_t rightTopIndex = FindExistVertexMultiThreads(i + 3, rightTop);
            // if(rightTopIndex != i + 3)
            //     j--;
            // else
            //     AddVertex(rightTop, color, normal);
            if(!surfaceOnly) {
                if (top == TOP) {
                    glm::vec3 sideTopRight(left, poolbottom, top);
                    glm::vec3 sideTopLeft(left + length, poolbottom, top);
                    AddVertex(sideTopRight, glassColor, glm::vec3(0.0, 0.0, -1.0f));
                    AddVertex(sideTopLeft, glassColor, glm::vec3(0.0, 0.0, -1.0f));
                    //side left, top right, top left, side right
                    AddIndex(i + j + 1, rightTopIndex, leftTopIndex, i + j);
                    j += 2;
                }
                if (left == LEFT) {
                    glm::vec3 sideBottomRight(left, poolbottom, top - length);
                    glm::vec3 sideBottomLeft(left, poolbottom, top);
                    AddVertex(sideBottomRight, glassColor, glm::vec3(-1.0f, 0.0, 0.0));
                    AddVertex(sideBottomLeft, glassColor, glm::vec3(-1.0f, 0.0, 0.0));
                    //side left, top left, left bottom, side right
                    AddIndex(i + j + 1, leftTopIndex, leftBottomIndex, i + j);
                    j += 2;
                }
                if (top - length <= BOTTOM) {
                    glm::vec3 sideBottomRight(left + length, poolbottom, top - length);
                    glm::vec3 sideBottomLeft(left, poolbottom, top - length);
                    AddVertex(sideBottomRight, glassColor, glm::vec3(0.0, 0.0, -1.0f));
                    AddVertex(sideBottomLeft, glassColor, glm::vec3(0.0, 0.0, -1.0f));
                    //side left, left bottom, right bottom, side right
                    AddIndex(i + j + 1, leftBottomIndex, rightBottomIndex, i + j);
                    j += 2;
                }
                if (left + length >= RIGHT) {
                    glm::vec3 sideBottomRight(left + length, poolbottom, top);
                    glm::vec3 sideBottomLeft(left + length, poolbottom, top - length);
                    AddVertex(sideBottomRight, glassColor, glm::vec3(1.0f, 0.0, 0.0));
                    AddVertex(sideBottomLeft, glassColor, glm::vec3(1.0f, 0.0, 0.0));
                    //side left, right bottom, roght top, side right
                    AddIndex(i + j + 1, rightBottomIndex, rightTopIndex, i + j);
                    j += 2;
                }
            }
            i += j;
            j = 0;
        }
    }
    //random
    std::random_device seed;
    mEngine.seed(seed());
}

/*
water surface LOD ver.
 make a rectangler
*/
AEWaterSurface::AEWaterSurface(float seaBase, float leftX, float rightX, float topZ, float bottomZ, glm::vec3 color, glm::vec3 cameraPos)
    : AEDrawObjectBase3D(), mRand(16, 24)
{
    //initialize
    mTop = topZ;
    mRight = rightX;
    mSeaBase = seaBase;
    mSpeed = 2.2f;
    mFreq = 6.0f;
    mAmp = 2.0f;
    mDz = 1.0f;
    float length = 0.04;
    //float length = 0.5;
    float y = mSeaBase;
    uint32_t i = 0;
    uint32_t j = 0;
    const float LEFT = leftX;
    const float RIGHT = rightX;
    const float TOP = topZ;
    const float BOTTOM = bottomZ;
    glm::vec3 glassColor(0.1f);
    uint32_t leftTopIndex = 0;
    uint32_t leftBottomIndex = 1;
    uint32_t rightBottomIndex = 2;
    uint32_t rightTopIndex = 3;
    std::vector<uint32_t> rightLane;
    for(float left = LEFT; left < RIGHT; left = left + length)
    {
        uint32_t colIndex = 0;
        for(float top = TOP; top > BOTTOM; top = top - length)
        {
            glm::vec3 leftTop(left, y, top);
            glm::vec3 leftBottom(left, y, top - length);
            glm::vec3 rightBottom(left + length, y, top - length);
            glm::vec3 rightTop(left + length, y, top);
            glm::vec3 normal = glm::normalize(glm::cross(leftBottom - leftTop, rightTop - leftTop));
            if(top == TOP && left == LEFT)
            {
                AddVertex(leftTop, color, normal);
                AddVertex(leftBottom, color, normal);
                AddVertex(rightBottom, color, normal);
                AddVertex(rightTop, color, normal);
                leftTopIndex = i;
                leftBottomIndex = i + 1;
                rightBottomIndex = i + 2;
                rightTopIndex = i + 3;
                j = 4;
                rightLane.push_back(rightTopIndex);
                rightLane.push_back(rightBottomIndex);
            }
            else if(left == LEFT)
            {
                AddVertex(leftBottom, color, normal);
                AddVertex(rightBottom, color, normal);
                //use cached index
                leftTopIndex = leftBottomIndex;
                rightTopIndex = rightBottomIndex;
                leftBottomIndex = i;
                rightBottomIndex = i + 1;
                j = 2;
                rightLane.push_back(rightBottomIndex);
            }
            else if(top == TOP)
            {
                AddVertex(rightBottom, color, normal);
                AddVertex(rightTop, color, normal);
                leftTopIndex = rightLane[0];
                leftBottomIndex = rightLane[1];
                rightBottomIndex = i;
                rightTopIndex = i + 1;
                //rightLane.erase(rightLane.begin(), rightLane.begin() + 1);
                j = 2;
                rightLane[0] = rightTopIndex;
                rightLane[1] = rightBottomIndex;
                colIndex++;
            }
            else
            {
                AddVertex(rightBottom, color, normal);
                leftTopIndex = leftBottomIndex;
                leftBottomIndex = rightLane[colIndex + 1];
                rightTopIndex = rightBottomIndex;
                rightBottomIndex = i;
                j = 1;
                rightLane[colIndex + 1] = rightBottomIndex;
                colIndex++;
            }
            AddIndex(leftTopIndex, leftBottomIndex, rightBottomIndex, rightTopIndex);
            //index
            // uint32_t leftTopIndex = FindExistVertexMultiThreads(i, leftTop);
            // if(leftTopIndex != i)
            //     j--;
            // else
            //     AddVertex(leftTop, color, normal);
            // uint32_t leftBottomIndex = FindExistVertexMultiThreads(i + 1, leftBottom);
            // if(leftBottomIndex != i + 1)
            //     j--;
            // else
            //     AddVertex(leftBottom, color, normal);
            // uint32_t rightBottomIndex = FindExistVertexMultiThreads(i + 2, rightBottom);
            // if(rightBottomIndex != i + 2)
            //     j--;
            // else
            //     AddVertex(rightBottom, color, normal);
            // uint32_t rightTopIndex = FindExistVertexMultiThreads(i + 3, rightTop);
            // if(rightTopIndex != i + 3)
            //     j--;
            // else
            //     AddVertex(rightTop, color, normal);
            if(top == TOP)
            {
                glm::vec3 sideTopRight(left, 0.0f, top);
                glm::vec3 sideTopLeft(left + length, 0.0f, top);
                AddVertex(sideTopRight, glassColor, glm::vec3(0.0, 0.0, -1.0f));
                AddVertex(sideTopLeft, glassColor, glm::vec3(0.0, 0.0, -1.0f));
                //side left, top right, top left, side right
                AddIndex(i + j + 1, rightTopIndex, leftTopIndex, i + j);
                j += 2;
            }
            if(left == LEFT)
            {
                glm::vec3 sideBottomRight(left, 0.0f, top - length);
                glm::vec3 sideBottomLeft(left, 0.0f, top);
                AddVertex(sideBottomRight, glassColor, glm::vec3(-1.0f, 0.0, 0.0));
                AddVertex(sideBottomLeft, glassColor, glm::vec3(-1.0f, 0.0, 0.0));
                //side left, top left, left bottom, side right
                AddIndex(i + j + 1, leftTopIndex, leftBottomIndex, i + j);
                j += 2;
            }
            if(top - length <= BOTTOM)
            {
                glm::vec3 sideBottomRight(left + length, 0.0f, top - length);
                glm::vec3 sideBottomLeft(left, 0.0f, top - length);
                AddVertex(sideBottomRight, glassColor, glm::vec3(0.0, 0.0, -1.0f));
                AddVertex(sideBottomLeft, glassColor, glm::vec3(0.0, 0.0, -1.0f));
                //side left, left bottom, right bottom, side right
                AddIndex(i + j + 1, leftBottomIndex, rightBottomIndex, i + j);
                j += 2;
            }
            if(left + length >= RIGHT)
            {
                glm::vec3 sideBottomRight(left + length, 0.0f, top);
                glm::vec3 sideBottomLeft(left + length, 0.0f, top - length);
                AddVertex(sideBottomRight, glassColor, glm::vec3(1.0f, 0.0, 0.0));
                AddVertex(sideBottomLeft, glassColor, glm::vec3(1.0f, 0.0, 0.0));
                //side left, right bottom, roght top, side right
                AddIndex(i + j + 1, rightBottomIndex, rightTopIndex, i + j);
                j += 2;
            }
            i += j;
            j = 0;
        }
    }
    //random
    std::random_device seed;
    mEngine.seed(seed());
}

/*
destructor
*/
AEWaterSurface::~AEWaterSurface()
{
    mComputePipeline.release();
}

/*
find existing vertex
*/
void AEWaterSurface::FindExistVertex(std::vector<uint32_t>& ret, uint32_t mod, glm::vec3 pos, uint32_t threadNum)
{
    for(uint32_t i = mod; i < mVertices.size(); i = i + threadNum)
    {
        if(mVertices[i].pos == pos)
        {
            ret[mod] = i;
            break;
        }
    }
}

/*
find existing vertex multi threads
*/
uint32_t AEWaterSurface::FindExistVertexMultiThreads(uint32_t index, glm::vec3 pos)
{
    const uint32_t threadNum = 28;
    std::array<std::unique_ptr<std::thread>, threadNum> threads;
    std::vector<uint32_t> ret(threadNum, index);
    for(uint32_t i = 0; i < threadNum; i++)
    {
        threads[i] = std::make_unique<std::thread>(&AEWaterSurface::FindExistVertex, this, std::ref(ret), i, pos, threadNum);
    }
    for(uint32_t i = 0; i < threadNum; i++)
        threads[i]->join();
    for(uint32_t i = 0; i < threadNum; i++)
        threads[i].reset();
    for(uint32_t i = 0; i < threadNum; i++)
    {
        if(ret[i] != index)
            return ret[i];
    }
    return index;
}

/*
calc sea level
*/
void AEWaterSurface::SeaLevel(float time)
{
    const uint32_t threadNum = 4;
    std::array<std::unique_ptr<std::thread>, threadNum> threads;
    for(uint32_t i = 0; i < threadNum; i++)
        threads[i] = std::make_unique<std::thread>(&AEWaterSurface::CalcSeaLevel, this, i, time, threadNum);
    for(uint32_t i = 0; i < threadNum; i++)
        threads[i]->join();
    for(uint32_t i = 0; i < threadNum; i++)
        threads[i].reset();
    //uint32_t colSize = mCols.size();
    // for(uint32_t i = 0; i < colSize; i++)
    //     CalcSeaLevel(i, time, 1);
}

/*
calc sea level
*/
void AEWaterSurface::CalcSeaLevel(uint32_t mod, float time, uint32_t threadNum)
{
    uint32_t vertexSize = GetVertexSize();
    for(uint32_t i = 0 + mod; i < vertexSize; i = i + threadNum)
    {
        glm::vec3& pos = mVertices[i].pos;
//        if(pos.y > -0.0001f)
//            continue;
        // float relativeZ = (pos.z + mEdge) / (2.0f * mEdge);
        // float relativeX = (pos.x) / (2.0f * mEdge);
        // float sinY = sin(time * M_PI);
        //pos.y = -1.5 + (sin(relativeZ - time) + sin(relativeX - time)) / mRand(mEngine);
        //wave
        // pos.y = -1.5 + (sin(relativeZ - 3.0f * time) + sin(relativeX - 3.0f * time)) / 24.0f;
        // pos.y += 0.033f * sin(relativeZ - 0.71f * time) + 0.045f * sin(relativeX - 0.53f * time);
        // pos.y += 0.029f * sin(relativeZ - 0.19f * time) + 0.031f * sin(relativeX - 0.37f * time);

        //NVIDIA Forum
        // float waves = 0.0f;
        // for(uint32_t i = 0; i < 8; i++)
        //     waves -= relativeZ * ((1.0f / 10.0f) * cos(powf(2.0f, i) * (2.0f * M_PI * 0.001f) * (relativeX) * (relativeZ) - time * 2.0f)) /  powf(2.0f, i);
        // pos.y = -1.5 + waves;
        //circle wave
        //pos.y = -1.5 + sin(time * 5.0f - glm::length(pos - glm::vec3(0, pos.y, 0)) * 8.0f) / 16.0f;
        pos.y = mSeaBase;
        int sign = -1;
        for(float f = -1.0f; f <= 1.0f; f = f + mDz)
        {
            OneWave(pos, glm::vec3(mRight * f, mSeaBase, mTop * 1.5f + 0.1f * sign * mTop), mSpeed, mFreq, mAmp, time);
            sign *= -1;
        }
        //SyntheticWave(pos, glm::vec3(0, -mSeaBase, mEdge), speed, freq, mSeaBase * 0.3f, time);

    }
   //float div = (float)mCols.size() / (float)threadNum;
//    Gerstner(mCols[/*div * */mod], glm::vec3(0.0, 0.0, -1.0), 0.25, 10.0, 5.0, 1.0, time);
}

/*
Gerstner waves
*/
void AEWaterSurface::Gerstner(uint32_t index, glm::vec3 waveVector, float amp, float freq, float speed, float steep, float time)
{
    
    float f = glm::dot(waveVector, mVertices[index].pos) * freq - speed * time;
    glm::vec3 diffPos(0.0f);
    diffPos += waveVector * freq * sin(f);
    uint32_t posIndex = FindClosestPoint(mVertices[index].pos - diffPos);
    float waveSum = 0.0f;
    waveSum += amp * cos(f);
    mVertices[posIndex].pos.y = -1.0 + waveSum;
    
   /*
    glm::vec2 d = glm::normalize(glm::vec2(waveVector.x, waveVector.z));
    float f =(glm::dot(waveVector, mVertices[index].pos) * freq + time * speed);
    glm::vec3 pos = steep * amp * mVertices[index].pos * cos(f);
    uint32_t waveTo = FindClosestPoint(pos);
    mVertices[waveTo].pos.y = amp * sin(f);
    */
}

/*
find closest point
*/
uint32_t AEWaterSurface::FindClosestPoint(glm::vec3 pos)
{
    /*
    uint32_t vertexSize = mVertices.size();
    float LEFT = mVertices[0].pos.x;
    float RIGHT = mEdge * 2.0f;
    float TOP = mVertices[0].pos.z;
    float BOTTOM = -mEdge * 2.0f;
    //x
    float posX = pos.x;
    uint32_t leftIndex = 0;
    while(leftIndex < rightIndex)
    {               
        //left side
        if(posX - LEFT < RIGHT - posX)
            rightIndex = rightIndex - (rightIndex - leftIndex + 1) * 0.5;
        else
            leftIndex = leftIndex + (rightIndex - leftIndex + 1) * 0.5;
        LEFT = mVertices[mCols[leftIndex]].pos.x;
        RIGHT = mVertices[mCols[rightIndex]].pos.x;
    }
    //z
    float posZ = pos.z;
    uint32_t topIndex = 0;
    while(topIndex < bottomIndex)
    {
        //top side
        if(TOP - posZ < posZ - BOTTOM)
            bottomIndex = bottomIndex - ((bottomIndex - topIndex + 1) * 0.5);
        else
            topIndex = topIndex + ((bottomIndex - topIndex + 1) * 0.5);
        TOP = mVertices[mRows[topIndex]].pos.z;
        BOTTOM = mVertices[mRows[bottomIndex]].pos.z;
    }
    uint32_t ret = mCols[leftIndex] + mRows[topIndex];
    if(mVertices[ret].pos != pos)
        return ret;
    else
        return mCols[rightIndex] + mRows[bottomIndex];
    */
   return 0;
}

/*
one wave
all (x, y) to origin
*/
void AEWaterSurface::OneWave(glm::vec3& pos, glm::vec3 origin, float speed, float freq, float amp, float time)
{
        //circle wave
        //pos.y = -1.5 + sin(time * 5.0f - glm::length(pos - glm::vec3(0, pos.y, 0)) * 8.0f) / 16.0f;
    glm::vec3 v = pos - origin;
    float d = glm::length(v);
    float d2 = glm::dot(v, v);
    glm::vec3 waveVector = glm::normalize(v);
    //float f = glm::dot(waveVector, pos) * freq - speed * time;
    float f = d * freq - speed * time;
    //float dy = (amp * cos(f)) / (1 + d + time * time);
    float dy = (amp * cos(f)) / (1 + d);
    //float dy = (amp * Wave(f)) / (1 + d2);
    //lambda
    //float lambda = speed * (1.0f / freq);
    if(time * speed > d)
    {
        if(dy < 0)
            pos.y += dy * 1.0f;
        else
            //pos.y += dy * 0.2f;
            pos.y += dy * 1.0f;
    }
}

/*
thenthetic wave
*/
void AEWaterSurface::SyntheticWave(glm::vec3& pos, glm::vec3 origin, float speed, float freq, float amp, float time)
{
        //circle wave
        //pos.y = -1.5 + sin(time * 5.0f - glm::length(pos - glm::vec3(0, pos.y, 0)) * 8.0f) / 16.0f;
    glm::vec3 v = pos - origin;
    float d = glm::length(v);
    float d2 = glm::dot(v, v);
    glm::vec3 waveVector = glm::normalize(v);
    float magni = 3.0f;
    float period = 1.0f / freq;
    float localPeriod = magni * period;
    float localTime = fmod(time, localPeriod);
    float f = d * freq - speed * time;
    float dy = sin(f);
    if(localTime > period)
        dy = 0.0f;
    if(time * speed > d)
    {
        if(dy < 0)
            pos.y += dy;
        else
            ;
    }
    //float dy = (amp * sin(f)) / (1 /*+ d + time * time*/);
    //lambda
    //float lambda = speed * (1.0f / freq);
}

/*
wave
*/
float AEWaterSurface::Wave(float x)
{
    float t = fmod(x, 2.0f) - 1.0f;
    if(t <= 0.0f)
        return powf(t, 3.0f) + t + 1;
    else
        return -powf(t, 3.0f) - t + 1;
}

/*
 * dispatch wave
 */
void AEWaterSurface::DispatchWave(AELogicalDevice* device, AECommandBuffer* command, AEDeviceQueue* queue, AECommandPool* commandPool,
                                  AEFence* fence, VkSemaphore *waitSemaphore, VkSemaphore* signalSemaphore,
                                  double time, AEEvent* event)
{
    //update buffer
    WaveUBO wu = {};
    wu.amp = mAmp;
    wu.dz = mDz;
    wu.freq = mFreq;
    wu.paddle = 1.5f;
    wu.seabase = mSeaBase;
    wu.speed = mSpeed;
    wu.time = (float)time;
    wu.top = mTop;
    wu.right = mRight;
    mWaveUniformBuffer->CopyData((void*)&wu, sizeof(WaveUBO));
    //command record for each joint
    AECommand::BeginCommand(command);
    //prepare event
    vkCmdResetEvent(*command->GetCommandBuffer(), *event->GetEvent(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    AECommand::BindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline.get());
    uint32_t threads = mVertices.size();
    AECommand::BindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE,
                                  mComputePipeline->GetPipelineLayout(),
                                  1, mDSs[0]->GetDescriptorSet());
    //dispatch
    //each work groups
    uint32_t groups = (threads / 1024) + 1;
    vkCmdDispatch(*command->GetCommandBuffer(), groups, 1, 1);
    vkCmdSetEvent(*command->GetCommandBuffer(), *event->GetEvent(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    //each local thread
    //vkCmdDispatch(*command->GetCommandBuffer(), 740, 1, 1);
    AECommand::EndCommand(command);
    //submit
    VkSubmitInfo submit_info = {};
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    if(waitSemaphore != nullptr) {
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphore,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 0,
                .pSignalSemaphores = nullptr};
    } else if(signalSemaphore != nullptr){
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = waitSemaphore,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signalSemaphore};
    }else{
        submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 0,
                .pWaitSemaphores = nullptr,
                .pWaitDstStageMask = 0,
                .commandBufferCount = 1,
                .pCommandBuffers = command->GetCommandBuffer(),
                .signalSemaphoreCount = 0,
                .pSignalSemaphores = nullptr};
    }
    if(fence != nullptr) {
        vkQueueSubmit(queue->GetQueue(0), 1, &submit_info, *fence->GetFence());
    }
    else{
        vkQueueSubmit(queue->GetQueue(0), 1, &submit_info, VK_NULL_HANDLE);
    }
}

/*
 * wave prepare
 */
void AEWaterSurface::WavePrepare(android_app *app, AELogicalDevice *device, std::vector<const char *> &shaders,
                 AEBufferBase **buffer, AEDeviceQueue *queue, AECommandPool *commandPool,
                 AEDescriptorPool *descriptorPool)
{   //compute pipeline
    AEDescriptorSetLayout cl(device);
    cl.AddDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.AddDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     VK_SHADER_STAGE_COMPUTE_BIT, 1,
                                     nullptr);
    cl.CreateDescriptorSetLayout();
    mComputePipeline = std::make_unique<AEComputePipeline>(device, shaders, &cl, app);
    //uniform buffer
    mWaveUniformBuffer = std::make_unique<AEBufferUniform>(device, sizeof(WaveUBO));
    WaveUBO wu = {};
    mWaveUniformBuffer->CreateBuffer();
    mWaveUniformBuffer->CopyData((void*)&wu, sizeof(WaveUBO));
    //descriptor set
    std::unique_ptr<AEDescriptorSet> ds = std::make_unique<AEDescriptorSet>(device, &cl, descriptorPool);
    if(buffer != nullptr){
        ds->BindDescriptorBuffer(0, buffer[0]->GetBuffer(), GetVertexBufferSize(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        ds->BindDescriptorBuffer(0, mWaveUniformBuffer->GetBuffer(), sizeof(WaveUBO), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        mDSs.emplace_back(std::move(ds));
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "aqoole wave", "wave descriptor error %c", 0);
        throw std::runtime_error("fail to create desctiptor set at wave");
    }
}
