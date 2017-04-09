#ifndef LAKE_LAKE_HPP
#define LAKE_LAKE_HPP

#include "Error.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <algorithm>
#include <numeric>
#include <iostream>
#include <vector>

template<typename Ptr>
struct PointerComparator {
    bool operator()(Ptr lhs, Ptr rhs) const {
        return *lhs < *rhs;
    }
};

struct Ferry {
    std::size_t from;
    std::size_t to;
    int time;
    int skippedBikeTime;
};

struct Problem {
    int timeLimit;
    std::vector<std::string> cityNames;
    std::vector<int> bikePaths;
    int totalBikePathLength;

    std::vector<Ferry> ferries;
};


std::ostream& operator<<(std::ostream& os, const Problem& p) {
    os << p.cityNames.size() << std::endl;
    for (const auto& city : p.cityNames) {
        os << city << std::endl;
    }
    //os << p.bikePaths.size() << std::endl;
    for (const auto& bikePath : p.bikePaths) {
        os << bikePath << ' ';
    }
    os << std::endl;
    os << p.ferries.size() << std::endl;
    for (const auto& ferry : p.ferries) {
        os << p.cityNames[ferry.from] << ' ' << p.cityNames[ferry.to]
           << ' ' << ferry.time << std::endl;
    }
    os << p.timeLimit;
    return os;
}

bool isFerryBlockingAnother(const Ferry& main, const Ferry& other) {
    return (other.from < main.to && other.to > main.from);
}

bool operator<(const Ferry& lhs, const Ferry& rhs) {
    return lhs.from < rhs.from || (lhs.from == rhs.from && lhs.to < rhs.to);
}

class VertexIterator : public boost::iterator_facade<
        VertexIterator, std::size_t, boost::random_access_traversal_tag,
        std::size_t> {
public:
    VertexIterator() : value(0) {}

    VertexIterator(std::size_t value) : value(value) {}

    std::size_t dereference() const { return value; }

    bool equal(const VertexIterator& other) const {
        return other.value == value;
    }

    void increment() { ++value; }
    void decrement() { --value; }
    void advance(std::ptrdiff_t n) {
        value += n;
    }

    std::ptrdiff_t distance_to(const VertexIterator& other) {
        return other.value - value;
    }

private:
    std::size_t value;
};

using Edge = std::pair<std::size_t, std::size_t>;

class EdgeIterator : public boost::iterator_facade<
        EdgeIterator, Edge, boost::random_access_traversal_tag, Edge> {
private:
    using InnerIterator = std::vector<Ferry>::const_iterator;
public:
    EdgeIterator() {
        // std::cerr << this << ": default construct\n";
    }

    EdgeIterator(InnerIterator begin, InnerIterator iterator,
            std::size_t vertex, std::size_t numVertices) :
            begin(begin), iterator(iterator), vertex(vertex),
            numVertices(numVertices) {
        // std::cerr << this << ": construct: " << std::distance(begin, iterator)
        //         << "\n";
    }

    Edge dereference() const {
        // std::cerr << this << ": dereference: " << std::distance(begin, iterator)
        //         << "\n";
        if (isAtBeginning()) {
            return {vertex, (vertex + 1) % numVertices};
        }
        return {iterator->from, iterator->to};
    }

    bool equal(const EdgeIterator& other) const {
        return other.iterator == iterator;
    }

    void increment() {
        ++iterator;
        // std::cerr << this << ": increment: " << std::distance(begin, iterator)
        //         << "\n";
    }
    void decrement() {
        --iterator;
        // std::cerr << this << ": decrement: " << std::distance(begin, iterator)
        //         << "\n";
    }
    void advance(std::ptrdiff_t n) {
        iterator += n;
        // std::cerr << this << ": advance " << n << ": "
        //         << std::distance(begin, iterator) << "\n";
    }

    std::ptrdiff_t distance_to(const EdgeIterator& other) {
        return std::distance(iterator, other.iterator);
    }

private:
    InnerIterator begin;
    InnerIterator iterator;
    std::size_t vertex;
    std::size_t numVertices;

    bool isAtBeginning() const {
        return std::distance(begin, iterator) < 0;
    }
};

