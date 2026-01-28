/**
 * @file RegionTree.h
 * @brief Hierarchical Region Tree using Divide and Conquer (QuadTree-style)
 * 
 * ALGORITHM: Divide and Conquer with Hierarchical Decomposition
 * 
 * The RegionTree recursively partitions the image into quadrants,
 * creating a hierarchical structure where:
 * - The root represents the entire image
 * - Each internal node has 4 children (NW, NE, SW, SE quadrants)
 * - Leaf nodes represent fixed-size blocks (MIN_REGION_SIZE × MIN_REGION_SIZE)
 * 
 * DIVIDE STEP:
 *   Split current region into 4 equal quadrants
 * 
 * CONQUER STEP:
 *   Compute statistics for each sub-region and aggregate
 * 
 * COMBINE STEP:
 *   Each internal node stores aggregate info about its subtree
 * 
 * COMPLEXITY ANALYSIS:
 *   - Build Time: O(n²) - each pixel is part of O(log n) nodes but visited O(1) times
 *   - Build Space: O(n²/B² × log n) where B = MIN_REGION_SIZE
 *   - Traversal: O(number of nodes visited), with pruning can be O(log n) for focused queries
 * 
 * WHY QUADTREE?
 *   - Natural spatial decomposition for 2D images
 *   - Enables hierarchical pruning in queries
 *   - Coarse-to-fine analysis
 */

#ifndef REGION_TREE_H
#define REGION_TREE_H

#include "Utils.h"
#include "PrefixSum.h"
#include <vector>
#include <functional>
#include <memory>

namespace SatelliteAnalytics {

/**
 * @struct RegionTreeNode
 * @brief A node in the hierarchical region tree
 * 
 * Each node stores:
 * - Region bounds
 * - Pre-computed statistics (using prefix sums)
 * - Anomaly score and flag
 * - Child pointers (null for leaf nodes)
 */
struct RegionTreeNode {
    int id;                     // Unique node identifier
    Region bounds;              // Region this node represents
    RegionStats stats;          // Pre-computed statistics
    double anomalyScore;        // Deviation from global mean
    bool isAnomaly;             // Flagged as anomalous
    int depth;                  // Depth in tree (root = 0)
    
    // Children: NW=0, NE=1, SW=2, SE=3
    // Using indices into a vector for cache-friendly access
    int children[4];            // -1 if no child
    int parent;                 // -1 if root
    
    bool isLeaf() const {
        return children[0] == -1 && children[1] == -1 && 
               children[2] == -1 && children[3] == -1;
    }
    
    RegionTreeNode() : id(-1), anomalyScore(0), isAnomaly(false), 
                       depth(0), parent(-1) {
        children[0] = children[1] = children[2] = children[3] = -1;
    }
};

/**
 * @class RegionTree
 * @brief QuadTree-style hierarchical decomposition of the image
 * 
 * The tree enables:
 * 1. Hierarchical anomaly detection at multiple scales
 * 2. Efficient top-K queries with pruning
 * 3. Spatial indexing for region queries
 */
class RegionTree {
private:
    std::vector<RegionTreeNode> nodes;  // Flat storage for cache efficiency
    const PrefixSum* prefixSum;         // Pointer to prefix sum engine
    int rootIndex;
    int nodeCount;
    int leafCount;
    int maxDepth;
    int minRegionSize;
    
    // Build statistics
    double buildTimeMs;
    
    /**
     * @brief Recursive divide-and-conquer tree construction
     * @param region Current region to process
     * @param depth Current depth in tree
     * @param parentIdx Parent node index (-1 for root)
     * @return Index of the created node
     * 
     * BASE CASE: Region size <= MIN_REGION_SIZE → create leaf
     * RECURSIVE CASE: Split into 4 quadrants → recurse on each
     */
    int buildRecursive(const Region& region, int depth, int parentIdx);
    
    /**
     * @brief Split a region into 4 quadrants
     * @param region Region to split
     * @param quadrants Output array for the 4 sub-regions
     */
    void splitRegion(const Region& region, Region quadrants[4]) const;
    
    /**
     * @brief Compute statistics for a node using prefix sums
     */
    void computeNodeStats(RegionTreeNode& node);

public:
    RegionTree();
    
    /**
     * @brief Build the hierarchical region tree
     * @param prefixSum Pointer to initialized prefix sum structure
     * @param minSize Minimum region size for leaf nodes
     * 
     * TIME COMPLEXITY: O(n²) - visits each pixel's worth of area O(1) times
     * SPACE COMPLEXITY: O(n²/B²) nodes where B = minRegionSize
     */
    void build(const PrefixSum* prefixSum, int minSize = Config::MIN_REGION_SIZE);
    
    /**
     * @brief Traverse tree with a visitor function
     * @param visitor Function called for each node
     * @param pruneFunc Optional function to prune subtrees
     * 
     * The visitor is called with (node, shouldDescend) where
     * shouldDescend can be set to false to skip children.
     * 
     * TIME COMPLEXITY: O(number of nodes visited)
     */
    void traverse(std::function<void(const RegionTreeNode&, bool&)> visitor) const;
    
    /**
     * @brief Traverse only leaf nodes
     * @param visitor Function called for each leaf
     */
    void traverseLeaves(std::function<void(const RegionTreeNode&)> visitor) const;
    
    /**
     * @brief Get all leaf nodes
     * @return Vector of pointers to leaf nodes
     */
    std::vector<const RegionTreeNode*> getLeaves() const;
    
    /**
     * @brief Get all nodes at a specific depth
     */
    std::vector<const RegionTreeNode*> getNodesAtDepth(int depth) const;
    
    /**
     * @brief Get nodes intersecting a query region
     * @param queryRegion Region to search within
     * @return Vector of leaf nodes that intersect the query
     */
    std::vector<const RegionTreeNode*> queryRegion(const Region& queryRegion) const;
    
    // ========================================================================
    // ACCESSORS
    // ========================================================================
    
    const RegionTreeNode& getRoot() const { return nodes[rootIndex]; }
    const RegionTreeNode& getNode(int index) const { return nodes[index]; }
    RegionTreeNode& getNodeMutable(int index) { return nodes[index]; }
    
    int getNodeCount() const { return nodeCount; }
    int getLeafCount() const { return leafCount; }
    int getMaxDepth() const { return maxDepth; }
    double getBuildTimeMs() const { return buildTimeMs; }
    
    const std::vector<RegionTreeNode>& getAllNodes() const { return nodes; }
    std::vector<RegionTreeNode>& getAllNodesMutable() { return nodes; }
    
    /**
     * @brief Print tree statistics
     */
    void printStats() const;
};

} // namespace SatelliteAnalytics

#endif // REGION_TREE_H
