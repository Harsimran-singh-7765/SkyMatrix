# SkyMatrix

![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg?style=for-the-badge&logo=c%2B%2B)
![Build](https://img.shields.io/badge/build-Make-orange.svg?style=for-the-badge)
![License](https://img.shields.io/badge/license-MIT-green.svg?style=for-the-badge)
![Status](https://img.shields.io/badge/status-Active-success.svg?style=for-the-badge)
![Algorithms](https://img.shields.io/badge/focus-Algorithms-purple.svg?style=for-the-badge)


### Hierarchical Satellite Image Analytics Engine for Large-Scale Change Detection

SkyMatrix is a high-performance satellite image analytics engine built in **C++17**, designed to efficiently detect large-scale terrain changes in massive satellite imagery using algorithmic optimization techniques.

Instead of processing entire satellite images using computationally expensive AI models, SkyMatrix intelligently identifies **regions of interest** using fast spatial algorithms, enabling scalable and cost-efficient analysis.

---

##  Why SkyMatrix?

Modern satellites generate extremely large images covering forests, oceans, and land regions. Processing these images entirely using deep learning models is inefficient because:

* Most regions remain unchanged.
* Machine learning inference over full-resolution imagery is expensive.
* Large-scale monitoring requires real-time or near real-time processing.

SkyMatrix solves this by answering the first and most important question:

> **Where should AI look?**

By reducing the search space before AI processing, SkyMatrix improves efficiency, scalability, and system performance.

---

##  Real-World Application

### Forest Change & Deforestation Monitoring

Environmental agencies continuously monitor forests using satellite imagery to detect:

* Illegal logging
* Forest degradation
* Land-use changes
* Mining or construction activity

Running CNN models on entire satellite images is computationally expensive and prone to unnecessary processing.

SkyMatrix introduces a two-stage intelligent pipeline:

```
Satellite Image
        ↓
SkyMatrix
(Fast statistical change detection)
        ↓
Suspicious Regions (Top-K anomalies)
        ↓
CNN / AI Model
(Change classification)
        ↓
Deforestation / Settlement / Agriculture / Natural Change
```

### Key Idea

SkyMatrix does **not classify deforestation directly**.

Instead, it detects **statistical deviations** in forest patterns (vegetation density, brightness variation, texture changes), allowing AI models to focus only on meaningful regions.

This reduces false positives and improves processing efficiency.

---

##  Core Design Philosophy

SkyMatrix combines classical algorithmic techniques with modern AI workflows:

| SkyMatrix Role               | AI Model Role                   |
| ---------------------------- | ------------------------------- |
| Fast large-scale filtering   | Detailed semantic understanding |
| Detect *where change exists* | Identify *what the change is*   |
| Algorithm-driven             | Learning-driven                 |

This hybrid design reflects how real-world large-scale geospatial systems operate.

---

## ⚙️ Key Features & Algorithms

| Feature               | Algorithm / Data Structure | Purpose                         | Complexity |
| --------------------- | -------------------------- | ------------------------------- | ---------- |
| Fast Region Queries   | 2D Prefix Sums             | Constant-time region statistics | O(1)       |
| Spatial Indexing      | QuadTree Decomposition     | Hierarchical image division     | O(n log n) |
| Change Detection      | Z-Score Analysis           | Detect abnormal regions         | O(1)       |
| Top-K Selection       | Min Heap                   | Efficient anomaly ranking       | O(k log k) |
| Connectivity Analysis | Union-Find (DSU)           | Region grouping                 | O(n α(n))  |

---

##  How It Works

###  Hierarchical Decomposition

The image is recursively divided into smaller regions using a QuadTree structure. Uniform regions are skipped while areas with higher variation are explored further.

###  Prefix Sum Preprocessing

A prefix sum matrix allows instant computation of region averages and statistics without repeatedly scanning pixels.

###  Statistical Change Detection

Regions significantly deviating from normal patterns are marked as anomalies.

###  Candidate Selection

A min-heap stores the most suspicious regions, which can be passed to AI models for detailed analysis.

---

##  Installation & Build

Ensure `g++` and `make` are installed.

```bash
git clone https://github.com/Harsimran-singh-7765/SkyMatrix.git
cd SkyMatrix
make
```

---

## Usage

### Quick Test (Synthetic Data)

```bash
./satellite_analytics --size 512 --anomalies 10 --topk 10
```

---

### Large Simulation

```bash
./satellite_analytics --size 2048 --anomalies 50 --threshold 2.5
```

---

### Real Satellite Image

```bash
./fetch_test_image.py 512
./satellite_analytics --input real_satellite.pgm
```

---

##  Project Structure

```
src/                    Core implementation
include/                Header files
docs/                   Algorithm documentation
understanding_guide/    Beginner-friendly explanations
```

---

##  Academic Foundations

SkyMatrix demonstrates real-world applications of:

* Divide & Conquer
* Dynamic Programming
* Spatial Data Structures
* Statistical Analysis
* Priority Queues
* Amortized Complexity

Designed as a practical implementation for **Design & Analysis of Algorithms (DAA)**.

---

##  Future Improvements

* CNN integration for automated change classification
* Multi-temporal satellite comparison
* Parallel QuadTree construction
* GPU acceleration
* Visual anomaly overlays

---

##  Contribution

Contributions are welcome. Please fork the repository and submit a Pull Request.

---

##  Author

**Harsimran Singh**
SkyMatrix — Algorithmic Intelligence for Large-Scale Earth Observation


