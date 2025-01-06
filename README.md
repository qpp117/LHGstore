# Learned Hierarchical Graph Storage for Dynamic Graphs

This repository contains the code and baselines for the manuscript:

> [Learned Hierarchical Graph Storage for Dynamic Graphs](https://github.com/qpp117/LHGstore)
>

Various applications model problems as dynamic graphs, which need to require rapid updates and run graph algorithms on the updated graph. Furthermore, these dynamic graphs follow a skewed distribution of vertex degrees, where there are a few high-degree vertices and many low-degree vertices. However, existing methods fail to support both graph updates and graph algorithms efficiently. To our knowledge, there is no work to enable ML-based indexes to support graph storage. In this paper, we propose LHGstore, a novel Learned Hierarchical Graph storage framework that leverages learned indexes to store graphs. Specifically, LHGstore employs a two-hierarchical design to effectively improve the throughput of graph updates while ensuring high-performance graph analysis. Moreover, considering real-world dynamic graphs follow skewed vertex degree distribution, LHGstore stores a vertex’s edges in different data structures depending on the degree of that vertex, instead of a ``one-size-fits-all'' design. LHGstore employs simple array-based structures for low-degree vertices to optimize traversal efficiency, whereas for high-degree vertices, it leverages learned indexes to improve update performance. Experimental results show that our proposed graph storage framework outperforms existing graph systems in terms of throughput for transactional workloads and performance for graph algorithms. This study pioneers the integration of ML-based index into graph storage, offering a robust solution for dynamic graph management and analysis.

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
- LHGstore and LGstore
1. GCC Compilation: Run make.sh to generate either the build or build_debug directory.
2. Visual Studio Compilation: Open the folder, select Build, and choose Rebuild All.

-Teseo
Compile [TESEO](https://github.com/cwida/teseo) and link it with your project.
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
