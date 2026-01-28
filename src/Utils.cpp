/**
 * @file Utils.cpp
 * @brief Implementation of utility functions
 */

#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace SatelliteAnalytics {

// ============================================================================
// REGION METHODS
// ============================================================================

bool Region::isAdjacentTo(const Region& other) const {
    // Two regions are adjacent if they share an edge (not diagonally touching)
    
    // Check if regions overlap (not adjacent, they intersect)
    bool xOverlap = !(col2 < other.col1 || other.col2 < col1);
    bool yOverlap = !(row2 < other.row1 || other.row2 < row1);
    
    if (xOverlap && yOverlap) {
        return false;  // They overlap, not just adjacent
    }
    
    // Check horizontal adjacency (share vertical edge)
    if (yOverlap) {
        if (col2 + 1 == other.col1 || other.col2 + 1 == col1) {
            return true;
        }
    }
    
    // Check vertical adjacency (share horizontal edge)
    if (xOverlap) {
        if (row2 + 1 == other.row1 || other.row2 + 1 == row1) {
            return true;
        }
    }
    
    return false;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

std::string formatNumber(int64_t number) {
    std::string numStr = std::to_string(number);
    std::string result;
    
    int count = 0;
    for (int i = numStr.length() - 1; i >= 0; i--) {
        if (count > 0 && count % 3 == 0) {
            result = "," + result;
        }
        result = numStr[i] + result;
        count++;
    }
    
    return result;
}

std::string formatTime(double milliseconds) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    if (milliseconds < 1.0) {
        oss << (milliseconds * 1000.0) << " µs";
    } else if (milliseconds < 1000.0) {
        oss << milliseconds << " ms";
    } else {
        oss << (milliseconds / 1000.0) << " s";
    }
    
    return oss.str();
}

void printDivider(char ch, int length) {
    std::cout << std::string(length, ch) << std::endl;
}

void printHeader(const std::string& title) {
    printDivider('=', 70);
    std::cout << "  " << title << std::endl;
    printDivider('=', 70);
}

} // namespace SatelliteAnalytics
