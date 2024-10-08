#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <thread>
#include <array>

#include <filesystem>
#include <unordered_map>

#include "FS.hpp"
#include "Checksum.hpp"
#include "ScreenRatio.hpp"
#include "SpliString.hpp"
#include "WidthHeightEnum.hpp"
#include "USC.hpp"

namespace nihil {
	namespace nstd {
        struct Vertex {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;

            Vertex(float _x, float _y, float _z)
            {
                x = _x;
                y = _y;
                z = _z;
            }

            Vertex() {};

            std::string to_string()
            {
                std::string out;
                out += std::to_string(x) + '/';
                out += std::to_string(y) + '/';
                out += std::to_string(z) + '/';
                return out;
            }

            static std::string makeKey(std::vector<std::string> objData)
            {
                std::string out;
                out += objData[1] + '/';
                out += objData[2] + '/';
                out += objData[3] + '/';
                return out;
            }
        };
        struct Face {
            uint64_t vi1;
            uint64_t ti1;
            uint64_t ni1;

            uint64_t vi2;
            uint64_t ti2;
            uint64_t ni2;

            uint64_t vi3;
            uint64_t ti3;
            uint64_t ni3;

            std::string to_string()
            {
                std::string out;
                out += std::to_string(vi1) + '/';
                out += std::to_string(ti1) + '/';
                out += std::to_string(ni1) + '/';
                out += std::to_string(vi2) + '/';
                out += std::to_string(ti2) + '/';
                out += std::to_string(ni2) + '/';
                out += std::to_string(vi3) + '/';
                out += std::to_string(ti3) + '/';
                out += std::to_string(ni3) + '/';
                return out;
            }
        };

        struct Normal {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;

            Normal(float _x, float _y, float _z)
            {
                x = _x;
                y = _y;
                z = _z;
            }
            Normal() {};

            std::string to_string()
            {
                std::string out;
                out += std::to_string(x) + '/';
                out += std::to_string(y) + '/';
                out += std::to_string(z) + '/';
                return out;
            }

            static std::string makeKey(std::vector<std::string> objData)
            {
                std::string out;
                out += objData[1] + '/';
                out += objData[2] + '/';
                out += objData[3] + '/';
                return out;
            }
        };

        struct TexCoord {
            float x = 0.0f;
            float y = 0.0f;

            TexCoord(float _x, float _y)
            {
                x = _x;
                y = _y;
            }
            TexCoord() {};

            std::string to_string()
            {
                std::string out;
                out += std::to_string(x) + '/';
                out += std::to_string(y) + '/';
                return out;
            }

            static std::string makeKey(std::vector<std::string> objData)
            {
                std::string out;
                out += objData[1] + '/';
                out += objData[2] + '/';
                return out;
            }
        };

        struct Point {
            uint64_t vi = 0;
            uint64_t ti = 0;
            uint64_t ni = 0;

            uint64_t index = 0;

            Point(uint64_t _vi, uint64_t _ti, uint64_t _ni)
            {
                vi = _vi;
                ti = _ti;
                ni = _ni;
            }
            Point() {}

            std::string to_string()
            {
                std::string out;
                out += std::to_string(vi) + '/';
                out += std::to_string(ti) + '/';
                out += std::to_string(ni) + '/';
                return out;
            }
        };

        struct VertexBundle {
            Vertex vertex;
            TexCoord texCoord;
            Normal normal;
        };

        enum class LoadBinObj {
            Bin,
            Obj,
            DontCare
        };

        class OBJ 
        {
        public:
            std::string fileContent;
            std::vector<Vertex> vertices;
            std::vector<Face> faces;
            std::vector<float> verticesRender;
            std::vector<uint32_t> indicesRender;

