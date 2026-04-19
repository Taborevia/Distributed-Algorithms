#pragma once
#include <vector>
#include <deque>
#include <memory>
#include <utility>

struct Node {
    bool seenMessage = false;
    int depth = -1;
    int parent = -1;
    int timeReceived = -1;
};

class GraphAL {
public:
    GraphAL(int vertices);

    void addEdge(int u, int v);
    int getNumberOfVertices() const;
    std::pair<int,int> floading(int source);
    std::pair<int,int> floadingBFS(int source);
    std::pair<int,int> asynchronicFloadingBFS(int source);

private:
    int numberOfVertices_;
    std::vector<std::vector<int>> adjList_;
public:
    std::vector<Node> vertices_;
};