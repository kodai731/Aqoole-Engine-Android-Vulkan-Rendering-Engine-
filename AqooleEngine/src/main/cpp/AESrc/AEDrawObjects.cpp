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

#include "AEDrawObjects.hpp"
#include "AEUBO.hpp"
#include "AEMatrix.hpp"


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
AEDrawObjectBaseCollada::AEDrawObjectBaseCollada(const char* filePath, android_app* app)
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
        std::vector<std::string> vertexWeights;
        //read images
        for(const ptree::value_type& images : tree.get_child("COLLADA.library_images"))
        {
            if (boost::optional<std::string> imageId = images.second.get_optional<std::string>("<xmlattr>.id"))
            {
                auto imageChild = images.second.get_child("init_from");
                std::string imageName(imageChild.data());
                while(imageName.find("\\") != std::string::npos)
                {
                    imageName.replace(0, imageName.find("\\") + 1, "");
                }
                imageName += std::string(".png");
                mTextureFiles.emplace_back(imageName);
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
                                    //one3DObj.pos = oneVec3;
                                    //mVertices.push_back(one3DObj);
                                    mPositions.push_back(oneVec3);
                                }
                                else if(std::regex_search(floatArrayStr.c_str(), std::regex("normal", std::regex::icase)))
                                {
                                    //mVertices[i].normal = oneVec3;
                                    mNormals.push_back(oneVec3);
                                }
                            }
                        }
                        else if(std::regex_search(floatArrayStr.c_str(), std::regex("map", std::regex::icase)) ||
                                std::regex_search(floatArrayStr.c_str(), std::regex("uv", std::regex::icase)))
                        {
                            std::string oneLinePositions = geometryChild.second.get<std::string>("float_array");
                            AEDrawObject::Split(fields, oneLinePositions, ' ');
                            uint32_t size = fields.size() / 2;
                            for(uint32_t i = 0; i < size; i++)
                            {
                                oneVec2.x = std::stof(fields[i * 2]);
                                oneVec2.y = 1.0f - std::stof(fields[i * 2 + 1]);
                                //mVertices[i].texcoord = oneVec2;
                                mMaps.emplace_back(oneVec2);
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
            ReadSkeletonNode(obj, mRoot);
        }
        //read skinning information
        //bind_shape_matrix
        ptree skinNodes = tree.get_child("COLLADA.library_controllers.controller.skin");
        //for loop to "source"
        for(ptree::const_iterator obj = skinNodes.begin(); obj != skinNodes.end(); ++obj)
        {            
            if(strncmp(obj->first.data(), "<xmlattr>", 10) == 0)
                continue;
            //bind_shape_matrix
            if(strncmp(obj->first.data(), "bind_shape_matrix", 18) == 0)
            {
                std::string matrixString = obj->second.get_value<std::string>();
                AEDrawObject::Split(fields, matrixString, ' ');
                glm::mat4 bindShapeMatrix;
                for(uint32_t i = 0; i < 4; i++)
                    for(uint32_t j = 0; j < 4; j++)
                        bindShapeMatrix[i][j] = std::stof(fields[4 * i + j]);
                mBSM = bindShapeMatrix;
            }
            //source id = skin-joints
            if(strncmp(obj->first.data(), "source", 7) == 0)
            {
                if(std::regex_search(obj->second.get_optional<std::string>("<xmlattr>.id").get(), std::regex("joint", std::regex::icase)))
                {
                    for(boost::property_tree::ptree::const_iterator child = obj->second.begin();
                        child != obj->second.end(); ++child)
                    {
                        //find Name_array
                        if(strncmp(child->first.data(), "Name_array", 11) != 0)
                            continue;
                        std::string nameArrayString = child->second.get_value_optional<std::string>().get();
                        AEDrawObject::Split(mSkinJointsArray, nameArrayString, ' ');
                    }
                }
            }
            //source id = skin-weights
            if(strncmp(obj->first.data(), "source", 7) == 0)
            {
                if(std::regex_search(obj->second.get_optional<std::string>("<xmlattr>.id").get(), std::regex("weight", std::regex::icase)))
                {
                    for(boost::property_tree::ptree::const_iterator child = obj->second.begin();
                        child != obj->second.end(); ++child)
                    {
                        //find float_array
                        if(strncmp(child->first.data(), "float_array", 11) != 0)
                            continue;
                        std::string weightString = child->second.get_value_optional<std::string>().get();
                        AEDrawObject::Split(vertexWeights, weightString, ' ');
                    }
                }
            }
            //vertex_weights
            if(strncmp(obj->first.data(), "vertex_weights", 15) == 0)
            {
                auto vcountString = obj->second.get_optional<std::string>("vcount").get();
                std::vector<std::string> influenceCounts;
                AEDrawObject::Split(influenceCounts, vcountString, ' ');
                auto vString = obj->second.get_optional<std::string>("v").get();
                std::vector<std::string> jointWeightList;
                AEDrawObject::Split(jointWeightList, vString, ' ');
                uint32_t nowPointing = 0;
                for(uint32_t i = 0; i < influenceCounts.size(); i++)
                {
                    JointWeight oneJointWeight;
                    uint32_t influenceCount = std::stoi(influenceCounts[i]);
                    for(uint32_t j = 0; j < influenceCount; j++)
                    {
                        oneJointWeight.jointIndices.push_back(std::stoi(jointWeightList[nowPointing]));
                        nowPointing++;
                        oneJointWeight.weights.push_back(std::stof(vertexWeights[std::stoi(jointWeightList[nowPointing])]));
                        nowPointing++;
                    }
                    //check total weight
                    float total = 0.0f;
                    for(uint32_t j = 0; j < influenceCount; j++)
                        total += oneJointWeight.weights[j];
                    if( !(0.99f < total && total < 1.01f))
                        __android_log_print(ANDROID_LOG_DEBUG, "collada", "total weight is not 1.0f", 0);
                    mJointWeights.push_back(oneJointWeight);
                }
            }
            //inverse matrices
            if(strncmp(obj->first.data(), "joints", 7) == 0)
            {
                //get inverse matrices id
                std::string inverseMatricesId;
                for(boost::property_tree::ptree::const_iterator child = obj->second.begin(); 
                    child != obj->second.end(); ++child)
                {
                    //find Name_array
                    if(strncmp(child->first.data(), "input", 6) != 0)
                        continue;
                    if(strncmp(child->second.get_optional<std::string>("<xmlattr>.semantic").get().c_str(),
                        "INV_BIND_MATRIX", 16) == 0)
                        inverseMatricesId = child->second.get_optional<std::string>("<xmlattr>.source").get();
                }
                //begin from start to find inverse matrices
                for(ptree::const_iterator innerObj = skinNodes.begin(); innerObj != skinNodes.end(); ++innerObj)
                {
                    if(std::strncmp(innerObj->first.data(), "source", 7) == 0)
                    {
                        auto sourceId = innerObj->second.get_optional<std::string>("<xmlattr>.id").get();
                        if(sourceId.find(inverseMatricesId.c_str()) != std::string::npos ||
                            inverseMatricesId.find(sourceId.c_str()) != std::string::npos)
                        {
                            for(auto child = innerObj->second.begin(); child != innerObj->second.end(); ++child)
                            {
                                //find float_array
                                if(strncmp(child->first.data(), "float_array", 11) != 0)
                                    continue;
                                std::string inverseString = child->second.get_value_optional<std::string>().get();
                                AEDrawObject::Split(fields, inverseString, ' ');
                                glm::mat4 oneMatrix;
                                for(uint32_t i = 0; i * 16 < fields.size(); i++)
                                {
                                    for(uint32_t j = 0; j < 4; j++)
                                        for(uint32_t k = 0; k < 4; k++)
                                            oneMatrix[j][k] = std::stof(fields[16 * i + 4 * j + k]);
                                    mInverseMatrices.push_back(oneMatrix);
                                }
                            }
                        }
                    }
                }
            }
        }
         MakeVertices();
    }
}

