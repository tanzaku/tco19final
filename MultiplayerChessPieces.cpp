// C++11
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class Timer
{
  chrono::system_clock::time_point start;

public:
  Timer() { reset(); }
  void reset() { start = chrono::system_clock::now(); }

  double elapsed()
  {
    auto end = chrono::system_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
  }
};

class XorShiftL
{
  uint64_t x = 88172645463325252LL;

public:
  uint64_t xor64()
  {
    x = x ^ (x << 7);
    return x = x ^ (x >> 9);
  }
  int nextInt(int n) { return xor64() % n; }
};

const int MAX_N = 50;
const int MAX_N2 = MAX_N * MAX_N;

int N, N2, C;
char *grid;
array<int, 5> points;
array<vector<int>, MAX_N2> neighbor;
XorShiftL rng;

Timer timer;

vector<vector<vector<vector<int>>>> moves; // moves[pos][piece kind][dir][idx]

const string PIECE = "KQRBN";
vector<int> candidatePieces;

int P(int x, int y)
{
  return y * N + x;
}

bool inside(int p)
{
  return (unsigned)p < (unsigned)N;
}

bool inside(int x, int y)
{
  return inside(x) && inside(y);
}

void init(int N_, int C_, char grid_[], int points_[])
{
  N = N_;
  N2 = N * N;
  C = C_;
  grid = grid_;
  for (int i = 0; i < (int)points.size(); i++) {
    points[i] = points_[i];
  }

  for (int y = 0; y < N; y++) {
    for (int x = 0; x < N; x++) {
      auto &nei = neighbor[P(x, y)];
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          if (dx != 0 || dy != 0) {
            int tx = x + dx, ty = y + dy;
            if (inside(tx, ty)) {
              nei.push_back(P(tx, ty));
            }
          }
        }
      }
    }
  }

  moves.resize(N2);
  for (int y = 0; y < N; y++) {
    for (int x = 0; x < N; x++) {
      auto &moveP = moves[P(x, y)];
      moveP.resize(points.size());

      // king
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          int tx = x + dx, ty = y + dy, t = P(tx, ty);
          if ((dx == 0 && dy == 0) || !inside(tx, ty) || grid[t] == '#') {
            continue;
          }
          vector<int> mv(1, t);
          moveP[0].push_back(mv);
        }
      }

      // queen
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          int tx = x + dx, ty = y + dy, t = P(tx, ty);
          if ((dx == 0 && dy == 0) || !inside(tx, ty) || grid[t] == '#') {
            continue;
          }
          vector<int> mv(1, t);
          while (1) {
            tx += dx;
            ty += dy;
            t = P(tx, ty);
            if (!inside(tx, ty) || grid[t] == '#') {
              break;
            }
            mv.push_back(t);
          }
          moveP[1].push_back(mv);
        }
      }

      // rook
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          int tx = x + dx, ty = y + dy, t = P(tx, ty);
          if ((dx == 0 && dy == 0) || (dx != 0 && dy != 0) || !inside(tx, ty) || grid[t] == '#') {
            continue;
          }
          vector<int> mv(1, t);
          while (1) {
            tx += dx;
            ty += dy;
            t = P(tx, ty);
            if (!inside(tx, ty) || grid[t] == '#') {
              break;
            }
            mv.push_back(t);
          }
          moveP[2].push_back(mv);
        }
      }

      // bishop
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          int tx = x + dx, ty = y + dy, t = P(tx, ty);
          if (dx == 0 || dy == 0 || !inside(tx, ty) || grid[t] == '#') {
            continue;
          }
          vector<int> mv(1, t);
          while (1) {
            tx += dx;
            ty += dy;
            t = P(tx, ty);
            if (!inside(tx, ty) || grid[t] == '#') {
              break;
            }
            mv.push_back(t);
          }
          moveP[3].push_back(mv);
        }
      }

      // knight
      for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
          if (!((abs(dx) == 2 && abs(dy) == 1) || (abs(dx) == 1 && abs(dy) == 2))) {
            continue;
          }
          int tx = x + dx, ty = y + dy, t = P(tx, ty);
          if (!inside(tx, ty) || grid[t] == '#') {
            continue;
          }
          vector<int> mv(1, t);
          moveP[4].push_back(mv);
        }
      }
    }
  }

  // pieces (point descending order)
  for (int i = 0; i < (int)points.size(); i++) {
    candidatePieces.push_back(i);
  }
  sort(candidatePieces.begin(), candidatePieces.end(), [](int i, int j) {
    return points[i] > points[j];
  });
}

