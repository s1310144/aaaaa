#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXN 2000
#define INF 1e9
#define EPS 1e-8

typedef struct {
    int to;
    double cost;
} Edge;

typedef struct {
    double x, y;
    int id;
} PointOnEdge;

Edge graph[MAXN][MAXN];
int gsize[MAXN];
double x[MAXN], y[MAXN];
int N, M, P, Q;
int b[500], e[500];
int total;
float currentX = 50, currentY = 50;
int windowWidth = 800, windowHeight = 600;

int MaxXIndex(void){
  int m = 0;
  for(int i=1;i<total;i++){
    if(x[m]<x[i]) m=i;
  }
  return m;
}

int MinXIndex(void){
  int m = 0;
  for(int i=1;i<total;i++){
    if(x[m]>x[i]) m=i;
  }
  return m;
}

int MaxYIndex(void){
  int m = 0;
  for(int i=1;i<total;i++){
    if(y[m]<y[i]) m=i;
  }
  return m;
}

int MinYIndex(void){
  int m = 0;
  for(int i=1;i<total;i++){
    if(y[m]<y[i]) m=i;
  }
  return m;
}

void drawText(float x, float y, const char *text){
  glRasterPos2f(x, y);
  while(*text){
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *text);
    text++;
  }
}

void drawMap(float scale){
   glPointSize(5.0);
   double disX = x[MaxXIndex()] - x[MinXIndex()];
   double disY = y[MaxXIndex()] - y[MinXIndex()];

    // 座標系を原点中心に調整
    //glPushMatrix();
    //glScaled(1.0 / 1000.0, 1.0 / 1000.0, 1.0);

    glColor3f(0.1,0.1,1.0);
    glBegin(GL_TRIANGLES);
    glVertex3f(0.6, 0.6, 0.0);
    glVertex3f(0.9, 0.6, 0.0);
    glVertex3f(0.8, 0.9, 0.0);
    glEnd();

    // 道（エッジ）の描画（灰色）
    glColor3f(0.5, 0.5, 0.5);
    for (int i = 0; i < MAXN; i++) {
        for (int j = 0; j < gsize[i]; j++) {
            int to = graph[i][j].to;
            if (i < to) { // 重複防止
                glBegin(GL_LINES);
                glVertex2d(x[i]/disX, y[i]/disY);
                glVertex2d(x[to]/disX, y[to]/disY);
                glEnd();
            }
        }
    }
    // 地点の描画（赤）
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < N; i++) {
        glVertex2d(x[i]/disX, y[i]/disY);
    }
    glEnd();

    char label[5];
    // 交点（C）の描画（青）
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_POINTS);
    for (int i = N; i < total; i++) {
      sprintf(label, "%d", i);
        glVertex2d(x[i]/disX, y[i]/disY);
	drawText(x[i]+0.1, y[i]+0.1, label);
    }
    glEnd();

    //glPopMatrix();

    glColor3f(0.0, 0.0, 0.0);
    glPointSize(8.0);
    glBegin(GL_POINTS);
    glVertex2f(currentX*scale, currentY*scale);
    glEnd();
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    //mainMap
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(currentX - 100, currentX+100, currentY-75, currentY+75);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    drawMap(1.0);

    //miniMap
    //glClearColor(0.7, 0.7, 0.7, 0.7);

    glPushMatrix();
    glScaled(0.25, 0.25, 1.0);
    glViewport(windowWidth-200, windowHeight-200, 200, 200);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 100, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    drawMap(1.0);
    glPopMatrix();

    glutSwapBuffers();
}

void init(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0); // 背景色白
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1); // 仮のウィンドウ座標系
}

void reshape(int w, int h){
  windowWidth = w;
  windowHeight = h;
  glViewport(0, 0, w, h);
}

void keyboard(int key, int x, int y){
  switch(key){
  case GLUT_KEY_LEFT: currentX-=2; break;
  case GLUT_KEY_RIGHT: currentX+=2; break;
  case GLUT_KEY_UP: currentY+=2; break;
  case GLUT_KEY_DOWN: currentY-=2; break;
  }
  glutPostRedisplay();
}

void draw_window(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Map Viewer");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(keyboard);
    glutMainLoop();
}


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

void add_edge(int u, int v) {
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
                // 辞書順比較
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

    char s[10], t[10];
    char output[100][10000]; // 結果保存用バッファ
    int output_count = 0;

    for (int q = 0; q < Q; q++) {
        int si = -1, di = -1;
        scanf("%s %s%*s", s, t);

        if (s[0] == 'C') {
            int id = atoi(&s[1]);
            si = N + id - 1;
        } else {
            si = atoi(s) - 1;
        }

        if (t[0] == 'C') {
            int id = atoi(&t[1]);
            di = N + id - 1;
        } else {
            di = atoi(t) - 1;
        }

        if (si < 0 || si >= total || di < 0 || di >= total) {
            strcpy(output[output_count++], "NA\n");
            continue;
        }

        int prev[MAXN];
        double d = dijkstra_with_path(si, di, total, prev);

        if (d < 0) {
            strcpy(output[output_count++], "NA\n");
        } else {
            char buf[10000];
            int path[MAXN], len = 0;
            for (int v = di; v != -1; v = prev[v]) path[len++] = v;

            int offset = sprintf(buf, "%.5lf\n", d);
            for (int i = len - 1; i >= 0; i--) {
                if (path[i] < N)
                    offset += sprintf(buf + offset, "%d", path[i] + 1);
                else
                    offset += sprintf(buf + offset, "C%d", path[i] - N + 1);
                if (i > 0) offset += sprintf(buf + offset, " ");
            }
            sprintf(buf + offset, "\n");
            strcpy(output[output_count++], buf);
        }
    }

    for (int i = 0; i < output_count; i++) {
        fputs(output[i], stdout);
    }

    // OpenGLで描画ウィンドウ起動（仮引数対応）
    int fake_argc = 1;
    char* fake_argv[] = { "program" };
    draw_window(fake_argc, fake_argv);

    return 0;
}