std::size_t num_vertices(const Problem& graph) {
    return graph.bikePaths.size() + 1;
}

auto vertices(const Problem& graph) {
    return std::make_pair(VertexIterator{0},
            VertexIterator{num_vertices(graph)});
}

std::size_t source(const Edge& edge, const Problem&) {
    return edge.first;
}

std::size_t target(const Edge& edge, const Problem& graph) {
    return edge.second != 0 ? edge.second : num_vertices(graph) - 1;
}

struct FerryBeginningComparator {
    bool operator()(const Ferry& ferry, std::size_t vertex) {
        return ferry.from < vertex;
    }

    bool operator()(std::size_t vertex, const Ferry& ferry) {
        return vertex < ferry.from;
    }
};

auto out_edges(std::size_t vertex, const Problem& problem) {
    auto range = std::equal_range(problem.ferries.begin(),
            problem.ferries.end(), vertex, FerryBeginningComparator{});
    int offset = (vertex == num_vertices(problem) - 1) ? 0 : -1;
    return std::make_pair(
            EdgeIterator{range.first, range.first + offset, vertex,
                    num_vertices(problem)},
            EdgeIterator{range.first, range.second, vertex,
                    num_vertices(problem)});
}

std::size_t out_degree(std::size_t vertex, const Problem& problem) {
    auto range = std::equal_range(problem.ferries.begin(),
            problem.ferries.end(), vertex, FerryBeginningComparator{});
    return std::distance(range.first, range.second) + 1;
}

boost::identity_property_map get(boost::vertex_index_t, const Problem&) {
    return {};
}

bool isBikePath(const Edge& edge, const Problem& problem) {
    return target(edge, problem) - source(edge, problem) == 1;
}

const Ferry& getFerry(const Edge& edge, const Problem& problem) {
    auto range = std::equal_range(problem.ferries.begin(),
            problem.ferries.end(), source(edge, problem),
            FerryBeginningComparator{});
    return *std::find_if(range.first, range.second,
            [&edge](const Ferry& ferry) {
                return ferry.to == edge.second;
            });
}

auto get(boost::edge_weight_t, const Problem& problem) {
    return boost::make_function_property_map<Edge>(
            [&problem](const Edge& edge) {
                if (isBikePath(edge, problem)) {
                    return problem.bikePaths[edge.first];
                }
                return getFerry(edge, problem).time;
            });
}

struct DebugVisitor {
    template<typename U, typename G>
    void initialize_vertex(U u, const G& g) {
        printVertex("Initialize", u, g);
    }

    template<typename U, typename G>
    void examine_vertex(U u, const G& g) {
        printVertex("Examine", u, g);
    }

    template<typename E, typename G>
    void examine_edge(E e, const G& g) {
        printEdge("Examine", e, g);
    }

    template<typename U, typename G>
    void discover_vertex(U u, const G& g) {
        printVertex("Discover", u, g);
    }

    template<typename E, typename G>
    void edge_relaxed(E e, const G& g) {
        printEdge("Relaxed", e, g);
    }

    template<typename E, typename G>
    void edge_not_relaxed(E e, const G& g) {
        printEdge("Not relaxed", e, g);
    }

    template<typename U, typename G>
    void finish_vertex(U u, const G& g) {
        printVertex("Finish", u, g);
    }

    template<typename U, typename G>
    void printVertex(const char* label, U u, const G&) {
        std::cerr << label << ": " << u << "\n";
    }

    template<typename E, typename G>
    void printEdge(const char* label, E e, const G& g) {
        std::cerr << label << ": " << source(e, g) << "->" << target(e, g)
                << " w=" << get(get(boost::edge_weight, g), e) << "\n";
    }
};

namespace boost {

template<>
struct graph_traits<Problem> {
    using vertex_descriptor = std::size_t;
    using edge_descriptor = Edge;
    using directed_category = directed_tag;
    using edge_parallel_category = disallow_parallel_edge_tag;
    struct traversal_category : incidence_graph_tag, vertex_list_graph_tag {};
    using out_edge_iterator = EdgeIterator;
    using degree_size_type = std::size_t;
    using vertex_iterator = VertexIterator;
    using vertices_size_type = std::size_t;
};

template<>
struct property_map<Problem, vertex_index_t> {
    using const_type = identity_property_map;
};

template<>
struct property_map<Problem, edge_weight_t> {
    using const_type = decltype(get(edge_weight, std::declval<Problem>()));
};

}

