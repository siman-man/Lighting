#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <set>
#include <string>

using namespace std;
typedef long long ll;

const ll CYCLE_PER_SEC = 2400000000;
const int SCALE = 1;
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

struct Point {
  int y;
  int x;

  Point(int y = 0, int x = 0) {
    this->y = y;
    this->x = x;
  }

  string to_s() {
    return to_string(x) + ".50" + " " + to_string(y) + ".50";
  }
};

int g_LightDistance;
int g_width;
int g_height;

class Lighting {
  public:
    vector<string> setLights(vector<string> map, int D, int L) {
      vector<string> ret;
      vector<Point> points;

      g_LightDistance = D * SCALE;
      g_height = map.size();
      g_width = map[0].size();

      srand(123);
      int S = map.size();
      for (int i = 0; i < L; ++i) {
        Point p(rand() % S, rand() % 5);
        ret.push_back(p.to_s());
        //ret.push_back(to_string(rand() % S) + "." + to_string(rand() % 90 +10) + " " + to_string(rand() % S) + "." + to_string(rand() % 90 +10));
      }
      return ret;
    }

    void markPointsIlluminated(Point p) {
      int boxX1 = max(0, p.x - g_LightDistance);
      int boxX2 = min(g_width-1, p.x + g_LightDistance);
      int boxY1 = max(0, p.y - g_LightDistance);
      int boxY2 = min(g_height-1, p.y + g_LightDistance);

      vector<int> localWallsInd;
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
