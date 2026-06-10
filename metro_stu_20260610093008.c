/**
 * 地铁线路图查询器（学生版）
 * 实验任务：完成所有标记为 // TODO 的函数实现。
 * 编译：gcc -o metro metro_student.c -std=c99
 * 运行：./metro
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_NAME_LEN 32
#define MAX_LINE_NAME 20

// 邻接表边结点
typedef struct EdgeNode {
    int adjVex;               // 邻接站点编号
    int weight;               // 权值（运行时间，分钟）
    struct EdgeNode *next;
} EdgeNode;

// 顶点结点（站点）
typedef struct VertexNode {
    char name[MAX_NAME_LEN];  // 站点名称
    EdgeNode *firstEdge;      // 第一条边
    int *lineIds;             // 该站点所属的线路编号数组（动态分配）
    int lineCount;            // 所属线路数量
} VertexNode;

// 图结构
typedef struct {
    VertexNode *vertices;     // 顶点数组
    int vertexNum;            // 实际顶点数
    int vertexCapacity;       // 顶点数组容量
    int edgeNum;              // 边数
    int isDirected;           // 0:无向, 1:有向
} Graph;

// 辅助队列（用于BFS）
typedef struct Queue {
    int *data;
    int front, rear, size, capacity;
} Queue;

// 函数声明
Graph* createGraph(int initCapacity, int isDirected);
void resizeGraph(Graph *g);
int addVertex(Graph *g, const char *name);
int findVertexIndex(Graph *g, const char *name);
void addEdge(Graph *g, int u, int v, int weight);
void addLineToStation(Graph *g, int stationIdx, int lineId);
void readMetroFile(const char *filename, Graph *g);
void printAdjList(Graph *g);

// 以下是需要实现的函数（TODO）
void DFSRecursive(Graph *g, int v, int *visited);
void DFSTraversal(Graph *g, int start);
void BFSTraversal(Graph *g, int start);
void connectivityAnalysis(Graph *g);
void dijkstra(Graph *g, int start, int *dist, int *prev);
void printPath(Graph *g, int *prev, int start, int end);
void shortestPathByTime(Graph *g, int start, int end);
void shortestPathByTransfer(Graph *g, int start, int end);
void freeGraph(Graph *g);

void printMenu();

// 队列操作
Queue* createQueue(int capacity);
void enqueue(Queue *q, int val);
int dequeue(Queue *q);
int isEmpty(Queue *q);
void freeQueue(Queue *q);

// ---------- 主函数 ----------
int main() {
    Graph *g = createGraph(100, 0);  // 无向图

    readMetroFile("metro.txt", g);

    int choice, start, end;
    char startName[MAX_NAME_LEN], endName[MAX_NAME_LEN];

    do {
        printMenu();
        printf("请输入选择：");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                printAdjList(g);
                break;
            case 2:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) {
                    fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
                } else {
                    printf("\nDFS 遍历序列（从 %s 开始）：\n", startName);
                    DFSTraversal(g, start);
                }
                break;
            case 3:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) {
                    fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
                } else {
                    printf("\nBFS 遍历序列（从 %s 开始）：\n", startName);
                    BFSTraversal(g, start);
                }
                break;
            case 4:
                connectivityAnalysis(g);
                break;
            case 5:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                endName[strcspn(endName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1) {
                    fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
                } else if (end == -1) {
                    fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
                } else {
                    shortestPathByTime(g, start, end);
                }
                break;
            case 6:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                endName[strcspn(endName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1) {
                    fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
                } else if (end == -1) {
                    fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
                } else {
                    shortestPathByTransfer(g, start, end);
                }
                break;
            case 0:
                printf("退出程序。\n");
                break;
            default:
                printf("无效选择，请重新输入。\n");
        }
        printf("\n");
    } while (choice != 0);

    freeGraph(g);
    return 0;
}

// ---------- 以下函数已完整实现（无需修改）----------

// 创建图
Graph* createGraph(int initCapacity, int isDirected) {
    Graph *g = (Graph*)malloc(sizeof(Graph));
    g->vertexCapacity = initCapacity;
    g->vertexNum = 0;
    g->edgeNum = 0;
    g->isDirected = isDirected;
    g->vertices = (VertexNode*)malloc(sizeof(VertexNode) * initCapacity);
    for (int i = 0; i < initCapacity; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    return g;
}

// 动态扩容
void resizeGraph(Graph *g) {
    int newCap = g->vertexCapacity * 2;
    g->vertices = (VertexNode*)realloc(g->vertices, sizeof(VertexNode) * newCap);
    for (int i = g->vertexCapacity; i < newCap; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    g->vertexCapacity = newCap;
}

// 添加站点，返回编号
int addVertex(Graph *g, const char *name) {
    int idx = findVertexIndex(g, name);
    if (idx != -1) return idx;

    if (g->vertexNum >= g->vertexCapacity) {
        resizeGraph(g);
    }
    strcpy(g->vertices[g->vertexNum].name, name);
    g->vertices[g->vertexNum].firstEdge = NULL;
    g->vertices[g->vertexNum].lineIds = NULL;
    g->vertices[g->vertexNum].lineCount = 0;
    return g->vertexNum++;
}

// 查找站点编号
int findVertexIndex(Graph *g, const char *name) {
    for (int i = 0; i < g->vertexNum; i++) {
        if (strcmp(g->vertices[i].name, name) == 0)
            return i;
    }
    return -1;
}

// 添加边（无向图加双向）
void addEdge(Graph *g, int u, int v, int weight) {
    if (u < 0 || u >= g->vertexNum || v < 0 || v >= g->vertexNum) return;

    EdgeNode *e = (EdgeNode*)malloc(sizeof(EdgeNode));
    e->adjVex = v;
    e->weight = weight;
    e->next = g->vertices[u].firstEdge;
    g->vertices[u].firstEdge = e;

    if (!g->isDirected) {
        e = (EdgeNode*)malloc(sizeof(EdgeNode));
        e->adjVex = u;
        e->weight = weight;
        e->next = g->vertices[v].firstEdge;
        g->vertices[v].firstEdge = e;
    }
    g->edgeNum++;
}

// 为站点添加所属线路编号
void addLineToStation(Graph *g, int stationIdx, int lineId) {
    if (stationIdx < 0 || stationIdx >= g->vertexNum) return;
    for (int i = 0; i < g->vertices[stationIdx].lineCount; i++) {
        if (g->vertices[stationIdx].lineIds[i] == lineId)
            return;
    }
    g->vertices[stationIdx].lineCount++;
    g->vertices[stationIdx].lineIds = (int*)realloc(g->vertices[stationIdx].lineIds,
                                                    sizeof(int) * g->vertices[stationIdx].lineCount);
    g->vertices[stationIdx].lineIds[g->vertices[stationIdx].lineCount - 1] = lineId;
}

// 读取地铁文件
void readMetroFile(const char *filename, Graph *g) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "无法打开文件 %s\n", filename);
        exit(1);
    }

    char line[256];
    int routeCount = 0;
    // 跳过前两行（总站点数和线路数，这里简单处理：读取直到遇到第一条线路数据）
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        sscanf(line, "%d", &routeCount);
        break;
    }
    // 再读一行（线路数）
    fgets(line, sizeof(line), fp);

    for (int rid = 0; rid < routeCount; rid++) {
        if (!fgets(line, sizeof(line), fp)) break;
        if (line[0] == '#' || line[0] == '\n') {
            rid--;
            continue;
        }

        char lineName[MAX_LINE_NAME];
        int stationCount;
        char *token = strtok(line, " \t\n");
        if (!token) continue;
        strcpy(lineName, token);

        token = strtok(NULL, " \t\n");
        if (!token) continue;
        stationCount = atoi(token);

        int prevStation = -1;
        int timeVal = 1;
        for (int i = 0; i < stationCount; i++) {
            token = strtok(NULL, " \t\n");
            if (!token) break;

            // 判断是否为纯数字（时间）
            int isTime = 1;
            for (char *p = token; *p; p++) {
                if (!isdigit(*p)) { isTime = 0; break; }
            }
            if (isTime && i > 0) {
                timeVal = atoi(token);
                continue;
            }

            int idx = addVertex(g, token);
            addLineToStation(g, idx, rid);

            if (prevStation != -1) {
                addEdge(g, prevStation, idx, timeVal);
                timeVal = 1;
            }
            prevStation = idx;
        }
    }
    fclose(fp);
    printf("成功读取地铁数据：共 %d 个站点，%d 条边。\n", g->vertexNum, g->edgeNum);
}

// 输出邻接表及换乘站
void printAdjList(Graph *g) {
    printf("\n===== 邻接表 =====\n");
    for (int i = 0; i < g->vertexNum; i++) {
        printf("%s (%d条线路): ", g->vertices[i].name, g->vertices[i].lineCount);
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e) {
            printf("-> %s(%dmin) ", g->vertices[e->adjVex].name, e->weight);
            e = e->next;
        }
        printf("\n");
    }
    printf("\n===== 换乘站 =====\n");
    for (int i = 0; i < g->vertexNum; i++) {
        if (g->vertices[i].lineCount > 1) {
            printf("%s：%d 条线路\n", g->vertices[i].name, g->vertices[i].lineCount);
        }
    }
}

// 打印菜单
void printMenu() {
    printf("\n====== 地铁查询系统 ======\n");
    printf("1. 输出邻接表和换乘站\n");
    printf("2. DFS 遍历（从指定站点）\n");
    printf("3. BFS 遍历（从指定站点）\n");
    printf("4. 连通分量分析\n");
    printf("5. 最短路径（最少时间）\n");
    printf("6. 最短路径（最少换乘）\n");
    printf("0. 退出\n");
}

// ---------- 队列实现（已提供）----------
Queue* createQueue(int capacity) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->data = (int*)malloc(sizeof(int) * capacity);
    q->front = q->rear = q->size = 0;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue *q, int val) {
    if (q->size == q->capacity) return;
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

int dequeue(Queue *q) {
    if (q->size == 0) return -1;
    int val = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return val;
}

int isEmpty(Queue *q) {
    return q->size == 0;
}

void freeQueue(Queue *q) {
    free(q->data);
    free(q);
}

// ---------- 以下为需要实现的函数（TODO）----------

void DFSRecursive(Graph *g, int v, int *visited) {
	visited[v] = 1; // 标记当前站点已访问
	    printf("%s -> ", g->vertices[v].name); // 输出站点名称
	    // 遍历当前站点的所有邻接边
	    EdgeNode *e = g->vertices[v].firstEdge;
	    while (e != NULL) {
	        if (!visited[e->adjVex]) {
	            DFSRecursive(g, e->adjVex, visited);
	        }
	        e = e->next;
	    }
    // TODO: 实现递归深度优先遍历
}

void DFSTraversal(Graph *g, int start) {
	 // 分配并初始化 visited 数组
	    int *visited = (int *)calloc(g->vertexNum, sizeof(int));
	    if (!visited) return;
	    DFSRecursive(g, start, visited);
	    printf("NULL\n"); // 按照常见格式，遍历结束输出 NULL
	    
	    free(visited); // 释放动态分配的内存
    // TODO: 调用 DFSRecursive 从 start 开始遍历并输出序列
}

void BFSTraversal(Graph *g, int start) {
	int *visited = (int *)calloc(g->vertexNum, sizeof(int));
	    if (!visited) return;
	    Queue *q = createQueue(g->vertexNum);
	    enqueue(q, start);
	    visited[start] = 1;
	    printf("%s -> ", g->vertices[start].name);
	    while (!isEmpty(q)) {
	        int u = dequeue(q);  
	        EdgeNode *e = g->vertices[u].firstEdge;
	        while (e != NULL) {
	            if (!visited[e->adjVex]) {
	                visited[e->adjVex] = 1;
	                printf("%s -> ", g->vertices[e->adjVex].name);
	                enqueue(q, e->adjVex);
	            }
	            e = e->next;
	        }
	    }
	    printf("NULL\n");
	    free(visited);
	    freeQueue(q);
    // TODO: 使用队列实现广度优先遍历，输出序列
}

void connectivityAnalysis(Graph *g) {
	 if (g->vertexNum == 0) { 
	        printf("图为空。\n"); 
	        return; 
	    }
	    int *visited = (int *)calloc(g->vertexNum, sizeof(int));
	    if (!visited) return;
	    int componentCount = 0;
	    for (int i = 0; i < g->vertexNum; i++) {
	        if (!visited[i]) {
	            componentCount++;
	            printf("连通分量 %d: ", componentCount);
	            // 对未访问的顶点启动一次 DFS/BFS
	            DFSRecursive(g, i, visited);
	            printf("NULL\n");
	        }
	    }
	    printf("总共有 %d 个连通分量。\n", componentCount);
	    
	    free(visited);
    // TODO: 计算并输出连通分量个数及每个分量的站点列表
}

void dijkstra(Graph *g, int start, int *dist, int *prev) {
	int n = g->vertexNum;
	    int *visited = (int *)calloc(n, sizeof(int));
	    if (!visited) return;
	    // 初始化距离和前驱节点
	    for (int i = 0; i < n; i++) {
	        dist[i] = INT_MAX;
	        prev[i] = -1;
	    }
	    dist[start] = 0;
	    for (int count = 0; count < n; count++) {
	        // 寻找未访问且距离最小的顶点 u
	        int minVal = INT_MAX, u = -1;
	        for (int v = 0; v < n; v++) {
	            if (!visited[v] && dist[v] < minVal) {
	                minVal = dist[v];
	                u = v;
	            }
	        }
	        if (u == -1) break; // 剩余顶点不可达，提前退出
	        visited[u] = 1;
	        // 松弛操作：更新 u 的所有邻接点
	        EdgeNode *e = g->vertices[u].firstEdge;
	        while (e != NULL) {
	            int v = e->adjVex;
	            if (!visited[v] && dist[u] != INT_MAX && 
	                dist[u] + e->weight < dist[v]) {
	                dist[v] = dist[u] + e->weight;
	                prev[v] = u;
	            }
	            e = e->next;
	        }
	    }
	    free(visited);
    // TODO: 实现 Dijkstra 算法，计算最短距离和前驱数组
}

void printPath(Graph *g, int *prev, int start, int end) {
	if (start == end) {
	        printf("%s", g->vertices[start].name);
	        return;
	    }
	    if (prev[end] == -1) {
	        printf("无路径");
	        return;
	    }
	    printPath(g, prev, start, prev[end]);
	    printf(" -> %s", g->vertices[end].name);
    // TODO: 递归输出从 start 到 end 的路径
}

void shortestPathByTime(Graph *g, int start, int end) {
	 int *dist = (int *)malloc(sizeof(int) * g->vertexNum);
	    int *prev = (int *)malloc(sizeof(int) * g->vertexNum);
	    if (!dist || !prev) return; 
	    dijkstra(g, start, dist, prev);
	    if (dist[end] == INT_MAX) {
	        printf("无法从 '%s' 到达 '%s'。\n", g->vertices[start].name, g->vertices[end].name);
	    } else {
	        printf("最优时间路线 (耗时 %d 分钟): \n", dist[end]);
	        printPath(g, prev, start, end);
	        printf("\n");
	    }
	    free(dist); 
	    free(prev);
    // TODO: 使用 dijkstra 输出最少时间路径及总时间
}

void shortestPathByTransfer(Graph *g, int start, int end) {
	    for (int i = 0; i < g->vertexNum; i++) {
	        EdgeNode *e = g->vertices[i].firstEdge;
	        while (e != NULL) {
	            e->weight = 1;
	            e = e->next;
	        }
	    } 
	    int *dist = (int *)malloc(sizeof(int) * g->vertexNum);
	    int *prev = (int *)malloc(sizeof(int) * g->vertexNum);
	    if (!dist || !prev) return; 
	    dijkstra(g, start, dist, prev);
	    if (dist[end] == INT_MAX) {
	        printf("无法从 '%s' 到达 '%s'。\n", g->vertices[start].name, g->vertices[end].name);
	    } else {
	        // 换乘次数 = 经过的边数 - 1
	        int transfers = (dist[end] > 0) ? dist[end] - 1 : 0;
	        printf("最少换乘路线 (换乘 %d 次): \n", transfers);
	        printPath(g, prev, start, end);
	        printf("\n");
	    }
	    free(dist); 
	    free(prev); 
	    // 【重要】恢复原始权重：重新读取文件以还原被修改的边权
	    readMetroFile("metro.txt", g); 
    // TODO: 将边权临时设为1，调用 dijkstra，输出最少换乘路径及换乘次数，然后恢复原权值
}

void freeGraph(Graph *g) {
	if (!g) return;
	    // 释放每个顶点的边链表和线路数组
	    for (int i = 0; i < g->vertexNum; i++) {
	        EdgeNode *e = g->vertices[i].firstEdge;
	        while (e != NULL) {
	            EdgeNode *temp = e;
	            e = e->next;
	            free(temp);
	        }
	        if (g->vertices[i].lineIds != NULL) {
	            free(g->vertices[i].lineIds);
	        }
	    }
	    // 释放顶点数组和图结构本身
	    free(g->vertices);
	    free(g);
    // TODO: 释放所有动态分配的内存（边结点、lineIds、顶点数组、图结构）
}
