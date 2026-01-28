/**
 * @file QueryEngine.cpp
 * @brief Implementation of Query Processing with Priority Queue and Union-Find
 * 
 * This file contains two major algorithms:
 * 
 * 1. TOP-K QUERIES using Priority Queue (Min-Heap)
 * 2. CONNECTED COMPONENT DETECTION using Union-Find with Path Compression
 * 
 * Both are fundamental algorithms in Design and Analysis of Algorithms.
 */

#include "QueryEngine.h"
#include <algorithm>
#include <queue>
#include <stack>
#include <unordered_set>
#include <iostream>
#include <cmath>

namespace SatelliteAnalytics {

// ============================================================================
// UNION-FIND (DISJOINT SET UNION) IMPLEMENTATION
// ============================================================================

/**
 * UNION-FIND DATA STRUCTURE
 * =========================
 * 
 * Union-Find (also called Disjoint Set Union) efficiently handles two operations:
 * 
 * 1. FIND(x): Determine which set x belongs to (find the root)
 * 2. UNION(x, y): Merge the sets containing x and y
 * 
 * OPTIMIZATIONS IMPLEMENTED:
 * 
 * 1. PATH COMPRESSION (in find):
 *    When finding the root, make every node on the path point directly to root.
 *    This flattens the tree structure for faster future queries.
 * 
 *    Before: a -> b -> c -> d (root)
 *    After:  a -> d, b -> d, c -> d (all point to root)
 * 
 * 2. UNION BY RANK:
 *    When merging two trees, attach the smaller tree under the larger one.
 *    This keeps trees balanced and limits height to O(log n).
 * 
 * COMPLEXITY with both optimizations:
 *    - Single operation: O(α(n)) amortized, where α is inverse Ackermann
 *    - α(n) < 5 for any practical n (grows incredibly slowly)
 *    - Effectively O(1) amortized per operation
 */

UnionFind::UnionFind(int n) : numComponents(n) {
    parent.resize(n);
    rank.resize(n, 0);
    size.resize(n, 1);
    
    // Initially, each element is its own parent (self-loop)
    for (int i = 0; i < n; i++) {
        parent[i] = i;
    }
}

int UnionFind::find(int x) {
    /**
     * PATH COMPRESSION:
     * 
     * Recursively find the root, then make x point directly to it.
     * This flattens the tree structure, making future finds O(1).
     * 
     * Iterative version with path compression:
     */
    if (parent[x] != x) {
        parent[x] = find(parent[x]);  // Path compression: point to root
    }
    return parent[x];
}

bool UnionFind::unite(int x, int y) {
    /**
     * UNION BY RANK:
     * 
     * Always attach the tree with lower rank under the tree with higher rank.
     * Rank is an upper bound on tree height.
     * 
     * This keeps trees balanced:
     * - If ranks differ: attach smaller under larger, keep larger rank
     * - If ranks equal: arbitrary choice, but increment winner's rank
     */
    int rootX = find(x);
    int rootY = find(y);
    
    if (rootX == rootY) {
        return false;  // Already in same set
    }
    
    // Union by rank: attach smaller tree under larger
    if (rank[rootX] < rank[rootY]) {
        parent[rootX] = rootY;
        size[rootY] += size[rootX];
    } else if (rank[rootX] > rank[rootY]) {
        parent[rootY] = rootX;
        size[rootX] += size[rootY];
    } else {
        parent[rootY] = rootX;
        size[rootX] += size[rootY];
        rank[rootX]++;
    }
    
    numComponents--;
    return true;
}

bool UnionFind::connected(int x, int y) {
    return find(x) == find(y);
}

void UnionFind::setSize(int x, int64_t s) {
    size[find(x)] = s;
}

int64_t UnionFind::getSize(int x) {
    return size[find(x)];
}

// ============================================================================
// QUERY ENGINE IMPLEMENTATION
// ============================================================================

QueryEngine::QueryEngine() 
    : regionTree(nullptr), prefixSum(nullptr), detector(nullptr) {}

void QueryEngine::initialize(const RegionTree* tree, const PrefixSum* prefix,
                            const AnomalyDetector* det) {
    regionTree = tree;
    prefixSum = prefix;
    detector = det;
}

bool QueryEngine::areAdjacent(const Region& a, const Region& b) const {
    // Check if regions share an edge (orthogonally adjacent)
    bool xOverlap = (a.col1 <= b.col2 && b.col1 <= a.col2);
    bool yOverlap = (a.row1 <= b.row2 && b.row1 <= a.row2);
    
    // Horizontally adjacent (share vertical edge)
    if (yOverlap && (a.col2 + 1 == b.col1 || b.col2 + 1 == a.col1)) {
        return true;
    }
    
    // Vertically adjacent (share horizontal edge)
    if (xOverlap && (a.row2 + 1 == b.row1 || b.row2 + 1 == a.row1)) {
        return true;
    }
    
    return false;
}

bool QueryEngine::regionsIntersect(const Region& a, const Region& b) const {
    return !(a.row2 < b.row1 || a.row1 > b.row2 ||
             a.col2 < b.col1 || a.col1 > b.col2);
}

Region QueryEngine::mergeBounds(const Region& a, const Region& b) const {
    return Region(
        std::min(a.row1, b.row1),
        std::min(a.col1, b.col1),
        std::max(a.row2, b.row2),
        std::max(a.col2, b.col2)
    );
}

// ============================================================================
// TOP-K QUERIES (PRIORITY QUEUE / HEAP)
// ============================================================================

QueryResult QueryEngine::topKAnomalies(int k, bool leafOnly) {
    /**
     * TOP-K ALGORITHM using MIN-HEAP
     * ==============================
     * 
     * Problem: Find the K regions with highest anomaly scores
     * 
     * Naive approach: Sort all regions by score O(n log n), take top K
     * 
     * Better approach: Use a min-heap of size K
     * 
     * ALGORITHM:
     * 1. Maintain a min-heap of size at most K
     * 2. For each region:
     *    - If heap has < K elements: push region
     *    - Else if region.score > heap.top().score: pop top, push region
     * 3. The heap now contains the K highest-scoring regions
     * 4. Extract and reverse to get descending order
     * 
     * WHY MIN-HEAP?
     * - We keep track of the K largest elements
     * - The smallest of these K is at the top (min-heap property)
     * - If a new element is larger than the smallest, it should replace it
     * 
     * TIME COMPLEXITY: O(n log k)
     *   - Process n regions
     *   - Each heap operation is O(log k)
     *   - Much better than O(n log n) when k << n
     * 
     * SPACE COMPLEXITY: O(k)
     */
    
    Timer timer;
    timer.start();
    
    QueryResult result;
    if (!regionTree) return result;
    
    // Min-heap: smallest score at top
    // Using std::greater for min-heap behavior
    std::priority_queue<AnomalyRegion, std::vector<AnomalyRegion>,
                       std::greater<AnomalyRegion>> minHeap;
    
    const auto& nodes = regionTree->getAllNodes();
    
    for (const auto& node : nodes) {
        // Skip non-leaf nodes if requested
        if (leafOnly && !node.isLeaf()) continue;
        
        result.nodesVisited++;
        
        AnomalyRegion ar(node.bounds, node.anomalyScore, node.id);
        
        if (static_cast<int>(minHeap.size()) < k) {
            // Heap not full yet, just push
            minHeap.push(ar);
        } else if (ar.anomalyScore > minHeap.top().anomalyScore) {
            // New region has higher score than current minimum in top-K
            minHeap.pop();
            minHeap.push(ar);
        }
        // Else: skip this region (not in top-K)
    }
    
    // Extract results from heap (will be in ascending order)
    while (!minHeap.empty()) {
        result.regions.push_back(minHeap.top());
        minHeap.pop();
    }
    
    // Reverse to get descending order (highest score first)
    std::reverse(result.regions.begin(), result.regions.end());
    
    timer.stop();
    result.queryTimeMs = timer.elapsedMs();
    
    return result;
}

QueryResult QueryEngine::topKWithPruning(int k) {
    /**
     * TOP-K WITH TREE PRUNING
     * =======================
     * 
     * Enhancement: Use tree structure to skip entire subtrees.
     * 
     * If a subtree's maximum possible score is less than the current
     * minimum in our top-K heap, we can skip the entire subtree.
     * 
     * This is similar to branch-and-bound optimization.
     * 
     * BEST CASE: O(k log k) if we can prune most of the tree
     * WORST CASE: O(n log k) if no pruning possible
     */
    
    Timer timer;
    timer.start();
    
    QueryResult result;
    if (!regionTree) return result;
    
    std::priority_queue<AnomalyRegion, std::vector<AnomalyRegion>,
                       std::greater<AnomalyRegion>> minHeap;
    
    std::queue<int> toVisit;
    toVisit.push(0);  // Start from root
    
    const auto& nodes = regionTree->getAllNodes();
    
    while (!toVisit.empty()) {
        int idx = toVisit.front();
        toVisit.pop();
        
        if (idx < 0 || idx >= static_cast<int>(nodes.size())) continue;
        
        const auto& node = nodes[idx];
        result.nodesVisited++;
        
        // Pruning condition: if heap is full and this node's score
        // is less than the minimum in heap, skip subtree
        bool canPrune = static_cast<int>(minHeap.size()) >= k && 
                       node.anomalyScore < minHeap.top().anomalyScore;
        
        if (canPrune && !node.isLeaf()) {
            result.nodesPruned++;
            continue;  // Skip this subtree
        }
        
        if (node.isLeaf()) {
            AnomalyRegion ar(node.bounds, node.anomalyScore, node.id);
            
            if (static_cast<int>(minHeap.size()) < k) {
                minHeap.push(ar);
            } else if (ar.anomalyScore > minHeap.top().anomalyScore) {
                minHeap.pop();
                minHeap.push(ar);
            }
        } else {
            // Add children to visit queue
            for (int i = 0; i < 4; i++) {
                if (node.children[i] >= 0) {
                    toVisit.push(node.children[i]);
                }
            }
        }
    }
    
    // Extract and reverse results
    while (!minHeap.empty()) {
        result.regions.push_back(minHeap.top());
        minHeap.pop();
    }
    std::reverse(result.regions.begin(), result.regions.end());
    
    timer.stop();
    result.queryTimeMs = timer.elapsedMs();
    
    return result;
}

// ============================================================================
// CONNECTED COMPONENT DETECTION (UNION-FIND)
// ============================================================================

std::vector<ConnectedComponent> QueryEngine::findConnectedComponents() {
    /**
     * CONNECTED COMPONENTS using UNION-FIND
     * =====================================
     * 
     * Problem: Find all connected groups of anomalous regions
     * 
     * Two regions are considered connected if they are adjacent (share an edge).
     * 
     * ALGORITHM:
     * 1. Get all anomalous leaf nodes
     * 2. Create Union-Find with n elements (one per anomalous node)
     * 3. For each pair of adjacent anomalous nodes: UNION them
     * 4. Group nodes by their FIND (root) to form components
     * 
     * TIME COMPLEXITY: O(n² × α(n)) for pairwise adjacency check
     *   - The α(n) factor is from Union-Find operations
     *   - α(n) is effectively constant (< 5 for any practical n)
     * 
     * Could be optimized to O(n log n) using spatial indexing.
     */
    
    std::vector<ConnectedComponent> components;
    if (!regionTree) return components;
    
    // Get all anomalous leaf nodes
    std::vector<const RegionTreeNode*> anomalousNodes;
    auto leaves = regionTree->getLeaves();
    
    for (const auto* leaf : leaves) {
        if (leaf->isAnomaly) {
            anomalousNodes.push_back(leaf);
        }
    }
    
    int n = anomalousNodes.size();
    if (n == 0) return components;
    
    // Create Union-Find structure
    UnionFind uf(n);
    
    // Set initial sizes (area of each region)
    for (int i = 0; i < n; i++) {
        uf.setSize(i, anomalousNodes[i]->bounds.area());
    }
    
    // Check all pairs for adjacency - O(n²)
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (areAdjacent(anomalousNodes[i]->bounds, anomalousNodes[j]->bounds)) {
                // Merge these two regions into the same component
                if (uf.unite(i, j)) {
                    // Update the size to be sum of areas
                    int root = uf.find(i);
                    // Size is already updated in unite()
                }
            }
        }
    }
    
    // Group nodes by their root
    std::unordered_map<int, std::vector<int>> rootToNodes;
    for (int i = 0; i < n; i++) {
        int root = uf.find(i);
        rootToNodes[root].push_back(i);
    }
    
    // Build component structures
    int componentId = 0;
    for (const auto& [root, nodeIndices] : rootToNodes) {
        ConnectedComponent comp;
        comp.id = componentId++;
        comp.totalArea = 0;
        comp.maxScore = 0;
        double scoreSum = 0;
        
        // Initialize bounding box
        comp.boundingBox = anomalousNodes[nodeIndices[0]]->bounds;
        
        for (int idx : nodeIndices) {
            const auto* node = anomalousNodes[idx];
            comp.nodeIndices.push_back(node->id);
            comp.totalArea += node->bounds.area();
            comp.maxScore = std::max(comp.maxScore, node->anomalyScore);
            scoreSum += node->anomalyScore;
            
            // Expand bounding box
            comp.boundingBox = mergeBounds(comp.boundingBox, node->bounds);
        }
        
        comp.avgScore = scoreSum / nodeIndices.size();
        components.push_back(comp);
    }
    
    // Sort by total area (descending)
    std::sort(components.begin(), components.end(),
              [](const ConnectedComponent& a, const ConnectedComponent& b) {
                  return a.totalArea > b.totalArea;
              });
    
    return components;
}

