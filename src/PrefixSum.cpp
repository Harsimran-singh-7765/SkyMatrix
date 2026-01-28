/**
 * @file PrefixSum.cpp
 * @brief Implementation of 2D Prefix Sum using Dynamic Programming
 * 
 * ALGORITHM EXPLANATION:
 * 
 * The 2D prefix sum (also known as integral image or summed area table)
 * is a powerful technique from dynamic programming that allows us to
 * compute the sum of any rectangular region in O(1) time after O(n²)
 * preprocessing.
 * 
 * BUILD PHASE (Dynamic Programming):
 * =================================
 * 
 * For prefix[i][j] (1-indexed), we want the sum of all elements 
 * from image[0][0] to image[i-1][j-1].
 * 
 * Recurrence relation:
 *   prefix[i][j] = image[i-1][j-1] 
 *                + prefix[i-1][j]   // sum above
 *                + prefix[i][j-1]   // sum to the left
 *                - prefix[i-1][j-1] // subtract double-counted corner
 * 
 * Visual representation:
 *   +-------+----+
 *   |   A   | B  |
 *   +-------+----+
 *   |   C   | X  |
 *   +-------+----+
 * 
 *   prefix[X] = image[X] + prefix[B] + prefix[C] - prefix[A]
 * 
 * QUERY PHASE (Inclusion-Exclusion):
 * ==================================
 * 
 * To get sum of region (r1,c1) to (r2,c2):
 * 
 *   +-------+-------+
 *   |   A   |   B   |
 *   +-------+-------+
 *   |   C   | QUERY |
 *   +-------+-------+
 * 
 *   sum = prefix[r2+1][c2+1]    // entire region from origin
 *       - prefix[r1][c2+1]      // subtract region above
 *       - prefix[r2+1][c1]      // subtract region to the left
 *       + prefix[r1][c1]        // add back double-subtracted corner
 * 
 * TIME COMPLEXITY: O(n²) build, O(1) query
 * SPACE COMPLEXITY: O(n²) for the prefix matrix
 */

#include "PrefixSum.h"
#include <cmath>
#include <iostream>

