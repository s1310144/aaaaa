#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXN         25000
#define MAXM         200
#define MAXQ         100
#define MAXK         100
#define MAX_DEG      (MAXM * 2)
#define MAX_PATH_LEN 512
#define INF          1e18
#define EPS          1e-8

typedef struct {
    int to;
    double cost;
} Edge;

typedef struct {
    double x, y;
    int id;
} PointOnEdge;

typedef struct {               // 追加: Path構造体
    int nodes[MAX_PATH_LEN];
    int len;
    double cost;
} Path;

typedef struct {
    int start, goal;
    Path paths[MAXK]; // k個までの最短路
    int numPaths;
} QueryResult;

static Edge graph[MAXN][MAX_DEG];
int gsize[MAXN];
double x[MAXN], y[MAXN];
int N, M, P, Q;
int b[MAXM], e[MAXM];
int total; //add

// Yen用無効化配列 追加
static int disabled_node[MAXN];
static int disabled_edge_idx[MAXN][MAX_DEG];

// K最短路結果 追加
static Path paths[MAXK];
static int numPaths = 0;
static int currentPath = 0;

float currentX = 50, currentY = 50;
int windowWidth = 800, windowHeight = 800;
double maxX, maxY, minX, minY;
double viewWidth, viewHeight;
double mapMinX, mapMaxX, mapMinY, mapMaxY;
double mapPadRate = 0.05;
int showMiniMap = 1;
int showShortestPath = 1;
int showHelp = 1;
float zoom = 0.5f, zoomRate = 3.0f;
static int startNode = -1;
static int goalNode = -1;
QueryResult queryResults[MAXQ];
int totalQueries = 0;
int currentQuery = 0;

/*** 関数プロトタイプ ***/
double cross(double, double, double, double);
int same_point(double, double, double, double);
int intersect(double, double, double, double,
    double, double, double, double,
    double*, double*);
double dist(int, int);
double dist2(double, double, double, double);
int add_point(double, double, int*);
void add_edge(int, int);

void getMinMaxIndices(int* minX, int* maxX, int* minY, int* maxY) {
    int i;
    *minX = *maxX = *minY = *maxY = 0;
    for (i = 1; i < total; i++) {
        if (x[i] < x[*minX]) *minX = i;
        if (x[i] > x[*maxX]) *maxX = i;
        if (y[i] < y[*minY]) *minY = i;
        if (y[i] > y[*maxY]) *maxY = i;
    }
}

void drawCircle(double cx, double cy, double r, int num_segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(cx, cy);
    for (int i = 0; i <= num_segments; i++) {
        double theta = 2.0 * M_PI * (double)i / (double)num_segments;
        double x = r * cos(theta);
        double y = r * sin(theta);
        glVertex2d(cx + x, cy + y);
    }
    glEnd();
}

void drawText(float x, float y, const char *text){
  glColor3f(0.f, 1.f, 0.f); //
  glRasterPos2f(x, y);
  while(*text){
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *text);
    text++;
  }
}

void drawKeyHelp() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.0, 0.0, 0.0); //テキストの色（黒）

    int x = 10;
    int y = windowHeight - 20;

    const char *lines[] = {
        "WASD : Move",
        "+ / - : Zoom In/Out",
        "M/m : Show Mini Map",
        "p : Show Shortest Path",
        "n/b : Next / Previous Path",
        "e/q : Next/Previous Query (pair)",
        "h : Show Help",
        "ESC  : Quit"
    };
    int n = sizeof(lines) / sizeof(lines[0]);

    for (int i = 0; i < n; i++) {
        glRasterPos2i(x, y - i * 20);
        const char *text = lines[i];
        while (*text) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *text++);
        }
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawPath(Path* p) {
    if (!p) return;
    glColor3f(0.0, 0.8, 0.8);
    glLineWidth(3.0);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < p->len; i++) {
        int v = p->nodes[i];
        glVertex2d(x[v], y[v]);
    }
    glEnd();
    glLineWidth(1.0);
}

