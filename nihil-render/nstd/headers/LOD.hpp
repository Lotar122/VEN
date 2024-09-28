#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <limits>

namespace nihil::nstd {
    // Mesh simplification function
    std::pair<std::vector<float>, std::vector<uint32_t>> simplifyMesh(std::vector<float>& _vertices, std::vector<uint32_t>& _edges) {
        std::cout << "run" << std::endl;

        struct Vertex {
            float x, y, z;
            std::vector<int> connectedVertices;
        };

        struct Edge {
            int v1, v2;
            float cost;
            bool removed;
        };

        std::vector<Vertex> vertices;
        vertices.reserve((_vertices.size() / 11));

        for (int i = 0; i < _vertices.size(); i += 11)
        {
            Vertex vert = {};
            vert.x = _vertices[i];
            vert.y = _vertices[i + 1];
            vert.z = _vertices[i + 2];

            vertices.push_back(vert);
        }

        std::vector<Edge> edges;
        edges.reserve((_edges.size() / 11));

        for (int i = 0; i < _edges.size(); i += 3) {
            int v1Idx = _edges[i];
            int v2Idx = _edges[i + 1];
            int v3Idx = _edges[i + 2];

            edges.push_back({ v1Idx, v2Idx, 0.0f, false });
            edges.push_back({ v2Idx, v3Idx, 0.0f, false });
            edges.push_back({ v3Idx, v1Idx, 0.0f, false });
        }

        // Calculate edge collapse costs
        for (Edge& edge : edges) {
            // Calculate cost (e.g., distance between vertices)
            // For simplicity, let's assume the cost is the Euclidean distance between vertices
            Vertex& v1 = vertices[edge.v1];
            Vertex& v2 = vertices[edge.v2];
            edge.cost = sqrt((v1.x - v2.x) * (v1.x - v2.x) +
                (v1.y - v2.y) * (v1.y - v2.y) +
                (v1.z - v2.z) * (v1.z - v2.z));
        }

        // Simplification loop
        while (!edges.empty()) {
            // Find the edge with the smallest cost
            float minCost = std::numeric_limits<float>::max();
            int minCostEdgeIdx = -1;
            for (int i = 0; i < edges.size(); ++i) {
                if (!edges[i].removed && edges[i].cost < minCost) {
                    minCost = edges[i].cost;
                    minCostEdgeIdx = i;
                }
            }

            if (minCostEdgeIdx == -1) {
                // No valid edge found, exit loop
                break;
            }

            // Collapse the edge
            Edge& minCostEdge = edges[minCostEdgeIdx];
            Vertex& v1 = vertices[minCostEdge.v1];
            Vertex& v2 = vertices[minCostEdge.v2];

            // Update the position of v1 to the midpoint of v1 and v2
            v1.x = (v1.x + v2.x) / 2;
            v1.y = (v1.y + v2.y) / 2;
            v1.z = (v1.z + v2.z) / 2;

            // Update neighboring edges and vertices
            for (int i = 0; i < v2.connectedVertices.size(); ++i) {
                if (v2.connectedVertices[i] != minCostEdge.v1) {
                    // Update connected vertices to point to v1 instead of v2
                    vertices[v2.connectedVertices[i]].connectedVertices.push_back(minCostEdge.v1);
                }
            }
            v1.connectedVertices.insert(v1.connectedVertices.end(), v2.connectedVertices.begin(), v2.connectedVertices.end());

            // Remove collapsed vertex
            v2.connectedVertices.clear();

            // Mark the edge as removed
            minCostEdge.removed = true;

            // Update neighboring edges and their costs
            for (int i = 0; i < v1.connectedVertices.size(); ++i) {
                int connectedVertexIdx = v1.connectedVertices[i];
                for (int j = 0; j < edges.size(); ++j) {
                    Edge& currentEdge = edges[j];
                    if (!currentEdge.removed && (currentEdge.v1 == connectedVertexIdx || currentEdge.v2 == connectedVertexIdx)) {
                        // Recalculate cost for neighboring edges connected to the collapsed vertex v2
                        Vertex& connectedVertex = vertices[connectedVertexIdx];
                        float newCost = sqrt((v1.x - connectedVertex.x) * (v1.x - connectedVertex.x) +
                            (v1.y - connectedVertex.y) * (v1.y - connectedVertex.y) +
                            (v1.z - connectedVertex.z) * (v1.z - connectedVertex.z));
                        currentEdge.cost = newCost;

                        // Update the vertex index to point to v1 instead of v2
                        if (currentEdge.v1 == connectedVertexIdx) {
                            currentEdge.v1 = minCostEdge.v1;
                        }
                        else {
                            currentEdge.v2 = minCostEdge.v1;
                        }
                    }
                }
            }
        }

        std::vector<float> vertexBuffer;
        std::vector<uint32_t> indexBuffer;

        // Iterate over vertices and add their positions to the vertex buffer
        for (const Vertex& vertex : vertices) {
            vertexBuffer.push_back(vertex.x);
            vertexBuffer.push_back(vertex.y);
            vertexBuffer.push_back(vertex.z);
        }

        // Iterate over edges and add vertex indices to the index buffer
        for (const Edge& edge : edges) {
            // Add vertex indices for each edge
            indexBuffer.push_back(edge.v1);
            indexBuffer.push_back(edge.v2);
        }

        return std::make_pair(vertexBuffer, indexBuffer);
    }
}
