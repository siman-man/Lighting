#include <algorithm>
#include <cstdlib>
#include <math.h>
#include <iostream>
#include <string.h>
#include <map>
#include <sstream>
#include <vector>
#include <set>
#include <string>

using namespace std;
typedef long long ll;

const ll CYCLE_PER_SEC = 2400000000;
const int SCALE = 1;
const double eps = 1e-6;
const int f = 100;
double MAX_TIME = 20.0;

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
  ll y;
  ll x;
  double xd;
  double yd;

  P(ll y = 0, ll x = 0) {
    this->y = y;
    this->x = x;
    this->xd = x / 2.0 / f;
    this->yd = y / 2.0 / f;
  }

  ll P2(ll a) {
    return a * a;
  }

  ll dist2(P other) {
    return P2(x - other.x) + P2(y - other.y);
  }

  bool near(P other, ll d) {
    return dist2(other) <= P2(2 * d * f);
  }

  string to_s() {
    return to_string(x) + ".50" + " " + to_string(y) + ".50";
  }
};

bool boundBoxIntersect(ll a, ll b, ll c, ll d) {
  return max(min(a,b), min(c,d)) <= min(max(a,b), max(c,d));
}

ll orientedAreaSign(P a, P b, P c) {
  int area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  return area == 0 ? 0 : area / abs(area);
}

struct Wall {
  P start;
  P end;

  Wall(P st, P e) {
    start = st;
    end = e;
  }

  bool intersect(Wall other) {
    return boundBoxIntersect(start.x, end.x, other.start.x, other.end.x) &&
           boundBoxIntersect(start.y, end.y, other.start.y, other.end.y) &&
           orientedAreaSign(start, end, other.start) * orientedAreaSign(start, end, other.end) <= 0 &&
           orientedAreaSign(other.start, other.end, start) * orientedAreaSign(other.start, other.end, end) <= 0;
  }
};

ll g_LightDistance;
ll g_LightCount;
ll g_width;
ll g_height;
int S;
vector<Wall> g_walls;
vector<P> g_lights;

class Lighting {
  public:
    void extractWalls(vector<string> map) {

      for (int i = 0; i < S-1; i++) {
        int j = 0;

        while (j < S) {
          if (map[i][j] == map[i+1][j]) {
            j++;
            continue;
          }

          P start(j*2*f, (i+1)*2*f);
          while (j < S && map[i][j] != map[i+1][j]) {
            j++;
          }
          P end(j*2*f, (i+1)*2*f);
          Wall w(start, end);
          g_walls.push_back(w);
        }
      }
    }

    vector<string> setLights(vector<string> map, int D, int L) {
      vector<string> ret;
      vector<P> points;
      g_LightDistance = D;
      g_LightCount = L;
      S = map.size();
      g_height = map.size();
      g_width = map[0].size();

      extractWalls(map);

      srand(123);
      int S = map.size();
      for (int i = 0; i < L; ++i) {
        P p(rand() % S, rand() % 5);
        ret.push_back(p.to_s());
        //ret.push_back(to_string(rand() % S) + "." + to_string(rand() % 90 +10) + " " + to_string(rand() % S) + "." + to_string(rand() % 90 +10));
      }
      return ret;
    }

    double calcScore(vector<string> map) {
      vector<vector<int> > points(S, vector<int>(S, 0));

      for (int i = 0; i < g_LightCount; i++) {
        markPointsIlluminated(i, points);
      }

      int nIllum = 0;
      int nTotal = 0;

      for (int r = 0; r < S; r++) {
        for (int c = 0; c < S; c++) {
          if (map[r][c] == '#') continue;

          for (int x = 0; x < f; x++) {
            for (int y = 0; y < f; y++) {
              nTotal++;
              if (points[r*f+y][c*f+x] > 0) {
                nIllum++;
              }
            }
          }
        }
      }

      return nIllum * 1.0 / nTotal;
    }

    void markPointsIlluminated(int lightInd, vector<vector<int> > &points) {
      P light = g_lights[lightInd];

      int boxX1 = max(0LL, light.x - g_LightDistance);
      int boxX2 = min(g_width-1, light.x + g_LightDistance);
      int boxY1 = max(0LL, light.y - g_LightDistance);
      int boxY2 = min(g_height-1, light.y + g_LightDistance);

      vector<int> localWallsInd;
      for (int i = 0; i < g_walls.size(); i++) {
        Wall w = g_walls[i];
        if (boundBoxIntersect(boxX1, boxX2, w.start.x, w.end.x) &&
            boundBoxIntersect(boxY1, boxY2, w.start.y, w.end.y)) {
              localWallsInd.push_back(i);
            }
      }

      for (int x = (int)boxX1 / 2; x <= boxX2 / 2; x++) {
        for (int y = (int)boxY1 / 2; y <= boxY2 / 2; y++) {
          if (points[y][x] != 0) {
            continue;
          }
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
            points[y][x] = 1 + lightInd;
          }
        }
      }
    }
};

// -------8<------- end of solution submitted to the website -------8<-------

template<class T> void getVector(vector<T>& v) {
  for (int i = 0; i < v.size(); ++i)
    cin >> v[i];
}

int main() {
  MAX_TIME = 2.0;
  Lighting l;
  int S;
  cin >> S;
  vector<string> map(S);
  getVector(map);

  int D;
  cin >> D;

  int maxL;
  cin >> maxL;

  vector<string> ret = l.setLights(map, D, maxL);
  cout << ret.size() << endl;
  for (int i = 0; i < (int)ret.size(); ++i)
    cout << ret[i] << endl;
  cout.flush();
}