            void loadOBJAdvanced(std::string path, ScreenRatio screenRatio)
            {
                std::cout << "Loading " << path << std::endl;
                std::string fileContent = LoadFile(path);

                std::vector<Vertex> vertexVec;
                std::vector<Face> facesVec;
                std::vector<TexCoord> texCoordVec;
                std::vector<Normal> normalVec;

                std::unordered_map<std::string, Point> faceMap;

                std::vector<uint32_t> indicesVec;
                std::vector<float> verticesVec;

                //parse the obj file
                // Use std::istringstream to iterate over lines
                std::istringstream iss(fileContent);
                std::string line;

                size_t verticesSize = 0;
                size_t facesSize = 0;
                size_t texCoordsSize = 0;
                size_t normalsSize = 0;

                std::vector<std::string> verticesS;
                std::vector<std::string> texCoordsS;
                std::vector<std::string> normalsS;
                std::vector<std::string> facesS;

                //qualify lines for parsing
                while (std::getline(iss, line)) {
                    if (line.data()[0] == 'v' && line.data()[1] == ' ') {
                        verticesSize++;
                        verticesS.push_back(line);
                    }
                    else if (line.data()[0] == 'f' && line.data()[1] == ' ') {
                        facesSize++;
                        facesS.push_back(line);
                    }
                    else if (line.data()[0] == 'v' && line.data()[1] == 't') {
                        texCoordsSize++;
                        texCoordsS.push_back(line);
                    }
                    else if (line.data()[0] == 'v' && line.data()[1] == 'n') {
                        normalsSize++;
                        normalsS.push_back(line);
                    }
                }

                uint16_t tCount = std::thread::hardware_concurrency();

                //qualify tokens for parsing
                if (tCount < 8) 
                {
                    if (tCount <= 4)
                    {
                        std::thread t1 = std::thread([]() {

                        });
                        std::thread t2 = std::thread([]() {

                        });
                    }
                    else
                    {

                    }
                }
                else if (tCount >= 8)
                {
                    if (tCount > 12)
                    {
                        std::thread t1 = std::thread([&verticesS, &vertexVec]() {
                            for (std::string& s : verticesS)
                            {
                                std::vector<std::string> sv = splitString(s, ' ');
                                Vertex v = {};
                                for (int i = 1; i < 4; i++)
                                {
                                    switch (i)
                                    {
                                    case 1:
                                        v.x = std::stof(sv[i]);
                                        break;
                                    case 2:
                                        v.y = std::stof(sv[i]);
                                        break;
                                    case 3:
                                        v.z = std::stof(sv[i]);
                                        break;
                                    }
                                }
                                vertexVec.push_back(v);
                            }
                            std::cout << "Loaded vertices" << std::endl;
                        });
                        std::thread t2 = std::thread([&facesS, &facesVec]() {
                            for (std::string& s : facesS)
                            {
                                std::vector<std::string> sv = splitString(s, ' ');
                                Face f = {};
                                std::vector<std::vector<std::string>> ssv(3);
                                for (int i = 1; i < sv.size(); i++)
                                {
                                    switch (i) {
                                    case 1:
                                        ssv[i - 1] = splitString(sv[i], '/');
                                        for (int z = 0; z < ssv[i - 1].size(); z++)
                                        {
                                            switch (z)
                                            {
                                            case 0:
                                                f.vi1 = std::stoi(ssv[i - 1][0]);
                                                break;
                                            case 1:
                                                f.ti1 = std::stoi(ssv[i - 1][1]);
                                                break;
                                            case 2:
                                                f.ni1 = std::stoi(ssv[i - 1][2]);
                                                break;
                                            }
                                        }
                                        break;
                                    case 2:
                                        ssv[i - 1] = splitString(sv[i], '/');
                                        for (int z = 0; z < ssv[i - 1].size(); z++)
                                        {
                                            switch (z)
                                            {
                                            case 0:
                                                f.vi2 = std::stoi(ssv[i - 1][0]);
                                                break;
                                            case 1:
                                                f.ti2 = std::stoi(ssv[i - 1][1]);
                                                break;
                                            case 2:
                                                f.ni2 = std::stoi(ssv[i - 1][2]);
                                                break;
                                            }
                                        }
                                        break;
                                    case 3:
                                        ssv[i - 1] = splitString(sv[i], '/');
                                        for (int z = 0; z < ssv[i - 1].size(); z++)
                                        {
                                            switch (z)
                                            {
                                            case 0:
                                                f.vi3 = std::stoi(ssv[i - 1][0]);
                                                break;
                                            case 1:
                                                f.ti3 = std::stoi(ssv[i - 1][1]);
                                                break;
                                            case 2:
                                                f.ni3 = std::stoi(ssv[i - 1][2]);
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }
                                facesVec.push_back(f);
                            }
                            std::cout << "Loaded faces" << std::endl;
                        });
                        std::thread t3 = std::thread([&normalsS, &normalVec]() {
                            for (std::string& s : normalsS)
                            {
                                std::vector<std::string> sv = splitString(s, ' ');
                                Normal n = {};
                                for (int i = 1; i < sv.size(); i++)
                                {
                                    switch (i)
                                    {
                                    case 1:
                                        n.x = std::stof(sv[i]);
                                        break;
                                    case 2:
                                        n.y = std::stof(sv[i]);
                                        break;
                                    case 3:
                                        n.z = std::stof(sv[i]);
                                        break;
                                    }
                                }
                                normalVec.push_back(n);
                            }
                            std::cout << "Loaded normals" << std::endl;
                        });
                        std::thread t4 = std::thread([&texCoordsS, &texCoordVec]() {
                            for (std::string& s : texCoordsS)
                            {
                                std::vector<std::string> sv = splitString(s, ' ');
                                TexCoord t = {};
                                for (int i = 1; i < sv.size(); i++)
                                {
                                    switch (i)
                                    {
                                    case 1:
                                        t.x = std::stof(sv[i]);
                                        break;
                                    case 2:
                                        t.y = std::stof(sv[i]);
                                        break;
                                    }
                                }
                                texCoordVec.push_back(t);
                            }
                            std::cout << "Loaded texCoords" << std::endl;
                        });
                        t1.join();
                        t2.join();
                        t3.join();
                        t4.join();
                    }
                    else 
                    {
                        std::thread t1 = std::thread([&verticesS, &vertexVec]() {
                            for (std::string& s : verticesS)
                            {
                                std::vector<std::string> sv = splitString(s, ' ');
                                Vertex v = {};
                                for (int i = 1; i < sv.size(); i++)
                                {
                                    switch (i)
                                    {
                                    case 1:
                                        v.x = std::stof(sv[i]);
                                        break;
                                    case 2:
                                        v.y = std::stof(sv[i]);
                                        break;
                                    case 3:
                                        v.z = std::stof(sv[i]);
                                        break;
                                    }
                                }
                                vertexVec.push_back(v);
                            }
                            });
                        std::thread t2 = std::thread([&facesS, &facesVec]() {
                            for (std::string& s : facesS)
                            {
                                std::vector<std::string> sv = splitString(s, ' ');
                                Face f = {};
                                std::vector<std::vector<std::string>> ssv(3);
                                for (int i = 1; i < sv.size(); i++)
                                {
                                    switch (i) {
                                    case 1:
                                        ssv[i - 1] = splitString(sv[i], '/');
                                        for (int z = 0; z < ssv[i - 1].size(); z++)
                                        {
                                            switch (z)
                                            {
                                            case 0:
                                                f.vi1 = std::stoi(ssv[i - 1][0]);
                                                break;
                                            case 1:
                                                f.ti1 = std::stoi(ssv[i - 1][1]);
                                                break;
                                            case 2:
                                                f.ni1 = std::stoi(ssv[i - 1][2]);
                                                break;
                                            }
                                        }
                                        break;
                                    case 2:
                                        ssv[i - 1] = splitString(sv[i], '/');
                                        for (int z = 0; z < ssv[i - 1].size(); z++)
                                        {
                                            switch (z)
                                            {
                                            case 0:
                                                f.vi2 = std::stoi(ssv[i - 1][0]);
                                                break;
                                            case 1:
                                                f.ti2 = std::stoi(ssv[i - 1][1]);
                                                break;
                                            case 2:
                                                f.ni2 = std::stoi(ssv[i - 1][2]);
                                                break;
                                            }
                                        }
                                        break;
                                    case 3:
                                        ssv[i - 1] = splitString(sv[i], '/');
                                        for (int z = 0; z < ssv[i - 1].size(); z++)
                                        {
                                            switch (z)
                                            {
                                            case 0:
                                                f.vi3 = std::stoi(ssv[i - 1][0]);
                                                break;
                                            case 1:
                                                f.ti3 = std::stoi(ssv[i - 1][1]);
                                                break;
                                            case 2:
                                                f.ni3 = std::stoi(ssv[i - 1][2]);
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }
                                facesVec.push_back(f);
                            }
                            });
                        std::thread t3 = std::thread([&normalsS, &normalVec]() {
                            for (std::string& s : normalsS)
                            {
                                std::vector<std::string> sv = splitString(s, ' ');
                                Normal n = {};
                                for (int i = 1; i < 4; i++)
                                {
                                    switch (i)
                                    {
                                    case 1:
                                        n.x = std::stof(sv[i]);
                                        break;
                                    case 2:
                                        n.y = std::stof(sv[i]);
                                        break;
                                    case 3:
                                        n.z = std::stof(sv[i]);
                                        break;
                                    }
                                }
                                normalVec.push_back(n);
                            }
                            });
                        std::thread t4 = std::thread([&texCoordsS, &texCoordVec]() {
                            for (std::string& s : texCoordsS)
                            {
                                std::vector<std::string> sv = splitString(s, ' ');
                                TexCoord t = {};
                                for (int i = 1; i < 3; i++)
                                {
                                    switch (i)
                                    {
                                    case 1:
                                        t.x = std::stof(sv[i]);
                                        break;
                                    case 2:
                                        t.y = std::stof(sv[i]);
                                        break;
                                    }
                                }
                                texCoordVec.push_back(t);
                            }
                            });
                        t1.join();
                        t2.join();
                        t3.join();
                        t4.join();
                    }
                }

                uint64_t x = 0;
                for (Face& f : facesVec)
                {
                    //find if there are any occurences of the vertex and if not push it to the hash map with its correspondng index

                    std::array<Point, 3> points;
                    points[0] = Point(f.vi1, f.ti1, f.ni1);
                    points[1] = Point(f.vi2, f.ti2, f.ni2);
                    points[2] = Point(f.vi3, f.ti3, f.ni3);

                    for (int i = 0; i < 3; i++)
                    {
                        //figure out how to calculate the index
                        if (faceMap.find(points[i].to_string()) == faceMap.end())
                        {
                            points[i].index = x;
                            faceMap.insert(std::make_pair(points[i].to_string(), points[i]));
                            x++;
                        }
                    }

                    for (int i = 0; i < 3; i++)
                    {
                        auto it = faceMap.find(points[i].to_string());
                        if (it != faceMap.end())
                        {
                            indicesVec.push_back(it->second.index);
                        }
                        else
                        {
                            std::cerr << "Error whilst parsing .obj, the following point's index was not found: points[i].to_string()" << std::endl;
                            std::abort();
                        }
                    }
                }

                std::cout << "loaded the faces" << std::endl;

                std::vector<VertexBundle> vb;
                vb.resize(x + 1);

                verticesVec.resize(11 * faceMap.size());

                uint8_t color = 1;

                for (auto& mv : faceMap)
                {
                    VertexBundle b = {};
                    b.vertex = vertexVec[mv.second.vi - 1];
                    b.texCoord = texCoordVec[mv.second.ti - 1];
                    b.normal = normalVec[mv.second.ni - 1];

                    verticesVec[mv.second.index * 11 + 0] = b.vertex.x;
                    verticesVec[mv.second.index * 11 + 1] = b.vertex.y;
                    verticesVec[mv.second.index * 11 + 2] = b.vertex.z;

                    color++;
                    verticesVec[mv.second.index * 11 + 3] = color * 0.1f;
                    verticesVec[mv.second.index * 11 + 4] = color * 0.2f;
                    verticesVec[mv.second.index * 11 + 5] = color * 0.3f;
                    if (color >= 3) color = 1;

                    verticesVec[mv.second.index * 11 + 6] = b.texCoord.x;
                    verticesVec[mv.second.index * 11 + 7] = b.texCoord.y;

                    verticesVec[mv.second.index * 11 + 8] = b.normal.x;
                    verticesVec[mv.second.index * 11 + 9] = b.normal.y;
                    verticesVec[mv.second.index * 11 + 10] = b.normal.z;

                    //indicesVec[index] = mv.second.index;
                    //index++;
                }

                verticesRender = verticesVec;
                indicesRender = indicesVec;

                /*
                prepare the buffers(edit all of the vulkan stuff to have a vertex buffer with a size of 11 * sizeof(float)) 
                make the unordered map store all possible variants of vertices then after registering all of them move them to
                a vector and find their indexes(i think ask chathGPT)
                */

                vertices = vertexVec;
                faces = facesVec;
            }

            std::vector<Point> points;

            void loadObjv2(std::string path, ScreenRatio screenRatio)
            {
                std::string fileContent = LoadFile(path);

                std::vector<Vertex> vertexVec;
                std::vector<Face> facesVec;
                std::vector<TexCoord> texCoordVec;
                std::vector<Normal> normalVec;

                std::unordered_map<std::string, Point> faceMap;

                std::vector<uint32_t> indicesVec;
                std::vector<float> verticesVec;

                //parse the obj file
                // Use std::istringstream to iterate over lines
                std::istringstream iss(fileContent);
                std::string line;

                size_t verticesSize = 0;
                size_t facesSize = 0;
                size_t texCoordsSize = 0;
                size_t normalsSize = 0;

                std::vector<std::string> verticesS;
                std::vector<std::string> texCoordsS;
                std::vector<std::string> normalsS;
                std::vector<std::string> facesS;

                //qualify lines for parsing
                while (std::getline(iss, line)) {
                    if (line.data()[0] == 'v' && line.data()[1] == ' ') {
                        verticesSize++;
                        verticesS.push_back(line);
                    }
                    else if (line.data()[0] == 'f' && line.data()[1] == ' ') {
                        facesSize++;
                        facesS.push_back(line);
                    }
                    else if (line.data()[0] == 'v' && line.data()[1] == 't') {
                        texCoordsSize++;
                        texCoordsS.push_back(line);
                    }
                    else if (line.data()[0] == 'v' && line.data()[1] == 'n') {
                        normalsSize++;
                        normalsS.push_back(line);
                    }
                }

                std::thread t1 = std::thread([&verticesS, &vertexVec]() {
                    for (std::string& s : verticesS)
                    {
                        std::vector<std::string> sv = splitString(s, ' ');
                        Vertex v = {};
                        for (int i = 1; i < sv.size(); i++)
                        {
                            switch (i)
                            {
                            case 1:
                                v.x = std::stof(sv[i]);
                                break;
                            case 2:
                                v.y = std::stof(sv[i]);
                                break;
                            case 3:
                                v.z = std::stof(sv[i]);
                                break;
                            }
                        }
                        vertexVec.push_back(v);
                    }
                    });
                std::thread t2 = std::thread([&facesS, &facesVec]() {
                    for (std::string& s : facesS)
                    {
                        std::vector<std::string> sv = splitString(s, ' ');
                        Face f = {};
                        std::vector<std::vector<std::string>> ssv(3);
                        for (int i = 1; i < sv.size(); i++)
                        {
                            switch (i) {
                            case 1:
                                ssv[i - 1] = splitString(sv[i], '/');
                                for (int z = 0; z < ssv[i - 1].size(); z++)
                                {
                                    switch (z)
                                    {
                                    case 0:
                                        f.vi1 = std::stoi(ssv[i - 1][0]);
                                        break;
                                    case 1:
                                        f.ti1 = std::stoi(ssv[i - 1][1]);
                                        break;
                                    case 2:
                                        f.ni1 = std::stoi(ssv[i - 1][2]);
                                        break;
                                    }
                                }
                                break;
                            case 2:
                                ssv[i - 1] = splitString(sv[i], '/');
                                for (int z = 0; z < ssv[i - 1].size(); z++)
                                {
                                    switch (z)
                                    {
                                    case 0:
                                        f.vi2 = std::stoi(ssv[i - 1][0]);
                                        break;
                                    case 1:
                                        f.ti2 = std::stoi(ssv[i - 1][1]);
                                        break;
                                    case 2:
                                        f.ni2 = std::stoi(ssv[i - 1][2]);
                                        break;
                                    }
                                }
                                break;
                            case 3:
                                ssv[i - 1] = splitString(sv[i], '/');
                                for (int z = 0; z < ssv[i - 1].size(); z++)
                                {
                                    switch (z)
                                    {
                                    case 0:
                                        f.vi3 = std::stoi(ssv[i - 1][0]);
                                        break;
                                    case 1:
                                        f.ti3 = std::stoi(ssv[i - 1][1]);
                                        break;
                                    case 2:
                                        f.ni3 = std::stoi(ssv[i - 1][2]);
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        facesVec.push_back(f);
                    }
                    });
                std::thread t3 = std::thread([&normalsS, &normalVec]() {
                    for (std::string& s : normalsS)
                    {
                        std::vector<std::string> sv = splitString(s, ' ');
                        Normal n = {};
                        for (int i = 1; i < 4; i++)
                        {
                            switch (i)
                            {
                            case 1:
                                n.x = std::stof(sv[i]);
                                break;
                            case 2:
                                n.y = std::stof(sv[i]);
                                break;
                            case 3:
                                n.z = std::stof(sv[i]);
                                break;
                            }
                        }
                        normalVec.push_back(n);
                    }
                    });
                std::thread t4 = std::thread([&texCoordsS, &texCoordVec]() {
                    for (std::string& s : texCoordsS)
                    {
                        std::vector<std::string> sv = splitString(s, ' ');
                        TexCoord t = {};
                        for (int i = 1; i < 3; i++)
                        {
                            switch (i)
                            {
                            case 1:
                                t.x = std::stof(sv[i]);
                                break;
                            case 2:
                                t.y = std::stof(sv[i]);
                                break;
                            }
                        }
                        texCoordVec.push_back(t);
                    }
                    });
                t1.join();
                t2.join();
                t3.join();
                t4.join();

                uint32_t index = 0;
                for (const Face& f : facesVec)
                {
                    std::array<Point, 3> points;
                    points[0] = Point(f.vi1, f.ti1, f.ni1);
                    points[1] = Point(f.vi2, f.ti2, f.ni2);
                    points[2] = Point(f.vi3, f.ti3, f.ni3);

                    for (int i = 0; i < points.size(); i++)
                    {
                        if (faceMap.find(points[i].to_string()) == faceMap.end())
                        {
                            points[i].index = index;
                            faceMap.insert(std::make_pair(points[i].to_string(), points[i]));
                            index++;
                        }

                        auto it = faceMap.find(points[i].to_string());
                        indicesRender.push_back(it->second.index);
                    }
                }

                verticesRender.resize(faceMap.size() * 11);

                uint64_t count = 1;
                for (auto& f : faceMap)
                {
                    verticesRender[f.second.index * 11 + 0] = vertexVec[f.second.vi - 1].x;
                    verticesRender[f.second.index * 11 + 1] = vertexVec[f.second.vi - 1].y;
                    verticesRender[f.second.index * 11 + 2] = vertexVec[f.second.vi - 1].z;

                    verticesRender[f.second.index * 11 + 3] = 0.4f * count;
                    verticesRender[f.second.index * 11 + 4] = 0.2f * count;
                    verticesRender[f.second.index * 11 + 5] = 0.3f * count;

                    if (count >= 3) count = 0;

                    verticesRender[f.second.index * 11 + 6] = texCoordVec[f.second.ti - 1].x;
                    verticesRender[f.second.index * 11 + 7] = texCoordVec[f.second.ti - 1].y;

                    verticesRender[f.second.index * 11 + 8] = normalVec[f.second.ni - 1].x;
                    verticesRender[f.second.index * 11 + 9] = normalVec[f.second.ni - 1].y;
                    verticesRender[f.second.index * 11 + 10] = normalVec[f.second.ni - 1].z;

                    count++;
                }

                vertices = vertexVec;
                points.resize(faceMap.size());
                for (auto& f : faceMap)
                {
                    points[f.second.index] = f.second;
                }
            }

            void loadOBJ(std::string path, ScreenRatio screenRatio)
            {
                auto start = std::chrono::high_resolution_clock::now();
                loadObjv2(path, screenRatio);
                auto end = std::chrono::high_resolution_clock::now();
                std::cout << "Advanced Loading: " << end - start << std::endl;
                //important save before applying USC
                saveAsBinaryLib(path);

                //apply USC
                for (int i = 0; i < verticesRender.size(); i += 11)
                {
                    verticesRender[i] = USC::NDC_u(verticesRender[i] * 1000, screenRatio, WidthHeightEnum::Width);
                    verticesRender[i + 1] = USC::NDC_u(verticesRender[i + 1] * 1000, screenRatio, WidthHeightEnum::Height);
                    verticesRender[i + 2] = USC::NDC_u(verticesRender[i + 2] * 1000, screenRatio, WidthHeightEnum::Depth);
                }
            }

            void loadObjFile()
            {

            }

            void Load(std::string path, LoadBinObj whatToLoad, ScreenRatio screenRatio)
            {
                if (whatToLoad == LoadBinObj::Obj)
                {
                    loadOBJ(path, screenRatio);
                }
                else if (whatToLoad == LoadBinObj::Bin)
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    loadFromBinaryLib(path, screenRatio);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
                }
                else if (whatToLoad == LoadBinObj::DontCare)
                {
                    if (std::filesystem::exists(path + ".indices.bin") && std::filesystem::exists(path + ".vertices.bin"))
                    {
                        auto start = std::chrono::high_resolution_clock::now();
                        loadFromBinaryLib(path, screenRatio);
                        auto end = std::chrono::high_resolution_clock::now();
                        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
                    }
                    else 
                    {
                        loadOBJ(path, screenRatio);
                    }
                }
            }

            void LoadDebug(std::string path, LoadBinObj whatToLoad, ScreenRatio screenRatio)
            {
                if (whatToLoad == LoadBinObj::Obj)
                {
                    loadOBJ(path, screenRatio);
                }
                else if (whatToLoad == LoadBinObj::Bin)
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    loadFromBinaryLib(path, screenRatio);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
                }
                else if (whatToLoad == LoadBinObj::DontCare)
                {
                    if (std::filesystem::exists(path + ".indices.bin") && std::filesystem::exists(path + ".vertices.bin") && !checkForChanges(path))
                    {
                        auto start = std::chrono::high_resolution_clock::now();
                        loadFromBinaryLib(path, screenRatio);
                        auto end = std::chrono::high_resolution_clock::now();
                        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
                    }
                    else
                    {
                        loadOBJ(path, screenRatio);
                    }
                }
            }

            void saveAsBinaryLib(std::string path)
            {
                std::vector<std::vector<char>> files = makeBinaryLib();

                // Open a file for writing in binary mode
                std::ofstream outputFile(path + ".indices.bin", std::ios::binary);

                if (outputFile.is_open()) {
                    // Write the byte array to the file
                    outputFile.write(files[0].data(), files[0].size());

                    // Close the file
                    outputFile.close();
                }
                else {
                    std::cerr << "Unable to open the file for writing." << std::endl;
                }

                // Open a file for writing in binary mode
                outputFile = std::ofstream(path + ".vertices.bin", std::ios::binary);

                if (outputFile.is_open()) {
                    // Write the byte array to the file
                    outputFile.write(files[1].data(), files[1].size());

                    // Close the file
                    outputFile.close();
                }
                else {
                    std::cerr << "Unable to open the file for writing." << std::endl;
                }
            }

            void loadFromBinaryLib(std::string path, ScreenRatio screenRatio)
            {
                // Specify the file name
                std::string fileName = path + ".indices.bin";

                // Open the file in binary mode
                std::ifstream fileStream(fileName, std::ios::binary);

                std::vector<char> indicesFile(0);

                if (fileStream.is_open()) {
                    // Determine the size of the file
                    fileStream.seekg(0, std::ios::end);
                    size_t fileSize = static_cast<size_t>(fileStream.tellg());
                    fileStream.seekg(0, std::ios::beg);

                    // Create a vector to hold the file content
                    indicesFile.resize(fileSize);

                    // Read the file into the vector
                    fileStream.read(indicesFile.data(), fileSize);

                    // Close the file
                    fileStream.close();
                }
                else {
                    std::cerr << "Failed to load model from a binary lib please rebake the binary files" << std::endl;
                }

                fileName = path + ".vertices.bin";
                fileStream = std::ifstream(fileName, std::ios::binary);
                
                std::vector<char> verticesFile(0);

                if (fileStream.is_open()) {
                    // Determine the size of the file
                    fileStream.seekg(0, std::ios::end);
                    size_t fileSize = static_cast<size_t>(fileStream.tellg());
                    fileStream.seekg(0, std::ios::beg);

                    // Create a vector to hold the file content
                    verticesFile.resize(fileSize);

                    // Read the file into the vector
                    fileStream.read(verticesFile.data(), fileSize);

                    // Close the file
                    fileStream.close();

                    std::cout << "Binary File loaded successfully." << std::endl;
                }

                // Ensure the size of the byte array is a multiple of sizeof(uint32_t)
                size_t numElements = indicesFile.size() / sizeof(uint32_t);

                indicesRender.resize(numElements);
                memcpy(indicesRender.data(), indicesFile.data(), indicesFile.size());

                numElements = verticesFile.size() / sizeof(float);

                verticesRender.resize(numElements);
                memcpy(verticesRender.data(), verticesFile.data(), verticesFile.size());

                //apply USC
                for (int i = 0; i < verticesRender.size(); i += 11)
                {
                    verticesRender[i] = USC::NDC_u(verticesRender[i] * 1000, screenRatio, WidthHeightEnum::Width);
                    verticesRender[i + 1] = USC::NDC_u(verticesRender[i + 1] * 1000, screenRatio, WidthHeightEnum::Height);
                    verticesRender[i + 2] = USC::NDC_u(verticesRender[i + 2] * 1000, screenRatio, WidthHeightEnum::Depth);
                }
            }

            std::vector<std::vector<char>> makeBinaryLib() 
            {
                std::vector<char> indicesBinary;
                indicesBinary.resize(indicesRender.size() * sizeof(uint32_t));
                memcpy(indicesBinary.data(), indicesRender.data(), indicesBinary.size());

                std::vector<char> verticesBinary;
                verticesBinary.resize(verticesRender.size() * sizeof(float));
                memcpy(verticesBinary.data(), verticesRender.data(), verticesBinary.size());

                std::vector<std::vector<char>> finalFiles;
                finalFiles.push_back(indicesBinary);
                finalFiles.push_back(verticesBinary);

                return finalFiles;
            }
        };
	}
}