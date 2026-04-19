#include <iostream>
#include "GraphAL.hpp"

void reportFlood(GraphAL& graph, int source, const std::string& graphName) {
    auto [rounds, messages] = graph.asynchronicFloadingBFS(source);
    std::cout << graphName << ": Rounds = " << rounds << ", Messages = " << messages << std::endl;
    for (int i = 0; i < graph.getNumberOfVertices(); ++i) {
        std::cout << "Node "<< i <<" depth: " << graph.vertices_.at(i).depth <<" parent: "<< graph.vertices_.at(i).parent << " time: " << graph.vertices_.at(i).timeReceived << std::endl;
    }
}

int main() {
    int n = 9;
    GraphAL lineGraph(n);
    for (int i = 0; i < n - 1; ++i) {
        lineGraph.addEdge(i, i + 1);
    }
    reportFlood(lineGraph, 0, "Path");

    GraphAL cycleGraph(n);
    for (int i = 0; i < n; ++i) {
        cycleGraph.addEdge(i, (i + 1) % n);
    }
    reportFlood(cycleGraph, 0, "Cycle");

    GraphAL treeGraph(n);
    for (int i = 0; i < n / 2; ++i) {
        treeGraph.addEdge(i, 2 * i + 1);
        treeGraph.addEdge(i, 2 * i + 2);
    }
    reportFlood(treeGraph, 0, "Binary tree");

    GraphAL cliqueGraph(n);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            cliqueGraph.addEdge(i, j);
        }
    }
    reportFlood(cliqueGraph, 0, "Clique");

    return 0;
}
