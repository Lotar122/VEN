#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <unordered_map>

struct VertexCoordinates
{
    float x, y, z;
};

struct TextureCoordinates
{
    float x, y;
};

struct NormalCoordinates
{
    float x, y, z;
};

struct Point
{
    size_t vi, ti, ni;
};

struct Face
{
    Point p1;
    Point p2;
    Point p3;
};

struct BasicString
{
    size_t size = 0;
    char* src = nullptr;
};

static std::pair<std::vector<float>, std::vector<uint32_t>> readOBJFile(std::string path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open() || !file.good()) throw std::runtime_error(std::string("Failed to open the file:") + path);

    std::streamsize size = file.tellg(); // Get file size
    file.seekg(0, std::ios::beg);        // Seek back to beginning

    std::vector<char> buffer(size); // Allocate buffer
    if (!file.read(buffer.data(), size)) { // Read file content into buffer
        throw std::runtime_error("Failed to read file");
    }

    file.close();

    std::vector<BasicString> lines;
    std::vector<VertexCoordinates> vertexCoordinates;
    std::vector<TextureCoordinates> textureCoordinates;
    std::vector<NormalCoordinates> normalCoordinates;
    std::unordered_map<std::string, std::pair<Point, size_t>> points;
    std::vector<Face> faces;

    size_t start = 0, end;

    for(int i = 0; i < buffer.size(); i++)
    {
        if(buffer[i] == '\n' || buffer[i] == '\r')
        {
            end = i;
            if(!(*(buffer.data() + start) == '\n' || *(buffer.data() + start) == '#')) lines.push_back(BasicString{end - start, buffer.data() + start});
            start = i + 1;
        }
    }

    //? no multithreading for now
    //* for multithreading just pregroup the lines as vertices, texcoords and normals. then process them on seperate threads.
    for(BasicString& bs : lines)
    {
        if(bs.size >= 2 && bs.src[0] == 'v' && bs.src[1] == ' ')
        {
            size_t beg1 = 2, end1, beg2, end2, beg3, end3;
            int fi = 1;

            for(int i = 2; i < bs.size; i++)
            {
                //handle a line with a '\n' at the end and a line without it
                if(bs.src[i] == ' ' || bs.src[i] == '\n' || i == bs.size - 1)
                {
                    switch(fi) {
                        case 1:
                        end1 = i;
                        beg2 = i + 1;
                        break;
                        case 2:
                        end2 = i;
                        beg3 = i + 1;
                        break;
                        case 3:
                        end3 = i + 1;
                        break;
                    };

                    fi++;
                }
            }

            vertexCoordinates.push_back(VertexCoordinates{
                std::stof(std::string(bs.src + beg1, end1 - beg1)), 
                std::stof(std::string(bs.src + beg2, end2 - beg2)),
                std::stof(std::string(bs.src + beg3, end3 - beg3))
            });

        }

        else if(bs.size >= 2 && bs.src[0] == 'v' && bs.src[1] == 't')
        {
            size_t beg1 = 3, end1, beg2, end2;
            int fi = 1;

            for(int i = 3; i < bs.size; i++)
            {
                //handle a line with a '\n' at the end an a line without it
                if(bs.src[i] == ' ' || bs.src[i] == '\n' || i == bs.size - 1)
                {
                    switch(fi) {
                        case 1:
                        end1 = i;
                        beg2 = i + 1;
                        break;
                        case 2:
                        end2 = i + 1;
                        break;
                    };

                    fi++;
                }
            }

            textureCoordinates.push_back(TextureCoordinates{
                std::stof(std::string(bs.src + beg1, end1 - beg1)), 
                std::stof(std::string(bs.src + beg2, end2 - beg2))
            });
        }

        else if(bs.size >= 2 && bs.src[0] == 'v' && bs.src[1] == 'n')
        {
            size_t beg1 = 3, end1, beg2, end2, beg3, end3;
            int fi = 1;

            for(int i = 3; i < bs.size; i++)
            {
                //handle a line with a '\n' at the end an a line without it
                if(bs.src[i] == ' ' || bs.src[i] == '\n' || i == bs.size - 1)
                {
                    switch(fi) {
                        case 1:
                        end1 = i;
                        beg2 = i + 1;
                        break;
                        case 2:
                        end2 = i;
                        beg3 = i + 1;
                        break;
                        case 3:
                        end3 = i + 1;
                        break;
                    };

                    fi++;
                }
            }
            normalCoordinates.push_back(NormalCoordinates{
                std::stof(std::string(bs.src + beg1, end1 - beg1)), 
                std::stof(std::string(bs.src + beg2, end2 - beg2)),
                std::stof(std::string(bs.src + beg3, end3 - beg3))
            });
        }
        
        else if(bs.size >= 2 && bs.src[0] == 'f' && bs.src[1] == ' ')
        {
            //grab each point and put it into a stack allocated array of 3 points.
            //iterate through the stack array and properly parse each point.
            //get the parsed points and create a Face from it. later insert it to the table.
            Point pointsArr[3];

            size_t beg[4], end[3];
            beg[0] = 2;
            int pi = 0;

            for(int i = 2; i < bs.size; i++)
            {
                if(bs.src[i] == ' ' || bs.src[i] == '\n' || i == bs.size - 1)
                {
                    beg[pi + 1] = i + 1;
                    //if its not the end of line without \n then decrease the end index by 1
                    end[pi] = i == bs.size - 1 ? i + 1 : i;

                    Point p{};

                    //parse the point
                    char* pointBegin = bs.src + beg[pi];

                    size_t begi[4], endi[3], ii = 0;
                    begi[0] = 0;

                    for(int j = 0; j < end[pi] - beg[pi]; j++)
                    {
                        if(pointBegin[j] == '/' || pointBegin[j] == ' ' || pointBegin[j] == '\n' || j == end[pi] - beg[pi] - 1)
                        {
                            begi[ii + 1] = j + 1;
                            endi[ii] = j;

                            switch(ii) {
                                case 0:
                                p.vi = std::stoi(std::string(pointBegin + begi[ii], endi[ii] - begi[ii] == 0 ? 1 : endi[ii] - begi[ii]));
                                break;
                                case 1:
                                p.ti = std::stoi(std::string(pointBegin + begi[ii], endi[ii] - begi[ii] == 0 ? 1 : endi[ii] - begi[ii]));
                                break;
                                case 2:
                                p.ni = std::stoi(std::string(pointBegin + begi[ii], endi[ii] - begi[ii] == 0 ? 1 : endi[ii] - begi[ii]));
                                break;
                            }

                            ii++;
                        }
                    }

                    pointsArr[pi] = p;

                    points.insert(std::make_pair(
                        std::string(pointBegin, end[pi] - beg[pi]),
                        std::make_pair(p, -1)
                    ));

                    pi++;
                }
            }

            //TODO: construct the Face from Points. 
            //? add a custom constructor to do so.

            //!make the key just the numbers in order.

            faces.push_back(
                Face{pointsArr[0], pointsArr[1], pointsArr[2]}
            );
        }
    }

    //*buffer creation below.

    //? create the vertex buffer
    std::vector<Point> vBuffer;
    vBuffer.reserve(points.size());

    size_t vertexIndex = 0;
    for(auto& p : points)
    {
        vBuffer.push_back(p.second.first);
        p.second.second = vertexIndex;
        vertexIndex++;
    }

    std::vector<uint32_t> iBuffer;
    iBuffer.reserve(faces.size() * 3);

    for(auto& f : faces)
    {
        Point* pointsArr[3] = { &f.p1, &f.p2, &f.p3 };

        for(int i = 0; i < 3; i++)
        {
            Point& p = *pointsArr[i];

            auto it = points.find(std::to_string(p.vi) + '/' + std::to_string(p.ti) + '/' + std::to_string(p.ni));
            if(it == points.end())
            {
                throw std::runtime_error("Something went kaboom! This is an absolutely random Error. your .obj model file might be cooked.");
            }

            iBuffer.push_back(it->second.second);
        }
    }

    std::vector<float> flatVBuffer;
    flatVBuffer.reserve(vBuffer.size() * 3 * /*size of each vertex in number of floats*/ 8);

    for(const auto& p : vBuffer)
    {
        VertexCoordinates& vc = vertexCoordinates[p.vi - 1];
        TextureCoordinates& tc = textureCoordinates[p.ti - 1];
        NormalCoordinates& nc = normalCoordinates[p.ni - 1];
        flatVBuffer.push_back(vc.x);
        flatVBuffer.push_back(vc.y);
        flatVBuffer.push_back(vc.z);

        flatVBuffer.push_back(tc.x);
        flatVBuffer.push_back(tc.y);

        flatVBuffer.push_back(nc.x);
        flatVBuffer.push_back(nc.y);
        flatVBuffer.push_back(nc.z);
    }

    return std::make_pair(flatVBuffer, iBuffer);
}