void drawPathInfo(void) {
     glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.0, 0.0, 0.0);

    if (totalQueries > 0) {
        QueryResult *qr = &queryResults[currentQuery];
        int si = qr->start;
        int di = qr->goal;
        int np = qr->numPaths;

        char info[256];
        char startLabel[32], goalLabel[32];
        if (si < N) sprintf(startLabel, "%d", si + 1);
        else        sprintf(startLabel, "C%d", si - N + 1);
        if (di < N) sprintf(goalLabel, "%d", di + 1);
        else        sprintf(goalLabel, "C%d", di - N + 1);

        if (np > 0) {
            double cost = qr->paths[currentPath].cost;
            sprintf(info, "From %s to %s | Path %d/%d | Distance: %.5lf",
                    startLabel, goalLabel,
                    currentPath + 1, np, cost);
        } else {
            glColor3f(1.0, 0.0, 0.0);
            sprintf(info, "No path from %s to %s", startLabel, goalLabel);
        }

        glRasterPos2i(10, 30);
        for (char* p = info; *p; p++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
        }

    } else {
        glColor3f(1.0, 0.0, 0.0);
        const char* msg = "No query data loaded";
        glRasterPos2i(10, 30);
        for (const char* p = msg; *p; p++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
        }
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawMap(double disX, double disY, int isMainMap){
    glPointSize(5.0);
    glLineWidth(1.0);

   // 道路（太線：灰色）
    glColor3f(0.3f, 0.3f, 0.3f);
    glLineWidth(6.0f);
    for (int i = 0; i < MAXN; i++) {
        for (int j = 0; j < gsize[i]; j++) {
            int to = graph[i][j].to;
            if (i < to) {
                glBegin(GL_LINES);
                glVertex2d(x[i], y[i]);
                glVertex2d(x[to], y[to]);
                glEnd();
            }
        }
    }

    // 中央線（細線：白）
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);  // 細め
    for (int i = 0; i < MAXN; i++) {
        for (int j = 0; j < gsize[i]; j++) {
            int to = graph[i][j].to;
            if (i < to) {
                glBegin(GL_LINES);
                glVertex2d(x[i], y[i]);
                glVertex2d(x[to], y[to]);
                glEnd();
            }
        }
    }
    
    // 地点（主要地点は赤い円）
    //glColor3f(0.8f, 0.1f, 0.1f);
   // for (int i = 0; i < N; i++) {
    //    drawCircle(x[i], y[i], disX * 0.005, 20);
   // }
    
    // 追加地点（薄青い円）
   // glColor3f(0.1f, 0.4f, 0.8f);
    //for (int i = N; i < total; i++) {
     //   drawCircle(x[i], y[i], disX * 0.003, 16);
    //}

    if (isMainMap) {
        double dis = sqrt(disX * disX + disY * disY);
        for (int i = 0; i < total; i++) {
            char label[100];

            if (i < N) {
                glColor3f(0.8f, 0.1f, 0.1f);
                drawCircle(x[i], y[i], disX * 0.005, 20);
                sprintf(label, "%d", i + 1);
            }
            else if ((i - N) % 2 == 0) {
                glColor3f(0.1f, 0.4f, 0.8f);
                drawCircle(x[i], y[i], disX * 0.004, 16);
                sprintf(label, "P%d", (i - N) / 2 + 1);
            }
            else {
                glColor3f(0.2f, 0.6f, 0.2f);
                drawCircle(x[i], y[i], disX * 0.003, 12);
                sprintf(label, "C%d", (i - N - 1) / 2 + 1);
            }

            drawText(x[i] + dis / 500, y[i] + dis / 500, label);
        }
    }
    else {
        // ミニマップではラベルなし＋色分けのみ
        for (int i = 0; i < total; i++) {
            if (i < N) {
                glColor3f(0.8f, 0.1f, 0.1f);
                drawCircle(x[i], y[i], disX * 0.005, 20);
            }
            else if ((i - N) % 2 == 0) {
                glColor3f(0.1f, 0.4f, 0.8f);
                drawCircle(x[i], y[i], disX * 0.004, 16);
            }
            else {
                glColor3f(0.2f, 0.6f, 0.2f);
                drawCircle(x[i], y[i], disX * 0.003, 12);
            }
        }
    }


    // 1経路ずつ切り替え
    if (showShortestPath && numPaths > 0) {
        Path* p = &queryResults[currentQuery].paths[currentPath];
        drawPath(p);
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    double disX = mapMaxX - mapMinX;
    double disY = mapMaxY - mapMinY;
    
    viewWidth = disX / zoomRate * zoom;
    viewHeight = disY / zoomRate * zoom;
    if(viewWidth < 1) viewWidth = 1;
    if(viewHeight < 1) viewHeight = 1;

    if(currentX - viewWidth / 2 < mapMinX) currentX = mapMinX + viewWidth / 2;
    if(currentX + viewWidth / 2 > mapMaxX) currentX = mapMaxX - viewWidth / 2;
    if(currentY - viewHeight / 2 < mapMinY) currentY = mapMinY + viewHeight / 2;
    if(currentY + viewHeight / 2 > mapMaxY) currentY = mapMaxY - viewHeight / 2;

    //mainMap
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(currentX - viewWidth / 2, currentX + viewWidth / 2,
           currentY - viewHeight / 2, currentY + viewHeight / 2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    drawMap(disX, disY, 1);

    double dis = sqrt(disX * disX + disY * disY);

    //for (int i = 0; i < total; i++) {
    //char label[100];
    //sprintf(label, "C%d (%.1f, %.1f)", i - N + 1, x[i], y[i]);
    //drawText(x[i] + dis/500, y[i] + dis/500, label);
    //}

    // 現在の経路番号表示
    if (showShortestPath && numPaths > 0) {
        drawPathInfo();
    }


    if(showHelp){
      drawKeyHelp();
    }

    
    //miniMap
    if(showMiniMap){
    int miniMapSize = 200;
    double padX = disX * 0.05, padY = disY * 0.05;
    double framePadRate = 0.05;
    glViewport(windowWidth-miniMapSize, windowHeight-miniMapSize,
	       miniMapSize, miniMapSize);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(mapMinX - padX, mapMaxX + padX, mapMinY - padY, mapMaxY + padY);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //背景
    glColor3f(0.95, 0.95, 0.95);
    glBegin(GL_QUADS);
    glVertex2f(mapMinX - padX*framePadRate, mapMinY - padY*framePadRate);
    glVertex2f(mapMaxX + padX*framePadRate, mapMinY - padY*framePadRate);
    glVertex2f(mapMaxX + padX*framePadRate, mapMaxY + padY*framePadRate);
    glVertex2f(mapMinX - padX*framePadRate, mapMaxY + padY*framePadRate);
    glEnd();

    drawMap(disX, disY, 0);
    
    //ミニマップの枠線
    glColor3f(0.6, 0.6, 0.6);
    glBegin(GL_LINE_LOOP);
    glVertex2f(mapMinX - padX*framePadRate, mapMinY - padY*framePadRate);
    glVertex2f(mapMaxX + padX*framePadRate, mapMinY - padY*framePadRate);
    glVertex2f(mapMaxX + padX*framePadRate, mapMaxY + padY*framePadRate);
    glVertex2f(mapMinX - padX*framePadRate, mapMaxY + padY*framePadRate);
    glEnd();

    //ミニマップに表示される現在地
    double viewMinX = currentX - viewWidth / 2.0;
    double viewMaxX = currentX + viewWidth / 2.0;
    double viewMinY = currentY - viewHeight / 2.0;
    double viewMaxY = currentY + viewHeight / 2.0;

    glColor3f(1.0, 0.5, 0.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(viewMinX, viewMinY);
    glVertex2f(viewMaxX, viewMinY);
    glVertex2f(viewMaxX, viewMaxY);
    glVertex2f(viewMinX, viewMaxY);
    glEnd();
    }

    glutSwapBuffers();
}

void init(void) {
    getMinMaxIndices(&minX, &maxX, &minY, &maxY);
    double disX = maxX - minX;
    double disY = maxY - minY;
    double mapPadX = disX*mapPadRate;
    double mapPadY = disY*mapPadRate;
    mapMinX = minX - mapPadX;
    mapMaxX = maxX + mapPadX;
    mapMinY = minY - mapPadY;
    mapMaxY = maxY + mapPadY;
    
    currentX = (mapMinX + mapMaxX) / 2.0;
    currentY = (mapMinY + mapMaxY) / 2.0;
    
    glClearColor(0.95, 0.95, 0.95, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
}

void reshape(int w, int h){
  windowWidth = w;
  windowHeight = h;
  glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == 3) { // ホイールアップ
            zoom *= 0.9;
            if (zoom < 0.1) zoom = 0.1;
        }
        else if (button == 4) { // ホイールダウン
            zoom *= 1.1;
            if (zoom > zoomRate) zoom = zoomRate;
        }
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y){
  switch (key) {
        case 'a':  //left
            currentX -= 1 * zoom;
            break;
        case 'd':  //right
            currentX += 1 * zoom;
            break;
        case 'w':  //up
            currentY += 1 * zoom;
            break;
        case 's':  //down
            currentY -= 1 * zoom;
            break;
        case '+':
        case '=':
            zoom *= 0.9f;
            if (zoom < 0.1f) zoom = 0.1f;
            break;
        case '-':
        case '_':
            zoom *= 1.1f;
            if (zoom > zoomRate) zoom = zoomRate;
            break;
        case 'm':
        case 'M':
	    showMiniMap = 1 - showMiniMap;
	    break;
        case 'p':
            showShortestPath = 1 - showShortestPath;
            break;
        case 'h':
            showHelp = 1 - showHelp;
            break;
        case 'q': // 前のペア
            if (totalQueries > 0)
                currentQuery = (currentQuery - 1 + totalQueries) % totalQueries;
            currentPath = 0;
            break;

        case 'e': // 次のペア
            if (totalQueries > 0)
                currentQuery = (currentQuery + 1) % totalQueries;
            currentPath = 0;
            break;

        case 'n':// 次のK経路
            if (numPaths > 0)
                currentPath = (currentPath + 1) % numPaths;
            break;
        case 'b':// 前のK経路
            if (numPaths > 0)
                currentPath = (currentPath - 1 + numPaths) % numPaths;
            break;
        case 27:
            exit(0);
            break;
    }
  //printf("X = %f, Y = %f\n", currentX, currentY);
  

  
  glutPostRedisplay();
}

void draw_window(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Map Viewer");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
}

void disable_edge(int u, int v) {
    for (int i = 0; i < gsize[u]; i++) {
        if (graph[u][i].to == v) {
            disabled_edge_idx[u][i] = 1;
            return;
        }
    }
}

int dijkstra_spur(int start, int goal, int total, int prev[]) {
    static double d[MAXN];
    static int used[MAXN];
    for (int i = 0; i < total; i++) {
        d[i] = INF; used[i] = 0; prev[i] = -1;
    }
    if (disabled_node[start] || disabled_node[goal]) return 0;
    d[start] = 0;
    while (1) {
        int v = -1;
        for (int i = 0; i < total; i++) {
            if (!used[i] && (v < 0 || d[i] < d[v])) v = i;
        }
        if (v < 0 || d[v] >= INF) break;
        used[v] = 1;
        if (v == goal) break;
        for (int ei = 0; ei < gsize[v]; ei++) {
            int u = graph[v][ei].to;
            if (disabled_node[u] || disabled_edge_idx[v][ei]) continue;
            double w = graph[v][ei].cost;
            if (d[u] > d[v] + w + EPS) {
                d[u] = d[v] + w;
                prev[u] = v;
            }
        }
    }
    return d[goal] < INF;
}

void build_path(int start, int goal, int prev[], Path* p) {
    int tmp[MAX_PATH_LEN], t = 0;
    for (int v = goal; v != -1 && t < MAX_PATH_LEN; v = prev[v]) {
        tmp[t++] = v;
    }
    p->len = t;
    for (int i = 0; i < t; i++) {
        p->nodes[i] = tmp[t - 1 - i];
    }
    p->cost = 0;
    for (int i = 0; i + 1 < p->len; i++) {
        p->cost += dist(p->nodes[i], p->nodes[i + 1]);
    }
}

// Yen の k 最短路（変更不要）
int yen_k_shortest(int s, int t, int K, Path result[], int total) {
    Path A[MAXK], B[MAXK * 10];
    int asize = 0, bsize = 0, prev[MAXN];
    // 第1最短路
    if (!dijkstra_spur(s, t, total, prev)) return 0;
    build_path(s, t, prev, &A[0]);
    asize = 1;
    // k>=2
    for (int k = 1; k < K; k++) {
        bsize = 0;
        for (int i = 0; i < A[k - 1].len - 1; i++) {
            int spur = A[k - 1].nodes[i];
            int root_len = i + 1;
            int root_nodes[MAX_PATH_LEN];
            memcpy(root_nodes, A[k - 1].nodes, root_len * sizeof(int));
            // 同じ root を持つ既存候補をエッジ無効化
            for (int j = 0; j < asize; j++) {
                if (A[j].len > i &&
                    memcmp(A[j].nodes, root_nodes, root_len * sizeof(int)) == 0) {
                    disable_edge(root_nodes[i], A[j].nodes[i + 1]);
                    disable_edge(A[j].nodes[i + 1], root_nodes[i]);
                }
            }
            // root ノードを無効化
            for (int j = 0; j < root_len - 1; j++) {
                disabled_node[root_nodes[j]] = 1;
            }
            if (dijkstra_spur(spur, t, total, prev)) {
                Path spur_p; build_path(spur, t, prev, &spur_p);
                if (root_len - 1 + spur_p.len <= MAX_PATH_LEN) {
                    Path tot; tot.len = root_len - 1 + spur_p.len;
                    memcpy(tot.nodes, root_nodes, (root_len - 1) * sizeof(int));
                    memcpy(&tot.nodes[root_len - 1], spur_p.nodes, spur_p.len * sizeof(int));
                    tot.cost = 0;
                    for (int z = 0; z + 1 < tot.len; z++) {
                        tot.cost += dist(tot.nodes[z], tot.nodes[z + 1]);
                    }
                    // 重複除去
                    int dup = 0;
                    for (int z = 0; z < bsize; z++) {
                        if (fabs(B[z].cost - tot.cost) < EPS &&
                            B[z].len == tot.len &&
                            memcmp(B[z].nodes, tot.nodes, tot.len * sizeof(int)) == 0) {
                            dup = 1; break;
                        }
                    }
                    if (!dup) B[bsize++] = tot;
                }
            }
            // 無効化リセット
            for (int j = 0; j < root_len - 1; j++) {
                disabled_node[root_nodes[j]] = 0;
            }
            for (int u = 0; u < total; u++) {
                for (int e = 0; e < gsize[u]; e++) {
                    disabled_edge_idx[u][e] = 0;
                }
            }
        }
        if (bsize == 0) break;
        // B から最小を A に
        int idx = 0;
        for (int i = 1; i < bsize; i++) {
            if (B[i].cost < B[idx].cost) idx = i;
        }
        A[asize++] = B[idx];
        memmove(&B[idx], &B[idx + 1], (bsize - idx - 1) * sizeof(Path));
    }
    int out = asize < K ? asize : K;
    for (int i = 0; i < out; i++) result[i] = A[i];
    return out;
}
/*** Yen関数群ここまで ***/

double cross(double x1, double y1, double x2, double y2) {
    return x1 * y2 - y1 * x2;
}

int same_point(double x1, double y1, double x2, double y2) {
    return fabs(x1 - x2) < EPS && fabs(y1 - y2) < EPS;
}

int intersect(double ax, double ay, double bx, double by,
              double cx, double cy, double dx, double dy,
              double *ix, double *iy) {
    double A = bx - ax, B = by - ay;
    double C = dx - cx, D = dy - cy;
    double E = cx - ax, F = cy - ay;

    double denom = cross(A, B, C, D);
    if (fabs(denom) < EPS) return 0;

    double s = cross(E, F, C, D) / denom;
    double t = cross(E, F, A, B) / denom;

    if (s < -EPS || s > 1 + EPS || t < -EPS || t > 1 + EPS) return 0;

    *ix = ax + s * A;
    *iy = ay + s * B;

    if (same_point(*ix, *iy, ax, ay) || same_point(*ix, *iy, bx, by) ||
        same_point(*ix, *iy, cx, cy) || same_point(*ix, *iy, dx, dy)) return 0;

    return 1;
}

double dist(int a, int b) {
    double dx = x[a] - x[b];
    double dy = y[a] - y[b];
    return sqrt(dx * dx + dy * dy);
}

double dist2(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return dx * dx + dy * dy;
}

int add_point(double px, double py, int *total) {
    for (int i = 0; i < *total; i++) {
        if (same_point(x[i], y[i], px, py)) return i;
    }
    x[*total] = px;
    y[*total] = py;
    return (*total)++;
}

void add_edge(int u, int v){
    double d = dist(u, v);
    graph[u][gsize[u]].to = v;
    graph[u][gsize[u]++].cost = d;
    graph[v][gsize[v]].to = u;
    graph[v][gsize[v]++].cost = d;
}

double dijkstra_with_path(int start, int goal, int total, int *prev) {
    double d[MAXN];
    int used[MAXN] = {0};
    for (int i = 0; i < total; i++) {
        d[i] = INF;
        prev[i] = -1;
    }
    d[start] = 0;

    while (1) {
        int v = -1;
        for (int i = 0; i < total; i++) {
            if (!used[i] && (v == -1 || d[i] < d[v])) v = i;
        }
        if (v == -1) break;
        used[v] = 1;

        for (int i = 0; i < gsize[v]; i++) {
            Edge e = graph[v][i];
            double new_dist = d[v] + e.cost;
            if (d[e.to] > new_dist + EPS) {
                d[e.to] = new_dist;
                prev[e.to] = v;
            } else if (fabs(d[e.to] - new_dist) < EPS) {
                // «är
                if (prev[e.to] == -1 || v < prev[e.to]) {
                    prev[e.to] = v;
                }
            }
        }
    }
    return d[goal] < INF ? d[goal] : -1;
}

void print_path(int from, int to, int *prev) {
    int path[MAXN], len = 0;
    for (int v = to; v != -1; v = prev[v]) {
        path[len++] = v;
    }
    for (int i = len - 1; i >= 0; i--) {
        if (path[i] < N) printf("%d", path[i] + 1);
        else printf("C%d", path[i] - N + 1);
        if (i > 0) printf(" ");
    }
    printf("\n");
}

int main(void) {
    int intersectionCount = 0; // 交差点総数（元からのtotal-Nを把握しても良い）
    int addedPointsCount = 0;  // P追加点の個数

    scanf("%d %d %d %d", &N, &M, &P, &Q);
    total = N;

    for (int i = 0; i < N; i++) scanf("%lf %lf", &x[i], &y[i]);

    PointOnEdge segpts[500][100];
    int segcnt[500] = {0};

    for (int i = 0; i < M; i++) {
        scanf("%d %d", &b[i], &e[i]);
        b[i]--; e[i]--;
        segpts[i][segcnt[i]++] = (PointOnEdge){x[b[i]], y[b[i]], b[i]};
        segpts[i][segcnt[i]++] = (PointOnEdge){x[e[i]], y[e[i]], e[i]};
    }

    // 交差判定と交点追加
    for (int i = 0; i < M; i++) {
        for (int j = i + 1; j < M; j++) {
            double ix, iy;
            if (intersect(x[b[i]], y[b[i]], x[e[i]], y[e[i]],
                          x[b[j]], y[b[j]], x[e[j]], y[e[j]], &ix, &iy)) {
                int pid = add_point(ix, iy, &total);
                segpts[i][segcnt[i]++] = (PointOnEdge){ix, iy, pid};
                segpts[j][segcnt[j]++] = (PointOnEdge){ix, iy, pid};
            }
        }
    }

    // 点順序整列と辺追加
    for (int i = 0; i < M; i++) {
        PointOnEdge *pts = segpts[i];
        int cnt = segcnt[i];
        double bx_ = x[b[i]], by_ = y[b[i]];
        for (int j = 0; j < cnt - 1; j++) {
            for (int k = j + 1; k < cnt; k++) {
                if (dist2(pts[j].x, pts[j].y, bx_, by_) > dist2(pts[k].x, pts[k].y, bx_, by_)) {
                    PointOnEdge tmp = pts[j]; pts[j] = pts[k]; pts[k] = tmp;
                }
            }
        }
        for (int j = 0; j < cnt - 1; j++) {
            add_edge(pts[j].id, pts[j + 1].id);
        }
    }

    // --- 5. P個の追加地点の処理（小課題777の内容） ---
// 既存の道路網と追加済み道路網を使って繋ぐ点を決定し、
// 新たにsegpts[]やsegcnt[]、辺を更新する

for (int p = 0; p < P; p++) {
    double nx, ny;
    scanf("%lf %lf", &nx, &ny);

    double bestDist2 = 1e300;
    double bestCx = 0, bestCy = 0;
    int bestSeg = -1;

    // 既存の全ての道路セグメント(拡張済み)を対象に最短距離点を探索
    for (int s = 0; s < M + addedPointsCount; s++) {
        // s番目の道路の区間の点列（segpts[s]）を利用
        PointOnEdge *pts = segpts[s];
        int cnt = segcnt[s];
        // 道路区間は複数の点で分割されているため、区間ごとに最短点を求める
        for (int idx = 0; idx + 1 < cnt; idx++) {
            double x1 = pts[idx].x, y1 = pts[idx].y;
            double x2 = pts[idx + 1].x, y2 = pts[idx + 1].y;
            double vx = x2 - x1;
            double vy = y2 - y1;
            double wx = nx - x1;
            double wy = ny - y1;
            double c1 = vx * wx + vy * wy;
            double c2 = vx * vx + vy * vy;
            double t = (c2 > EPS) ? (c1 / c2) : 0.0;
            double cx, cy;
            if (t <= 0.0) {
                cx = x1; cy = y1;
            } else if (t >= 1.0) {
                cx = x2; cy = y2;
            } else {
                cx = x1 + t * vx; cy = y1 + t * vy;
            }
            double dx = nx - cx;
            double dy = ny - cy;
            double dist2 = dx * dx + dy * dy;
            if (dist2 + EPS < bestDist2) {
                bestDist2 = dist2;
                bestCx = cx;
                bestCy = cy;
                bestSeg = s;
            }
            // tie-breakは早いセグメント番号優先でOK（forループ順序で保証）
        }
    }

    // 新地点を座標配列に追加
    x[total] = nx;
    y[total] = ny;

    // 新交差点（接続点）を座標配列に追加
    x[total + 1] = bestCx;
    y[total + 1] = bestCy;

    // 新たな道路区間を追加
    // bestSegに新しい点を接続するためsegptsとsegcntも更新
    // 既存の道路区間の点列に bestCx,bestCy 点を追加する
    // (bestSegのsegptsにbestCx,bestCy点を入れる)
    segpts[bestSeg][segcnt[bestSeg]++] = (PointOnEdge){bestCx, bestCy, total + 1};

    // 追加点用の新しい道路セグメントも作成
    int newSegIndex = M + addedPointsCount;
    segpts[newSegIndex][0] = (PointOnEdge){nx, ny, total};
    segpts[newSegIndex][1] = (PointOnEdge){bestCx, bestCy, total + 1};
    segcnt[newSegIndex] = 2;

    // 新しい道路の本数を増やす
    addedPointsCount++;

    // 新しい辺を追加 (totalは追加点の番号、total+1は交差点の番号)
    add_edge(total, total + 1);

    // 交差点数も増える（追加点の接続点）
    intersectionCount++;

    // 点の総数を2増やす（追加点と接続点）
    total += 2;
}

    // Q==0の場合はパス
    if (Q > 0) {
        // 元のQ処理は残すが出力はしない（OpenGLで描画）
        for (int q = 0; q < Q; q++) {
            int si = -1, di = -1, k = 1;
            char s[10], t[10];
            scanf("%s %s %d", s, t, &k);

            si = (s[0] == 'C') ? N + atoi(s + 1) - 1 : atoi(s) - 1;
            di = (t[0] == 'C') ? N + atoi(t + 1) - 1 : atoi(t) - 1;

            if (si < 0 || si >= total || di < 0 || di >= total) {
                // 異常入力は無視 or OpenGL側でNA表示用のフラグを立てる
                continue;
            }

            // 無効化リセット
            for (int u = 0; u < total; u++) {
                disabled_node[u] = 0;
                for (int e = 0; e < gsize[u]; e++) disabled_edge_idx[u][e] = 0;
            }

            int k_result = yen_k_shortest(si, di, k, paths, total);
            numPaths = k_result;
            currentPath = 0;
            startNode = si;
            goalNode = di;

            if (k_result == 0) {
                // OpenGL側でNA表示する場合はここでフラグ立てる
                continue;
            }

            // Yenの結果をqueryResultsに保存（変更なし）
            QueryResult* qr = &queryResults[totalQueries++];
            qr->start = si;
            qr->goal = di;
            qr->numPaths = k_result;
            for (int i = 0; i < k_result; i++) {
                qr->paths[i] = paths[i];  // コピー
            }
        }
    }

    // OpenGL
    int fake_argc = 1;
    char* fake_argv[] = { "program" };
    draw_window(fake_argc, fake_argv);

    return 0;
}
