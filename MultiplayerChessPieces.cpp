// C++11
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#include <chrono>

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

class MultiplayerChessPieces
{
public:
  //place kings such that they don't attack each other
  vector<char> findSolution(int N, int C, char grid[], int points[])
  {
    vector<char> out(2 * N * N);

    int id = 0;
    for (int r = 0, cur = 0; r < N; r++)
      for (int c = 0; c < N; c++, cur++) {
        if (r % 2 == 0 && c % 2 == 0 && grid[cur] == '.') {
          out[cur] = 'K';
          out[cur + N * N] = (char)('0' + (id % C));
          id++;
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
  MultiplayerChessPieces cp;
  int N;
  int C;
  cin >> N;
  cin >> C;

  char grid[N * N];
  for (int i = 0; i < N * N; i++)
    cin >> grid[i];

  int points[5];
  for (int i = 0; i < 5; i++)
    cin >> points[i];

  vector<char> ret = cp.findSolution(N, C, grid, points);
  cout << ret.size() << endl;
  for (int i = 0; i < (int)ret.size(); ++i)
    cout << ret[i] << endl;
  cout.flush();
}