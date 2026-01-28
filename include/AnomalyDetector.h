/**
 * @file AnomalyDetector.h
 * @brief Statistical anomaly detection using deviation scoring
 * 
 * ALGORITHM: Z-Score based Anomaly Detection
 * 
 * For each region R:
 *   anomalyScore = |mean(R) - global_mean| / global_stddev
 * 
 * A region is flagged as anomalous if:
 *   anomalyScore > threshold (default: 2.0 standard deviations)
 * 
 * WHY Z-SCORE?
 *   - Simple and interpretable
 *   - No training required (fits DAA constraints)
 *   - Based on solid statistical foundation
 *   - O(1) computation per region using prefix sums
 * 
 * COMPLEXITY:
 *   - Per region: O(1) using prefix sums
 *   - All regions: O(number of regions)
 *   - Total: O(n²/B²) where B = leaf region size
 */

#ifndef ANOMALY_DETECTOR_H
#define ANOMALY_DETECTOR_H

#include "Utils.h"
#include "PrefixSum.h"
#include "RegionTree.h"
#include <vector>

namespace SatelliteAnalytics {

/**
 * @struct AnomalyStats
 * @brief Statistics about detected anomalies
 */
struct AnomalyStats {
    int totalRegions;
    int anomalousRegions;
    double minScore;
    double maxScore;
    double meanScore;
    double detectionTimeMs;
};

/**
 * @class AnomalyDetector
 * @brief Detects anomalous regions based on statistical deviation
 * 
 * The detector uses the global image statistics (computed via prefix sums)
 * to score each region based on how much its mean deviates from the
 * global mean, normalized by the global standard deviation.
 * 
 * This is essentially a Z-score computation for each region.
 */
class AnomalyDetector {
private:
    const PrefixSum* prefixSum;
    double threshold;
    double globalMean;
    double globalStdDev;
    
    // Detection results
    AnomalyStats stats;
    bool detectionComplete;

public:
    /**
     * @brief Constructor
     * @param threshold Number of standard deviations for anomaly (default 2.0)
     */
    AnomalyDetector(double threshold = Config::DEFAULT_ANOMALY_THRESHOLD);
    
    /**
     * @brief Initialize with prefix sum data
     * @param prefixSum Pointer to initialized prefix sum structure
     */
    void initialize(const PrefixSum* prefixSum);
    
    /**
     * @brief Compute anomaly score for a single region
     * @param region The region to score
     * @return Anomaly score (Z-score magnitude)
     * 
     * FORMULA:
     *   score = |region_mean - global_mean| / global_stddev
     * 
     * TIME COMPLEXITY: O(1)
     */
    double computeScore(const Region& region) const;
    
    /**
     * @brief Check if region is anomalous based on threshold
     * @param region The region to check
     * @return true if anomaly score exceeds threshold
     */
    bool isAnomalous(const Region& region) const;
    
    /**
     * @brief Check if a given score indicates anomaly
     */
    bool isAnomalous(double score) const;
    
    /**
     * @brief Detect anomalies in the region tree
     * @param tree The region tree to analyze
     * 
     * Updates anomalyScore and isAnomaly fields for all nodes.
     * 
     * TIME COMPLEXITY: O(n²/B²) where B = leaf region size
     */
    void detectInTree(RegionTree& tree);
    
    /**
     * @brief Get all anomalous regions from the tree
     * @param tree The analyzed region tree
     * @return Vector of anomalous regions with scores
     */
    std::vector<AnomalyRegion> getAnomalousRegions(const RegionTree& tree) const;
    
    /**
     * @brief Get detection statistics
     */
    const AnomalyStats& getStats() const { return stats; }
    
    // ========================================================================
    // CONFIGURATION
    // ========================================================================
    
    void setThreshold(double t) { threshold = t; }
    double getThreshold() const { return threshold; }
    double getGlobalMean() const { return globalMean; }
    double getGlobalStdDev() const { return globalStdDev; }
};

} // namespace SatelliteAnalytics

#endif // ANOMALY_DETECTOR_H
