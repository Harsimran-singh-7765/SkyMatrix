/**
 * @file AnomalyDetector.cpp
 * @brief Implementation of statistical anomaly detection
 * 
 * ALGORITHM: Z-SCORE BASED ANOMALY DETECTION
 * ==========================================
 * 
 * The Z-score (standard score) measures how many standard deviations
 * a value is from the mean:
 * 
 *   Z = (X - μ) / σ
 * 
 * For our region-based analysis:
 * 
 *   anomalyScore = |mean(region) - global_mean| / global_stddev
 * 
 * A region is flagged as anomalous if:
 *   anomalyScore > threshold
 * 
 * Default threshold = 2.0 (approximately 5% of normal regions would exceed this)
 * 
 * WHY THIS APPROACH?
 * ==================
 * 
 * 1. NO TRAINING: Fits DAA course constraints (no ML)
 * 2. INTERPRETABLE: Z-score has clear statistical meaning
 * 3. EFFICIENT: O(1) per region using prefix sums
 * 4. ROBUST: Based on well-understood statistics
 * 
 * COMPLEXITY:
 * ===========
 * Per region: O(1) - uses prefix sum queries
 * All regions: O(n²/B²) where B = leaf region size
 */

#include "AnomalyDetector.h"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace SatelliteAnalytics {

AnomalyDetector::AnomalyDetector(double threshold)
    : prefixSum(nullptr), threshold(threshold),
      globalMean(0), globalStdDev(0), detectionComplete(false) {
    stats = AnomalyStats();
}

void AnomalyDetector::initialize(const PrefixSum* prefix) {
    prefixSum = prefix;
    
    if (prefixSum && prefixSum->isBuilt()) {
        globalMean = prefixSum->getGlobalMean();
        globalStdDev = prefixSum->getGlobalStdDev();
    }
    
    detectionComplete = false;
}

double AnomalyDetector::computeScore(const Region& region) const {
    /**
     * Z-SCORE COMPUTATION:
     * 
     * score = |region_mean - global_mean| / global_stddev
     * 
     * The absolute value captures both unusually bright and unusually dark regions.
     * 
     * We use the global standard deviation (not the region's) because we want
     * to measure how different the region is from the overall image.
     * 
     * TIME COMPLEXITY: O(1) because:
     * - region_mean is computed in O(1) using prefix sums
     * - global_mean and global_stddev are precomputed
     */
    
    if (!prefixSum || !prefixSum->isBuilt()) return 0.0;
    if (globalStdDev < 1e-10) return 0.0;  // Avoid division by zero
    
    double regionMean = prefixSum->queryMean(region);
    double deviation = std::abs(regionMean - globalMean);
    
    return deviation / globalStdDev;
}

bool AnomalyDetector::isAnomalous(const Region& region) const {
    return computeScore(region) > threshold;
}

bool AnomalyDetector::isAnomalous(double score) const {
    return score > threshold;
}

void AnomalyDetector::detectInTree(RegionTree& tree) {
    Timer timer;
    timer.start();
    
    if (!prefixSum || !prefixSum->isBuilt()) {
        std::cerr << "Error: PrefixSum not initialized in AnomalyDetector" << std::endl;
        return;
    }
    
    // Initialize statistics
    stats.totalRegions = 0;
    stats.anomalousRegions = 0;
    stats.minScore = std::numeric_limits<double>::max();
    stats.maxScore = 0;
    double totalScore = 0;
    
    // Process all nodes in the tree
    auto& nodes = tree.getAllNodesMutable();
    
    for (auto& node : nodes) {
        // Compute anomaly score for this node
        double score = computeScore(node.bounds);
        node.anomalyScore = score;
        node.isAnomaly = isAnomalous(score);
        
        // Update statistics (count only leaf nodes)
        if (node.isLeaf()) {
            stats.totalRegions++;
            totalScore += score;
            stats.minScore = std::min(stats.minScore, score);
            stats.maxScore = std::max(stats.maxScore, score);
            
            if (node.isAnomaly) {
                stats.anomalousRegions++;
            }
        }
    }
    
    // Compute mean score
    stats.meanScore = (stats.totalRegions > 0) ? totalScore / stats.totalRegions : 0;
    
    timer.stop();
    stats.detectionTimeMs = timer.elapsedMs();
    detectionComplete = true;
}

std::vector<AnomalyRegion> AnomalyDetector::getAnomalousRegions(const RegionTree& tree) const {
    std::vector<AnomalyRegion> result;
    
    auto leaves = tree.getLeaves();
    for (const auto* node : leaves) {
        if (node->isAnomaly) {
            result.emplace_back(node->bounds, node->anomalyScore, node->id);
        }
    }
    
    // Sort by anomaly score (descending)
    std::sort(result.begin(), result.end(), 
              [](const AnomalyRegion& a, const AnomalyRegion& b) {
                  return a.anomalyScore > b.anomalyScore;
              });
    
    return result;
}

} // namespace SatelliteAnalytics
