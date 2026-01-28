/**
 * @file Visualizer.cpp
 * @brief Implementation of visualization utilities
 */

#include "Visualizer.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

namespace SatelliteAnalytics {

Visualizer::Visualizer(int width, int height) 
    : consoleWidth(width), consoleHeight(height) {}

char Visualizer::valueToChar(double normalizedValue) const {
    // Map [0,1] to ASCII gradient
    static const char gradient[] = " .:-=+*#%@";
    int idx = static_cast<int>(normalizedValue * 9);
    idx = std::max(0, std::min(9, idx));
    return gradient[idx];
}

char Visualizer::pixelToChar(Pixel value) const {
    return valueToChar(value / 255.0);
}

void Visualizer::renderASCII(const Matrix& image, int scale) const {
    if (image.empty()) return;
    
    int height = image.size();
    int width = image[0].size();
    
    int outHeight = std::min(consoleHeight, height / scale);
    int outWidth = std::min(consoleWidth, width / scale);
    
    std::cout << "\n";
    for (int r = 0; r < outHeight; r++) {
        for (int c = 0; c < outWidth; c++) {
            // Average pixel values in the block
            int avgVal = 0;
            int count = 0;
            
            for (int dr = 0; dr < scale && r * scale + dr < height; dr++) {
                for (int dc = 0; dc < scale && c * scale + dc < width; dc++) {
                    avgVal += image[r * scale + dr][c * scale + dc];
                    count++;
                }
            }
            
            avgVal = count > 0 ? avgVal / count : 0;
            std::cout << pixelToChar(avgVal);
        }
        std::cout << "\n";
    }
}

void Visualizer::renderAnomalyMap(const Matrix& image, const RegionTree& tree,
                                   int scale) const {
    if (image.empty()) return;
    
    int height = image.size();
    int width = image[0].size();
    
    // Create a mask for anomalies
    std::vector<std::vector<bool>> anomalyMask(height, std::vector<bool>(width, false));
    
    auto leaves = tree.getLeaves();
    for (const auto* leaf : leaves) {
        if (leaf->isAnomaly) {
            for (int r = leaf->bounds.row1; r <= leaf->bounds.row2 && r < height; r++) {
                for (int c = leaf->bounds.col1; c <= leaf->bounds.col2 && c < width; c++) {
                    anomalyMask[r][c] = true;
                }
            }
        }
    }
    
    int outHeight = std::min(consoleHeight, height / scale);
    int outWidth = std::min(consoleWidth, width / scale);
    
    std::cout << "\n";
    for (int r = 0; r < outHeight; r++) {
        for (int c = 0; c < outWidth; c++) {
            // Check if any pixel in this block is anomalous
            bool isAnomaly = false;
            int avgVal = 0;
            int count = 0;
            
            for (int dr = 0; dr < scale && r * scale + dr < height; dr++) {
                for (int dc = 0; dc < scale && c * scale + dc < width; dc++) {
                    int row = r * scale + dr;
                    int col = c * scale + dc;
                    avgVal += image[row][col];
                    count++;
                    if (anomalyMask[row][col]) isAnomaly = true;
                }
            }
            
            avgVal = count > 0 ? avgVal / count : 0;
            
            if (isAnomaly) {
                // Highlight anomalies with special character
                std::cout << 'X';
            } else {
                std::cout << pixelToChar(avgVal);
            }
        }
        std::cout << "\n";
    }
    
    std::cout << "\nLegend: 'X' = Anomalous region\n";
}

void Visualizer::renderComponents(const Matrix& image,
                                  const std::vector<ConnectedComponent>& components,
                                  int scale) const {
    if (image.empty()) return;
    
    int height = image.size();
    int width = image[0].size();
    
    // Create component ID map
    std::vector<std::vector<int>> componentMap(height, std::vector<int>(width, -1));
    
    for (size_t i = 0; i < components.size() && i < 9; i++) {
        const auto& comp = components[i];
        const auto& bb = comp.boundingBox;
        
        for (int r = bb.row1; r <= bb.row2 && r < height; r++) {
            for (int c = bb.col1; c <= bb.col2 && c < width; c++) {
                if (componentMap[r][c] == -1) {
                    componentMap[r][c] = i;
                }
            }
        }
    }
    
    int outHeight = std::min(consoleHeight, height / scale);
    int outWidth = std::min(consoleWidth, width / scale);
    
    std::cout << "\n";
    for (int r = 0; r < outHeight; r++) {
        for (int c = 0; c < outWidth; c++) {
            int compId = -1;
            
            for (int dr = 0; dr < scale && r * scale + dr < height; dr++) {
                for (int dc = 0; dc < scale && c * scale + dc < width; dc++) {
                    int row = r * scale + dr;
                    int col = c * scale + dc;
                    if (componentMap[row][col] >= 0) {
                        compId = componentMap[row][col];
                    }
                }
            }
            
            if (compId >= 0) {
                std::cout << static_cast<char>('0' + compId);
            } else {
                std::cout << '.';
            }
        }
        std::cout << "\n";
    }
    
    std::cout << "\nLegend: Numbers = Component IDs, '.' = Normal regions\n";
}

void Visualizer::renderTreeStructure(const RegionTree& tree, int maxDepth) const {
    std::cout << "\n--- Tree Structure (first " << maxDepth << " levels) ---\n";
    
    const auto& root = tree.getRoot();
    std::cout << "Root: [" << root.bounds.row1 << "," << root.bounds.col1 
              << "]-[" << root.bounds.row2 << "," << root.bounds.col2 << "]\n";
    
    for (int d = 1; d <= maxDepth; d++) {
        auto nodesAtDepth = tree.getNodesAtDepth(d);
        std::cout << "Level " << d << ": " << nodesAtDepth.size() << " nodes\n";
    }
}

Matrix Visualizer::createAnomalyOverlay(const Matrix& source, 
                                        const RegionTree& tree) const {
    Matrix result = source;
    
    auto leaves = tree.getLeaves();
    for (const auto* leaf : leaves) {
        if (leaf->isAnomaly) {
            // Highlight by setting to bright
            for (int r = leaf->bounds.row1; r <= leaf->bounds.row2; r++) {
                for (int c = leaf->bounds.col1; c <= leaf->bounds.col2; c++) {
                    if (r >= 0 && r < static_cast<int>(result.size()) &&
                        c >= 0 && c < static_cast<int>(result[0].size())) {
                        // Brighten the pixel
                        int val = result[r][c];
                        val = std::min(255, val + 100);
                        result[r][c] = static_cast<Pixel>(val);
                    }
                }
            }
            
            // Draw border
            for (int r = leaf->bounds.row1; r <= leaf->bounds.row2; r++) {
                if (r >= 0 && r < static_cast<int>(result.size())) {
                    if (leaf->bounds.col1 >= 0 && 
                        leaf->bounds.col1 < static_cast<int>(result[0].size())) {
                        result[r][leaf->bounds.col1] = 255;
                    }
                    if (leaf->bounds.col2 >= 0 && 
                        leaf->bounds.col2 < static_cast<int>(result[0].size())) {
                        result[r][leaf->bounds.col2] = 255;
                    }
                }
            }
            for (int c = leaf->bounds.col1; c <= leaf->bounds.col2; c++) {
                if (c >= 0 && c < static_cast<int>(result[0].size())) {
                    if (leaf->bounds.row1 >= 0 && 
                        leaf->bounds.row1 < static_cast<int>(result.size())) {
                        result[leaf->bounds.row1][c] = 255;
                    }
                    if (leaf->bounds.row2 >= 0 && 
                        leaf->bounds.row2 < static_cast<int>(result.size())) {
                        result[leaf->bounds.row2][c] = 255;
                    }
                }
            }
        }
    }
    
    return result;
}

Matrix Visualizer::createComponentOverlay(const Matrix& source,
                                          const ConnectedComponent& component,
                                          const RegionTree& tree) const {
    Matrix result = source;
    
    // Highlight the bounding box
    const auto& bb = component.boundingBox;
    
    // Fill with highlight
    for (int r = bb.row1; r <= bb.row2; r++) {
        for (int c = bb.col1; c <= bb.col2; c++) {
            if (r >= 0 && r < static_cast<int>(result.size()) &&
                c >= 0 && c < static_cast<int>(result[0].size())) {
                int val = result[r][c];
                val = std::min(255, val + 80);
                result[r][c] = static_cast<Pixel>(val);
            }
        }
    }
    
    // Draw thick border
    for (int i = 0; i < 2; i++) {
        int r1 = bb.row1 + i;
        int r2 = bb.row2 - i;
        int c1 = bb.col1 + i;
        int c2 = bb.col2 - i;
        
        for (int r = r1; r <= r2; r++) {
            if (r >= 0 && r < static_cast<int>(result.size())) {
                if (c1 >= 0 && c1 < static_cast<int>(result[0].size())) {
                    result[r][c1] = 255;
                }
                if (c2 >= 0 && c2 < static_cast<int>(result[0].size())) {
                    result[r][c2] = 255;
                }
            }
        }
        for (int c = c1; c <= c2; c++) {
            if (c >= 0 && c < static_cast<int>(result[0].size())) {
                if (r1 >= 0 && r1 < static_cast<int>(result.size())) {
                    result[r1][c] = 255;
                }
                if (r2 >= 0 && r2 < static_cast<int>(result.size())) {
                    result[r2][c] = 255;
                }
            }
        }
    }
    
    return result;
}

bool Visualizer::savePGM(const Matrix& image, const std::string& filename) const {
    if (image.empty()) return false;
    
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;
    
    int height = image.size();
    int width = image[0].size();
    
    file << "P5\n" << width << " " << height << "\n255\n";
    
    for (const auto& row : image) {
        for (Pixel p : row) {
            file.put(static_cast<char>(p));
        }
    }
    
    return true;
}

void Visualizer::printAnomalySummary(const std::vector<AnomalyRegion>& regions) const {
    std::cout << "\n";
    printDivider('-', 70);
    std::cout << std::left << std::setw(6) << "Rank"
              << std::setw(25) << "Region"
              << std::setw(15) << "Score"
              << std::setw(15) << "Area" << "\n";
    printDivider('-', 70);
    
    for (size_t i = 0; i < regions.size(); i++) {
        const auto& r = regions[i];
        std::ostringstream regionStr;
        regionStr << "[" << r.region.row1 << "," << r.region.col1 << "]-["
                  << r.region.row2 << "," << r.region.col2 << "]";
        
        std::cout << std::left << std::setw(6) << (i + 1)
                  << std::setw(25) << regionStr.str()
                  << std::fixed << std::setprecision(3) 
                  << std::setw(15) << r.anomalyScore
                  << std::setw(15) << formatNumber(r.region.area()) << "\n";
    }
    
    printDivider('-', 70);
}

void Visualizer::printComponentSummary(
    const std::vector<ConnectedComponent>& components) const {
    
    std::cout << "\n";
    printDivider('-', 70);
    std::cout << std::left << std::setw(6) << "ID"
              << std::setw(10) << "Regions"
              << std::setw(15) << "Total Area"
              << std::setw(12) << "Max Score"
              << std::setw(12) << "Avg Score" << "\n";
    printDivider('-', 70);
    
    for (const auto& comp : components) {
        std::cout << std::left << std::setw(6) << comp.id
                  << std::setw(10) << comp.nodeIndices.size()
                  << std::setw(15) << formatNumber(comp.totalArea)
                  << std::fixed << std::setprecision(3)
                  << std::setw(12) << comp.maxScore
                  << std::setw(12) << comp.avgScore << "\n";
    }
    
    printDivider('-', 70);
}

void Visualizer::printQueryResult(const QueryResult& result, 
                                  const std::string& queryName) const {
    std::cout << "\n";
    std::cout << "Query: " << queryName << "\n";
    std::cout << "  Results: " << result.regions.size() << "\n";
    std::cout << "  Nodes visited: " << formatNumber(result.nodesVisited) << "\n";
    if (result.nodesPruned > 0) {
        std::cout << "  Nodes pruned: " << formatNumber(result.nodesPruned) << "\n";
    }
    std::cout << "  Time: " << formatTime(result.queryTimeMs) << "\n";
}

void Visualizer::printProgressBar(double progress, int width) const {
    int filled = static_cast<int>(progress * width);
    std::cout << "\r[";
    for (int i = 0; i < width; i++) {
        std::cout << (i < filled ? "█" : "░");
    }
    std::cout << "] " << std::fixed << std::setprecision(1) 
              << (progress * 100) << "%" << std::flush;
}

} // namespace SatelliteAnalytics
