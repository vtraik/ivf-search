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
./search –d \<input file\> –q \<query file\> –kclusters \<int\> -nprobe \<int\> -ο
\<output file\>  
-Ν \<number of nearest neighbors\> -R \<radius\> -type \<flag\> -range \<true|false\>  
-ivfflat –seed \<int\>

- For Ivfpq:  
./search –d \<input file\> –q \<query file\> –kclusters \<int\> -nprobe \<int\> -M \<int\>
-ο \<output file\>  
-Ν \<number of nearest neighbors\> -R \<radius\> -type \<flag\> -nbits \<int\> -
range \<true|false\>  
-ivfpq –seed \<int\> 

---

## 3. Report
In REPORT.md there is a report on these algorithms for the mnist/sift(1M) datasets.