/*
destructor
*/
AEDrawObjectBaseCollada::~AEDrawObjectBaseCollada()
{

}

/*
process geometry part
*/
void AEDrawObjectBaseCollada::ProcessGeometry(std::ifstream &file)
{
    // std::string oneLine;
    // std::vector<std::string> fields;
    // while(std::getline(file, oneLine))
    // {
    //     //geometry end
    //     if(oneLine.find("</library_geometries>"))
    //         break;
    //     //geometry header
    //     if(oneLine.find("<geometry"))
    //     {
    //         AEDrawObject::GetInAngleBrackets(oneLine, oneLine);
    //         AEDrawObject::Split(fields, oneLine, ' ');
    //         //get ID
    //         mGeometryID.push_back(fields[1].substr(4, fields[1].size() - 1));
    //         //read source
    //         while(std::getline(file, oneLine))
    //         {
    //             if(oneLine.find("<source"))
    //             {

    //             }
    //         }
    //     }
    // }
}

/*
scale
*/
void AEDrawObjectBaseCollada::Scale(float scale)
{
    uint32_t size = mVertices.size();
    for(uint32_t i = 0; i < size; i++)
        mVertices[i].pos *= scale;
}

/*
make vertices data
*/
void AEDrawObjectBaseCollada::MakeVertices()
{
    uint32_t materials = mPositionIndices.size();
    Vertex3DObj oneVertex;
    uint32_t index = 0;
    for(uint32_t i = 0; i < materials; i++)
    {
        uint32_t vertices = mPositionIndices[i].size();
        for(uint32_t j = 0; j < vertices; j++)
        {
            oneVertex.pos = mPositions[mPositionIndices[i][j]];
            oneVertex.normal = mNormals[mNormalsIndices[i][j]];
            oneVertex.texcoord = mMaps[mMapIndices[i][j]];
            mVertices.emplace_back(oneVertex);
            mIndices.emplace_back(index);
            index++;
        }
    }
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
    if(sidName.is_initialized())
    {
        skeletonNode->sidName = sidName.get();
        skeletonNode->id = nowNode->second.get_optional<std::string>("<xmlattr>.id")->c_str();
    }
    //matrix
    std::vector<std::string> fields;
    std::string matrixString = nowNode->second.get_optional<std::string>("matrix").get();
    AEDrawObject::Split(fields, matrixString, ' ');
    for(uint32_t i = 0; i < 4; i++)
        for(uint32_t j = 0; j < 4; j++)
            skeletonNode->matrix[i][j] = std::stof(fields[(i * 4) + j]);
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
    if(node.second.get_optional<std::string>("<xmlattr>.id") != boost::none)
        a.id = node.second.get_optional<std::string>("<xmlattr>.id").get();
    BOOST_FOREACH(const auto& source, node.second.get_child("animation"))
    {
        auto childId = source.first.data();
        if(strcmp(childId, "<xmlattr>") == 0)
            continue;
        if(strcmp(childId, "source") == 0)
        {
            auto floatId = source.second.get_optional<std::string>("<xmlattr>.id")->c_str();
            if (std::regex_search(source.second.get_optional<std::string>("<xmlattr>.id")->c_str(), std::regex("input")))
            {
                std::string timeListS = source.second.get<std::string>("float_array");
                std::vector<std::string> timeList;
                AEDrawObject::Split(timeList, timeListS, ' ');
                for (uint32_t i = 0; i < timeList.size(); i++) {
                    float time = std::stof(timeList[i]);
                    a.timeList.emplace_back(time);
                }
            }
            else if(std::regex_search(source.second.get_optional<std::string>("<xmlattr>.id")->c_str(), std::regex("output")))
            {
                std::string matrixListS = source.second.get<std::string>("float_array");
                std::vector<std::string> matrixList;
                AEDrawObject::Split(matrixList, matrixListS, ' ');
                for (uint32_t i = 0; i < matrixList.size(); i =  i + 16)
                {
                    glm::mat4 m;
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
debug root node
*/
void AEDrawObjectBaseCollada::DebugRootNode()
{

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
    float half = len / 2.0f;
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

#ifdef __RAY_TRACING__PC
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
AEWaterSurface::AEWaterSurface(float seaBase, float leftX, float rightX, float topZ, float bottomZ, glm::vec3 color)
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
water surface LOD ver.
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
    const uint32_t threadNum = 30;
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
        if(pos.y > -0.0001f)
            continue;
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
        return powf32(t, 3.0f) + t + 1;
    else
        return -powf32(t, 3.0f) - t + 1;
}


#endif