/**
 * @file main.cpp
 * @brief Main driver for the Hierarchical Satellite Image Analytics Engine
 * 
 * PROJECT: Hierarchical Satellite Image Analytics Engine
 * COURSE: Design and Analysis of Algorithms
 * 
 * This program demonstrates the following algorithms:
 * 
 * 1. DYNAMIC PROGRAMMING: 2D Prefix Sums for O(1) region queries
 * 2. DIVIDE AND CONQUER: QuadTree construction for hierarchical decomposition
 * 3. PRIORITY QUEUE: Top-K anomaly queries using min-heap
 * 4. GRAPH ALGORITHMS: Union-Find and DFS for connected component detection
 * 
 * USAGE:
 *   ./satellite_analytics [options]
 * 
 * OPTIONS:
 *   --size N        Image size NxN (default: 512)
 *   --anomalies N   Number of anomalies to generate (default: 8)
 *   --topk N        Top-K regions to find (default: 10)
 *   --threshold T   Anomaly threshold in std devs (default: 2.0)
 *   --help          Show this help message
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>

#include "Utils.h"
#include "ImageLoader.h"
#include "PrefixSum.h"
#include "RegionTree.h"
#include "AnomalyDetector.h"
#include "QueryEngine.h"
#include "Visualizer.h"

using namespace SatelliteAnalytics;

// ============================================================================
// CONFIGURATION
// ============================================================================

struct AppConfig {
    int imageSize = 512;
    int numAnomalies = 8;
    int topK = 10;
    double threshold = 2.0;
    bool verbose = true;
    bool showVisualization = true;
    int visualScale = 8;
    std::string inputFile = "";
    std::string outputFile = "output_anomalies.pgm";
};

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --size N        Image size NxN (default: 512)\n";
    std::cout << "  --anomalies N   Number of anomalies to generate (default: 8)\n";
    std::cout << "  --topk N        Top-K regions to find (default: 10)\n";
    std::cout << "  --threshold T   Anomaly threshold (default: 2.0 std devs)\n";
    std::cout << "  --input FILE    Load PGM image instead of generating\n";
    std::cout << "  --output FILE   Output file for visualization (default: output_anomalies.pgm)\n";
    std::cout << "  --no-visual     Disable ASCII visualization\n";
    std::cout << "  --quiet         Reduce output verbosity\n";
    std::cout << "  --help          Show this help message\n";
}

AppConfig parseArgs(int argc, char* argv[]) {
    AppConfig cfg;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            cfg.imageSize = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "--anomalies") == 0 && i + 1 < argc) {
            cfg.numAnomalies = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "--topk") == 0 && i + 1 < argc) {
            cfg.topK = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            cfg.threshold = std::stod(argv[++i]);
        } else if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            cfg.inputFile = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            cfg.outputFile = argv[++i];
        } else if (strcmp(argv[i], "--no-visual") == 0) {
            cfg.showVisualization = false;
        } else if (strcmp(argv[i], "--quiet") == 0) {
            cfg.verbose = false;
        }
    }
    
    return cfg;
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main(int argc, char* argv[]) {
    AppConfig cfg = parseArgs(argc, argv);
    
    // Print banner
    printHeader("HIERARCHICAL SATELLITE IMAGE ANALYTICS ENGINE");
    std::cout << "\nA Design and Analysis of Algorithms Project\n";
    std::cout << "Demonstrating: Dynamic Programming, Divide & Conquer,\n";
    std::cout << "               Priority Queues, and Graph Algorithms\n\n";
    
    Timer totalTimer;
    totalTimer.start();
    
    // ========================================================================
    // STAGE 1: IMAGE LOADING / GENERATION
    // ========================================================================
    
    printHeader("STAGE 1: IMAGE ACQUISITION");
    
    ImageLoader loader;
    Timer stageTimer;
    
    if (!cfg.inputFile.empty()) {
        std::cout << "Loading image from: " << cfg.inputFile << "\n";
        stageTimer.start();
        if (!loader.loadFromPGM(cfg.inputFile)) {
            std::cerr << "Error: Failed to load image\n";
            return 1;
        }
        stageTimer.stop();
    } else {
        std::cout << "Generating synthetic satellite image...\n";
        std::cout << "  Size: " << cfg.imageSize << "x" << cfg.imageSize << "\n";
        std::cout << "  Anomalies: " << cfg.numAnomalies << "\n";
        
        stageTimer.start();
        loader.generateSyntheticImage(cfg.imageSize, cfg.numAnomalies, 42);
        stageTimer.stop();
    }
    
    const Matrix& image = loader.getImage();
    std::cout << "\nImage dimensions: " << loader.getHeight() << " x " 
              << loader.getWidth() << "\n";
    std::cout << "Total pixels: " << formatNumber(
        static_cast<int64_t>(loader.getHeight()) * loader.getWidth()) << "\n";
    std::cout << "Image load/generation time: " << formatTime(stageTimer.elapsedMs()) << "\n";
    
    // ========================================================================
    // STAGE 2: PREFIX SUM CONSTRUCTION (DYNAMIC PROGRAMMING)
    // ========================================================================
    
    printHeader("STAGE 2: PREFIX SUM CONSTRUCTION");
    std::cout << "\nBuilding 2D prefix sum matrices using Dynamic Programming...\n";
    std::cout << "This enables O(1) region sum and variance queries.\n";
    
    PrefixSum prefixSum;
    stageTimer.start();
    prefixSum.build(image);
    stageTimer.stop();
    
    std::cout << "\nPrefix sum build time: " << formatTime(stageTimer.elapsedMs()) << "\n";
    std::cout << "Global statistics:\n";
    std::cout << "  Mean: " << std::fixed << std::setprecision(2) 
              << prefixSum.getGlobalMean() << "\n";
    std::cout << "  Std Dev: " << prefixSum.getGlobalStdDev() << "\n";
    
    // Verify prefix sum correctness
    if (cfg.verbose) {
        Region testRegion(0, 0, 31, 31);
        bool correct = prefixSum.verify(image, testRegion);
        std::cout << "  Verification: " << (correct ? "PASSED" : "FAILED") << "\n";
    }
    
    // ========================================================================
    // STAGE 3: REGION TREE CONSTRUCTION (DIVIDE AND CONQUER)
    // ========================================================================
    
    printHeader("STAGE 3: REGION TREE CONSTRUCTION");
    std::cout << "\nBuilding hierarchical QuadTree using Divide and Conquer...\n";
    std::cout << "Each recursive call divides the region into 4 quadrants.\n";
    
    RegionTree regionTree;
    stageTimer.start();
    regionTree.build(&prefixSum, SatelliteAnalytics::Config::MIN_REGION_SIZE);
    stageTimer.stop();
    
    regionTree.printStats();
    
    // ========================================================================
    // STAGE 4: ANOMALY DETECTION
    // ========================================================================
    
    printHeader("STAGE 4: ANOMALY DETECTION");
    std::cout << "\nDetecting anomalies using Z-score deviation...\n";
    std::cout << "Threshold: " << cfg.threshold << " standard deviations\n";
    
    AnomalyDetector detector(cfg.threshold);
    detector.initialize(&prefixSum);
    
    stageTimer.start();
    detector.detectInTree(regionTree);
    stageTimer.stop();
    
    const auto& stats = detector.getStats();
    std::cout << "\nDetection Results:\n";
    std::cout << "  Total regions analyzed: " << formatNumber(stats.totalRegions) << "\n";
    std::cout << "  Anomalous regions: " << formatNumber(stats.anomalousRegions) 
              << " (" << std::fixed << std::setprecision(1) 
              << (100.0 * stats.anomalousRegions / stats.totalRegions) << "%)\n";
    std::cout << "  Score range: [" << std::setprecision(3) << stats.minScore 
              << ", " << stats.maxScore << "]\n";
    std::cout << "  Detection time: " << formatTime(stats.detectionTimeMs) << "\n";
    
    // ========================================================================
    // STAGE 5: QUERY EXECUTION
    // ========================================================================
    
    printHeader("STAGE 5: QUERY EXECUTION");
    
    QueryEngine queryEngine;
    queryEngine.initialize(&regionTree, &prefixSum, &detector);
    Visualizer visualizer;
    
    // --- TOP-K QUERY (Priority Queue) ---
    std::cout << "\n--- Query 1: Top-" << cfg.topK << " Anomalous Regions ---\n";
    std::cout << "Using min-heap priority queue for efficient selection.\n";
    std::cout << "Time complexity: O(n log k)\n";
    
    auto topKResult = queryEngine.topKAnomalies(cfg.topK, true);
    visualizer.printQueryResult(topKResult, "Top-K Anomalies");
    visualizer.printAnomalySummary(topKResult.regions);
    
    // --- TOP-K WITH PRUNING ---
    std::cout << "\n--- Query 2: Top-" << cfg.topK << " with Tree Pruning ---\n";
    std::cout << "Enhanced version that prunes subtrees early.\n";
    
    auto prunedResult = queryEngine.topKWithPruning(cfg.topK);
    visualizer.printQueryResult(prunedResult, "Top-K with Pruning");
    
    double pruneEfficiency = 100.0 * prunedResult.nodesPruned / 
                            (prunedResult.nodesVisited + prunedResult.nodesPruned);
    std::cout << "  Pruning efficiency: " << std::fixed << std::setprecision(1) 
              << pruneEfficiency << "% of nodes skipped\n";
    
    // --- CONNECTED COMPONENTS (Union-Find) ---
    std::cout << "\n--- Query 3: Connected Components (Union-Find) ---\n";
    std::cout << "Finding connected anomalous regions using Union-Find.\n";
    std::cout << "Uses path compression and union by rank.\n";
    std::cout << "Time complexity: O(n × α(n)) ≈ O(n)\n";
    
    stageTimer.start();
    auto components = queryEngine.findConnectedComponents();
    stageTimer.stop();
    
    std::cout << "\nConnected components found: " << components.size() << "\n";
    std::cout << "Query time: " << formatTime(stageTimer.elapsedMs()) << "\n";
    
    if (!components.empty()) {
        visualizer.printComponentSummary(components);
        
        // --- LARGEST CONNECTED REGION ---
        std::cout << "\n--- Query 4: Largest Connected Anomalous Region ---\n";
        const auto& largest = components[0];
        std::cout << "Component ID: " << largest.id << "\n";
        std::cout << "Total area: " << formatNumber(largest.totalArea) << " pixels\n";
        std::cout << "Number of regions: " << largest.nodeIndices.size() << "\n";
        std::cout << "Bounding box: [" << largest.boundingBox.row1 << "," 
                  << largest.boundingBox.col1 << "]-[" 
                  << largest.boundingBox.row2 << "," 
                  << largest.boundingBox.col2 << "]\n";
    }
    
    // --- CONNECTED COMPONENTS (DFS Alternative) ---
    std::cout << "\n--- Query 5: Connected Components (DFS) ---\n";
    std::cout << "Alternative approach using graph DFS.\n";
    std::cout << "Time complexity: O(n + m) where m = edges\n";
    
    stageTimer.start();
    auto dfsComponents = queryEngine.findConnectedComponentsDFS();
    stageTimer.stop();
    
    std::cout << "DFS found " << dfsComponents.size() << " components\n";
    std::cout << "Query time: " << formatTime(stageTimer.elapsedMs()) << "\n";
    
    // --- RECTANGULAR REGION QUERY ---
    std::cout << "\n--- Query 6: Rectangular Region Query ---\n";
    std::cout << "Finding anomalies within a specific region.\n";
    
    int querySize = std::min(256, cfg.imageSize / 2);
    Region queryRegion(0, 0, querySize - 1, querySize - 1);
    std::cout << "Query region: [0,0]-[" << querySize-1 << "," << querySize-1 << "]\n";
    
    auto rectResult = queryEngine.queryRectangle(queryRegion);
    visualizer.printQueryResult(rectResult, "Rectangle Query");
    
    // ========================================================================
    // STAGE 6: VISUALIZATION
    // ========================================================================
    
    if (cfg.showVisualization) {
        printHeader("STAGE 6: VISUALIZATION");
        
        // Calculate appropriate scale
        int scale = std::max(1, cfg.imageSize / 64);
        
        std::cout << "\n--- Original Image (scaled) ---\n";
        visualizer.renderASCII(image, scale);
        
        std::cout << "\n--- Anomaly Map ---\n";
        visualizer.renderAnomalyMap(image, regionTree, scale);
        
        if (!components.empty()) {
            std::cout << "\n--- Connected Components ---\n";
            visualizer.renderComponents(image, components, scale);
        }
        
        // Save output image
        auto overlayImage = visualizer.createAnomalyOverlay(image, regionTree);
        if (visualizer.savePGM(overlayImage, cfg.outputFile)) {
            std::cout << "\nSaved anomaly overlay to: " << cfg.outputFile << "\n";
        }
    }
    
    // ========================================================================
    // SUMMARY
    // ========================================================================
    
    totalTimer.stop();
    
    printHeader("EXECUTION SUMMARY");
    
    std::cout << "\n+----------------------------------+------------------+\n";
    std::cout << "| Metric                           | Value            |\n";
    std::cout << "+----------------------------------+------------------+\n";
    std::cout << "| Image size                       | " << std::setw(16) 
              << (std::to_string(loader.getHeight()) + "x" + 
                  std::to_string(loader.getWidth())) << " |\n";
    std::cout << "| Total pixels                     | " << std::setw(16)
              << formatNumber(static_cast<int64_t>(loader.getHeight()) * 
                             loader.getWidth()) << " |\n";
    std::cout << "| Tree nodes                       | " << std::setw(16)
              << formatNumber(regionTree.getNodeCount()) << " |\n";
    std::cout << "| Leaf regions                     | " << std::setw(16)
              << formatNumber(regionTree.getLeafCount()) << " |\n";
    std::cout << "| Anomalous regions                | " << std::setw(16)
              << formatNumber(stats.anomalousRegions) << " |\n";
    std::cout << "| Connected components             | " << std::setw(16)
              << components.size() << " |\n";
    std::cout << "+----------------------------------+------------------+\n";
    std::cout << "| Total execution time             | " << std::setw(16)
              << formatTime(totalTimer.elapsedMs()) << " |\n";
    std::cout << "+----------------------------------+------------------+\n";
    
    std::cout << "\n";
    printHeader("COMPLEXITY ANALYSIS");
    
    std::cout << R"(
+---------------------------+---------------+---------------+
| Operation                 | Time          | Space         |
+---------------------------+---------------+---------------+
| Image loading             | O(n²)         | O(n²)         |
| Prefix sum build          | O(n²)         | O(n²)         |
| Region tree build         | O(n²/B²)      | O(n²/B²)      |
| Anomaly detection         | O(n²/B²)      | O(1)          |
| Top-K query               | O(m log k)    | O(k)          |
| Connected components (UF) | O(m² α(m))    | O(m)          |
| Connected components (DFS)| O(m + edges)  | O(m)          |
| Region query              | O(log n + r)  | O(r)          |
+---------------------------+---------------+---------------+

Where:
  n = image dimension
  B = minimum region size ()" << SatelliteAnalytics::Config::MIN_REGION_SIZE << R"()
  m = number of leaf regions (n²/B²)
  k = query parameter
  r = result size
  α = inverse Ackermann function (effectively constant)
)";
    
    std::cout << "\n";
    printDivider('=', 70);
    std::cout << "Program completed successfully.\n";
    printDivider('=', 70);
    
    return 0;
}
