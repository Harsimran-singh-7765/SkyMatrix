/**
 * @file RegionTree.cpp
 * @brief Implementation of Hierarchical Region Tree (QuadTree) using Divide and Conquer
 * 
 * ALGORITHM: DIVIDE AND CONQUER
 * =============================
 * 
 * The QuadTree is a classic application of the divide-and-conquer paradigm:
 * 
 * 1. DIVIDE: Split the current region into 4 equal quadrants (NW, NE, SW, SE)
 * 
 * 2. CONQUER: Recursively build subtrees for each quadrant
 * 
 * 3. COMBINE: Each internal node aggregates information from its children
 * 
 * Base Case: When region size <= MIN_REGION_SIZE, create a leaf node
 * 
 * COMPLEXITY ANALYSIS:
 * ====================
 * 
 * Let n = image dimension, B = MIN_REGION_SIZE
 * 
 * Number of levels: O(log(n/B)) = O(log n) assuming B is constant
 * 
 * Number of nodes:
 *   - Leaves: (n/B)² = O(n²/B²)
 *   - Internal nodes: Sum_{i=0}^{log(n/B)-1} 4^i = O(n²/B²)
 *   - Total: O(n²/B²)
 * 
 * Build Time: O(n²/B²) - proportional to number of nodes
 *             Each node does O(1) work using prefix sums
 * 
 * Space: O(n²/B²) for storing all nodes
 * 
 * Query Traversal: O(result size + log n) with pruning
 */

#include "RegionTree.h"
#include <iostream>
#include <iomanip>
#include <queue>
#include <algorithm>

