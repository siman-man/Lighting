#include <algorithm>
#include <cstdlib>
#include <math.h>
#include <iostream>
#include <string.h>
#include <cassert>
#include <map>
#include <sstream>
#include <iomanip>
#include <vector>
#include <set>
#include <string>

using namespace std;
typedef long long ll;

const ll CYCLE_PER_SEC = 2400000000;
const int WALL = -1;
const bool ON = true;
const bool OFF = false;
int SCALE = 10;
double TIME_LIMIT = 19.0;

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

unsigned long long int getCycle() {
  unsigned int low, high;
  __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
  return ((unsigned long long int)low) | ((unsigned long long int)high << 32);
}

double getTime(unsigned long long int begin_cycle) {
  return (double)(getCycle() - begin_cycle) / CYCLE_PER_SEC;
}

struct P {
  int y;
  int x;
  double xd;
  double yd;

  P(int x = 0, int y = 0) {
    this->y = y;
    this->x = x;
    this->xd = x / 2.0 / SCALE;
    this->yd = y / 2.0 / SCALE;
  }

  inline int P2(int a) {
    return a * a;
  }

  inline int dist2(P &other) {
    return P2(x-other.x) + P2(y-other.y);
  }

  bool near(P &other, int d) {
    return dist2(other) <= P2(2 * d * SCALE);
  }

  string to_s() {
    stringstream stream;
    stream << fixed << setprecision(2) << xd << " " << fixed << setprecision(2) << yd;
    return stream.str();
  }
};

inline bool boundBoxIntersect(int a, int b, int c, int d) {
  return max(min(a,b), min(c,d)) <= min(max(a,b), max(c,d));
}

inline int orientedAreaSign(P &a, P &b, P &c) {
  int area = (b.x-a.x) * (c.y-a.y) - (b.y-a.y) * (c.x-a.x);
  return area == 0 ? 0 : area / abs(area);
}

int getCoord(int c) {
  return (c + 0.5) * SCALE * 2;
}

struct Wall {
  P start;
  P end;

  Wall(P st, P e) {
    start = st;
    end = e;
  }

  bool intersect(Wall &other) {
    return boundBoxIntersect(start.x, end.x, other.start.x, other.end.x) &&
           boundBoxIntersect(start.y, end.y, other.start.y, other.end.y) &&
           orientedAreaSign(start, end, other.start) * orientedAreaSign(start, end, other.end) <= 0 &&
           orientedAreaSign(other.start, other.end, start) * orientedAreaSign(other.start, other.end, end) <= 0;
  }
};

int g_LightDistance;
int g_LightCount;
int S;
ll startCycle;
vector<Wall> g_walls;
vector<vector<int> > g_points;
vector<P> g_lights;
vector<string> g_map;

class Lighting {
  public:
    void init(vector<string> map, int D, int L) {
      startCycle = getCycle();

      g_LightDistance = D;
      g_LightCount = L;
      g_map = map;
      S = map.size();
      g_lights = vector<P>(L);
      SCALE = ceil(sqrt(2500 / (S * S)));
      assert(SCALE > 0);

      g_points = vector<vector<int> >(S*SCALE, vector<int>(S*SCALE, 0));

      for (int r = 0; r < S; r++) {
        for (int c = 0; c < S; c++) {
          if (g_map[r][c] != '#') continue;

          for (int x = c*SCALE; x < (c+1)*SCALE; x++) {
            for (int y = r*SCALE; y < (r+1)*SCALE; y++) {
              g_points[y][x] = WALL;
            }
          }
        }
      }

      cerr << "S = " << S << ", D = " << D << ", L = " << L << ", SCALE = " << SCALE << endl;
    }

    void extractWalls(vector<string> map) {

      for (int i = 0; i < S-1; i++) {
        int j = 0;

        while (j < S) {
          if (map[i][j] == map[i+1][j]) {
            j++;
            continue;
          }

          P start(j*2*SCALE, (i+1)*2*SCALE);
          while (j < S && map[i][j] != map[i+1][j]) {
            j++;
          }
          P end(j*2*SCALE, (i+1)*2*SCALE);
          Wall w(start, end);
          g_walls.push_back(w);
        }
      }

      for (int j = 0; j < S-1; j++) {
        int i = 0;
        while (i < S) {
          if (map[i][j] == map[i][j+1]) {
            i++;
            continue;
          }

          P start((j+1)*2*SCALE, i*2*SCALE);
          while (i < S && map[i][j] != map[i][j+1]) {
            i++;
          }
          P end((j+1)*2*SCALE, i*2*SCALE);
          Wall w(start, end);
          g_walls.push_back(w);
        }
      }
    }

