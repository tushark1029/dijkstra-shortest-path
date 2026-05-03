#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// ========== GRAPH STRUCTURES ==========

struct Node {
    int vertex;
    int weight;
    struct Node* next;
};

struct Graph {
    int numVertices;
    struct Node** adjLists;
};

struct Node* createNode(int v, int w) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->vertex = v;
    newNode->weight = w;
    newNode->next = NULL;
    return newNode;
}

struct Graph* createGraph(int vertices) {
    struct Graph* graph = (struct Graph*)malloc(sizeof(struct Graph));
    graph->numVertices = vertices;
    graph->adjLists = (struct Node**)malloc(vertices * sizeof(struct Node*));
    for (int i = 0; i < vertices; i++)
        graph->adjLists[i] = NULL;
    return graph;
}

void addEdge(struct Graph* graph, int src, int dest, int weight) {
    struct Node* newNode = createNode(dest, weight);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;

    // Undirected graph
    newNode = createNode(src, weight);
    newNode->next = graph->adjLists[dest];
    graph->adjLists[dest] = newNode;
}

void printGraph(struct Graph* graph) {
    printf("\n=== GRAPH ADJACENCY LIST ===\n");
    for (int i = 0; i < graph->numVertices; i++) {
        struct Node* temp = graph->adjLists[i];
        printf("Vertex %d: ", i);
        while (temp) {
            printf("-> %d(w:%d) ", temp->vertex, temp->weight);
            temp = temp->next;
        }
        printf("\n");
    }
    printf("=============================\n");
}

// ========== MIN HEAP STRUCTURES ==========

struct HeapNode {
    int vertex;
    int distance;
};

struct MinHeap {
    int size;
    int capacity;
    int *pos;
    struct HeapNode **array;
};

struct HeapNode* newHeapNode(int v, int dist) {
    struct HeapNode* heapNode = (struct HeapNode*)malloc(sizeof(struct HeapNode));
    heapNode->vertex = v;
    heapNode->distance = dist;
    return heapNode;
}

struct MinHeap* createMinHeap(int capacity) {
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    minHeap->pos = (int*)malloc(capacity * sizeof(int));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (struct HeapNode**)malloc(capacity * sizeof(struct HeapNode*));
    return minHeap;
}