ConnectedComponent QueryEngine::findLargestConnectedRegion() {
    auto components = findConnectedComponents();
    
    if (components.empty()) {
        return ConnectedComponent();
    }
    
    // Already sorted by area, so first is largest
    return components[0];
}

std::vector<ConnectedComponent> QueryEngine::findConnectedComponentsDFS() {
    /**
     * ALTERNATIVE: DFS-based Connected Components
     * ===========================================
     * 
     * This is the classic graph DFS approach to finding connected components.
     * 
     * ALGORITHM:
     * 1. Build adjacency list for anomalous nodes
     * 2. For each unvisited node, run DFS to explore component
     * 3. All nodes reached in one DFS form a component
     * 
     * TIME COMPLEXITY: O(n + m) where m = number of edges (adjacencies)
     * SPACE COMPLEXITY: O(n) for visited array and stack
     */
    
    std::vector<ConnectedComponent> components;
    if (!regionTree) return components;
    
    std::vector<const RegionTreeNode*> anomalousNodes;
    auto leaves = regionTree->getLeaves();
    
    for (const auto* leaf : leaves) {
        if (leaf->isAnomaly) {
            anomalousNodes.push_back(leaf);
        }
    }
    
    int n = anomalousNodes.size();
    if (n == 0) return components;
    
    // Build adjacency list
    std::vector<std::vector<int>> adj(n);
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (areAdjacent(anomalousNodes[i]->bounds, anomalousNodes[j]->bounds)) {
                adj[i].push_back(j);
                adj[j].push_back(i);
            }
        }
    }
    
    // DFS to find components
    std::vector<bool> visited(n, false);
    int componentId = 0;
    
    for (int start = 0; start < n; start++) {
        if (visited[start]) continue;
        
        // DFS from this node
        ConnectedComponent comp;
        comp.id = componentId++;
        comp.totalArea = 0;
        comp.maxScore = 0;
        double scoreSum = 0;
        
        std::stack<int> stack;
        stack.push(start);
        
        bool first = true;
        
        while (!stack.empty()) {
            int u = stack.top();
            stack.pop();
            
            if (visited[u]) continue;
            visited[u] = true;
            
            const auto* node = anomalousNodes[u];
            comp.nodeIndices.push_back(node->id);
            comp.totalArea += node->bounds.area();
            comp.maxScore = std::max(comp.maxScore, node->anomalyScore);
            scoreSum += node->anomalyScore;
            
            if (first) {
                comp.boundingBox = node->bounds;
                first = false;
            } else {
                comp.boundingBox = mergeBounds(comp.boundingBox, node->bounds);
            }
            
            // Visit neighbors
            for (int v : adj[u]) {
                if (!visited[v]) {
                    stack.push(v);
                }
            }
        }
        
        comp.avgScore = scoreSum / comp.nodeIndices.size();
        components.push_back(comp);
    }
    
    // Sort by total area
    std::sort(components.begin(), components.end(),
              [](const ConnectedComponent& a, const ConnectedComponent& b) {
                  return a.totalArea > b.totalArea;
              });
    
    return components;
}

