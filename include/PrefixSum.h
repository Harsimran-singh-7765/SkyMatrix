/**
 * @file PrefixSum.h
 * @brief 2D Prefix Sum implementation using Dynamic Programming
 * 
 * ALGORITHM: 2D Prefix Sum (Integral Image / Summed Area Table)
 * 
 * This is a classic Dynamic Programming technique that enables
 * O(1) range sum queries after O(n²) preprocessing.
 * 
 * For each cell (i,j), prefix[i][j] = sum of all pixels from (0,0) to (i,j)
 * 
 * RECURRENCE RELATION:
 *   prefix[i][j] = image[i][j] 
 *                  + prefix[i-1][j] 
 *                  + prefix[i][j-1] 
 *                  - prefix[i-1][j-1]
 * 
 * RANGE SUM QUERY: (r1,c1) to (r2,c2)
 *   sum = prefix[r2][c2] 
 *         - prefix[r1-1][c2] 
 *         - prefix[r2][c1-1] 
 *         + prefix[r1-1][c1-1]
 * 
 * COMPLEXITY ANALYSIS:
 *   - Build Time: O(n²)
 *   - Build Space: O(n²) for prefix sum matrix
 *   - Query Time: O(1)
 *   - Query Space: O(1)
 * 
 * EXTENSION: We also maintain prefix sum of squares to enable O(1) variance queries
 * using the formula: Var(X) = E[X²] - (E[X])²
 */

#ifndef PREFIX_SUM_H
#define PREFIX_SUM_H

#include "Utils.h"

namespace SatelliteAnalytics {

/**
 * @class PrefixSum
 * @brief 2D Prefix Sum data structure for O(1) region queries
 * 
 * This is the core data structure that makes efficient queries possible.
 * Without prefix sums, each query would require O(n²) time to sum pixels.
 * With prefix sums, we achieve O(1) query time after O(n²) preprocessing.
 * 
 * KEY INSIGHT: For K queries on an image of size N×N:
 *   - Naive approach: O(K × N²)
 *   - Prefix sum approach: O(N² + K)
 * 
 * This is a massive speedup when K is large (many queries).
 */
class PrefixSum {
private:
    // prefix[i][j] = sum of all elements from (0,0) to (i-1,j-1)
    // Uses 1-based indexing internally to avoid boundary checks
    PrefixMatrix prefix;
    
    // prefixSquares[i][j] = sum of squares of all elements from (0,0) to (i-1,j-1)
    // Used for variance computation: Var = E[X²] - (E[X])²
    PrefixMatrix prefixSquares;
    
    int height;
    int width;
    bool built;
    
    // Global statistics (computed during build)
    double globalMean;
    double globalVariance;
    double globalStdDev;
    int64_t totalSum;
    int64_t totalPixels;

public:
    PrefixSum();
    
    /**
     * @brief Build prefix sum matrices from image data
     * @param image The source grayscale image matrix
     * 
     * ALGORITHM:
     * For i = 0 to height-1:
     *   For j = 0 to width-1:
     *     prefix[i+1][j+1] = image[i][j] + prefix[i][j+1] + prefix[i+1][j] - prefix[i][j]
     * 
     * TIME COMPLEXITY: O(n²) where n is the image dimension
     * SPACE COMPLEXITY: O(n²) for storing two prefix matrices
     */
    void build(const Matrix& image);
    
    /**
     * @brief Query the sum of pixels in a rectangular region
     * @param region The rectangular region to query
     * @return Sum of all pixel values in the region
     * 
     * ALGORITHM (using inclusion-exclusion):
     *   Let A = prefix[r2+1][c2+1]  (sum from origin to bottom-right)
     *   Let B = prefix[r1][c2+1]    (sum above the region)
     *   Let C = prefix[r2+1][c1]    (sum left of the region)
     *   Let D = prefix[r1][c1]      (sum above-left, counted twice)
     *   Return A - B - C + D
     * 
     * TIME COMPLEXITY: O(1)
     */
    int64_t querySum(const Region& region) const;
    int64_t querySum(int r1, int c1, int r2, int c2) const;
    
    /**
     * @brief Query the sum of squared pixel values in a region
     * @param region The rectangular region to query
     * @return Sum of squared pixel values
     * 
     * Used for variance computation. Same algorithm as querySum.
     * TIME COMPLEXITY: O(1)
     */
    int64_t querySumSquares(const Region& region) const;
    int64_t querySumSquares(int r1, int c1, int r2, int c2) const;
    
    /**
     * @brief Query complete statistics for a region
     * @param region The rectangular region to query
     * @return RegionStats containing mean, variance, stdDev, sum, area
     * 
     * VARIANCE COMPUTATION:
     *   mean = sum / area
     *   E[X²] = sumSquares / area
     *   variance = E[X²] - mean²
     *   stdDev = sqrt(variance)
     * 
     * TIME COMPLEXITY: O(1)
     */
    RegionStats queryStats(const Region& region) const;
    RegionStats queryStats(int r1, int c1, int r2, int c2) const;
    
    /**
     * @brief Query mean pixel value for a region
     * TIME COMPLEXITY: O(1)
     */
    double queryMean(const Region& region) const;
    
    /**
     * @brief Query variance for a region
     * TIME COMPLEXITY: O(1)
     */
    double queryVariance(const Region& region) const;
    
    // ========================================================================
    // GLOBAL STATISTICS (computed during build)
    // ========================================================================
    
    double getGlobalMean() const { return globalMean; }
    double getGlobalVariance() const { return globalVariance; }
    double getGlobalStdDev() const { return globalStdDev; }
    int64_t getTotalSum() const { return totalSum; }
    int64_t getTotalPixels() const { return totalPixels; }
    
    // ========================================================================
    // UTILITY
    // ========================================================================
    
    bool isBuilt() const { return built; }
    int getHeight() const { return height; }
    int getWidth() const { return width; }
    
    /**
     * @brief Verify prefix sum correctness with brute force (for testing)
     * @param image Original image to verify against
     * @param region Region to verify
     * @return true if prefix sum matches brute force computation
     */
    bool verify(const Matrix& image, const Region& region) const;
};

} // namespace SatelliteAnalytics

#endif // PREFIX_SUM_H