Problem readInput(std::istream& stream) {
    Problem problem;
    std::size_t numCities = 0;
    stream >> numCities;
    problem.cityNames.reserve(numCities);
    std::map<std::string, std::size_t> cityNames;
    for (std::size_t i = 0; i < numCities; ++i) {
        std::string name;
        stream >> name;
        problem.cityNames.push_back(name);
        cityNames.emplace(name, i);
    }
    problem.bikePaths.reserve(numCities);
    problem.totalBikePathLength = 0;
    for (std::size_t i = 0; i < numCities; ++i) {
        int time = 0;
        stream >> time;
        problem.bikePaths.push_back(time);
        problem.totalBikePathLength += time;
    }

    std::size_t numFerries = 0;
    stream >> numFerries;
    for (std::size_t i = 0; i < numFerries; ++i) {
        std::string from, to;
        int time = 0;
        stream >> from >> to >> time;
        std::size_t fromIndex = cityNames.at(from);
        std::size_t toIndex = cityNames.at(to);
        problem.ferries.push_back(Ferry{fromIndex, toIndex, time,
                std::accumulate(
                        problem.bikePaths.begin() + fromIndex,
                        problem.bikePaths.begin() + toIndex, 0)});
    }
    std::sort(problem.ferries.begin(), problem.ferries.end());

    stream >> problem.timeLimit;
    //std::cerr << problem << std::endl << std::endl;
    return problem;
}

template<typename FerryChooser>
class Solver {
public:
    Solver(Problem problem, FerryChooser& ferryChooser,
            int iterationLimit) :
            problem(std::move(problem)), ferryChooser(&ferryChooser),
            iterationLimit(iterationLimit),
            bestBikeTime(0), bestTotalTime(0), bikeTime(0), totalTime(0) {
        ferryChooser.initialize(*this);
        int totalBikeTime = std::accumulate(this->problem.bikePaths.begin(),
                this->problem.bikePaths.end(), 0);
        // std::cerr << "Time of full bike path: " << totalBikeTime
        //         << " time limit: " << problem.timeLimit << "\n";
        if (totalBikeTime <= problem.timeLimit) {
            bestBikeTime = totalBikeTime;
            bestTotalTime = totalBikeTime;
            bestSolution.clear();
        }
    }

    void findShortestPath() {
        if (bestBikeTime != 0 && bestSolution.empty()) {
            std::cerr << "Not finding path: already have a solution.\n";
            return;
        }
        std::vector<std::size_t> predecessors(num_vertices(problem));
        std::vector<int> distances(num_vertices(problem));
        boost::dag_shortest_paths(problem, 0,
                boost::predecessor_map(&predecessors[0])
                .distance_map(&distances[0])
                /* .visitor(DebugVisitor{}) */);
        totalTime = distances.back();
        bikeTime = 0;
        clearUsedFerries();
        for (std::size_t vertex = num_vertices(problem) - 1;
                vertex != 0; vertex = predecessors[vertex]) {
            auto edge = std::make_pair(predecessors[vertex], vertex);
            if (isBikePath(edge, problem)) {
                int weight = get(get(boost::edge_weight, problem), edge);
                bikeTime += weight;
            } else {
                const Ferry& ferry = getFerry(edge, problem);
                // std::cerr << "Using ferry: " << problem.cityNames[ferry.from]
                //         << " -> " << problem.cityNames[ferry.to]
                //         << " t=" << ferry.time
                //         << " bt=" << ferry.skippedBikeTime << "\n";
                auto iterator = usableFerries.lower_bound(&ferry);
                assert(*iterator == &ferry);
                addFerryManageSets(iterator);
            }

        }
        // Shortest path is guarantee to be a solution
        // (assuming at least one solution exists)
        checkNewPath();
    }