// ============================================================================
// REGION QUERIES
// ============================================================================

QueryResult QueryEngine::queryRectangle(const Region& queryRegion) {
    Timer timer;
    timer.start();
    
    QueryResult result;
    if (!regionTree) return result;
    
    auto matchingNodes = regionTree->queryRegion(queryRegion);
    
    for (const auto* node : matchingNodes) {
        result.nodesVisited++;
        if (node->isAnomaly) {
            result.regions.emplace_back(node->bounds, node->anomalyScore, node->id);
        }
    }
    
    // Sort by score descending
    std::sort(result.regions.begin(), result.regions.end(),
              [](const AnomalyRegion& a, const AnomalyRegion& b) {
                  return a.anomalyScore > b.anomalyScore;
              });
    
    timer.stop();
    result.queryTimeMs = timer.elapsedMs();
    
    return result;
}

RegionStats QueryEngine::queryRegionStats(const Region& region) {
    if (!prefixSum) return RegionStats();
    return prefixSum->queryStats(region);
}

int QueryEngine::countAnomalousRegions() const {
    if (!regionTree) return 0;
    
    int count = 0;
    auto leaves = regionTree->getLeaves();
    for (const auto* leaf : leaves) {
        if (leaf->isAnomaly) count++;
    }
    return count;
}

int64_t QueryEngine::getTotalAnomalousArea() const {
    if (!regionTree) return 0;
    
    int64_t total = 0;
    auto leaves = regionTree->getLeaves();
    for (const auto* leaf : leaves) {
        if (leaf->isAnomaly) {
            total += leaf->bounds.area();
        }
    }
    return total;
}

} // namespace SatelliteAnalytics
