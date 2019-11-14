// C++11
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
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
  static const int LOG_CACHE_SIZE = 0x10000;
  int logIndex = 0;
  array<double, LOG_CACHE_SIZE> logCache;
  uint64_t x = 88172645463325252LL;

public:
  XorShiftL()
  {
    for (int i = 0; i < LOG_CACHE_SIZE; i++) {
      logCache[i] = log((i + 0.5) / 0x10000);
    }
    shuffle(logCache);
  }
  uint64_t xor64()
  {
    x = x ^ (x << 7);
    return x = x ^ (x >> 9);
  }
  int nextInt(int n) { return xor64() % n; }

  double nextLog()
  {
    const double res = logCache[logIndex++];
    if (logIndex == LOG_CACHE_SIZE) logIndex = 0;
    return res;
  }

  template <typename T>
  void shuffle(T &s)
  {
    for (int i = 1; i < (int)s.size(); i++) {
      int j = nextInt(i + 1);
      if (i != j) {
        std::swap(s[i], s[j]);
      }
    }
  }
};

class FSet
{
  int capacity;
  int len;
  vector<int> value;
  vector<int> index;

  void swapValue(int i, int j)
  {
    auto &x = value[i];
    auto &y = value[j];
    // cerr << "FSet " << i << " " << j << " " << x << " "  << y << endl;
    std::swap(x, y);
    std::swap(index[x], index[y]);
  }

public:
  void init(int c)
  {
    capacity = c;
    len = 0;
    value.clear();
    value.reserve(capacity);
    index.clear();
    index.reserve(capacity);
    for (int i = 0; i < capacity; i++) {
      value.push_back(i);
      index.push_back(i);
    }
  }

  vector<int>::iterator begin()
  {
    return value.begin();
  }

  vector<int>::iterator end()
  {
    return value.begin() + len;
  }

  bool contains(int v) const
  {
    return index[v] < len;
  }

  int size() const { return len; }
  int unusedSize() const { return capacity - len; }

  void removeByValue(int v)
  {
    removeByIndex(index[v]);
  }

  void removeByIndex(int i)
  {
    if (i >= len) return;
    len--;
    swapValue(i, len);
  }

  void insertByValue(int v)
  {
    insertByIndex(index[v]);
  }

  void insertByIndex(int i)
  {
    if (i < len) return;
    // cerr << "FSet " << i << " " << len << " " << index.size() << endl;
    swapValue(i, len);
    len++;
  }

  void clear() { len = 0; }

  int operator[](int i) const
  {
    return value[i];
  }
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
vector<int> curPiece;

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
        if (region[q] != region[p]) {
          return false;
        }
        break;
      }
    }
  }
  return true;
}

int calcPlaceGreedyPiece(int p, vector<int> &region)
{
  if (grid[p] != '#' && region[p] >= 0) {
    for (auto candidatePiece : candidatePieces) {
      if (canPutPiece(region, p, candidatePiece)) {
        return candidatePiece;
      }
    }
  }
  return -1;
}

double calcTemp(double startTime, double endTime, double curTime, double startTemp, double endTemp)
{
  const double c = 1 - (curTime - startTime) / (endTime - startTime);
  return c * (startTemp - endTemp) + endTemp;
}