    void solve() {
        if (bestBikeTime != 0 && bestSolution.empty()) {
            std::cerr << "Not solving: already have a trivial solution.\n";
            return;
        }
        // TODO: Use time limit.
        for (int i = 0; i < iterationLimit; ++i) {
            // std::cerr << "--- iteratrion #" << i << "\n";
            int notChanged = 0;
            for (int j = 0; j < 1000 && notChanged < 6; ++j) {
                int currentBest = bestBikeTime;
                iteration = std::to_string(i) + "." + std::to_string(j);
                while (totalTime <= problem.timeLimit
                        || usableFerries.size() == 0) {
                    removeFerry();
                }
                while (totalTime > problem.timeLimit &&
                        usableFerries.size() != 0) {
                    addFerry();
                }
                if (currentBest != bestBikeTime) {
                    notChanged = 0;
                } else {
                    ++notChanged;
                }
            }
            clearUsedFerries();
            usedFerries.insert(boost::container::ordered_unique_range,
                    bestSolution.begin(), bestSolution.end());
            for (const Ferry* ferry : usedFerries) {
                removeBlockingFerries(ferry);
            }
            bikeTime = bestBikeTime;
            totalTime = bestTotalTime;
        }
    }

    auto getResult() const {
        std::vector<std::pair<const std::string&, const std::string&>> result;
        result.reserve(bestSolution.size());
        for (const auto& ferry : bestSolution) {
            result.emplace_back(problem.cityNames[ferry->from],
                    problem.cityNames[ferry->to]);
        }
        return result;
    }

    bool checkResult() const {
        int time = 0;
        int bikeTime = 0;
        std::size_t position = 0;
        bool result = true;
        for (const Ferry* ferry : bestSolution) {
            if (ferry->from < position) {
                std::cerr << "Wrong ferry chosen: "
                        << problem.cityNames[ferry->from] << " -> "
                        << problem.cityNames[ferry->to] << "\n";
                result = false;
            }
            for (; position < ferry->from; ++position) {
                time += problem.bikePaths[position];
                bikeTime += problem.bikePaths[position];
            }
            position = ferry->to;
            time += ferry->time;
        }
        for (; position < problem.bikePaths.size(); ++position) {
            time += problem.bikePaths[position];
            bikeTime += problem.bikePaths[position];
        }
        std::cerr << "Calculated time: " << time << " calculated bike time: "
                << bikeTime << "\n";
        if (time != bestTotalTime) {
            std::cerr << "Total time mismatch.\n";
            result = false;
        }
        if (bikeTime != bestBikeTime) {
            std::cerr << "Bike time mismatch.\n";
            result = false;
        }
        return result;
    }

    const Problem& getProblem() const { return problem; }
    int getBestBikeTime() const { return bestBikeTime; }
    int getBestTotalTime() const { return bestTotalTime; }

private:
    using FerrySet = boost::container::flat_set<const Ferry*,
            PointerComparator<const Ferry*>>;

    float getHash() const {
        constexpr int hashSize = 10000000;
        return static_cast<float>(
                std::hash<std::string>{}(iteration) % hashSize) / hashSize;
    }

    void clearUsedFerries() {
        usedFerries.clear();
        usableFerries.clear();
        usableFerries.reserve(problem.ferries.size());
        for (const Ferry& ferry : problem.ferries) {
            usableFerries.insert(&ferry);
        }
    }

    void removeFerryManageSets(FerrySet::iterator removedIterator) {
        auto nextIterator = removedIterator + 1;
        auto begin = (removedIterator == usedFerries.begin())
                ? problem.ferries.begin()
                : std::lower_bound(problem.ferries.begin(),
                        problem.ferries.end(), (*(removedIterator - 1))->to,
                        FerryBeginningComparator{});
        auto end = (nextIterator == usedFerries.end())
                ? problem.ferries.end()
                : std::upper_bound(problem.ferries.begin(),
                        problem.ferries.end(), (*nextIterator)->from,
                        FerryBeginningComparator{});
        const Ferry* next = nextIterator == usedFerries.end()
                ? nullptr : *nextIterator;

        usedFerries.erase(removedIterator);
        for (auto iterator = begin; iterator < end; ++iterator) {
            if (!next || !isFerryBlockingAnother(*next, *iterator)) {
                usableFerries.insert(&*iterator);
                // std::cerr << "Ferry now usable: "
                //         << problem.cityNames[iterator->from] << " -> "
                //         << problem.cityNames[iterator->to] << "\n";
            }
        }
    }