void swapHeapNode(struct HeapNode** a, struct HeapNode** b) {
    struct HeapNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(struct MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size &&
        minHeap->array[left]->distance < minHeap->array[smallest]->distance)
        smallest = left;

    if (right < minHeap->size &&
        minHeap->array[right]->distance < minHeap->array[smallest]->distance)
        smallest = right;

    if (smallest != idx) {
        struct HeapNode *smallestNode = minHeap->array[smallest];
        struct HeapNode *idxNode = minHeap->array[idx];

        minHeap->pos[smallestNode->vertex] = idx;
        minHeap->pos[idxNode->vertex] = smallest;

        swapHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

int isEmpty(struct MinHeap* minHeap) {
    return minHeap->size == 0;
}

struct HeapNode* extractMin(struct MinHeap* minHeap) {
    if (isEmpty(minHeap))
        return NULL;

    struct HeapNode* root = minHeap->array[0];
    struct HeapNode* lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;

    minHeap->pos[root->vertex] = minHeap->size - 1;
    minHeap->pos[lastNode->vertex] = 0;
    minHeap->size--;

    minHeapify(minHeap, 0);
    return root;
}

void decreaseKey(struct MinHeap* minHeap, int v, int dist) {
    int i = minHeap->pos[v];
    minHeap->array[i]->distance = dist;

    while (i && minHeap->array[i]->distance < minHeap->array[(i - 1) / 2]->distance) {
        minHeap->pos[minHeap->array[i]->vertex] = (i - 1) / 2;
        minHeap->pos[minHeap->array[(i - 1) / 2]->vertex] = i;
        swapHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

int isInMinHeap(struct MinHeap *minHeap, int v) {
    return minHeap->pos[v] < minHeap->size;
}

// ========== CONDITION 1: BASIC DIJKSTRA ==========

void dijkstra_basic(struct Graph* graph, int src) {
    int V = graph->numVertices;
    int dist[V];
    int visited[V];

    for (int i = 0; i < V; ++i) {
        dist[i] = INT_MAX;
        visited[i] = 0;
    }
    dist[src] = 0;

    struct MinHeap* minHeap = createMinHeap(V);
    for (int v = 0; v < V; ++v) {
        minHeap->array[v] = newHeapNode(v, dist[v]);
        minHeap->pos[v] = v;
    }
    minHeap->size = V;
    decreaseKey(minHeap, src, dist[src]);

    while (!isEmpty(minHeap)) {
        struct HeapNode* minNode = extractMin(minHeap);
        int u = minNode->vertex;
        visited[u] = 1;

        struct Node* adj = graph->adjLists[u];
        while (adj) {
            int v = adj->vertex;
            if (!visited[v] && dist[u] != INT_MAX &&
                dist[u] + adj->weight < dist[v]) {
                dist[v] = dist[u] + adj->weight;
                decreaseKey(minHeap, v, dist[v]);
            }
            adj = adj->next;
        }
    }

    printf("\n--- Condition 1: Basic Dijkstra ---\n");
    printf("Shortest distances from node %d:\n", src);
    for (int i = 0; i < V; ++i) {
        if (dist[i] == INT_MAX)
            printf("  Node %d -> Distance: INFINITY\n", i);
        else
            printf("  Node %d -> Distance: %d\n", i, dist[i]);
    }
}

// ========== CONDITION 2: EARLY STOP ==========

void dijkstra_early_stop(struct Graph* graph, int src, int dest) {
    int V = graph->numVertices;
    int dist[V];
    int visited[V];

    for (int i = 0; i < V; ++i) {
        dist[i] = INT_MAX;
        visited[i] = 0;
    }
    dist[src] = 0;

    struct MinHeap* minHeap = createMinHeap(V);
    for (int v = 0; v < V; ++v) {
        minHeap->array[v] = newHeapNode(v, dist[v]);
        minHeap->pos[v] = v;
    }
    minHeap->size = V;
    decreaseKey(minHeap, src, dist[src]);

    while (!isEmpty(minHeap)) {
        struct HeapNode* minNode = extractMin(minHeap);
        int u = minNode->vertex;

        if (u == dest) break;

        visited[u] = 1;

        struct Node* adj = graph->adjLists[u];
        while (adj) {
            int v = adj->vertex;
            if (!visited[v] && dist[u] != INT_MAX &&
                dist[u] + adj->weight < dist[v]) {
                dist[v] = dist[u] + adj->weight;
                decreaseKey(minHeap, v, dist[v]);
            }
            adj = adj->next;
        }
    }

    printf("\n--- Condition 2: Early Stop ---\n");
    if (dist[dest] == INT_MAX)
        printf("No path exists from %d to %d\n", src, dest);
    else
        printf("Shortest path from %d to %d = %d\n", src, dest, dist[dest]);
}

// ========== CONDITION 3: PRECOMPUTED DISTANCES FROM A LANDMARK NODE ==========

void dijkstra_precompute(struct Graph* graph, int landmark) {
    printf("\n--- Condition 3: Precomputing Distances from Landmark Node %d ---\n", landmark);
    
    int V = graph->numVertices;
    int dist[V];
    int visited[V];

    for (int i = 0; i < V; ++i) {
        dist[i] = INT_MAX;
        visited[i] = 0;
    }
    dist[landmark] = 0;

    struct MinHeap* minHeap = createMinHeap(V);
    for (int v = 0; v < V; ++v) {
        minHeap->array[v] = newHeapNode(v, dist[v]);
        minHeap->pos[v] = v;
    }
    minHeap->size = V;
    decreaseKey(minHeap, landmark, dist[landmark]);

    while (!isEmpty(minHeap)) {
        struct HeapNode* minNode = extractMin(minHeap);
        int u = minNode->vertex;
        visited[u] = 1;

        struct Node* adj = graph->adjLists[u];
        while (adj) {
            int v = adj->vertex;
            if (!visited[v] && dist[u] != INT_MAX &&
                dist[u] + adj->weight < dist[v]) {
                dist[v] = dist[u] + adj->weight;
                decreaseKey(minHeap, v, dist[v]);
            }
            adj = adj->next;
        }
    }

    printf("Precomputed distances from landmark node %d:\n", landmark);
    for (int i = 0; i < V; ++i) {
        if (dist[i] == INT_MAX)
            printf("  Node %d -> Distance: INFINITY\n", i);
        else
            printf("  Node %d -> Distance: %d\n", i, dist[i]);
    }
}

// ========== CONDITION 4: DIJKSTRA WITH POTENTIALS (CORRECTED) ==========

void dijkstra_with_potentials(struct Graph* graph, int src, int dest, int *pot) {
    int V = graph->numVertices;
    int dist_prime[V];
    for (int i = 0; i < V; ++i) dist_prime[i] = INT_MAX;

    struct MinHeap* minHeap = createMinHeap(V);
    for (int i = 0; i < V; ++i) {
        minHeap->array[i] = newHeapNode(i, INT_MAX);
        minHeap->pos[i] = i;
    }
    dist_prime[src] = 0;
    decreaseKey(minHeap, src, 0);
    minHeap->size = V;

    while (!isEmpty(minHeap)) {
        struct HeapNode* minNode = extractMin(minHeap);
        int u = minNode->vertex;

        struct Node* adj = graph->adjLists[u];
        while (adj) {
            int v = adj->vertex;
            long long wprime = (long long)adj->weight + pot[u] - pot[v];
            if (wprime < 0) wprime = 0;
            if (dist_prime[u] != INT_MAX && dist_prime[u] + (int)wprime < dist_prime[v]) {
                dist_prime[v] = dist_prime[u] + (int)wprime;
                decreaseKey(minHeap, v, dist_prime[v]);
            }
            adj = adj->next;
        }
    }

    printf("\n--- Condition 4: Dijkstra with Potentials ---\n");
    printf("Using potentials: ");
    for (int i = 0; i < V; ++i) printf("%d ", pot[i]);
    printf("\n");
    
    for (int i = 0; i < V; ++i) {
        if (dist_prime[i] == INT_MAX)
            printf("Node %d -> Distance INF\n", i);
        else {
            long long original = (long long)dist_prime[i] - pot[src] + pot[i];
            printf("Node %d -> Distance %lld\n", i, original);
        }
    }
    if (dist_prime[dest] != INT_MAX) {
        long long original = (long long)dist_prime[dest] - pot[src] + pot[dest];
        printf("Heuristic distance from %d to %d = %lld\n", src, dest, original);
    }
}

// ========== CONDITION 5: MAX POTENTIAL TIE-BREAKER ==========

void dijkstra_with_max_potentials(struct Graph* graph, int src, int dest, int *pot) {
    int V = graph->numVertices;
    int dist[V];
    int visited[V];
    for (int i = 0; i < V; ++i) { dist[i] = INT_MAX; visited[i] = 0; }
    dist[src] = 0;

    struct MinHeap* minHeap = createMinHeap(V);
    for (int i = 0; i < V; ++i) {
        minHeap->array[i] = newHeapNode(i, INT_MAX);
        minHeap->pos[i] = i;
    }
    decreaseKey(minHeap, src, 0);
    minHeap->size = V;

    while (minHeap->size > 0) {
        int best_idx = 0;
        for (int i = 1; i < minHeap->size; ++i) {
            int vi = minHeap->array[i]->vertex;
            int vb = minHeap->array[best_idx]->vertex;
            int di = minHeap->array[i]->distance;
            int db = minHeap->array[best_idx]->distance;
            if (di < db) best_idx = i;
            else if (di == db && pot[vi] > pot[vb]) best_idx = i;
        }

        struct HeapNode* bestNode = minHeap->array[best_idx];
        struct HeapNode* lastNode = minHeap->array[minHeap->size - 1];
        minHeap->array[best_idx] = lastNode;
        minHeap->pos[lastNode->vertex] = best_idx;
        minHeap->size--;
        minHeapify(minHeap, best_idx);

        int u = bestNode->vertex;
        visited[u] = 1;

        struct Node* adj = graph->adjLists[u];
        while (adj) {
            int v = adj->vertex;
            if (!visited[v] && dist[u] != INT_MAX && dist[u] + adj->weight < dist[v]) {
                dist[v] = dist[u] + adj->weight;
                decreaseKey(minHeap, v, dist[v]);
            }
            adj = adj->next;
        }
    }

    printf("\n--- Condition 5: Dijkstra with Max Potential Tie-Breaker ---\n");
    printf("Using potentials: ");
    for (int i = 0; i < V; ++i) printf("%d ", pot[i]);
    printf("\n");
    
    for (int i = 0; i < V; ++i) {
        if (dist[i] == INT_MAX)
            printf("Node %d -> Distance INF (pot=%d)\n", i, pot[i]);
        else
            printf("Node %d -> Distance %d (pot=%d)\n", i, dist[i], pot[i]);
    }
    if (dist[dest] != INT_MAX)
        printf("Modified distance from %d to %d = %d\n", src, dest, dist[dest]);
}

// ========== CONDITION 6: PATH VISUALIZATION ==========

void dijkstra_visualize(struct Graph* graph, int src, int dest) {
    int V = graph->numVertices;
    int dist[V];
    int visited[V];
    for (int i = 0; i < V; ++i) { dist[i] = INT_MAX; visited[i] = 0; }
    dist[src] = 0;

    printf("\n--- Condition 6: Path Visualization ---\n");
    printf("Starting traversal from node %d to node %d:\n", src, dest);

    struct MinHeap* minHeap = createMinHeap(V);
    for (int i = 0; i < V; ++i) {
        minHeap->array[i] = newHeapNode(i, INT_MAX);
        minHeap->pos[i] = i;
    }
    decreaseKey(minHeap, src, 0);
    minHeap->size = V;

    int step = 1;
    while (!isEmpty(minHeap)) {
        struct HeapNode* minNode = extractMin(minHeap);
        int u = minNode->vertex;
        printf("Step %d: Visited Node: %d (Current Distance: %d)\n", step++, u, dist[u]);
        visited[u] = 1;

        if (u == dest) {
            printf("*** REACHED DESTINATION NODE %d ***\n", dest);
            break;
        }

        struct Node* adj = graph->adjLists[u];
        while (adj) {
            int v = adj->vertex;
            if (!visited[v] && dist[u] != INT_MAX && dist[u] + adj->weight < dist[v]) {
                dist[v] = dist[u] + adj->weight;
                decreaseKey(minHeap, v, dist[v]);
                printf("    Updated neighbor %d -> New distance: %d\n", v, dist[v]);
            }
            adj = adj->next;
        }
    }

    if (dist[dest] == INT_MAX)
        printf("No path found from %d to %d\n", src, dest);
    else
        printf("Final shortest distance from %d to %d = %d\n", src, dest, dist[dest]);
}

// ========== INPUT FUNCTIONS ==========

void inputGraph(struct Graph** graph) {
    int V, E;
    
    printf("\n=== GRAPH INPUT ===\n");
    printf("Enter number of vertices: ");
    scanf("%d", &V);
    
    *graph = createGraph(V);
    
    printf("Enter number of edges: ");
    scanf("%d", &E);
    
    printf("\nEnter edges (source destination weight):\n");
    for (int i = 0; i < E; i++) {
        int src, dest, weight;
        printf("Edge %d: ", i + 1);
        scanf("%d %d %d", &src, &dest, &weight);
        
        if (src < 0 || src >= V || dest < 0 || dest >= V) {
            printf("Error: Vertex out of range! Valid vertices: 0 to %d\n", V-1);
            i--;
            continue;
        }
        
        addEdge(*graph, src, dest, weight);
    }
}

void inputPotentials(int pot[], int V) {
    printf("\nEnter potential values for each vertex (0 to %d):\n", V-1);
    for (int i = 0; i < V; i++) {
        printf("Potential for vertex %d: ", i);
        scanf("%d", &pot[i]);
    }
}

// ========== MAIN DRIVER WITH MENU ==========

int main() {
    struct Graph* graph = NULL;
    int choice;
    int src, dest, landmark;
    int *potentials = NULL;
    int V = 0;
    
    printf("========== DIJKSTRA ALGORITHM DEMONSTRATION ==========\n");
    
    do {
        printf("\n=== MAIN MENU ===\n");
        printf("1. Input Graph\n");
        printf("2. Display Graph\n");
        printf("3. Basic Dijkstra (All shortest paths)\n");
        printf("4. Early Stop Dijkstra (Source to Destination)\n");
        printf("5. Precompute from Landmark\n");
        printf("6. Dijkstra with Potentials\n");
        printf("7. Dijkstra with Max Potential Tie-Breaker\n");
        printf("8. Path Visualization\n");
        printf("9. Run All Algorithms\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                inputGraph(&graph);
                if (graph) {
                    V = graph->numVertices;
                    potentials = (int*)malloc(V * sizeof(int));
                    // Initialize with default potentials
                    for (int i = 0; i < V; i++) potentials[i] = i;
                }
                break;
                
            case 2:
                if (graph) {
                    printGraph(graph);
                } else {
                    printf("Please input a graph first!\n");
                }
                break;
                
            case 3:
                if (graph) {
                    printf("Enter source node (0 to %d): ", V-1);
                    scanf("%d", &src);
                    if (src >= 0 && src < V) {
                        dijkstra_basic(graph, src);
                    } else {
                        printf("Invalid source node!\n");
                    }
                } else {
                    printf("Please input a graph first!\n");
                }
                break;
                
            case 4:
                if (graph) {
                    printf("Enter source node (0 to %d): ", V-1);
                    scanf("%d", &src);
                    printf("Enter destination node (0 to %d): ", V-1);
                    scanf("%d", &dest);
                    if (src >= 0 && src < V && dest >= 0 && dest < V) {
                        dijkstra_early_stop(graph, src, dest);
                    } else {
                        printf("Invalid nodes!\n");
                    }
                } else {
                    printf("Please input a graph first!\n");
                }
                break;
                
            case 5:
                if (graph) {
                    printf("Enter landmark node (0 to %d): ", V-1);
                    scanf("%d", &landmark);
                    if (landmark >= 0 && landmark < V) {
                        dijkstra_precompute(graph, landmark);
                    } else {
                        printf("Invalid landmark node!\n");
                    }
                } else {
                    printf("Please input a graph first!\n");
                }
                break;
                
            case 6:
                if (graph) {
                    printf("Enter source node (0 to %d): ", V-1);
                    scanf("%d", &src);
                    printf("Enter destination node (0 to %d): ", V-1);
                    scanf("%d", &dest);
                    if (src >= 0 && src < V && dest >= 0 && dest < V) {
                        printf("Do you want to use default potentials (0-%d) or input custom? (0=default, 1=custom): ", V-1);
                        int pot_choice;
                        scanf("%d", &pot_choice);
                        if (pot_choice == 1) {
                            inputPotentials(potentials, V);
                        }
                        dijkstra_with_potentials(graph, src, dest, potentials);
                    } else {
                        printf("Invalid nodes!\n");
                    }
                } else {
                    printf("Please input a graph first!\n");
                }
                break;
                
            case 7:
                if (graph) {
                    printf("Enter source node (0 to %d): ", V-1);
                    scanf("%d", &src);
                    printf("Enter destination node (0 to %d): ", V-1);
                    scanf("%d", &dest);
                    if (src >= 0 && src < V && dest >= 0 && dest < V) {
                        printf("Do you want to use default potentials (0-%d) or input custom? (0=default, 1=custom): ", V-1);
                        int pot_choice;
                        scanf("%d", &pot_choice);
                        if (pot_choice == 1) {
                            inputPotentials(potentials, V);
                        }
                        dijkstra_with_max_potentials(graph, src, dest, potentials);
                    } else {
                        printf("Invalid nodes!\n");
                    }
                } else {
                    printf("Please input a graph first!\n");
                }
                break;
                
            case 8:
                if (graph) {
                    printf("Enter source node (0 to %d): ", V-1);
                    scanf("%d", &src);
                    printf("Enter destination node (0 to %d): ", V-1);
                    scanf("%d", &dest);
                    if (src >= 0 && src < V && dest >= 0 && dest < V) {
                        dijkstra_visualize(graph, src, dest);
                    } else {
                        printf("Invalid nodes!\n");
                    }
                } else {
                    printf("Please input a graph first!\n");
                }
                break;
                
            case 9:
                if (graph) {
                    printf("Enter source node (0 to %d): ", V-1);
                    scanf("%d", &src);
                    printf("Enter destination node (0 to %d): ", V-1);
                    scanf("%d", &dest);
                    if (src >= 0 && src < V && dest >= 0 && dest < V) {
                        printf("Enter landmark node (0 to %d): ", V-1);
                        scanf("%d", &landmark);
                        if (landmark >= 0 && landmark < V) {
                            dijkstra_basic(graph, src);
                            dijkstra_early_stop(graph, src, dest);
                            dijkstra_precompute(graph, landmark);
                            dijkstra_with_potentials(graph, src, dest, potentials);
                            dijkstra_with_max_potentials(graph, src, dest, potentials);
                            dijkstra_visualize(graph, src, dest);
                        } else {
                            printf("Invalid landmark node!\n");
                        }
                    } else {
                        printf("Invalid nodes!\n");
                    }
                } else {
                    printf("Please input a graph first!\n");
                }
                break;
                
            case 0:
                printf("Exiting program...\n");
                break;
                
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 0);
    
    // Free memory
    if (graph) free(graph);
    if (potentials) free(potentials);
    
    printf("\n=== Program Ended Successfully ===\n");
    return 0;
}