int curScore;
vector<int> curRegion;

int bestScore;
vector<int> bestRegion;
vector<int> bestPiece;

bool can(int p, int c)
{
  // cerr << p << " " << curRegion.size() << endl;
  // cerr << p << " " << curRegion[p] << endl;
  if (curRegion[p] >= 0) return false;

  // TODO wall
  if (grid[p] == '#') return true;

  for (int q : neighbor[p]) {
    if (curRegion[q] >= 0 && curRegion[q] != c) {
      return false;
    }
  }
  return true;
}

bool canPutPiece(vector<int> &region, int p, int piece)
{
  for (auto &qs : moves[p][piece]) {
    for (int q : qs) {
      if (region[q] >= 0) {
        if (region[q] != region[p]) return false;
        break;
      }
    }
  }
  return true;
}

void solve()
{
  curRegion.resize(N2);
  bestRegion.resize(N2);
  bestPiece.resize(N2);
  bestScore = -100000;

  int iter = 0;
  while (timer.elapsed() < 9500) {
    iter++;

    vector<queue<int>> que(C);
    for (int i = 0; i < C; i++) {
      // TODO start from side?
      que[i].emplace(rng.nextInt(N2));
    }

    fill(curRegion.begin(), curRegion.end(), -1);
    while (1) {
      bool updated = false;
      for (int c = 0; c < C; c++) {
        int p;

        while (!que[c].empty()) {
          tie(p) = que[c].front();
          que[c].pop();
          // cerr << p << " " << c << endl;
          if (!can(p, c)) {
            continue;
          }
          for (int q : neighbor[p]) {
            que[c].push(q);
          }
          // if (grid[p] == '#') {
          //   continue;
          // }
          curRegion[p] = c;
          if (grid[p] == '#') {
            continue;
          }
          updated = true;
          break;
        }
      }
      if (!updated) break;
    }

    // TODO Calculate real score
    vector<int> cnt(C, 0);
    for (int p = 0; p < N2; p++) {
      if (curRegion[p] >= 0) {
        cnt[curRegion[p]]++;
      }
    }

    curScore = *min_element(cnt.begin(), cnt.end());
    if (bestScore < curScore) {
      bestScore = curScore;
      bestRegion = curRegion;
    }
  }

  vector<int> scores(C, 0);
  vector<char> out(2 * N * N);
  for (int p = 0; p < N2; p++) {
    if (grid[p] != '#' && bestRegion[p] >= 0) {
      for (auto candidatePiece : candidatePieces) {
        if (canPutPiece(bestRegion, p, candidatePiece)) {
          bestPiece[p] = PIECE[candidatePiece];
          scores[bestRegion[p]] += points[candidatePiece];
          break;
        }
      }
    }
  }

  cerr << "iter=" << iter << ", score=" << *min_element(scores.begin(), scores.end()) << endl;
}

class MultiplayerChessPieces
{
public:
  //place kings such that they don't attack each other
  vector<char> findSolution(int N_, int C_, char grid_[], int points_[])
  {
    init(N_, C_, grid_, points_);
    solve();

    vector<char> out(2 * N * N);

    for (int r = 0, cur = 0; r < N; r++)
      for (int c = 0; c < N; c++, cur++) {
        if (bestRegion[cur] >= 0 && grid[cur] == '.') {
          out[cur] = bestPiece[cur];
          out[cur + N * N] = (char)('0' + bestRegion[cur]);
        } else {
          out[cur] = grid[cur];
          out[cur + N * N] = '.'; //this player id will be ignored
        }
      }

    return out;
  }
};

int main()
{
  timer.reset();
  MultiplayerChessPieces cp;
  int N_;
  int C_;
  cin >> N_;
  cin >> C_;

  char grid_[N_ * N_];
  for (int i = 0; i < N_ * N_; i++)
    cin >> grid_[i];

  int points_[5];
  for (int i = 0; i < 5; i++)
    cin >> points_[i];

  vector<char> ret = cp.findSolution(N_, C_, grid_, points_);
  cout << ret.size() << endl;
  for (int i = 0; i < (int)ret.size(); ++i)
    cout << ret[i] << endl;
  cout.flush();
}