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

float zoom = 0.5f;

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
    if(y[m]>y[i]) m=i;
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

void drawMap(float scale, double minX, double minY, double disX, double disY){
   glPointSize(5.0);

    // ÀWnð´_SÉ²®
    //glPushMatrix();
    //glScaled(1.0 / 1000.0, 1.0 / 1000.0, 1.0);

    // ¹iGbWjÌ`æiDFj
    glColor3f(0.5, 0.5, 0.5);
    for (int i = 0; i < MAXN; i++) {
        for (int j = 0; j < gsize[i]; j++) {
            int to = graph[i][j].to;
            if (i < to) { // d¡h~
                glBegin(GL_LINES);
                //glVertex2d((x[i]-minX), (y[i]-minY));
                //glVertex2d((x[to]-minX), (y[to]-minY));
		glVertex2d(x[i], y[i]);
                glVertex2d(x[to], y[to]);
                glEnd();
            }
        }
    }
    // n_Ì`æiÔj
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < N; i++) {
      //glVertex2d((x[i]-minX), (y[i]-minY));
      glVertex2d(x[i], y[i]);
    }
    glEnd();
    
    // ð_iCjÌ`æiÂj
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_POINTS);
    for (int i = N; i < total; i++) {
      //glVertex2d((x[i]-minX), (y[i]-minY));
	glVertex2d(x[i], y[i]);
    }
    glEnd();

    /*for (int i = 0; i < total; i++) {
    char label[100];
    sprintf(label, "C%d (%.1f, %.1f)", i - N + 1, x[i], y[i]);
    drawText(x[i] + 0.05, y[i] + 0.05, label);
    }*/

    //glPopMatrix();

    /*glColor3f(0.0, 0.0, 0.0);
    glPointSize(8.0);
    glBegin(GL_POINTS);
    //glVertex2f((currentX - minX), (currentY - minY));
    glVertex2f((currentX), (currentY));
    glEnd();*/
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    double minX = x[MinXIndex()];
    double maxX = x[MaxXIndex()];
    double minY = y[MinYIndex()];
    double maxY = y[MaxYIndex()];
    double disX = maxX - minX;
    double disY = maxY - minY;
    

    // \¦{¦in}ÌTCYªLªêÎ©®IÉgå¦ª¬³­Èéj
    //double zoomRate = 0.5;  // {¦i0.5 = n}Ì50%TCYª\¦³êéj

// ©®Å\¦ÍÍðZo
double viewWidth = disX / 3.0 * zoom;   // i³Í}100j
double viewHeight = disY / 3.0 * zoom; 

    //mainMap
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(currentX - viewWidth / 2, currentX + viewWidth / 2,
           currentY - viewHeight / 2, currentY + viewHeight / 2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    drawMap(1.0, minX, minY, disX, disY);

    for (int i = 0; i < total; i++) {
    char label[100];
    sprintf(label, "C%d (%.1f, %.1f)", i - N + 1, x[i], y[i]);
    drawText(x[i] + 0.05, y[i] + 0.05, label);
    }

    //miniMap
    //glClearColor(0.7, 0.7, 0.7, 0.7);

    //glPushMatrix();
    //glScaled(0.25, 0.25, 1.0);
    
    int miniMapSize = 200;
    double padX = disX * 0.05, padY = disY * 0.05;
    glViewport(windowWidth-miniMapSize, windowHeight-miniMapSize, miniMapSize, miniMapSize);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(minX - padX, maxX + padX, minY - padY, maxY + padY);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    drawMap(1.0, minX, minY, disX, disY);
    
   glColor3f(0.6, 0.6, 0.6);
glBegin(GL_LINE_LOOP);
glVertex2f(minX - padX*0.5, minY - padY*0.5);
glVertex2f(maxX + padX*0.5, minY - padY*0.5);
glVertex2f(maxX + padX*0.5, maxY + padY*0.5);
glVertex2f(minX - padX*0.5, maxY + padY*0.5);
glEnd();

double vw = viewWidth, vh = viewHeight;
double viewMinX = currentX - minX - vw / 2.0;
double viewMaxX = currentX - minX + vw / 2.0;
double viewMinY = currentY - minY - vh / 2.0;
double viewMaxY = currentY - minY + vh / 2.0;

glColor3f(0.0, 0.0, 0.0);
glBegin(GL_LINE_LOOP);
glVertex2f(viewMinX, viewMinY);
glVertex2f(viewMaxX, viewMinY);
glVertex2f(viewMaxX, viewMaxY);
glVertex2f(viewMinX, viewMaxY);
glEnd();


    glutSwapBuffers();
}

void init(void) {
    currentX = (x[MinXIndex()] + x[MaxXIndex()]) / 2.0;
    currentY = (y[MinYIndex()] + y[MaxYIndex()]) / 2.0;
    glClearColor(1.0, 1.0, 1.0, 1.0); // wiF
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1); // ¼ÌEBhEÀWn
    
}

void reshape(int w, int h){
  windowWidth = w;
  windowHeight = h;
  glViewport(0, 0, w, h);
}

void keyboard(unsigned char key, int x, int y){
  switch (key) {
        case 'a':  // ¶Ú®
            currentX -= 1 * zoom;
            break;
        case 'd':  // EÚ®
            currentX += 1 * zoom;
            break;
        case 'w':  // ãÚ®
            currentY += 1 * zoom;
            break;
        case 's':  // ºÚ®
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
            if (zoom > 5.0f) zoom = 5.0f;
            break;
        case 27:  // ESCL[
            exit(0);
            break;
    }
  printf("X = %f, Y = %f\n", currentX, currentY);
  glutPostRedisplay();
}

void draw_window(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Map Viewer");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
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
    char output[100][10000]; // ÊÛ¶pobt@
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

    // OpenGLÅ`æEBhEN®i¼øÎj
    int fake_argc = 1;
    char* fake_argv[] = { "program" };
    draw_window(fake_argc, fake_argv);

    return 0;
}