namespace SatelliteAnalytics {

RegionTree::RegionTree() 
    : prefixSum(nullptr), rootIndex(-1), nodeCount(0), 
      leafCount(0), maxDepth(0), minRegionSize(Config::MIN_REGION_SIZE),
      buildTimeMs(0) {}

void RegionTree::splitRegion(const Region& region, Region quadrants[4]) const {
    int midRow = (region.row1 + region.row2) / 2;
    int midCol = (region.col1 + region.col2) / 2;
    
    // NW quadrant (top-left)
    quadrants[0] = Region(region.row1, region.col1, midRow, midCol);
    
    // NE quadrant (top-right)
    quadrants[1] = Region(region.row1, midCol + 1, midRow, region.col2);
    
    // SW quadrant (bottom-left)
    quadrants[2] = Region(midRow + 1, region.col1, region.row2, midCol);
    
    // SE quadrant (bottom-right)
    quadrants[3] = Region(midRow + 1, midCol + 1, region.row2, region.col2);
}

void RegionTree::computeNodeStats(RegionTreeNode& node) {
    if (!prefixSum || !prefixSum->isBuilt()) return;
    
    // Use prefix sums for O(1) statistics computation
    node.stats = prefixSum->queryStats(node.bounds);
}

int RegionTree::buildRecursive(const Region& region, int depth, int parentIdx) {
    /**
     * DIVIDE AND CONQUER: Recursive tree construction
     * 
     * This function demonstrates the classic divide-and-conquer pattern:
     * 
     * 1. BASE CASE: If region is small enough, create a leaf node
     *    - No further division needed
     *    - Compute and store statistics
     * 
     * 2. RECURSIVE CASE: Split into 4 quadrants and recurse
     *    - Divide the region into NW, NE, SW, SE
     *    - Recursively build subtrees for each valid quadrant
     *    - The parent-child relationships form the tree structure
     */
    
    // Create new node
    int nodeIdx = nodes.size();
    nodes.emplace_back();
    RegionTreeNode& node = nodes[nodeIdx];
    
    node.id = nodeIdx;
    node.bounds = region;
    node.depth = depth;
    node.parent = parentIdx;
    node.children[0] = node.children[1] = node.children[2] = node.children[3] = -1;
    
    // Compute statistics using O(1) prefix sum queries
    computeNodeStats(node);
    
    // Track maximum depth
    maxDepth = std::max(maxDepth, depth);
    nodeCount++;
    
    // BASE CASE: Region is small enough to be a leaf
    int regionHeight = region.row2 - region.row1 + 1;
    int regionWidth = region.col2 - region.col1 + 1;
    
    if (regionHeight <= minRegionSize || regionWidth <= minRegionSize) {
        leafCount++;
        return nodeIdx;
    }
    
    // DIVIDE: Split current region into 4 quadrants
    Region quadrants[4];
    splitRegion(region, quadrants);
    
    // CONQUER: Recursively build subtrees for each valid quadrant
    for (int i = 0; i < 4; i++) {
        if (quadrants[i].isValid()) {
            node.children[i] = buildRecursive(quadrants[i], depth + 1, nodeIdx);
        }
    }
    
    return nodeIdx;
}

void RegionTree::build(const PrefixSum* prefix, int minSize) {
    if (!prefix || !prefix->isBuilt()) {
        std::cerr << "Error: PrefixSum not initialized" << std::endl;
        return;
    }
    
    Timer timer;
    timer.start();
    
    prefixSum = prefix;
    minRegionSize = minSize;
    
    // Clear any existing tree
    nodes.clear();
    nodeCount = 0;
    leafCount = 0;
    maxDepth = 0;
    
    // Reserve estimated space (optimization for large images)
    int estimatedNodes = (prefixSum->getHeight() / minRegionSize + 1) *
                         (prefixSum->getWidth() / minRegionSize + 1) * 2;
    nodes.reserve(estimatedNodes);
    
    // Build the entire tree from root
    Region fullImage(0, 0, prefixSum->getHeight() - 1, prefixSum->getWidth() - 1);
    rootIndex = buildRecursive(fullImage, 0, -1);
    
    timer.stop();
    buildTimeMs = timer.elapsedMs();
}

void RegionTree::traverse(std::function<void(const RegionTreeNode&, bool&)> visitor) const {
    if (rootIndex < 0 || nodes.empty()) return;
    
    std::queue<int> queue;
    queue.push(rootIndex);
    
    while (!queue.empty()) {
        int idx = queue.front();
        queue.pop();
        
        const RegionTreeNode& node = nodes[idx];
        bool shouldDescend = true;
        
        visitor(node, shouldDescend);
        
        if (shouldDescend && !node.isLeaf()) {
            for (int i = 0; i < 4; i++) {
                if (node.children[i] >= 0) {
                    queue.push(node.children[i]);
                }
            }
        }
    }
}

void RegionTree::traverseLeaves(std::function<void(const RegionTreeNode&)> visitor) const {
    for (const auto& node : nodes) {
        if (node.isLeaf()) {
            visitor(node);
        }
    }
}

std::vector<const RegionTreeNode*> RegionTree::getLeaves() const {
    std::vector<const RegionTreeNode*> leaves;
    leaves.reserve(leafCount);
    
    for (const auto& node : nodes) {
        if (node.isLeaf()) {
            leaves.push_back(&node);
        }
    }
    
    return leaves;
}

std::vector<const RegionTreeNode*> RegionTree::getNodesAtDepth(int depth) const {
    std::vector<const RegionTreeNode*> result;
    
    for (const auto& node : nodes) {
        if (node.depth == depth) {
            result.push_back(&node);
        }
    }
    
    return result;
}

std::vector<const RegionTreeNode*> RegionTree::queryRegion(const Region& queryRegion) const {
    std::vector<const RegionTreeNode*> result;
    
    if (rootIndex < 0) return result;
    
    std::queue<int> queue;
    queue.push(rootIndex);
    
    while (!queue.empty()) {
        int idx = queue.front();
        queue.pop();
        
        const RegionTreeNode& node = nodes[idx];
        const Region& bounds = node.bounds;
        
        // Check for intersection with query region
        bool intersects = !(bounds.row2 < queryRegion.row1 || 
                           bounds.row1 > queryRegion.row2 ||
                           bounds.col2 < queryRegion.col1 || 
                           bounds.col1 > queryRegion.col2);
        
        if (!intersects) continue;  // Prune this branch
        
        if (node.isLeaf()) {
            result.push_back(&node);
        } else {
            for (int i = 0; i < 4; i++) {
                if (node.children[i] >= 0) {
                    queue.push(node.children[i]);
                }
            }
        }
    }
    
    return result;
}

void RegionTree::printStats() const {
    std::cout << "\n--- Region Tree Statistics ---\n";
    std::cout << "Total nodes: " << formatNumber(nodeCount) << "\n";
    std::cout << "Leaf nodes: " << formatNumber(leafCount) << "\n";
    std::cout << "Internal nodes: " << formatNumber(nodeCount - leafCount) << "\n";
    std::cout << "Maximum depth: " << maxDepth << "\n";
    std::cout << "Min region size: " << minRegionSize << "x" << minRegionSize << "\n";
    std::cout << "Build time: " << formatTime(buildTimeMs) << "\n";
}

} // namespace SatelliteAnalytics
