/**
 * @file ImageLoader.h
 * @brief Image loading and generation for the Satellite Image Analytics Engine
 * 
 * Supports:
 * - Loading grayscale PGM (P2/P5) images
 * - Generating synthetic satellite images for testing
 * 
 * Time Complexity: O(n²) where n is the image dimension
 * Space Complexity: O(n²) for storing the image matrix
 */

#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include "Utils.h"
#include <string>
#include <random>

namespace SatelliteAnalytics {

/**
 * @class ImageLoader
 * @brief Handles image loading and synthetic generation
 * 
 * For a Design and Analysis of Algorithms course, we focus on:
 * - Efficient matrix representation
 * - Synthetic data generation with controlled anomalies
 * 
 * The class provides methods to load real images or generate
 * test images with known anomaly regions for validation.
 */
class ImageLoader {
private:
    Matrix imageData;
    int height;
    int width;
    std::mt19937 rng;  // Mersenne Twister for high-quality random numbers
    
    /**
     * @brief Parse PGM header and validate format
     * @return true if valid PGM file
     */
    bool parsePGMHeader(std::ifstream& file, int& w, int& h, int& maxVal);

public:
    ImageLoader();
    
    // ========================================================================
    // IMAGE LOADING
    // ========================================================================
    
    /**
     * @brief Load a grayscale PGM image from file
     * @param filename Path to the PGM file
     * @return true if loading successful
     * 
     * Supports both ASCII (P2) and binary (P5) PGM formats.
     * Time Complexity: O(n²)
     */
    bool loadFromPGM(const std::string& filename);
    
    /**
     * @brief Load image from raw grayscale buffer
     * @param data Pointer to pixel data (row-major order)
     * @param w Image width
     * @param h Image height
     * 
     * Time Complexity: O(n²)
     */
    void loadFromBuffer(const Pixel* data, int w, int h);
    
    // ========================================================================
    // SYNTHETIC IMAGE GENERATION
    // ========================================================================
    
    /**
     * @brief Generate a synthetic satellite image with anomalies
     * @param size Image dimension (creates size x size image)
     * @param numAnomalies Number of anomalous regions to insert
     * @param seed Random seed for reproducibility
     * 
     * Generates a realistic-looking satellite image with:
     * - Base terrain with gradual variations (Perlin-like noise)
     * - Normal statistical distribution for regular areas
     * - Distinct anomalous regions (bright or dark spots)
     * 
     * Time Complexity: O(n²)
     */
    void generateSyntheticImage(int size, int numAnomalies = 5, unsigned int seed = 42);
    
    /**
     * @brief Generate simple gradient test image
     * @param size Image dimension
     * 
     * Creates a gradient for testing prefix sum correctness.
     * Time Complexity: O(n²)
     */
    void generateGradientImage(int size);
    
    /**
     * @brief Insert an anomalous region at specified location
     * @param region The region bounds
     * @param intensity Anomaly intensity (deviation from mean)
     * @param bright true for bright anomaly, false for dark
     */
    void insertAnomaly(const Region& region, double intensity, bool bright = true);
    
    // ========================================================================
    // ACCESSORS
    // ========================================================================
    
    const Matrix& getImage() const { return imageData; }
    Matrix& getImageMutable() { return imageData; }
    int getHeight() const { return height; }
    int getWidth() const { return width; }
    
    /**
     * @brief Get pixel value at coordinates
     * @return Pixel value or 0 if out of bounds
     */
    Pixel getPixel(int row, int col) const;
    
    /**
     * @brief Check if image is loaded
     */
    bool isLoaded() const { return !imageData.empty(); }
    
    /**
     * @brief Save image to PGM file
     * @param filename Output file path
     * @return true if saving successful
     */
    bool saveToPGM(const std::string& filename) const;
};

} // namespace SatelliteAnalytics

#endif // IMAGE_LOADER_H