    void addFerryManageSets(FerrySet::iterator iterator) {
        const Ferry* addedFerry = *iterator;
        usableFerries.erase(iterator);
        usedFerries.insert(addedFerry);
        removeBlockingFerries(addedFerry);
    }

    void removeBlockingFerries(const Ferry* addedFerry) {
        std::vector<const Ferry*> currentFerries(usableFerries.size());
        std::copy(usableFerries.begin(), usableFerries.end(),
                currentFerries.begin());
        for (const Ferry* ferry : currentFerries) {
            if (isFerryBlockingAnother(*addedFerry, *ferry)) {
                // std::cerr << "Ferry now unusable: "
                //         << problem.cityNames[ferry->from] << " -> "
                //         << problem.cityNames[ferry->to] << "\n";
                usableFerries.erase(ferry);
            }
        }
    }

    void removeFerry() {
        if (usedFerries.size() == 0) {
            std::cerr << "Cannot remove ferry.\n";
            throwError(Error::NoRemovebleFerries);
        }
        auto iterator = usedFerries.begin()
                + ferryChooser->chooseFerryToRemove(
                        usedFerries, usableFerries, bikeTime, totalTime,
                        getHash());
        const Ferry* ferry = *iterator;

        int newTotalTime = totalTime + ferry->skippedBikeTime - ferry->time;
        //if (totalTime <= problem.timeLimit
        //        && newTotalTime > problem.timeLimit) {
        //    if (bikeTime > bestBikeTime) {
        //        // std::cerr << "Iteration #" << iteration
        //        //         << ": new best solution: bike time = " << bikeTime
        //        //         << " total time = " << totalTime << "\n";
        //        bestSolution.resize(usedFerries.size());
        //        std::copy(usedFerries.begin(), usedFerries.end(),
        //                bestSolution.begin());
        //        bestBikeTime = bikeTime;
        //        bestTotalTime = totalTime;
        //    }
        //}

        bikeTime += ferry->skippedBikeTime;
        totalTime = newTotalTime;
        // std::cerr << "Removing ferry from " << problem.cityNames[ferry->from]
        //         << " to " << problem.cityNames[ferry->to]
        //         << " t=" << totalTime << " bt=" << bikeTime << "\n";
        removeFerryManageSets(iterator);
        checkNewPath();
    }

    void addFerry() {
        if (usableFerries.size() == 0) {
            std::cerr << "Cannot add ferry.\n";
            throwError(Error::NoAddableFerries);
        }
        auto iterator = usableFerries.begin()
                + ferryChooser->chooseFerryToAdd(
                        usedFerries, usableFerries, bikeTime, totalTime,
                        getHash());
        const Ferry* ferry = *iterator;
        bikeTime -= ferry->skippedBikeTime;
        totalTime -= ferry->skippedBikeTime;
        totalTime += ferry->time;
        // std::cerr << "Adding ferry from " << problem.cityNames[ferry->from]
        //         << " to " << problem.cityNames[ferry->to]
        //         << " t=" << totalTime << " bt=" << bikeTime << "\n";
        addFerryManageSets(iterator);
        checkNewPath();
    }

    void checkNewPath() {
        //std::cerr << "checkNewPath(): tt: " << totalTime << " bt: "
        //          << bikeTime << " bbt: " << bestBikeTime << std::endl;
        if (totalTime <= problem.timeLimit &&
                bikeTime > bestBikeTime) {
            std::cerr << "Iteration #" << iteration << ": new best path: t="
                    << totalTime << " bt=" << bikeTime << "\n";
            bestSolution.resize(usedFerries.size());
            std::copy(usedFerries.begin(), usedFerries.end(),
                    bestSolution.begin());
            bestBikeTime = bikeTime;
            bestTotalTime = totalTime;
        }
    }

    Problem problem;
    FerryChooser* ferryChooser;
    const int iterationLimit;
    FerrySet usedFerries;
    FerrySet usableFerries;
    std::vector<const Ferry*> bestSolution;
    int bestBikeTime;
    int bestTotalTime;
    int bikeTime;
    int totalTime;
    std::string iteration;
};


#endif // LAKE_LAKE_HPP
