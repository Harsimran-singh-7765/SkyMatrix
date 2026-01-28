# SkyMatrix: Hierarchical Satellite Image Analytics Engine

## Project Documentation for Design and Analysis of Algorithms

---

## Table of Contents

1. [Problem Statement](#1-problem-statement)
2. [Why Naive Approaches Are Inefficient](#2-why-naive-approaches-are-inefficient)
3. [System Architecture](#3-system-architecture)
4. [Module Descriptions](#4-module-descriptions)
5. [Algorithms Used](#5-algorithms-used)
6. [Complexity Analysis](#6-complexity-analysis)
7. [Academic Alignment](#7-academic-alignment)
8. [How to Compile and Run](#8-how-to-compile-and-run)
9. [Sample Output](#9-sample-output)

---

## 1. Problem Statement

### Background
Satellite imagery is fundamental to remote sensing applications including:
- Environmental monitoring
- Agricultural analysis  
- Urban planning
- Disaster response

Modern satellite images are **extremely large** — often thousands of pixels per side, resulting in matrices with millions of elements.

### The Challenge
Given a satellite image, we need to efficiently:
1. **Detect anomalous regions** (areas that deviate significantly from the norm)
2. **Find the Top-K most anomalous regions**
3. **Identify the largest connected anomalous area**
4. **Answer interactive queries** without re-scanning the entire image

### Core Insight
Treating the image as a matrix and applying classical algorithms from Design and Analysis of Algorithms enables us to build a system that:
- **Preprocesses once**: O(n²) build time
- **Answers queries fast**: O(1) to O(k log k) per query

---

## 2. Why Naive Approaches Are Inefficient

### Naive Approach: Per-Query Full Scan

For each query, scan all pixels in the region of interest:

```
For each query Q:
    For each pixel (i,j) in Q:
        sum += image[i][j]
    mean = sum / area
```

**Time Complexity**: O(n²) per query

If we have K queries: **O(K × n²)** — unacceptable for interactive systems.

### Naive Anomaly Detection

Compare each region against the global mean:

```
For each region R:
    For each pixel in R:
        compute sum, sum of squares
    compute mean, variance
    compare to global statistics
```

**Time Complexity**: O(regions × pixels_per_region)

### Naive Connected Component Detection

Check every pair of regions for adjacency:

```
For each pair (R1, R2):
    if adjacent(R1, R2) and both anomalous:
        merge into same component
```

**Time Complexity**: O(n⁴) in the worst case

---

## 3. System Architecture

```
┌──────────────────────────────────────────────────────────────────┐
│                     SATELLITE IMAGE INPUT                        │
│                    (Grayscale n×n matrix)                        │
└──────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌──────────────────────────────────────────────────────────────────┐
│               PREPROCESSING PHASE (One-time O(n²))               │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────────┐    ┌─────────────────────────────────┐ │
│  │    PREFIX SUM       │    │       REGION TREE               │ │
│  │    (DP: O(n²))      │    │   (Divide & Conquer: O(n²))     │ │
│  ├─────────────────────┤    ├─────────────────────────────────┤ │
│  │ • 2D Prefix Sum     │    │ • QuadTree decomposition        │ │
│  │ • Prefix Sum²       │    │ • Hierarchical statistics       │ │
│  │ • Global stats      │    │ • Spatial indexing              │ │
│  └─────────────────────┘    └─────────────────────────────────┘ │
│                                                                  │
│                              │                                   │
│                              ▼                                   │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │              ANOMALY DETECTOR (Z-score: O(n²/B²))           │ │
│  ├─────────────────────────────────────────────────────────────┤ │
│  │ • Score = |region_mean - global_mean| / global_stddev       │ │
│  │ • Flag regions where score > threshold                      │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌──────────────────────────────────────────────────────────────────┐
│                    QUERY PHASE (Fast)                            │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────┐  ┌──────────────────┐  ┌─────────────────┐ │
│  │   TOP-K QUERY   │  │ CONNECTED COMP.  │  │  REGION QUERY   │ │
│  │ (Heap: O(n log k))│  │ (UF: O(n α(n))) │  │ (Tree: O(log n))│ │
│  └─────────────────┘  └──────────────────┘  └─────────────────┘ │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌──────────────────────────────────────────────────────────────────┐
│                    OUTPUT & VISUALIZATION                        │
│              (ASCII rendering, PGM output)                       │
└──────────────────────────────────────────────────────────────────┘
```

---

## 4. Module Descriptions

### 4.1 Utils (Utils.h / Utils.cpp)

**Purpose**: Common types and utilities

**Key Components**:
- `Region`: Rectangular bounds with area/validity checks
- `RegionStats`: Statistical measures (mean, variance, stddev)
- `Timer`: High-resolution performance measurement
- `Config`: System configuration constants

### 4.2 ImageLoader (ImageLoader.h / ImageLoader.cpp)

**Purpose**: Image input and synthetic generation

**Key Features**:
- Load PGM format images (P2 ASCII, P5 binary)
- Generate synthetic satellite images with:
  - Multi-octave noise for realistic terrain
  - Configurable anomaly regions
- Save processed images

**Complexity**: O(n²)

### 4.3 PrefixSum (PrefixSum.h / PrefixSum.cpp)

**Purpose**: Enable O(1) region queries via Dynamic Programming

**Algorithm**: 2D Prefix Sum (Integral Image)

**Recurrence Relation**:
```
prefix[i][j] = image[i][j] + prefix[i-1][j] + prefix[i][j-1] - prefix[i-1][j-1]
```

**Query Formula** (Inclusion-Exclusion):
```
sum(r1,c1,r2,c2) = prefix[r2+1][c2+1] - prefix[r1][c2+1] 
                  - prefix[r2+1][c1] + prefix[r1][c1]
```

**Variance in O(1)**:
```
Var(X) = E[X²] - (E[X])²

E[X²] = (sum of squares) / area
E[X]  = mean = sum / area
```

**Complexity**:
- Build: O(n²)
- Query: O(1)

### 4.4 RegionTree (RegionTree.h / RegionTree.cpp)

**Purpose**: Hierarchical spatial decomposition

**Algorithm**: Divide and Conquer (QuadTree)

**Construction**:
```
function buildTree(region, depth):
    create node for region
    compute statistics using prefix sums
    
    if region.size <= MIN_SIZE:
        return node as leaf
    
    split region into 4 quadrants (NW, NE, SW, SE)
    
    for each quadrant:
        node.child[i] = buildTree(quadrant, depth + 1)
    
    return node
```

**Complexity**:
- Build: O(n²/B²) nodes, each O(1) using prefix sums
- Traversal: O(nodes visited), with pruning O(log n)

### 4.5 AnomalyDetector (AnomalyDetector.h / AnomalyDetector.cpp)

**Purpose**: Statistical outlier detection

**Algorithm**: Z-Score Based Detection

**Formula**:
```
anomalyScore = |region_mean - global_mean| / global_stddev
isAnomaly = (anomalyScore > threshold)
```

**Why This Works**:
- No training required (pure statistics)
- Interpretable threshold (e.g., 2σ means ~5% false positive rate)
- O(1) per region using prefix sums

### 4.6 QueryEngine (QueryEngine.h / QueryEngine.cpp)

**Purpose**: Efficient query processing

**Algorithm 1: Top-K using Min-Heap**
```
function topK(k):
    heap = MinHeap of size k
    
    for each region:
        if heap.size < k:
            heap.push(region)
        else if region.score > heap.top().score:
            heap.pop()
            heap.push(region)
    
    return heap.extractAll().reverse()
```

**Complexity**: O(n log k)

**Algorithm 2: Connected Components using Union-Find**
```
function findComponents():
    uf = UnionFind(n)
    
    for each pair (region_i, region_j):
        if adjacent(i, j) and both anomalous:
            uf.unite(i, j)
    
    group regions by uf.find(i)
    return components
```

**Union-Find Optimizations**:
1. **Path Compression**: During find(), point nodes directly to root
2. **Union by Rank**: Attach smaller tree under larger tree

**Complexity**: O(n × α(n)) per operation, where α is inverse Ackermann (effectively constant)

**Algorithm 3: DFS Alternative**
```
function findComponentsDFS():
    build adjacency list
    visited = [false] × n
    
    for each unvisited node:
        component = DFS(node)
        add component to results
    
    return components
```

**Complexity**: O(n + m) where m = number of edges

### 4.7 Visualizer (Visualizer.h / Visualizer.cpp)

**Purpose**: Result presentation

**Features**:
- ASCII art rendering (pixel → character gradient)
- Anomaly highlighting
- Component visualization
- PGM file output

---

## 5. Algorithms Used

| Algorithm | Module | Purpose | Paradigm |
|-----------|--------|---------|----------|
| 2D Prefix Sum | PrefixSum | O(1) region queries | Dynamic Programming |
| QuadTree Construction | RegionTree | Hierarchical decomposition | Divide & Conquer |
| Min-Heap Selection | QueryEngine | Top-K queries | Priority Queue |
| Union-Find | QueryEngine | Connected components | Graph/DSU |
| DFS | QueryEngine | Connected components | Graph Traversal |
| Z-Score Computation | AnomalyDetector | Anomaly scoring | Statistics |

---

## 6. Complexity Analysis

### Space Complexity

| Structure | Space |
|-----------|-------|
| Image matrix | O(n²) |
| Prefix sum matrix | O(n²) |
| Prefix sum² matrix | O(n²) |
| Region tree nodes | O(n²/B²) |
| Union-Find arrays | O(m) where m = #regions |
| **Total** | **O(n²)** |

### Time Complexity

| Operation | Time | Notes |
|-----------|------|-------|
| Image loading | O(n²) | Read all pixels |
| Prefix sum build | O(n²) | DP recurrence |
| Region tree build | O(n²/B²) | Each node O(1) |
| Anomaly detection | O(n²/B²) | Score all regions |
| Top-K query | O(m log k) | m = #regions |
| Connected components (UF) | O(m² α(m)) | Pairwise adjacency |
| Connected components (DFS) | O(m + edges) | Graph traversal |
| Region sum query | O(1) | Prefix sum lookup |
| Region variance query | O(1) | Prefix sum lookup |

Where:
- n = image dimension
- B = MIN_REGION_SIZE (default 16)
- m = number of leaf regions ≈ (n/B)²
- k = Top-K parameter
- α(n) = inverse Ackermann function (< 5 for practical n)

### Comparison with Naive Approaches

| Task | Naive | Our Approach | Speedup |
|------|-------|--------------|---------|
| K region sum queries | O(K × n²) | O(n² + K) | ~K× |
| Find all anomalies | O(n⁴) | O(n²) | ~n²× |
| Top-K search | O(n² log n²) | O(m log k) | ~n²/m × log(n)/log(k) |

---

## 7. Academic Alignment

This project demonstrates mastery of core DAA concepts:

### 7.1 Algorithm Design Paradigms

1. **Divide and Conquer**
   - QuadTree recursively splits problem into subproblems
   - Each level halves the region size
   - Natural logarithmic depth

2. **Dynamic Programming**
   - 2D prefix sums use optimal substructure
   - Each cell depends on previously computed cells
   - Enables O(1) queries after O(n²) preprocessing

3. **Greedy (implicit)**
   - Top-K algorithm greedily maintains best k elements
   - Union by rank greedily optimizes tree balance

### 7.2 Data Structures

1. **Priority Queue (Heap)**
   - Used for efficient Top-K selection
   - Min-heap maintains k largest elements
   - O(log k) insertion/deletion

2. **Disjoint Set Union (Union-Find)**
   - Path compression optimization
   - Union by rank optimization
   - Near-constant time operations

3. **Trees (QuadTree)**
   - Hierarchical spatial index
   - Enables pruned traversal
   - Logarithmic query paths

### 7.3 Graph Algorithms

1. **Depth-First Search (DFS)**
   - Alternative connected component detection
   - Classic graph exploration
   - O(V + E) complexity

2. **Union-Find**
   - Efficient connected component management
   - Demonstrates amortized analysis

### 7.4 Complexity Analysis

- **Asymptotic notation**: O, Ω, Θ analysis throughout
- **Amortized analysis**: Union-Find operations
- **Space-time tradeoffs**: Prefix sums trade O(n²) space for O(1) queries

---

## 8. How to Compile and Run

### Prerequisites

- g++ compiler with C++17 support
- Make build system

### Compilation

```bash
# Navigate to project directory
cd /path/to/DAA_pbl

# Build optimized version
make

# Or build debug version
make debug
```

### Running

```bash
# Default run (512x512 image)
./satellite_analytics

# Custom parameters
./satellite_analytics --size 1024 --anomalies 10 --topk 15 --threshold 1.5

# See all options
./satellite_analytics --help
```

### Command-Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--size N` | Image dimension (NxN) | 512 |
| `--anomalies N` | Number of anomalies | 8 |
| `--topk N` | Top-K parameter | 10 |
| `--threshold T` | Anomaly threshold (std devs) | 2.0 |
| `--input FILE` | Load PGM file instead of generating | - |
| `--output FILE` | Output visualization file | output_anomalies.pgm |
| `--no-visual` | Disable ASCII visualization | - |
| `--quiet` | Reduce output verbosity | - |

### Makefile Targets

```bash
make           # Build optimized version
make debug     # Build debug version
make run       # Build and run
make run-small # Run with 256x256 image
make run-large # Run with 1024x1024 image
make clean     # Remove build artifacts
make help      # Show all targets
```

---

## 9. Sample Output

```
======================================================================
  HIERARCHICAL SATELLITE IMAGE ANALYTICS ENGINE
======================================================================

A Design and Analysis of Algorithms Project
Demonstrating: Dynamic Programming, Divide & Conquer,
               Priority Queues, and Graph Algorithms

======================================================================
  STAGE 1: IMAGE ACQUISITION
======================================================================
Generating synthetic satellite image...
  Size: 512x512
  Anomalies: 8

Image dimensions: 512 x 512
Total pixels: 262,144
Image load/generation time: 45.23 ms

======================================================================
  STAGE 2: PREFIX SUM CONSTRUCTION
======================================================================

Building 2D prefix sum matrices using Dynamic Programming...
This enables O(1) region sum and variance queries.

Prefix sum build time: 12.45 ms
Global statistics:
  Mean: 127.34
  Std Dev: 28.76
  Verification: PASSED

======================================================================
  STAGE 3: REGION TREE CONSTRUCTION
======================================================================

Building hierarchical QuadTree using Divide and Conquer...
Each recursive call divides the region into 4 quadrants.

--- Region Tree Statistics ---
Total nodes: 1,365
Leaf nodes: 1,024
Internal nodes: 341
Maximum depth: 5
Min region size: 16x16
Build time: 3.21 ms

======================================================================
  STAGE 5: QUERY EXECUTION
======================================================================

--- Query 1: Top-10 Anomalous Regions ---
Using min-heap priority queue for efficient selection.
Time complexity: O(n log k)

Query: Top-K Anomalies
  Results: 10
  Nodes visited: 1,024
  Time: 0.89 ms

----------------------------------------------------------------------
Rank  Region                   Score          Area           
----------------------------------------------------------------------
1     [112,304]-[127,319]      4.523          256            
2     [256,192]-[271,207]      4.156          256            
...

======================================================================
  EXECUTION SUMMARY
======================================================================

+----------------------------------+------------------+
| Metric                           | Value            |
+----------------------------------+------------------+
| Image size                       |          512x512 |
| Total pixels                     |          262,144 |
| Tree nodes                       |            1,365 |
| Anomalous regions                |               87 |
| Connected components             |               12 |
+----------------------------------+------------------+
| Total execution time             |         156.7 ms |
+----------------------------------+------------------+
```

---

## Conclusion

This project demonstrates a complete algorithmic system for satellite image analysis. By combining:

- **Dynamic Programming** (prefix sums)
- **Divide and Conquer** (QuadTree)
- **Priority Queues** (Top-K selection)
- **Graph Algorithms** (Union-Find, DFS)

We achieve efficient preprocessing and fast query answering, transforming what would be O(n²) per-query operations into O(1) to O(log n) operations.

This represents a strong application of Design and Analysis of Algorithms principles to a practical problem domain.
