# SkyMatrix: Your Satellite Analytics Engine

This guide explains how your project works in simple language. Think of this system like "Google Maps" but instead of finding traffic, it finds "anomalies" (strange things) in satellite images.

---

## 1. What are we trying to do? ("Kya kar rahe hain?")

We have a large satellite image (a grid of numbers). We want to find areas that look "weird" (anomalous) compared to the rest of the image.

**Real-world example:** Finding a forest fire (hot/bright spot) in a dark forest image.

**The Problem:** The image is huge. Checking every pixel one-by-one is too slow.
**Our Solution:** We use smart algorithms (Divide & Conquer, Prefix Sums) to do it instantly.

---

## 2. How to "Navigate" and Run the Code

You entered several commands. Here is what each one did:

### Step 1: `make`
> *"Make se kya ho raha hai?"*

*   **Make** is a helper tool. It looks at your `Makefile` (the recipe).
*   It reads the recipe: "Take all `.cpp` files (source code) and turn them into an executable program called `satellite_analytics`."
*   If you say `make: Nothing to be done for 'all'`, it means you haven't changed any code since the last time you ran it. Your program is already built and ready!

### Step 2: `./satellite_analytics`
> *"Uske baad..."*

*   This runs your main program.
*   By default, it generates a **fake** 512x512 image (since you didn't give it a real one) and hunts for anomalies.

### Step 3: `./fetch_test_image.py`
> *"Ye kya hai?"*

*   This is a Python script I wrote for you.
*   It goes to the internet (NASA website), downloads a real picture of Earth, turns it black-and-white (PGM format), and saves it as `real_satellite.pgm`.
*   This proves your code works on **real data**, not just fake numbers!

---

## 3. Explaining Your Output (Line by Line)

Here is the breakdown of the log you pasted:

### STAGE 1: Image Acquisition
```text
Generating synthetic satellite image...
Size: 1024x1024
Total pixels: 1,048,576
Image load/generation time: 62.38 ms
```
*   **What happened:** The computer created a grid of 1 million pixels (1024x1024). It added some mountains and "noise" to look like land.
*   **Speed:** It took ~62 milliseconds.

### STAGE 2: Prefix Sum Construction
```text
Building 2D prefix sum matrices using Dynamic Programming...
Prefix sum build time: 7.64 ms
```
*   **The Magic Trick:** We build a special table (Prefix Sum) that lets us calculate the sum of ANY rectangular area in `O(1)` time (instantly).
*   **Why:** Without this, checking thousands of regions would take forever.

### STAGE 3: Region Tree Construction
```text
Building hierarchical QuadTree using Divide and Conquer...
Leaf nodes: 4,096
Build time: 637.98 µs
```
*   **Divide and Conquer:** We cut the image into 4 pieces, then those into 4 pieces, until we have small blocks (16x16 size).
*   **Result:** We have 4,096 small blocks to check.

### STAGE 4: Anomaly Detection
```text
Detecting anomalies using Z-score deviation...
Anomalous regions: 0 (0.0%)
```
*   **The Check:** For each block, we ask: "Is this block very different from the average?"
*   **Result:** In your 1024x1024 run, it found **0 anomalies** because the threshold (1.5) was maybe too high for that specific random image. (Don't worry, the real image test found 60!)

### STAGE 5: Query Execution
```text
Query: Top-K Anomalies
Results: 15
Time: 27.67 µs
```
*   **Priority Queue:** We asked for the "Top 15 weirdest spots".
*   Even though it found 0 "official" anomalies earlier (above the strict threshold), the Top-K query **forces** it to show the top 15 candidates, even if they aren't very weird.
*   **Speed:** 27 microseconds (0.000027 seconds). Super fast!

### STAGE 6: Visualization
```text
Legend: 'X' = Anomalous region
Saved anomaly overlay to: output_anomalies.pgm
```
*   **Output:** It created a picture file `output_anomalies.pgm` where the weird spots are highlighted so you can see them.

---

## Summary of Files ("Files kaise call ho rahi hain")

*   **`main.cpp`**: The Boss. It calls everyone else.
*   **`ImageLoader`**: The Artist. Draws the fake image or loads the real one.
*   **`PrefixSum`**: The Calculator. Prepares the math so it's fast.
*   **`RegionTree`**: The Organizer. Cuts the image into the QuadTree structure.
*   **`AnomalyDetector`**: The Detective. Decides if a region is "suspicious".
*   **`QueryEngine`**: The Reporter. Answers questions like "Show me the top 10".

You have built a very professional system! It uses high-level algorithms to solve a big data problem efficiently.
