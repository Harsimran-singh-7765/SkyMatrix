/**
 * @file ImageLoader.cpp
 * @brief Implementation of image loading and synthetic generation
 * 
 * Time Complexity: O(n²) for all operations
 */

#include "ImageLoader.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace SatelliteAnalytics {

ImageLoader::ImageLoader() : height(0), width(0), rng(42) {}

bool ImageLoader::parsePGMHeader(std::ifstream& file, int& w, int& h, int& maxVal) {
    std::string line;
    std::string magic;
    
    // Read magic number
    if (!std::getline(file, magic)) return false;
    
    // Skip comments
    while (file.peek() == '#') {
        std::getline(file, line);
    }
    
    // Read dimensions
    file >> w >> h;
    
    // Read max value
    file >> maxVal;
    
    // Skip whitespace after maxVal
    file.get();
    
    return (magic == "P2" || magic == "P5") && w > 0 && h > 0 && maxVal > 0;
}

bool ImageLoader::loadFromPGM(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    std::string magic;
    file >> magic;
    file.seekg(0);
    
    int w, h, maxVal;
    if (!parsePGMHeader(file, w, h, maxVal)) {
        std::cerr << "Error: Invalid PGM format" << std::endl;
        return false;
    }
    
    width = w;
    height = h;
    imageData.resize(height, std::vector<Pixel>(width));
    
    if (magic == "P2") {
        // ASCII format
        for (int r = 0; r < height; r++) {
            for (int c = 0; c < width; c++) {
                int val;
                file >> val;
                imageData[r][c] = static_cast<Pixel>(val * 255 / maxVal);
            }
        }
    } else {
        // Binary format (P5)
        for (int r = 0; r < height; r++) {
            for (int c = 0; c < width; c++) {
                unsigned char byte;
                file.read(reinterpret_cast<char*>(&byte), 1);
                imageData[r][c] = static_cast<Pixel>(byte * 255 / maxVal);
            }
        }
    }
    
    return true;
}

void ImageLoader::loadFromBuffer(const Pixel* data, int w, int h) {
    width = w;
    height = h;
    imageData.resize(height, std::vector<Pixel>(width));
    
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            imageData[r][c] = data[r * width + c];
        }
    }
}

void ImageLoader::generateSyntheticImage(int size, int numAnomalies, unsigned int seed) {
    rng.seed(seed);
    width = size;
    height = size;
    imageData.resize(height, std::vector<Pixel>(width));
    
    // Distribution for base terrain
    std::normal_distribution<double> terrainDist(128.0, 20.0);  // Mean 128, stddev 20
    std::uniform_real_distribution<double> noiseDist(-10.0, 10.0);
    
    // Generate base terrain with gradual variations
    // Using simplified Perlin-like noise with multiple octaves
    std::vector<std::vector<double>> noise(height, std::vector<double>(width, 0.0));
    
    // Generate multi-scale noise
    for (int octave = 0; octave < 4; octave++) {
        int scale = 1 << (5 - octave);  // 32, 16, 8, 4
        double amplitude = 1.0 / (1 << octave);  // 1, 0.5, 0.25, 0.125
        
        // Generate control points
        int gridSize = (size / scale) + 2;
        std::vector<std::vector<double>> control(gridSize, std::vector<double>(gridSize));
        
        for (int i = 0; i < gridSize; i++) {
            for (int j = 0; j < gridSize; j++) {
                control[i][j] = noiseDist(rng);
            }
        }
        
        // Interpolate
        for (int r = 0; r < height; r++) {
            for (int c = 0; c < width; c++) {
                double gr = static_cast<double>(r) / scale;
                double gc = static_cast<double>(c) / scale;
                
                int gi = static_cast<int>(gr);
                int gj = static_cast<int>(gc);
                
                double fr = gr - gi;
                double fc = gc - gj;
                
                // Bilinear interpolation
                gi = std::min(gi, gridSize - 2);
                gj = std::min(gj, gridSize - 2);
                
                double v00 = control[gi][gj];
                double v01 = control[gi][gj + 1];
                double v10 = control[gi + 1][gj];
                double v11 = control[gi + 1][gj + 1];
                
                double v0 = v00 * (1 - fc) + v01 * fc;
                double v1 = v10 * (1 - fc) + v11 * fc;
                double v = v0 * (1 - fr) + v1 * fr;
                
                noise[r][c] += v * amplitude * 30.0;
            }
        }
    }
    
    // Apply terrain and noise
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            double value = terrainDist(rng) + noise[r][c];
            value = std::max(0.0, std::min(255.0, value));
            imageData[r][c] = static_cast<Pixel>(value);
        }
    }
    
    // Insert anomalies
    std::uniform_int_distribution<int> posDist(size / 10, size - size / 10);
    std::uniform_int_distribution<int> sizeDist(size / 20, size / 8);
    std::uniform_real_distribution<double> intensityDist(50.0, 100.0);
    std::bernoulli_distribution brightDist(0.5);
    
    for (int i = 0; i < numAnomalies; i++) {
        int r1 = posDist(rng);
        int c1 = posDist(rng);
        int rSize = sizeDist(rng);
        int cSize = sizeDist(rng);
        int r2 = std::min(r1 + rSize, height - 1);
        int c2 = std::min(c1 + cSize, width - 1);
        
        Region anomalyRegion(r1, c1, r2, c2);
        double intensity = intensityDist(rng);
        bool bright = brightDist(rng);
        
        insertAnomaly(anomalyRegion, intensity, bright);
    }
}

void ImageLoader::generateGradientImage(int size) {
    width = size;
    height = size;
    imageData.resize(height, std::vector<Pixel>(width));
    
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            // Diagonal gradient for testing
            int value = (r + c) * 255 / (2 * size - 2);
            imageData[r][c] = static_cast<Pixel>(value);
        }
    }
}

void ImageLoader::insertAnomaly(const Region& region, double intensity, bool bright) {
    // Gaussian-like falloff for more realistic anomalies
    int centerR = (region.row1 + region.row2) / 2;
    int centerC = (region.col1 + region.col2) / 2;
    double radiusR = (region.row2 - region.row1) / 2.0;
    double radiusC = (region.col2 - region.col1) / 2.0;
    
    for (int r = region.row1; r <= region.row2; r++) {
        for (int c = region.col1; c <= region.col2; c++) {
            if (r < 0 || r >= height || c < 0 || c >= width) continue;
            
            // Distance from center (normalized)
            double dr = (r - centerR) / radiusR;
            double dc = (c - centerC) / radiusC;
            double dist = std::sqrt(dr * dr + dc * dc);
            
            // Gaussian falloff
            double falloff = std::exp(-dist * dist * 2.0);
            double delta = intensity * falloff;
            
            double newValue = imageData[r][c];
            if (bright) {
                newValue += delta;
            } else {
                newValue -= delta;
            }
            
            newValue = std::max(0.0, std::min(255.0, newValue));
            imageData[r][c] = static_cast<Pixel>(newValue);
        }
    }
}

Pixel ImageLoader::getPixel(int row, int col) const {
    if (row < 0 || row >= height || col < 0 || col >= width) {
        return 0;
    }
    return imageData[row][col];
}

bool ImageLoader::saveToPGM(const std::string& filename) const {
    if (imageData.empty()) return false;
    
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;
    
    // Write header (P5 = binary)
    file << "P5\n";
    file << width << " " << height << "\n";
    file << "255\n";
    
    // Write binary data
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            unsigned char byte = imageData[r][c];
            file.write(reinterpret_cast<char*>(&byte), 1);
        }
    }
    
    return true;
}

} // namespace SatelliteAnalytics
