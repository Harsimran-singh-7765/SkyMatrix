/**
 * @file Utils.h
 * @brief Common utility functions, types, and constants for the Satellite Image Analytics Engine
 * 
 * This header provides foundational types and utilities used across all components:
 * - Region struct for defining rectangular image regions
 * - Timer class for performance measurement
 * - Configuration constants for tuning the algorithms
 */

#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>
#include <vector>
#include <cstdint>

namespace SatelliteAnalytics {

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

using Pixel = uint8_t;                    // Grayscale pixel value [0-255]
using Matrix = std::vector<std::vector<Pixel>>;
using PrefixMatrix = std::vector<std::vector<int64_t>>;  // Larger type to prevent overflow

/**
 * @struct Region
 * @brief Represents a rectangular region in the image
 * 
 * Uses inclusive bounds: [row1, row2] x [col1, col2]
 * This is the fundamental unit for all region-based operations
 */
struct Region {
    int row1, col1;  // Top-left corner (inclusive)
    int row2, col2;  // Bottom-right corner (inclusive)
    
    Region() : row1(0), col1(0), row2(0), col2(0) {}
    Region(int r1, int c1, int r2, int c2) : row1(r1), col1(c1), row2(r2), col2(c2) {}
    
    // Calculate number of pixels in the region
    int64_t area() const { 
        return static_cast<int64_t>(row2 - row1 + 1) * (col2 - col1 + 1); 
    }
    
    // Check if region is valid (non-negative dimensions)
    bool isValid() const {
        return row1 <= row2 && col1 <= col2;
    }
    
    // Get center coordinates
    std::pair<int, int> center() const {
        return {(row1 + row2) / 2, (col1 + col2) / 2};
    }
    
    // Check if this region contains a point
    bool contains(int row, int col) const {
        return row >= row1 && row <= row2 && col >= col1 && col <= col2;
    }
    
    // Check if two regions are adjacent (share an edge)
    bool isAdjacentTo(const Region& other) const;
};

/**
 * @struct RegionStats
 * @brief Statistical measures for a region computed using prefix sums
 */
struct RegionStats {
    double mean;
    double variance;
    double stdDev;
    int64_t sum;
    int64_t area;
    
    RegionStats() : mean(0), variance(0), stdDev(0), sum(0), area(0) {}
};

/**
 * @struct AnomalyRegion
 * @brief A region with its computed anomaly score
 * 
 * Used for priority queue operations in top-K queries
 */
struct AnomalyRegion {
    Region region;
    double anomalyScore;
    int nodeId;  // Reference to RegionTree node
    
    AnomalyRegion() : anomalyScore(0), nodeId(-1) {}
    AnomalyRegion(const Region& r, double score, int id = -1) 
        : region(r), anomalyScore(score), nodeId(id) {}
    
    // Comparison for min-heap (lower score = lower priority)
    bool operator<(const AnomalyRegion& other) const {
        return anomalyScore > other.anomalyScore;  // Inverted for min-heap
    }
    
    // Comparison for max-heap (higher score = higher priority)  
    bool operator>(const AnomalyRegion& other) const {
        return anomalyScore < other.anomalyScore;
    }
};

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

namespace Config {
    // Minimum region size for leaf nodes in the QuadTree
    // Smaller = more precise but more nodes, larger = fewer nodes but coarser
    constexpr int MIN_REGION_SIZE = 16;
    
    // Default anomaly threshold (number of standard deviations from mean)
    constexpr double DEFAULT_ANOMALY_THRESHOLD = 2.0;
    
    // Default K for top-K queries
    constexpr int DEFAULT_TOP_K = 10;
    
    // Maximum image dimension supported (for memory allocation)
    constexpr int MAX_IMAGE_DIM = 8192;
    
    // Default test image size
    constexpr int DEFAULT_IMAGE_SIZE = 512;
}

// ============================================================================
// TIMER UTILITY
// ============================================================================

/**
 * @class Timer
 * @brief High-resolution timer for measuring execution time
 * 
 * Usage:
 *   Timer timer;
 *   timer.start();
 *   // ... work ...
 *   timer.stop();
 *   std::cout << "Elapsed: " << timer.elapsedMs() << " ms\n";
 */
class Timer {
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    
    TimePoint startTime;
    TimePoint endTime;
    bool running;

public:
    Timer() : running(false) {}
    
    void start() {
        startTime = Clock::now();
        running = true;
    }
    
    void stop() {
        endTime = Clock::now();
        running = false;
    }
    
    double elapsedMs() const {
        auto end = running ? Clock::now() : endTime;
        return std::chrono::duration<double, std::milli>(end - startTime).count();
    }
    
    double elapsedUs() const {
        auto end = running ? Clock::now() : endTime;
        return std::chrono::duration<double, std::micro>(end - startTime).count();
    }
    
    double elapsedSeconds() const {
        return elapsedMs() / 1000.0;
    }
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Format a number with thousand separators
 */
std::string formatNumber(int64_t number);

/**
 * @brief Format time in appropriate units (us, ms, s)
 */
std::string formatTime(double milliseconds);

/**
 * @brief Print a divider line for console output
 */
void printDivider(char ch = '=', int length = 70);

/**
 * @brief Print a section header
 */
void printHeader(const std::string& title);

} // namespace SatelliteAnalytics

#endif // UTILS_H