void solve()
{
  curRegion.resize(N2);
  bestRegion.resize(N2);
  curPiece.resize(N2);
  bestPiece.resize(N2);
  bestScore = -100000;

  vector<int> scores(C, 0);

  int iter1 = 0;
  int iter2 = 0;
  // while (timer.elapsed() < 9500) {
  while (timer.elapsed() < 2000) {
    // while (iter1 < 1000) { // for
    iter1++;

    vector<queue<int>> que(C);
    for (int i = 0; i < C; i++) {
      while (1) {
        int p = rng.nextInt(N2);
        if (grid[p] == '#') continue;
        que[i].emplace(p);
        break;
      }
    }

    fill(curRegion.begin(), curRegion.end(), -1);
    while (1) {
      bool updated = false;
      for (int c = 0; c < C; c++) {
        int p;

        while (!que[c].empty()) {
          tie(p) = que[c].front();
          que[c].pop();
          if (!can(p, c)) {
            continue;
          }
          for (int q : neighbor[p]) {
            que[c].push(q);
          }
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

    fill(scores.begin(), scores.end(), 0);
    for (int p = 0; p < N2; p++) {
      int pi = calcPlaceGreedyPiece(p, curRegion);
      if (pi >= 0) {
        scores[curRegion[p]] += points[pi];
      }
    }

    curScore = *min_element(scores.begin(), scores.end());
    if (bestScore < curScore) {
      bestScore = curScore;
      bestRegion = curRegion;
    }
    // cerr << curScore << endl;
  }

  curRegion = bestRegion;
  curPiece = bestPiece;

  // calculate curPiece
  fill(scores.begin(), scores.end(), 0);
  for (int p = 0; p < N2; p++) {
    int pi = calcPlaceGreedyPiece(p, curRegion);
    if (pi >= 0) {
      curPiece[p] = pi;
      scores[curRegion[p]] += points[pi];
    }
  }

  // annealing best solution
  FSet emptyPos;
  emptyPos.init(N2);

  for (int p = 0; p < N2; p++) {
    if (curRegion[p] >= 0 || grid[p] == '#') {
      continue;
    }
    emptyPos.insertByValue(p);
  }

  bestScore = *min_element(scores.begin(), scores.end());
  bestRegion = curRegion;
  bestPiece = curPiece;

  FSet updatePos;
  updatePos.init(N2);
  vector<int> newScores(C);

  double t0 = timer.elapsed();
  // vector<int> oldPieces;
  while (1) {
    const double t = timer.elapsed();
    if (t >= 9800) break;
    const double temp = calcTemp(t0, 9800, t, 100, 10);
    // for (int _ = 0; _ < 100; _++) {
    iter2++;

    int c = rng.nextInt(C);
    if (emptyPos.size() == 0) continue;
    if (scores[c] != *min_element(scores.begin(), scores.end())) continue;
    int p = emptyPos[rng.nextInt(emptyPos.size())];

    if (curRegion[p] >= 0) {
      cerr << "Illegal state : " << __LINE__ << endl;
      throw;
    }

    const int prevScore = *min_element(scores.begin(), scores.end());
    bool ok = false;
    for (auto q : neighbor[p]) {
      if (curRegion[q] >= 0 && prevScore == scores[curRegion[q]]) {
        ok = true;
        break;
      }
    }
    if (!ok) continue;

    const int PUT_PIECE_IDX = 4; // is knight
    curRegion[p] = c;
    curPiece[p] = PUT_PIECE_IDX;

    // remove all
    emptyPos.removeByValue(p);
    vector<pair<int, int>> removed;
    for (int q : neighbor[p]) {
      if (curRegion[q] >= 0 && curRegion[q] != c) {
        removed.emplace_back(q, curRegion[q]);
        emptyPos.insertByValue(q);
        curRegion[q] = -1;
      }
    }

    fill(newScores.begin(), newScores.end(), 0);
    for (int q = 0; q < N2; q++) {
      int pi = calcPlaceGreedyPiece(q, curRegion);
      if (pi >= 0) {
        curPiece[q] = pi;
        newScores[curRegion[q]] += points[pi];
      }
    }

    int oldScore = *min_element(scores.begin(), scores.end());
    int newScore = *min_element(newScores.begin(), newScores.end());
    // for (int i = 0; i < C; i++) {
    //   cerr << i << " " << scores[i] << " -> " << newScores[i] << endl;
    // }
    // cerr << t << " " << temp << " " << rng.nextLog() << endl;
    const double diff = (-newScore) - (-oldScore);
    if (diff <= -temp * rng.nextLog()) {
      // cerr << oldScore << " -> " << newScore << endl;
      scores = newScores;

      if (bestScore < newScore) {
        bestScore = newScore;
        bestRegion = curRegion;
        bestPiece = curPiece;
      }

      continue;
    }

    emptyPos.insertByValue(p);
    curRegion[p] = -1;
    for (auto q : removed) {
      emptyPos.removeByValue(q.first);
      curRegion[q.first] = q.second;
    }
  }

  cerr << "iter1=" << iter1 << ", iter2=" << iter2 << ", score=" << *min_element(scores.begin(), scores.end()) << endl;
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
          out[cur] = PIECE[bestPiece[cur]];
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