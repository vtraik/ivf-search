## 1. Introduction
In the field of high-dimensional vector search, finding the exact nearest neighbor is computationally expensive as the dataset grows (the "curse of dimensionality"). To address this, **Approximate Nearest Neighbor (ANN)** algorithms are used. 

This report evaluates two popular indexing methods:
* **IVFFlat (Inverted File Flat):** This algorithm clusters the vector space into **k** Voronoi cells. At search time, it only explores a subset of these cells (`nprobe`), significantly reducing the number of distance calculations.
* **IVFPQ (Inverted File with Product Quantization):** This builds on the IVF structure but adds **Product Quantization**. It compresses vectors into short codes using sub-quantizers (**M**). This not only speeds up calculations using look-up tables but also drastically reduces the memory footprint of the index.

```
├── cluster
│   ├── ivfflat
│   │   └── include
│   │       └── ivfflat.hpp
│   ├── ivfpq
│   │   └── include
│   │       └── ivfpq.hpp
│   └── utils
│       ├── cluster.hpp
│       └── lloyd.hpp
├── graphs
│   ├── flat_ar.png
│   ├── flat_qn.png
│   ├── flat_qr.png
│   ├── flat_rn.png
│   ├── pq_ar.png
│   ├── pq_qn.png
│   ├── pq_qn_sift.png
│   └── pq_qr.png
├── Makefile
├── README.md
├── REPORT.md
├── search.cpp
└── utils
    ├── include
    │   ├── ivfflat_out.hpp
    │   ├── ivfpq_out.hpp
    │   ├── knn.hpp
    │   ├── parser.hpp
    │   ├── utils.hpp
    │   └── Vector.hpp
    └── modules
        ├── ivfflat_out.cpp
        └── ivfpq_out.cpp
```
---

## 2. HOW TO USE
- In project's root:
```
make
```
- Run:

``` sh
./search <args>
```
- For Ivfflat:  

| Flag | Description |
| :--- |  :--- |
| `-d` | Train dataset. |
| `-q` | Query dataset.  |
| `-o` | Output file of ANN search. |
| `-kclusters` | Number of clusters. |
| `-nprobe` | Number of clusters to search in query time. |
| `-N` | Number of Nearest Neighbors. |
| `-type` | Dataset: MNIST/SIFT. |
| `-ivfflat` | Algorithm to use. |
| `-seed` | Seed for centroid init (kmeans++). |
| `-range` | If range search is applied or not: true/false.  |
| `-R` | Radius to search. |


- For Ivfpq:  

| Flag | Description |
| :--- |  :--- |
| `-d` | Train dataset. |
| `-q` | Query dataset. |
| `-o` | Output file of ANN search. |
| `-kclusters` | Number of clusters. |
| `-nprobe` | Number of clusters to search in query time. |
| `-N` | Number of Nearest Neighbors. |
| `-M` | Number of subvectors (subspace training).|
| `-nbits` | Number of clusters in subspaces (2^nbits).  |
| `-type` | Dataset: MNIST/SIFT. |
| `-ivfpq` | Algorithm to use. |
| `-seed` | Seed for centroid init (kmeans++).|
| `-range` | If range search is applied or not: true/false. |
| `-R` | Radius to search. |

---

## 3. Report
The file REPORT.md contains a report on these algorithms evaluated on the MNIST and SIFT(1M) datasets.