namespace SatelliteAnalytics {

PrefixSum::PrefixSum() 
    : height(0), width(0), built(false),
      globalMean(0), globalVariance(0), globalStdDev(0),
      totalSum(0), totalPixels(0) {}

void PrefixSum::build(const Matrix& image) {
    if (image.empty() || image[0].empty()) {
        std::cerr << "Error: Cannot build prefix sum from empty image" << std::endl;
        return;
    }
    
    height = image.size();
    width = image[0].size();
    totalPixels = static_cast<int64_t>(height) * width;
    
    // Initialize with 1-based indexing (add padding of zeros)
    // This eliminates boundary checks in queries
    prefix.assign(height + 1, std::vector<int64_t>(width + 1, 0));
    prefixSquares.assign(height + 1, std::vector<int64_t>(width + 1, 0));
    
    /**
     * DYNAMIC PROGRAMMING: Build prefix sums
     * 
     * dp[i][j] = sum of all elements in rectangle (0,0) to (i-1,j-1)
     * 
     * Transition: dp[i][j] = image[i-1][j-1] + dp[i-1][j] + dp[i][j-1] - dp[i-1][j-1]
     * 
     * The key insight is that each cell contains cumulative information,
     * and we use previously computed values to avoid redundant work.
     */
    for (int i = 1; i <= height; i++) {
        for (int j = 1; j <= width; j++) {
            int64_t pixelValue = image[i-1][j-1];
            
            // Prefix sum: classic DP recurrence
            prefix[i][j] = pixelValue 
                         + prefix[i-1][j] 
                         + prefix[i][j-1] 
                         - prefix[i-1][j-1];
            
            // Prefix sum of squares: same recurrence, different values
            // This enables O(1) variance computation
            prefixSquares[i][j] = pixelValue * pixelValue 
                                + prefixSquares[i-1][j] 
                                + prefixSquares[i][j-1] 
                                - prefixSquares[i-1][j-1];
        }
    }
    
    // Compute global statistics
    totalSum = prefix[height][width];
    globalMean = static_cast<double>(totalSum) / totalPixels;
    
    int64_t sumSquares = prefixSquares[height][width];
    double meanOfSquares = static_cast<double>(sumSquares) / totalPixels;
    
    // Variance = E[X²] - (E[X])²
    globalVariance = meanOfSquares - globalMean * globalMean;
    globalStdDev = std::sqrt(std::max(0.0, globalVariance));
    
    built = true;
}

int64_t PrefixSum::querySum(const Region& region) const {
    return querySum(region.row1, region.col1, region.row2, region.col2);
}

int64_t PrefixSum::querySum(int r1, int c1, int r2, int c2) const {
    if (!built) return 0;
    
    // Clamp to valid bounds
    r1 = std::max(0, r1);
    c1 = std::max(0, c1);
    r2 = std::min(height - 1, r2);
    c2 = std::min(width - 1, c2);
    
    if (r1 > r2 || c1 > c2) return 0;
    
    /**
     * INCLUSION-EXCLUSION PRINCIPLE:
     * 
     * To get the sum of rectangle (r1,c1) to (r2,c2):
     * 
     * sum = prefix[r2+1][c2+1]  // Everything from origin to (r2,c2)
     *     - prefix[r1][c2+1]    // Subtract everything above the region
     *     - prefix[r2+1][c1]    // Subtract everything left of the region
     *     + prefix[r1][c1]      // Add back the corner (subtracted twice)
     * 
     * This is a classic application of the inclusion-exclusion principle
     * that gives us O(1) query time!
     */
    return prefix[r2+1][c2+1] 
         - prefix[r1][c2+1] 
         - prefix[r2+1][c1] 
         + prefix[r1][c1];
}

int64_t PrefixSum::querySumSquares(const Region& region) const {
    return querySumSquares(region.row1, region.col1, region.row2, region.col2);
}

int64_t PrefixSum::querySumSquares(int r1, int c1, int r2, int c2) const {
    if (!built) return 0;
    
    r1 = std::max(0, r1);
    c1 = std::max(0, c1);
    r2 = std::min(height - 1, r2);
    c2 = std::min(width - 1, c2);
    
    if (r1 > r2 || c1 > c2) return 0;
    
    // Same inclusion-exclusion formula as querySum
    return prefixSquares[r2+1][c2+1] 
         - prefixSquares[r1][c2+1] 
         - prefixSquares[r2+1][c1] 
         + prefixSquares[r1][c1];
}

RegionStats PrefixSum::queryStats(const Region& region) const {
    return queryStats(region.row1, region.col1, region.row2, region.col2);
}

RegionStats PrefixSum::queryStats(int r1, int c1, int r2, int c2) const {
    RegionStats stats;
    
    if (!built || r1 > r2 || c1 > c2) return stats;
    
    stats.sum = querySum(r1, c1, r2, c2);
    stats.area = static_cast<int64_t>(r2 - r1 + 1) * (c2 - c1 + 1);
    
    if (stats.area == 0) return stats;
    
    stats.mean = static_cast<double>(stats.sum) / stats.area;
    
    /**
     * VARIANCE COMPUTATION using the formula:
     * 
     * Var(X) = E[X²] - (E[X])²
     * 
     * where:
     *   E[X²] = (sum of squares) / area
     *   E[X] = mean = sum / area
     * 
     * This avoids the need for a second pass over the data!
     */
    int64_t sumSquares = querySumSquares(r1, c1, r2, c2);
    double meanOfSquares = static_cast<double>(sumSquares) / stats.area;
    
    stats.variance = meanOfSquares - stats.mean * stats.mean;
    // Numerical stability: variance can be slightly negative due to floating point
    stats.variance = std::max(0.0, stats.variance);
    stats.stdDev = std::sqrt(stats.variance);
    
    return stats;
}

double PrefixSum::queryMean(const Region& region) const {
    if (!built) return 0;
    
    int64_t sum = querySum(region);
    int64_t area = region.area();
    
    return area > 0 ? static_cast<double>(sum) / area : 0;
}

double PrefixSum::queryVariance(const Region& region) const {
    return queryStats(region).variance;
}

bool PrefixSum::verify(const Matrix& image, const Region& region) const {
    // Brute force computation for verification
    int64_t bruteSum = 0;
    int64_t bruteSumSquares = 0;
    
    for (int r = region.row1; r <= region.row2; r++) {
        for (int c = region.col1; c <= region.col2; c++) {
            if (r >= 0 && r < static_cast<int>(image.size()) &&
                c >= 0 && c < static_cast<int>(image[0].size())) {
                int64_t val = image[r][c];
                bruteSum += val;
                bruteSumSquares += val * val;
            }
        }
    }
    
    int64_t prefixSumResult = querySum(region);
    int64_t prefixSumSquaresResult = querySumSquares(region);
    
    return (bruteSum == prefixSumResult) && (bruteSumSquares == prefixSumSquaresResult);
}

} // namespace SatelliteAnalytics
