#include <algorithm>
#include <cstdlib>
#include <math.h>
#include <iostream>
#include <string.h>
#include <cassert>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;
typedef long long ll;

const ll CYCLE_PER_SEC = 2400000000;
const int WALL = -1;
int SCALE = 10;
double FIRST_TIME_LIMIT = 15.0;
double SECOND_TIME_LIMIT = 20.6;
int g_LightDistance;
int g_LightCount;
int S;

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

struct Coord {
  int y;
  int x;

  Coord(int y = -1, int x = -1) {
    this->y = y;
    this->x = x;
  }
};

struct P {
  int y;
  int x;

  P(int x = 0, int y = 0) {
    this->y = y;
    this->x = x;
  }

  inline int P2(int a) {
    return a * a;
  }

  inline int dist2(P &other) {
    return P2(x-other.x) + P2(y-other.y);
  }

  inline bool near(P &other, int d) {
    return dist2(other) <= P2(2 * d * SCALE);
  }

  ll hashCode() {
    return y * S*2*SCALE + x;
  }

  string toString() {
    stringstream stream;
    stream << fixed << setprecision(2) << (x/2.0/SCALE) << " " << fixed << setprecision(2) << (y/2.0/SCALE);
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

int getCoord(int c, int t = 0) {
  if (t == 0) {
    return (c + (0.25*(xor128()%4))) * SCALE * 2;
  } else {
    return c * SCALE * 2;
  }
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

ll startCycle;
vector<Wall> g_walls;
int g_points[2000][2000];
P g_lights[20];
unordered_map<ll, vector<Coord> > g_lightMemo;
vector<string> g_map;

class Lighting {
  public:
    void init(vector<string> map, int D, int L) {
      startCycle = getCycle();

      g_LightDistance = D;
      g_LightCount = L;
      g_map = map;
      S = map.size();
      SCALE = ceil(sqrt(10000 / (S * S)));

      cleanPoints();
      cerr << "S = " << S << ", D = " << D << ", L = " << L << ", SCALE = " << SCALE << endl;
    }

    void cleanPoints() {
      memset(g_points, 0, sizeof(g_points));

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
    }

    void extractWalls(vector<string> map) {
      g_walls.clear();

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
          g_walls.push_back(Wall(start, end));
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
          g_walls.push_back(Wall(start, end));
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

      turnOnAllLights();
      replaceLights();

      fprintf(stderr,"score = %f\n", calcScore());

      rescaleMap(4*SCALE);
      turnOnAllLights();

      tweakLightsPosition();

      for (int i = 0; i < L; ++i) {
        ret.push_back(g_lights[i].toString());
      }

      fprintf(stderr,"score = %f\n", calcScore());

      return ret;
    }

    void rescaleMap(int scale) {
      g_lightMemo.clear();

      for(int i = 0; i < g_LightCount; i++) {
        P light = g_lights[i];
        int y = (light.y/2.0/SCALE) * 2.0 * scale;
        int x = (light.x/2.0/SCALE) * 2.0 * scale;
        g_lights[i] = P(x,y);
      }

      SCALE = scale;

      extractWalls(g_map);
      cleanPoints();
    }

    void turnOnAllLights() {
      for(int i = 0; i < g_LightCount; i++) {
        markOnIlluminated(i);
      }
    }

    vector<string> lights2answer() {
      vector<string> ret;

      for (P p : g_lights) {
        ret.push_back(p.toString());
      }

      return ret;
    }

    P createRandomPoint() {
      int x, y;
      do {
        x = xor128() % S;
        y = xor128() % S;
      } while (g_map[y][x] == '#');

      int t1 = xor128()%2;
      int t2 = xor128()%2;

      return P(getCoord(x, t1), getCoord(y, t2));
    }

    P moveLittlePoint(int oy, int ox) {
      int x, y;
      do {
        y = oy + (xor128()%6)-3;
        x = ox + (xor128()%6)-3;
      } while ((y < 0 || x < 0 || y >= S*2*SCALE || x >= S*2*SCALE));

      return P(x, y);
    }

    void replaceLights() {
      P bestLights[20];
      memcpy(bestLights, g_lights, sizeof(g_lights));
      double bestScore = calcScore();
      double goodScore = bestScore;
      double score = 0.0;
      ll tryCount = 0;

      double currentTime = getTime(startCycle);
      double k = 3;
      int R = 1000000;

      while (currentTime < FIRST_TIME_LIMIT) {
        double remainTime = FIRST_TIME_LIMIT - currentTime;
        int lightInd = xor128()%g_LightCount;
        P light = g_lights[lightInd];

        int diffScore = relocationLight(lightInd);
        score = goodScore + diffScore;

        if (bestScore < score) {
          bestScore = score;
          memcpy(bestLights, g_lights, sizeof(g_lights));
        }

        if (goodScore < score || (xor128()%R < R*exp(diffScore/(k*remainTime)))) {
          goodScore = score;
        } else {
          markOffIlluminated(lightInd);
          g_lights[lightInd] = light;
          markOnIlluminated(lightInd);
        }


        tryCount++;
        currentTime = getTime(startCycle);

        if (tryCount % 100000 == 0) {
          //fprintf(stderr,"diff = %d, rate = %f, remainTime = %4.2f\n", diffScore, exp(diffScore/(k*remainTime)), remainTime);
        }
      }

      cerr << "1th tryCount = " << tryCount << endl;
      memcpy(g_lights, bestLights, sizeof(bestLights));
    }

    void tweakLightsPosition() {
      double bestScore = calcScore();
      double score = 0.0;
      ll tryCount = 0;

      cerr << bestScore << endl;

      double currentTime = getTime(startCycle);

      while (currentTime < SECOND_TIME_LIMIT) {
        int lightInd = xor128()%g_LightCount;
        P light = g_lights[lightInd];

        int diffScore = tweakPosition(lightInd);
        score = bestScore + diffScore;

        if (bestScore < score) {
          bestScore = score;
        } else {
          markOffIlluminated(lightInd);
          g_lights[lightInd] = light;
          markOnIlluminated(lightInd);
        }

        tryCount++;
        currentTime = getTime(startCycle);
      }

      cerr << "2th tryCount = " << tryCount << endl;
    }

    int relocationLight(int lightInd) {
      int oldCount = markOffIlluminated(lightInd);

      g_lights[lightInd] = createRandomPoint();

      int newCount = markOnIlluminated(lightInd);

      return newCount - oldCount;
    }

    int tweakPosition(int lightInd) {
      int oldCount = markOffIlluminated(lightInd);

      P light = g_lights[lightInd];
      g_lights[lightInd] = moveLittlePoint(light.y, light.x);

      int newCount = markOnIlluminated(lightInd);

      return newCount - oldCount;
    }

    double calcScore() {
      int nIllum = 0;
      int nTotal = 0;

      for (int r = 0; r < S; r++) {
        for (int c = 0; c < S; c++) {
          if (g_map[r][c] == '#') continue;
          nTotal += SCALE * SCALE;

          for (int x = 0; x < SCALE; x++) {
            for (int y = 0; y < SCALE; y++) {
              if (g_points[r*SCALE+y][c*SCALE+x] > 0) nIllum++;
            }
          }
        }
      }

      return nIllum * 1.0 / nTotal;
    }

    int markOffIlluminated(int lightInd) {
      vector<Coord> *coords = getMarkPoints(lightInd);

      int lightingCount = 0;
      int mask = (1 << lightInd);
      int csize = coords->size();

      for (int i = 0; i < csize; i++) {
        Coord coord = coords->at(i);

        g_points[coord.y][coord.x] ^= mask;
        if (g_points[coord.y][coord.x] == 0) lightingCount++;
      }

      return lightingCount;
    }

    int markOnIlluminated(int lightInd) {
      vector<Coord> *coords = getMarkPoints(lightInd);

      int lightingCount = 0;
      int mask = (1 << lightInd);
      int csize = coords->size();

      for (int i = 0; i < csize; i++) {
        Coord coord = coords->at(i);

        if (g_points[coord.y][coord.x] == 0) lightingCount++;
        g_points[coord.y][coord.x] |= mask;
      }

      return lightingCount;
    }

    vector<Coord>* getMarkPoints(int lightInd) {
      P light = g_lights[lightInd];
      ll hashCode = light.hashCode();

      if (g_lightMemo.count(light.hashCode())) return &g_lightMemo[light.hashCode()];

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

      for (int x = boxX1 / 2; x <= boxX2 / 2; x++) {
        for (int y = boxY1 / 2; y <= boxY2 / 2; y++) {
          if (g_points[y][x] == WALL) continue;
          P point(x*2+1, y*2+1);
          if (!light.near(point, g_LightDistance)) continue;
          bool ok = true;
          Wall beam(point, light);
          int wsize = localWallsInd.size();
          for (int i = 0; i < wsize; i++) {
            if (beam.intersect(g_walls[localWallsInd[i]])) {
              ok = false;
              break;
            }
          }
          if (ok) {
            g_lightMemo[hashCode].push_back(Coord(y,x));
          }
        }
      }

      return &g_lightMemo[hashCode];
    }
};

// -------8<------- end of solution submitted to the website -------8<-------

template<class T> void getVector(vector<T>& v) { for (int i = 0; i < v.size(); ++i) cin >> v[i];}
int main() {
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
