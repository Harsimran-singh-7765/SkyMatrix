/**
 * @file Visualizer.h
 * @brief Simple visualization for anomaly detection results
 * 
 * Provides ASCII art and basic image output for visualizing:
 * - Detected anomalies
 * - Connected components
 * - Query results
 * 
 * This keeps the focus on algorithms rather than complex graphics,
 * suitable for a Design and Analysis of Algorithms course.
 */

#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "Utils.h"
#include "RegionTree.h"
#include "QueryEngine.h"
#include <string>
#include <vector>

namespace SatelliteAnalytics {

/**
 * @class Visualizer
 * @brief Handles visualization of analysis results
 */
class Visualizer {
private:
    int consoleWidth;
    int consoleHeight;
    
    /**
     * @brief Map a value to an ASCII character
     */
    char valueToChar(double normalizedValue) const;
    
    /**
     * @brief Map a pixel value to grayscale character
     */
    char pixelToChar(Pixel value) const;

public:
    Visualizer(int width = 80, int height = 40);
    
    // ========================================================================
    // ASCII VISUALIZATION
    // ========================================================================
    
    /**
     * @brief Render image as ASCII art
     * @param image The source image
     * @param scale Downscale factor (1 = no scale, 2 = half size, etc.)
     */
    void renderASCII(const Matrix& image, int scale = 1) const;
    
    /**
     * @brief Render anomaly map as ASCII
     * @param image Original image
     * @param tree Analyzed region tree
     * @param scale Downscale factor
     * 
     * Highlights anomalous regions with special characters.
     */
    void renderAnomalyMap(const Matrix& image, const RegionTree& tree, 
                          int scale = 1) const;
    
    /**
     * @brief Render connected components
     * @param image Original image
     * @param components Connected components to display
     * @param scale Downscale factor
     */
    void renderComponents(const Matrix& image, 
                         const std::vector<ConnectedComponent>& components,
                         int scale = 1) const;
    
    /**
     * @brief Render tree structure (simplified)
     * @param tree Region tree to visualize
     * @param maxDepth Maximum depth to display
     */
    void renderTreeStructure(const RegionTree& tree, int maxDepth = 3) const;
    
    // ========================================================================
    // IMAGE OUTPUT
    // ========================================================================
    
    /**
     * @brief Create image with highlighted anomalies
     * @param source Original image
     * @param tree Analyzed region tree
     * @return New image with anomalies highlighted
     */
    Matrix createAnomalyOverlay(const Matrix& source, const RegionTree& tree) const;
    
    /**
     * @brief Create image with highlighted components
     * @param source Original image
     * @param component Component to highlight
     * @return New image with component highlighted
     */
    Matrix createComponentOverlay(const Matrix& source, 
                                  const ConnectedComponent& component,
                                  const RegionTree& tree) const;
    
    /**
     * @brief Save visualization to PGM file
     * @param image Image matrix to save
     * @param filename Output filename
     * @return true if successful
     */
    bool savePGM(const Matrix& image, const std::string& filename) const;
    
    // ========================================================================
    // CONSOLE OUTPUT
    // ========================================================================
    
    /**
     * @brief Print a summary table of anomalous regions
     */
    void printAnomalySummary(const std::vector<AnomalyRegion>& regions) const;
    
    /**
     * @brief Print component information
     */
    void printComponentSummary(const std::vector<ConnectedComponent>& components) const;
    
    /**
     * @brief Print query result summary
     */
    void printQueryResult(const QueryResult& result, const std::string& queryName) const;
    
    /**
     * @brief Print a progress bar
     */
    void printProgressBar(double progress, int width = 50) const;
};

} // namespace SatelliteAnalytics

#endif // VISUALIZER_H
