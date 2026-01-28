/**
 * @file QueryEngine.h
 * @brief Query processing engine with Priority Queue and Graph Algorithms
 * 
 * ALGORITHMS IMPLEMENTED:
 * 
 * 1. TOP-K ANOMALOUS REGIONS (Priority Queue)
 *    - Uses min-heap of size K
 *    - Maintains K highest-scoring regions
 *    - Can prune tree branches if max possible score < min in heap
 *    - TIME: O(n log k) where n = number of regions
 *    - SPACE: O(k) for the heap
 * 
 * 2. LARGEST CONNECTED ANOMALOUS REGION (Union-Find / DFS)
 *    - Identifies adjacent anomalous regions
 *    - Merges them using Union-Find with path compression
 *    - Tracks component sizes to find largest
 *    - TIME: O(n α(n)) ≈ O(n) where α is inverse Ackermann
 *    - SPACE: O(n) for Union-Find arrays
 * 
 * 3. REGION QUERY (Tree Traversal)
 *    - Find all anomalous regions within a query rectangle
 *    - Prunes tree branches that don't intersect query
 *    - TIME: O(log n + k) where k = number of results
 */

#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H

#include "Utils.h"
#include "PrefixSum.h"
#include "RegionTree.h"
#include "AnomalyDetector.h"
#include <queue>
#include <vector>
#include <unordered_map>

namespace SatelliteAnalytics {

/**
 * @struct ConnectedComponent
 * @brief Represents a connected anomalous region (merged from adjacent regions)
 */
struct ConnectedComponent {
    int id;
    std::vector<int> nodeIndices;  // Indices of nodes in this component
    Region boundingBox;            // Bounding box of all regions
    int64_t totalArea;             // Total pixel area
    double maxScore;               // Maximum anomaly score in component
    double avgScore;               // Average anomaly score
    
    ConnectedComponent() : id(-1), totalArea(0), maxScore(0), avgScore(0) {}
};

/**
 * @struct QueryResult
 * @brief Generic result container for queries
 */
struct QueryResult {
    std::vector<AnomalyRegion> regions;
    double queryTimeMs;
    int nodesVisited;
    int nodesPruned;
    
    QueryResult() : queryTimeMs(0), nodesVisited(0), nodesPruned(0) {}
};

/**
 * @class UnionFind
 * @brief Disjoint Set Union (DSU) with path compression and union by rank
 * 
 * ALGORITHM: Union-Find with optimizations
 * 
 * PATH COMPRESSION:
 *   During find(), make all nodes on path point directly to root
 *   This flattens the tree for faster future queries
 * 
 * UNION BY RANK:
 *   Attach smaller tree under larger tree
 *   Keeps tree balanced
 * 
 * TIME COMPLEXITY:
 *   - Find: O(α(n)) amortized, where α = inverse Ackermann (effectively constant)
 *   - Union: O(α(n)) amortized
 *   - Overall for m operations: O(m α(n)) ≈ O(m)
 */
class UnionFind {
private:
    std::vector<int> parent;
    std::vector<int> rank;
    std::vector<int64_t> size;  // Size of each component (in terms of area)
    int numComponents;

public:
    UnionFind(int n);
    
    /**
     * @brief Find root of element with path compression
     */
    int find(int x);
    
    /**
     * @brief Union two elements with union by rank
     * @return true if union occurred (elements were in different sets)
     */
    bool unite(int x, int y);
    
    /**
     * @brief Check if two elements are in same set
     */
    bool connected(int x, int y);
    
    /**
     * @brief Set size for a single-element component
     */
    void setSize(int x, int64_t s);
    
    /**
     * @brief Get size of component containing x
     */
    int64_t getSize(int x);
    
    /**
     * @brief Get number of distinct components
     */
    int getNumComponents() const { return numComponents; }
};

/**
 * @class QueryEngine
 * @brief Executes efficient queries on the analyzed region tree
 */
class QueryEngine {
private:
    const RegionTree* regionTree;
    const PrefixSum* prefixSum;
    const AnomalyDetector* detector;
    
