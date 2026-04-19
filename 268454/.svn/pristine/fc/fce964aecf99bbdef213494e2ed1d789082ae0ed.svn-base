#include <vector>
#include <queue>
#include <tuple>
#include <memory>
#include <utility>
#include <iostream>

#include <GraphAL.hpp>

GraphAL::GraphAL(int vertices) : numberOfVertices_(vertices), adjList_(vertices), vertices_(vertices) {}

void GraphAL::addEdge(int u, int v){
    adjList_[u].emplace_back(v);
    adjList_[v].emplace_back(u);
}

int GraphAL::getNumberOfVertices() const{
    return numberOfVertices_;
}

std::pair<int,int> GraphAL::floading(int source){
    std::deque<int> messages;
    messages.insert(messages.end(), adjList_[source].begin(), adjList_[source].end());
    vertices_[source].seenMessage = true;
    vertices_[source].depth = 0;
    std::deque<int> new_messages;
    int roundsCounter = 0;
    int messagesCounter = 0;
    while (!messages.empty()) {
        roundsCounter++;
        while (!messages.empty()) {
            int current = messages.front();
            messages.pop_front();
            // std::cout << "Round: " << roundsCounter << ", current: " << current << std::endl;
            messagesCounter++;
            if (vertices_[current].seenMessage) {
                continue;
            }
            vertices_[current].depth = roundsCounter;
            vertices_[current].seenMessage = true;
            new_messages.insert(new_messages.end(), adjList_[current].begin(), adjList_[current].end());
        }
        messages = std::move(new_messages);
        new_messages.clear();
    }

    return std::make_pair(roundsCounter, messagesCounter);
}

std::pair<int,int> GraphAL::floadingBFS(int source){
    std::deque<std::pair<int,int>> messages;
    for (int neighbor : adjList_[source]) {
        messages.emplace_back(source, neighbor);
    }
    vertices_[source].seenMessage = true;
    vertices_[source].depth = 0;
    std::deque<std::pair<int,int>> new_messages;
    int roundsCounter = 0;
    int messagesCounter = 0;
    while (!messages.empty()) {
        roundsCounter++;
        while (!messages.empty()) {
            std::pair<int,int> message = messages.front();
            messages.pop_front();
            // std::cout << "Round: " << roundsCounter << ", current: " << current << std::endl;
            messagesCounter++;
            if (vertices_[message.second].seenMessage) {
                continue;
            }
            vertices_[message.second].depth = roundsCounter;
            vertices_[message.second].parent = message.first;
            vertices_[message.second].seenMessage = true;
            for (int neighbor : adjList_[message.second]) {
                new_messages.emplace_back(message.second, neighbor);
            }
        }
        messages = std::move(new_messages);
        new_messages.clear();
    }

    return std::make_pair(roundsCounter, messagesCounter);
}

std::pair<int,int> GraphAL::asynchronicFloadingBFS(int source){
    std::priority_queue<
    std::tuple<int, int, int>, 
    std::vector<std::tuple<int, int, int>>, 
    std::greater<std::tuple<int, int, int>>
    > messages;
    for (int neighbor : adjList_[source]) {
        int delay = rand() % 5 + 1;
        messages.emplace(0+delay, source, neighbor);
    }
    vertices_[source].seenMessage = true;
    vertices_[source].depth = 0;
    vertices_[source].timeReceived = 0;
    int messagesCounter = 0;
    int timeCounter = 0;    
    while (!messages.empty()) {
        std::tuple<int,int,int> message = messages.top();
        messages.pop();
        messagesCounter++;
        timeCounter = std::get<0>(message);
        std::cout << "Time: " << timeCounter << ", from: " << std::get<1>(message) << ", to: " << std::get<2>(message) << std::endl;
        if (vertices_[std::get<2>(message)].seenMessage) {
            continue;
        }
        vertices_[std::get<2>(message)].depth = vertices_[std::get<1>(message)].depth + 1;
        vertices_[std::get<2>(message)].parent = std::get<1>(message);
        vertices_[std::get<2>(message)].seenMessage = true;
        vertices_[std::get<2>(message)].timeReceived = std::get<0>(message);
        for (int neighbor : adjList_[std::get<2>(message)]) {
            int delay = rand() % 5 + 1;
            messages.emplace(std::get<0>(message) + delay, std::get<2>(message), neighbor);
        }
    }

    return std::make_pair(timeCounter, messagesCounter);
}