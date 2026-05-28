# LHGstore: A Learned Hierarchical Indexing for Fast Graph Updates and Analytics

This paper accepted by DAC 2026. This repository contains the code and baselines for the manuscript:

> [LHGstore: An In-Memory Learned Graph Storage for Fast Updates and Analytics](https://arxiv.org/abs/2603.11596)
>

Various real-world applications rely on in-memory dynamic graphs that must efficiently handle frequent updates while supporting low-latency analytics on evolving structures. Achieving both objectives remains challenging due to the trade-off between update efficiency and traversal locality, particularly under highly skewed degree distributions. This motivates the design of graph indexing schemes optimized for in-memory graph management on modern multi-core CPUs. We present \textbf{LHGstore}, a degree-aware \textbf{L}earned \textbf{H}ierarchical \textbf{G}raph storage that, for the first time, integrates learned indexing into graph management. LHGstore designs a two-level hierarchy that decouples vertex and edge access and further organizes each vertex’s edges using data structures adaptive to its degree. Lightweight arrays are used for low-degree vertices to maximize traversal locality, while learned indexes are applied to high-degree vertices to improve update throughput. Extensive experiments show that LHGstore achieves 5.9-28.2× higher throughput and significantly faster analytics than SOTA in-memory graph storage systems.

## Instructions

### Files Instructions

* `alex.h` : Operations related to learned index (named ALEX).
* `alex_nodes.h` : Definitions and operations for the two types of nodes in learned index.
* `alex_base.h` : Linear model implementation in learned index.
* `alex_fanout_tree.h` : Related to constructing the fanout tree.
* `algorithm.h` : Implementation of graph algorithms.
* `graph_query.h` : Contains the Graph class. Implements OLTP operations (CRUD).
* `graph.h` : Defines graph edges (class edge), payload values stored in the index (class payload), and the graph iterator (class Iterator).
* `main.cpp` : Reads graph data and performs tests. Wraps testing operations for CRUD and algorithms. The final tests use benchmark_read_write and benchmark_read_only.

### Running Instructions

Method
```
* `LHGstore` : Our proposed learned hierarchical graph storage.
* `LGStore-baseline` : Our proposed baseline directly using learned index (non-hierarchical).
* `Teseo` : comparison with the graph storage system.
```

Code Execution
```
* LHGstore and LGstore
1. GCC Compilation: Run make.sh to generate either the build or build_debug directory.
2. Visual Studio Compilation: Open the folder, select Build, and choose Rebuild All.

* Teseo
Compile [Teseo](https://github.com/cwida/teseo) and link it with your project.
```

Command Line Arguments
```
- graph_file
Path to the data file (default value can be modified in the code).

- threshold
Index threshold (default value can be modified in the code).

- directed
Indicates whether the graph is directed (default is undirected).
```