    /**
     * @brief Check if two regions are adjacent (share edge or corner)
     */
    bool areAdjacent(const Region& a, const Region& b) const;
    
    /**
     * @brief Check if regions intersect
     */
    bool regionsIntersect(const Region& a, const Region& b) const;
    
    /**
     * @brief Compute bounding box of two regions
     */
    Region mergeBounds(const Region& a, const Region& b) const;

public:
    QueryEngine();
    
    /**
     * @brief Initialize the query engine
     */
    void initialize(const RegionTree* tree, const PrefixSum* prefix, 
                   const AnomalyDetector* detector);
    
    // ========================================================================
    // TOP-K QUERIES (PRIORITY QUEUE)
    // ========================================================================
    
    /**
     * @brief Find top-K most anomalous regions
     * @param k Number of regions to return
     * @param leafOnly If true, only consider leaf nodes
     * @return QueryResult with top-K regions sorted by score (descending)
     * 
     * ALGORITHM:
     *   1. Initialize min-heap of capacity K
     *   2. Traverse tree (can prune if node's max score < heap's min)
     *   3. For each node:
     *      - If heap size < K: push node
     *      - Else if node.score > heap.top().score: pop top, push node
     *   4. Extract heap contents and sort descending
     * 
     * TIME COMPLEXITY: O(n log k)
     * SPACE COMPLEXITY: O(k)
     */
    QueryResult topKAnomalies(int k = Config::DEFAULT_TOP_K, bool leafOnly = true);
    
    /**
     * @brief Find top-K with pruning optimization
     * 
     * Enhanced version that prunes tree branches where the maximum
     * possible anomaly score is less than the current minimum in heap.
     * 
     * TIME COMPLEXITY: O(log n + k log k) best case with good pruning
     */
    QueryResult topKWithPruning(int k = Config::DEFAULT_TOP_K);
    
    // ========================================================================
    // CONNECTED COMPONENT QUERIES (UNION-FIND / DFS)
    // ========================================================================
    
    /**
     * @brief Find all connected components of anomalous regions
     * @return Vector of connected components
     * 
     * ALGORITHM:
     *   1. Get all anomalous leaf nodes
     *   2. Initialize Union-Find with n elements
     *   3. For each pair of adjacent anomalous nodes: union them
     *   4. Group nodes by their root to form components
     * 
     * TIME COMPLEXITY: O(n² × α(n)) for pairwise adjacency, O(n log n) with spatial indexing
     * SPACE COMPLEXITY: O(n)
     */
    std::vector<ConnectedComponent> findConnectedComponents();
    
    /**
     * @brief Find the largest connected anomalous region
     * @return The largest component (by area)
     */
    ConnectedComponent findLargestConnectedRegion();
    
    /**
     * @brief Find connected components using DFS
     * @return Vector of connected components
     * 
     * Alternative to Union-Find approach using graph DFS.
     * TIME COMPLEXITY: O(n + m) where m = number of edges (adjacencies)
     */
    std::vector<ConnectedComponent> findConnectedComponentsDFS();
    
    // ========================================================================
    // REGION QUERIES (TREE TRAVERSAL WITH PRUNING)
    // ========================================================================
    
    /**
     * @brief Find all anomalous regions within a query rectangle
     * @param queryRegion Rectangle to search within
     * @return QueryResult with matching regions
     * 
     * Uses tree structure to prune non-intersecting branches.
     * TIME COMPLEXITY: O(log n + k) where k = number of results
     */
    QueryResult queryRectangle(const Region& queryRegion);
    
    /**
     * @brief Get statistics for a query region
     */
    RegionStats queryRegionStats(const Region& region);
    
    // ========================================================================
    // UTILITY
    // ========================================================================
    
    /**
     * @brief Count total anomalous regions
     */
    int countAnomalousRegions() const;
    
    /**
     * @brief Get total anomalous area (pixels)
     */
    int64_t getTotalAnomalousArea() const;
};

} // namespace SatelliteAnalytics

#endif // QUERY_ENGINE_H