    vector<string> setLights(vector<string> map, int D, int L) {
      vector<string> ret;

      init(map, D, L);

      extractWalls(map);

      for (int i = 0; i < L; ++i) {
        P p = createRandomPoint();
        g_lights[i] = p;
      }

      for (int i = 0; i < g_LightCount; i++) {
        markPointsIlluminated(i);
      }

      replaceLights();

      for (int i = 0; i < L; ++i) {
        ret.push_back(g_lights[i].to_s());
      }

      fprintf(stderr,"score = %f\n", calcScore());

      return ret;
    }

    vector<string> lights2answer() {
      vector<string> ret;

      for (P p : g_lights) {
        ret.push_back(p.to_s());
      }

      return ret;
    }

    P createRandomPoint() {
      int x, y;
      do {
        x = xor128() % S;
        y = xor128() % S;
      } while (g_map[y][x] == '#');

      return P(getCoord(x), getCoord(y));
    }

    void replaceLights() {
      double bestScore = calcScore();
      double score = 0.0;
      ll tryCount = 0;
      vector<vector<int> > temp = g_points;

      while (true) {
        int lightInd = xor128()%g_LightCount;
        P light = g_lights[lightInd];

        relocationLight(lightInd);

        score = calcScore();

        if (bestScore < score) {
          temp = g_points;
          bestScore = score;
        } else {
          g_points = temp;
          g_lights[lightInd] = light;
        }

        tryCount++;

        if (TIME_LIMIT < getTime(startCycle)) break;
      }

      cerr << "tryCount = " << tryCount << endl;
    }

    void relocationLight(int lightInd) {
      markPointsIlluminated(lightInd, OFF);

      g_lights[lightInd] = createRandomPoint();

      markPointsIlluminated(lightInd, ON);
    }

    double calcScore() {
      int nIllum = 0;
      int nTotal = 0;

      for (int r = 0; r < S; r++) {
        for (int c = 0; c < S; c++) {
          if (g_map[r][c] == '#') continue;

          for (int x = 0; x < SCALE; x++) {
            for (int y = 0; y < SCALE; y++) {
              nTotal++;
              if (g_points[r*SCALE+y][c*SCALE+x] > 0) {
                nIllum++;
              }
            }
          }
        }
      }

      return nIllum * 1.0 / nTotal;
    }

    int markPointsIlluminated(int lightInd, bool swt = ON) {
      P light = g_lights[lightInd];

      int boxX1 = max(0, light.x - 2*SCALE*g_LightDistance);
      int boxX2 = min(2*(SCALE*S-1), light.x + 2*SCALE*g_LightDistance);
      int boxY1 = max(0, light.y - 2*SCALE*g_LightDistance);
      int boxY2 = min(2*(SCALE*S-1), light.y + 2*SCALE*g_LightDistance);

      vector<int> localWallsInd;
      for (int i = 0; i < g_walls.size(); i++) {
        Wall *w = &g_walls[i];
        if (boundBoxIntersect(boxX1, boxX2, w->start.x, w->end.x) &&
            boundBoxIntersect(boxY1, boxY2, w->start.y, w->end.y)) {
              localWallsInd.push_back(i);
            }
      }

      int lightingCount = 0;

      for (int x = boxX1 / 2; x <= boxX2 / 2; x++) {
        for (int y = boxY1 / 2; y <= boxY2 / 2; y++) {
          if (g_points[y][x] == WALL) continue;
          P point(x*2+1, y*2+1);
          if (!light.near(point, g_LightDistance)) continue;
          bool ok = true;
          Wall beam(point, light);
          for (int ind : localWallsInd) {
            if (beam.intersect(g_walls[ind])) {
              ok = false;
              break;
            }
          }
          if (ok) {
            assert(g_points[y][x] != WALL);
            if (swt) {
              if (g_points[y][x] == 0) lightingCount++;
              g_points[y][x] |= (1 << lightInd);
            } else {
              g_points[y][x] ^= (1 << lightInd);
              if (g_points[y][x] == 0) lightingCount++;
            }
          }
        }
      }

      return lightingCount;
    }
};

// -------8<------- end of solution submitted to the website -------8<-------

template<class T> void getVector(vector<T>& v) { for (int i = 0; i < v.size(); ++i) cin >> v[i];}
int main() {
  TIME_LIMIT = 2.0;
  Lighting l; int s;
  cin >> s;
  vector<string> map(s); getVector(map);
  int D;
  cin >> D;
  int maxL;
  cin >> maxL;
  vector<string> ret = l.setLights(map, D, maxL);
  cout << ret.size() << endl;
  for (int i = 0; i < (int)ret.size(); ++i) {
    cout << ret[i] << endl;}
  cout.flush();
}
