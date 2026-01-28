# SkyMatrix

![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg?style=for-the-badge&logo=c%2B%2B)
![Build](https://img.shields.io/badge/build-Make-orange.svg?style=for-the-badge)
![License](https://img.shields.io/badge/license-MIT-green.svg?style=for-the-badge)
![Status](https://img.shields.io/badge/status-Active-success.svg?style=for-the-badge)
![Algorithms](https://img.shields.io/badge/focus-Algorithms-purple.svg?style=for-the-badge)

**SkyMatrix** is a high-performance, hierarchical satellite image analytics engine designed to efficiently detect anomalies in large-scale terrain data. Built entirely in C++17 without heavy external dependencies, it demonstrates the practical application of advanced algorithmic concepts.

## Overview

Processing gigapixel-scale satellite imagery pixel-by-pixel is computationally prohibitive. SkyMatrix solves this by employing a **hierarchical decomposition strategy** combined with **integral images** (prefix sums) to achieve constant-time `O(1)` region queries and logarithmic-time `O(log n)` spatial searches.

It is engineered for efficiency, modularity, and academic rigor, serving as a comprehensive implementation of Design and Analysis of Algorithms (DAA) principles.

## Key Features & Algorithms

| Feature | Algorithm / Data Structure | Complexity |
|:---|:---|:---|
| **Fast Region Analysis** | 2D Prefix Sums (Dynamic Programming) | **O(1)** Query |
| **Spatial Indexing** | QuadTree Decomposition (Divide & Conquer) | **O(n log n)** Build |
| **Anomaly Detection** | Z-Score Statistical Deviation | **O(1)** per region |
| **Top-K Selection** | Min-Heap (Priority Queue) | **O(k log k)** |
| **Connectivity** | Union-Find (Disjoint Set Union) | **O(n α(n))** |

## Installation & Build

Ensure you have `g++` and `make` installed on your Linux system.

```bash
# Clone the repository
git clone https://github.com/Harsimran-singh-7765/SkyMatrix.git
cd SkyMatrix

# Build the optimized executable
make
```

## Usage

SkyMatrix includes a built-in synthetic data generator, so you can test it immediately without external files.

### 1. Quick Test (Synthetic Data)
Generate a 512x512 terrain and find the top 10 anomalies:
```bash
./satellite_analytics --size 512 --anomalies 10 --topk 10
```

### 2. Advanced Analysis
Run a large-scale simulation with a custom anomaly threshold:
```bash
./satellite_analytics --size 2048 --anomalies 50 --threshold 2.5
```

### 3. Real Satellite Imagery
Use the included helper script to fetch NASA imagery for analysis:
```bash
# Download sample image
./fetch_test_image.py 512

# Run engine on real data
./satellite_analytics --input real_satellite.pgm
```

## Project Structure

*   `src/`: Core C++ implementation files.
*   `include/`: Header files defining the architecture.
*   `docs/`: Detailed algorithmic documentation.
*   `understanding_guide/`: Simplified explanations for beginners.

## Contribution

Contributions are welcome! Please fork the repository and submit a Pull Request.

---
*Designed for the Design & Analysis of Algorithms Course.*
