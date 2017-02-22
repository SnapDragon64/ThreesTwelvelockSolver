#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
using namespace std;

#define HIGH_TILE_FREQ 21

int values[16] = {0, 1, 2, 3, 6, 12, 24, 48, 96, 192, 384, 768, 1536, 3072, 6144, -1};
string dirs[4] = {"RIGHT", "UP", "LEFT", "DOWN"};

unsigned short shift_right[1<<16], shift_left[1<<16];
void PrecalculateShifts() {
  int v1[4], v2[4];
  for (v1[0] = 0; v1[0] <= 15; v1[0]++)
  for (v1[1] = 0; v1[1] <= 15; v1[1]++)
  for (v1[2] = 0; v1[2] <= 15; v1[2]++)
  for (v1[3] = 0; v1[3] <= 15; v1[3]++) {
    for (int x = 3; x >= 0; x--) {
      if (v1[x] == 0) {
        for (int x2 = x-1; x2 >= 0; x2--) v2[x2+1] = v1[x2];
        v2[0] = 0;
        break;
      } else if (x == 0) {
        v2[x] = v1[x];
      } else if ((v1[x] == 1 && v1[x-1] == 2) || (v1[x] == 2 && v1[x-1] == 1) ||
                (v1[x] > 2 && v1[x] < 14 && v1[x] == v1[x-1])) {
        v2[x] = max(v1[x], v1[x-1]) + 1;
        for (int x2 = x-2; x2 >= 0; x2--) v2[x2+1] = v1[x2];
        v2[0] = 0;
        break;
      } else {
        v2[x] = v1[x];
      }
    }
    shift_right[v1[0] + (v1[1]<<4) + (v1[2]<<8) + (v1[3]<<12)] = v2[0] + (v2[1]<<4) + (v2[2]<<8) + (v2[3]<<12);
    shift_left [v1[3] + (v1[2]<<4) + (v1[1]<<8) + (v1[0]<<12)] = v2[3] + (v2[2]<<4) + (v2[1]<<8) + (v2[0]<<12);
  }
  for (int b = 0; b < (1<<16); b++) {
    if (shift_right[b] != b) shift_right[b] += 15;
    if (shift_left[b] != b) shift_left[b] += (15<<12);
  }
}

inline unsigned long long transpose(unsigned long long all) {
  return (all & 0xF0000F0000F0000FULL) |
        ((all & 0x0F0000F0000F0000ULL) >> 12) |
        ((all & 0x0000F0000F0000F0ULL) << 12) |
        ((all & 0x00F0000F00000000ULL) >> 24) |
        ((all & 0x00000000F0000F00ULL) << 24) |
        ((all & 0x000F000000000000ULL) >> 36) |
        ((all & 0x000000000000F000ULL) << 36);
}

inline unsigned long long hflip(unsigned long long all) {
  return ((all & 0xF000F000F000F000) >> 12) |
         ((all & 0x0F000F000F000F00) >> 4) |
         ((all & 0x00F000F000F000F0) << 4) |
         ((all & 0x000F000F000F000F) << 12);
}

struct Board {
  union {
    unsigned short rows[4];
    unsigned long long all;
  };
  char deck[4], highest, next;

  Board() : all(0), highest(3) {
    deck[0] = 0;
    deck[1] = deck[2] = deck[3] = 4;
    for (int i = 0; i < 9; i++) {
      int x = rand() % 4, y = rand() % 4;
      if (value(x, y)) { i--; continue; }
      select_next_random();
      set_value(x, y, next);
    }
    select_next_random();
  }

  int value(int x, int y) const {
    return (rows[y] >> (x*4)) & 15;
  }

  void set_value(int x, int y, int v) {
    rows[y] &= ~(15 << (x*4));
    rows[y] |= (v << (x*4));
  }

  void select_next_random() {
    if (highest >= 7 && rand()%HIGH_TILE_FREQ == 0) { next = 4; return; }

    int v = rand() % (deck[1] + deck[2] + deck[3]);
    if (v < deck[1])              { next = 1; deck[1]--; }
    else if (v < deck[1]+deck[2]) { next = 2; deck[2]--; }
    else                          { next = 3; deck[3]--; }

    if (deck[1]+deck[2]+deck[3] == 0) { deck[1] = deck[2] = deck[3] = 4; }
  }

  void set_next(int n) {
    next = n;
    if (n < 4) {
      if (deck[n] == 0) {
        cerr << "Deck is out of sync; resetting." << endl;
        deck[1] = deck[2] = deck[3] = 4;
      }
      deck[n]--;
      if (deck[1]+deck[2]+deck[3] == 0) { deck[1] = deck[2] = deck[3] = 4; }
    }
  }

  void computer_move_random(int d) {
    int xs, ys, dx, dy;
    if (d == 0)      { xs = 0; ys = 0; dx = 0; dy = 1; }
    else if (d == 1) { xs = 0; ys = 3; dx = 1; dy = 0; }
    else if (d == 2) { xs = 3; ys = 0; dx = 0; dy = 1; }
    else if (d == 3) { xs = 0; ys = 0; dx = 1; dy = 0; }
    int i, x, y, n = 0;
    for (i = 0, x = xs, y = ys; i < 4; i++, x += dx, y += dy) n += (value(x, y) == 15);
    assert(n > 0);

    int j = rand() % n;
    for (i = 0, x = xs, y = ys; i < 4; i++, x += dx, y += dy) {
      if (value(x, y) == 15) {
        if (j-- == 0) {
          set_value(x, y, (next == 4) ? 4 + rand() % (highest-6) : next);
        } else {
          set_value(x, y, 0);
        }
      }
    }

    select_next_random();
  }

  bool dead() const {
    for (int y = 0; y < 4; y++) if (shift_right[rows[y]] != rows[y]) return false;
    for (int y = 0; y < 4; y++) if (shift_left[rows[y]] != rows[y]) return false;
    union {
      unsigned short rows2[4];
      unsigned long long all2;
    };
    all2 = transpose(all);
    for (int y = 0; y < 4; y++) if (shift_right[rows2[y]] != rows2[y]) return false;
    for (int y = 0; y < 4; y++) if (shift_left[rows2[y]] != rows2[y]) return false;
    return true;
  }

  bool player_move(int d) {
    unsigned long long old_all = all;
    if (d == 0) {
      for (int y = 0; y < 4; y++) rows[y] = shift_right[rows[y]];
    } else if (d == 2) {
      for (int y = 0; y < 4; y++) rows[y] = shift_left[rows[y]];
    } else {
      union {
        unsigned short rows2[4];
        unsigned long long all2;
      };
      all2 = transpose(all);
      if (d == 3) {
        for (int y = 0; y < 4; y++) rows2[y] = shift_right[rows2[y]];
      } else {
        for (int y = 0; y < 4; y++) rows2[y] = shift_left[rows2[y]];
      }
      all = transpose(all2);
    }
    for (unsigned long long x = all; x; x >>= 4) highest = max(highest, (char)((x&15)%15));
    return all != old_all;
  }

  vector<pair<Board, double>> generate_moves(int d) const {
    vector<pair<Board, double>> ret;
    Board b(*this);
    if (!b.player_move(d)) return ret;

    int xs, ys, dx, dy;
    if (d == 0)      { xs = 0; ys = 0; dx = 0; dy = 1; }
    else if (d == 1) { xs = 0; ys = 3; dx = 1; dy = 0; }
    else if (d == 2) { xs = 3; ys = 0; dx = 0; dy = 1; }
    else if (d == 3) { xs = 0; ys = 0; dx = 1; dy = 0; }
    int i, x, y, n = 0, poss[4];
    for (i = 0, x = xs, y = ys; i < 4; i++, x += dx, y += dy) {
      if (b.value(x, y) == 15) {
        poss[i] = true;
        b.set_value(x, y, 0);
        n++;
      } else {
        poss[i] = false;
      }
    }
    assert(n > 0);

    for (b.next = 1; b.next <= 4; b.next++) {
      double prob;
      if (b.next <= 3) {
        if (!deck[b.next]) continue;
        prob = (double)deck[b.next] / (deck[1]+deck[2]+deck[3]);
        if (b.highest >= 7) prob *= (HIGH_TILE_FREQ-1) / (double)HIGH_TILE_FREQ;
        b.deck[1] = deck[1]; b.deck[2] = deck[2]; b.deck[3] = deck[3];
        b.deck[b.next]--;
        if (deck[1]+deck[2]+deck[3] == 1) b.deck[1] = b.deck[2] = b.deck[3] = 4;
      } else {
        if (b.highest < 7) continue;
        prob = 1.0 / HIGH_TILE_FREQ;
      }
      prob /= n;
      int top = next;
      if (top == 4) top = max(top, b.highest-3);
      prob /= top-next+1;

      for (i = 0, x = xs, y = ys; i < 4; i++, x += dx, y += dy) if (poss[i]) {
        for (int v = next; v <= top; v++) {
          ret.push_back(make_pair(b, prob));
          ret.back().first.set_value(x, y, v);
        }
      }
    }

    return ret;
  }

  vector<int> features() const {
    union {
      unsigned short h_rows[4];
      unsigned long long h_all;
    };
    union {
      unsigned short t_rows[4];
      unsigned long long t_all;
    };
    union {
      unsigned short th_rows[4];
      unsigned long long th_all;
    };
    h_all = hflip(all);
    t_all = transpose(all);
    th_all = hflip(t_all);

    vector<int> ret;
    ret.push_back(min(rows[0], h_rows[0]));
    ret.push_back((1<<16) + min(rows[1], h_rows[1]));
    ret.push_back((1<<16) + min(rows[2], h_rows[2]));
    ret.push_back(min(rows[3], h_rows[3]));
    ret.push_back(min(t_rows[0], th_rows[0]));
    ret.push_back((1<<16) + min(t_rows[1], th_rows[1]));
    ret.push_back((1<<16) + min(t_rows[2], th_rows[2]));
    ret.push_back(min(t_rows[3], th_rows[3]));
    ret.push_back(2*(1<<16) + value(0, 0) + (value(0, 1)<<4) + (value(1, 0)<<8) + (value(1, 1)<<12));
    ret.push_back(2*(1<<16) + value(0, 3) + (value(0, 2)<<4) + (value(1, 3)<<8) + (value(1, 2)<<12));
    ret.push_back(2*(1<<16) + value(3, 0) + (value(3, 1)<<4) + (value(2, 0)<<8) + (value(3, 0)<<12));
    ret.push_back(2*(1<<16) + value(3, 3) + (value(3, 2)<<4) + (value(2, 3)<<8) + (value(3, 3)<<12));
    ret.push_back(3*(1<<16) + value(1, 0) + (value(2, 0)<<4) + (value(1, 1)<<8) + (value(2, 1)<<12));
    ret.push_back(3*(1<<16) + value(3, 1) + (value(3, 2)<<4) + (value(2, 1)<<8) + (value(2, 2)<<12));
    ret.push_back(3*(1<<16) + value(2, 3) + (value(1, 3)<<4) + (value(2, 2)<<8) + (value(1, 2)<<12));
    ret.push_back(3*(1<<16) + value(0, 2) + (value(0, 1)<<4) + (value(1, 2)<<8) + (value(1, 1)<<12));
    ret.push_back(4*(1<<16) + min(value(1, 1), value(2, 2)) + (max(value(1, 1), value(2, 2))<<12) +
                              (min(value(2, 1), value(1, 2))<<4) + (max(value(2, 1), value(1, 2))<<8));
    ret.push_back(5*(1<<16) + min(value(0, 0), value(3, 3)) + (max(value(0, 0), value(3, 3))<<12) +
                              (min(value(3, 0), value(0, 3))<<4) + (max(value(3, 0), value(0, 3))<<8));
    return ret;
  }
};

ostream& operator<<(ostream& out, const Board& b) {
  out << "Next: " << values[b.next];
  if (values[b.next] == 6) out << "+";
  out << "  (deck " << (int)b.deck[1] << (int)b.deck[2] << (int)b.deck[3] << ")";
  out << endl;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (x) out << ',';
      out << setw(4) << values[b.value(x, y)];
    }
    out << endl;
  }
  return out;
}


double ThreelockScore(const Board& b) {
  int ret = 0;
  for (int par = 0; par < 2; par++) {
    int cur = 0;
    for (int y = 0; y < 4; y++) {
      int x = (y&1)^par;
      if (b.value(x, y) == 3 && b.value(x+1, y) > 0 && b.value(x+1, y) != 3 && b.value(x+2, y) == 3) {
        cur++;
      }
      if (b.value(y, x) == 3 && b.value(y, x+1) > 0 && b.value(y, x+1) != 3 && b.value(y, x+2) == 3) {
        cur++;
      }
    }
    ret = max(ret, cur);
  }
  if (b.dead()) return (ret == 8) ? 10 : -10;
  return ret;
}

double SixlockScore(const Board& b) {
  int ret = 0;
  for (int par = 0; par < 2; par++) {
    int cur = 0;
    for (int y = 0; y < 4; y++) {
      int x = (y&1)^par;
      if (b.value(x, y) == 4 && b.value(x+1, y) > 0 && b.value(x+1, y) != 4 && b.value(x+2, y) == 4) {
        cur++;
      }
      if (b.value(y, x) == 4 && b.value(y, x+1) > 0 && b.value(y, x+1) != 4 && b.value(y, x+2) == 4) {
        cur++;
      }
    }
    ret = max(ret, cur);
  }
  if (b.dead()) return (ret == 8) ? 10 : -10;
  return ret;
}

double TwelvelockScore(const Board& b) {
  double ret = 0;
  for (int par = 0; par < 2; par++) {
    double cur = 0;
    for (int y = 0; y < 4; y++) {
      int x = (y&1)^par;
      if (b.value(x, y) == 5 && b.value(x+1, y) > 0 && b.value(x+1, y) != 5 && b.value(x+2, y) == 5) {
        cur += 1.0;
      }
      if (b.value(y, x) == 5 && b.value(y, x+1) > 0 && b.value(y, x+1) != 5 && b.value(y, x+2) == 5) {
        cur += 1.0;
      }
    }
    ret = max(ret, cur);
  }
  if (b.dead()) return (ret == 8) ? 30 : -10;
  for (int y = 0; y < 4; y++)
  for (int x = 0; x < 4; x++) {
    if (b.value(x, y) == 5) ret += 0.1;
  }
  if (b.highest == 7 || b.highest == 9) ret += 1;
  if (b.highest == 8) ret += 2;
  return ret;
}


double FeatureScore[6*(1<<16)];

void TrainFeatureScore(const Board& b, double score) {
  vector<int> f = b.features();
  double cur = 0.0;
  for (auto x : f) cur += FeatureScore[x];
  cur /= f.size();
  for (auto x : f) FeatureScore[x] += (score-cur)/100.0;
}

double SixlockFeatureScore(const Board& b) {
  if (b.dead()) return SixlockScore(b);

  /*vector<int> f = b.features();
  double cur = 0.0;
  for (auto x : f) cur += FeatureScore[x];
  return cur / f.size();*/

  union {
    unsigned short h_rows[4];
    unsigned long long h_all;
  };
  union {
    unsigned short t_rows[4];
    unsigned long long t_all;
  };
  union {
    unsigned short th_rows[4];
    unsigned long long th_all;
  };
  h_all = hflip(b.all);
  t_all = transpose(b.all);
  th_all = hflip(t_all);

  vector<int> ret;
  double cur = 0.0;
  cur += FeatureScore[min(b.rows[0], h_rows[0])];
  cur += FeatureScore[(1<<16) + min(b.rows[1], h_rows[1])];
  cur += FeatureScore[(1<<16) + min(b.rows[2], h_rows[2])];
  cur += FeatureScore[min(b.rows[3], h_rows[3])];
  cur += FeatureScore[min(t_rows[0], th_rows[0])];
  cur += FeatureScore[(1<<16) + min(t_rows[1], th_rows[1])];
  cur += FeatureScore[(1<<16) + min(t_rows[2], th_rows[2])];
  cur += FeatureScore[min(t_rows[3], th_rows[3])];
  cur += FeatureScore[2*(1<<16) + b.value(0, 0) + (b.value(0, 1)<<4) + (b.value(1, 0)<<8) + (b.value(1, 1)<<12)];
  cur += FeatureScore[2*(1<<16) + b.value(0, 3) + (b.value(0, 2)<<4) + (b.value(1, 3)<<8) + (b.value(1, 2)<<12)];
  cur += FeatureScore[2*(1<<16) + b.value(3, 0) + (b.value(3, 1)<<4) + (b.value(2, 0)<<8) + (b.value(3, 0)<<12)];
  cur += FeatureScore[2*(1<<16) + b.value(3, 3) + (b.value(3, 2)<<4) + (b.value(2, 3)<<8) + (b.value(3, 3)<<12)];
  cur += FeatureScore[3*(1<<16) + b.value(1, 0) + (b.value(2, 0)<<4) + (b.value(1, 1)<<8) + (b.value(2, 1)<<12)];
  cur += FeatureScore[3*(1<<16) + b.value(3, 1) + (b.value(3, 2)<<4) + (b.value(2, 1)<<8) + (b.value(2, 2)<<12)];
  cur += FeatureScore[3*(1<<16) + b.value(2, 3) + (b.value(1, 3)<<4) + (b.value(2, 2)<<8) + (b.value(1, 2)<<12)];
  cur += FeatureScore[3*(1<<16) + b.value(0, 2) + (b.value(0, 1)<<4) + (b.value(1, 2)<<8) + (b.value(1, 1)<<12)];
  cur += FeatureScore[4*(1<<16) + min(b.value(1, 1), b.value(2, 2)) + (max(b.value(1, 1), b.value(2, 2))<<12) +
                                 (min(b.value(2, 1), b.value(1, 2))<<4) + (max(b.value(2, 1), b.value(1, 2))<<8)];
  cur += FeatureScore[5*(1<<16) + min(b.value(0, 0), b.value(3, 3)) + (max(b.value(0, 0), b.value(3, 3))<<12) +
                                 (min(b.value(3, 0), b.value(0, 3))<<4) + (max(b.value(3, 0), b.value(0, 3))<<8)];
  return cur / 18.0;
}

double TwelvelockFeatureScore(const Board& b) {
  if (b.dead()) return TwelvelockScore(b);

  /*vector<int> f = b.features();
  double cur = 0.0;
  for (auto x : f) cur += FeatureScore[x];
  return cur / f.size();*/

  union {
    unsigned short h_rows[4];
    unsigned long long h_all;
  };
  union {
    unsigned short t_rows[4];
    unsigned long long t_all;
  };
  union {
    unsigned short th_rows[4];
    unsigned long long th_all;
  };
  h_all = hflip(b.all);
  t_all = transpose(b.all);
  th_all = hflip(t_all);

  vector<int> ret;
  double cur = 0.0;
  cur += FeatureScore[min(b.rows[0], h_rows[0])];
  cur += FeatureScore[(1<<16) + min(b.rows[1], h_rows[1])];
  cur += FeatureScore[(1<<16) + min(b.rows[2], h_rows[2])];
  cur += FeatureScore[min(b.rows[3], h_rows[3])];
  cur += FeatureScore[min(t_rows[0], th_rows[0])];
  cur += FeatureScore[(1<<16) + min(t_rows[1], th_rows[1])];
  cur += FeatureScore[(1<<16) + min(t_rows[2], th_rows[2])];
  cur += FeatureScore[min(t_rows[3], th_rows[3])];
  cur += FeatureScore[2*(1<<16) + b.value(0, 0) + (b.value(0, 1)<<4) + (b.value(1, 0)<<8) + (b.value(1, 1)<<12)];
  cur += FeatureScore[2*(1<<16) + b.value(0, 3) + (b.value(0, 2)<<4) + (b.value(1, 3)<<8) + (b.value(1, 2)<<12)];
  cur += FeatureScore[2*(1<<16) + b.value(3, 0) + (b.value(3, 1)<<4) + (b.value(2, 0)<<8) + (b.value(3, 0)<<12)];
  cur += FeatureScore[2*(1<<16) + b.value(3, 3) + (b.value(3, 2)<<4) + (b.value(2, 3)<<8) + (b.value(3, 3)<<12)];
  cur += FeatureScore[3*(1<<16) + b.value(1, 0) + (b.value(2, 0)<<4) + (b.value(1, 1)<<8) + (b.value(2, 1)<<12)];
  cur += FeatureScore[3*(1<<16) + b.value(3, 1) + (b.value(3, 2)<<4) + (b.value(2, 1)<<8) + (b.value(2, 2)<<12)];
  cur += FeatureScore[3*(1<<16) + b.value(2, 3) + (b.value(1, 3)<<4) + (b.value(2, 2)<<8) + (b.value(1, 2)<<12)];
  cur += FeatureScore[3*(1<<16) + b.value(0, 2) + (b.value(0, 1)<<4) + (b.value(1, 2)<<8) + (b.value(1, 1)<<12)];
  cur += FeatureScore[4*(1<<16) + min(b.value(1, 1), b.value(2, 2)) + (max(b.value(1, 1), b.value(2, 2))<<12) +
                                 (min(b.value(2, 1), b.value(1, 2))<<4) + (max(b.value(2, 1), b.value(1, 2))<<8)];
  cur += FeatureScore[5*(1<<16) + min(b.value(0, 0), b.value(3, 3)) + (max(b.value(0, 0), b.value(3, 3))<<12) +
                                 (min(b.value(3, 0), b.value(0, 3))<<4) + (max(b.value(3, 0), b.value(0, 3))<<8)];
  return cur / 18.0;
}


int SearchedInternalNodes = 0;

pair<int, double> ExpectimaxSearch(const Board& b, double score_func(const Board&), int depth) {
  if (depth == 0) return make_pair(-1, (double)score_func(b));
  pair<int, double> ret(-1, -1e9);
  for (int move = 0; move < 4; move++) {
    double cur = 0.0;
    vector<pair<Board, double>> v = b.generate_moves(move);
    if (!v.size()) continue;
    for (auto it : v) cur += it.second * ExpectimaxSearch(it.first, score_func, depth-1).second;
    if (cur > ret.second) ret = make_pair(move, cur);
  }
  if (ret.first == -1) ret.second = score_func(b); else SearchedInternalNodes++;
  return ret;
}


// My attempt at a smarter search algorithm, but it didn't really help much.
struct SmartExpectimaxNode {
  static constexpr double MAX_SCORE = 10.0;
  static constexpr double MIN_SCORE = -10.0;
  static constexpr double SCORE_EPSILON = 1e-2;  // Consider scores this close to be tied.
  static constexpr double NARROW_RANGE = 0.9;  // Narrow range by this much each time narrow() is called.

  static double (*ScoreFunc)(const Board&);
  static int NodesExpanded;
  
  int depth;
  Board b;
  double prob;
  double bscore;

  bool expanded;
  priority_queue<pair<double, SmartExpectimaxNode*>> pq[4];
  double move_min_score[4], move_max_score[4];

  // Our current range of potential position values.
  // This is not an "estimate"; full expectimax should never compute a value out of this range.
  double min_score, max_score;

  SmartExpectimaxNode(int depth, const Board& b, double prob = 1.0) : depth(depth), b(b), prob(prob) {
    if (depth == 0 || b.dead()) {
      min_score = max_score = bscore = (*ScoreFunc)(b);
      expanded = true;
    } else {
      min_score = MIN_SCORE;
      max_score = MAX_SCORE;
      bscore = (*ScoreFunc)(b);
      expanded = false;
    }
  }

  ~SmartExpectimaxNode() {
    if (!expanded) return;
    for (int d = 0; d < 4; d++) {
      while (pq[d].size()) {
        delete pq[d].top().second;
        pq[d].pop();
      }
    }
  }

  void calc() {
    min_score = max_score = MIN_SCORE;
    for (int d = 0; d < 4; d++) {
      min_score = max(min_score, move_min_score[d]);
      max_score = max(max_score, move_max_score[d]);
    }
  }

  void expand() {
    if (expanded) return;
    expanded = true;
    NodesExpanded++;

    for (int d = 0; d < 4; d++) {
      vector<pair<Board, double>> m = b.generate_moves(d);
      if (!m.size()) {
        move_min_score[d] = move_max_score[d] = MIN_SCORE;
        continue;
      }
      
      move_min_score[d] = move_max_score[d] = 0.0;
      for (auto it : m) {
        SmartExpectimaxNode* nd = new SmartExpectimaxNode(depth-1, it.first, it.second);
        move_min_score[d] += nd->min_score * nd->prob;
        move_max_score[d] += nd->max_score * nd->prob;
        if (nd->max_score - nd->min_score >= SCORE_EPSILON) {
          pq[d].push(make_pair(nd->prob * (nd->max_score - nd->min_score), nd));
        } else {
          delete nd;
        }
      }
    }
    calc();
  }

  void narrow() {
    if (!expanded) {
      double goal = (max_score - min_score) * NARROW_RANGE;
      expand();
      if (max_score - min_score <= goal) return;
    }

    for (int d = 0; d < 4; d++) {
      if (move_max_score[d] - move_min_score[d] < SCORE_EPSILON) continue;
      if (move_max_score[d] < min_score + SCORE_EPSILON) continue;

      double goal = (move_max_score[d] - move_min_score[d]) * NARROW_RANGE;
      while (pq[d].size() && move_max_score[d] - move_min_score[d] > goal) {
        SmartExpectimaxNode* nd = pq[d].top().second;
        pq[d].pop();

        move_min_score[d] -= nd->min_score * nd->prob;
        move_max_score[d] -= nd->max_score * nd->prob;
        nd->narrow();
        move_min_score[d] += nd->min_score * nd->prob;
        move_max_score[d] += nd->max_score * nd->prob;

        if (nd->max_score - nd->min_score >= SCORE_EPSILON) {
          pq[d].push(make_pair(nd->prob * (nd->max_score - nd->min_score), nd));
        } else {
          delete nd;
        }
      }
    }

    calc();

    /*if (depth == 4) {
      cout << move_min_score[0] << ',' << move_max_score[0] << ' ';
      cout << move_min_score[1] << ',' << move_max_score[1] << ' ';
      cout << move_min_score[2] << ',' << move_max_score[2] << ' ';
      cout << move_min_score[3] << ',' << move_max_score[3] << endl;
    }*/
  }
};
double (*SmartExpectimaxNode::ScoreFunc)(const Board&);
int SmartExpectimaxNode::NodesExpanded = 0;

pair<int, double> SmartExpectimaxSearch(const Board& b, double score_func(const Board&), int depth) {
  if (b.dead()) return make_pair(-1, SmartExpectimaxNode::MIN_SCORE);
  bool valid[4];
  for (int d = 0; d < 4; d++) {
    Board b2 = b;
    valid[d] = b2.player_move(d);
  }

  SmartExpectimaxNode::ScoreFunc = score_func;

  SmartExpectimaxNode nd(depth, b);
  nd.expand();
  for (;;) {
    int bestd = -1;
    double bestsc = nd.MIN_SCORE;
    for (int d = 0; d < 4; d++) if (valid[d]) {
      if (bestd == -1 || nd.move_min_score[d] >= bestsc) {
        bestd = d;
        bestsc = nd.move_min_score[d];
      }
    }

    int numd = 0;
    for (int d = 0; d < 4; d++) {
      if (nd.move_max_score[d] > bestsc + SmartExpectimaxNode::SCORE_EPSILON) {
        bestd = d;
        numd++;
      }
    }
    if (numd <= 1) return make_pair(bestd, bestsc);
    nd.narrow();
  }
}


int main(int argc, char *argv[]) {
  if (argc < 3 || strcmp(argv[1], "load") && strcmp(argv[1], "train")) {
    cerr << "Usage: threes ['load' or 'train'] [feature file]" << endl;
    return 0;
  }
  srand(time(0));
  PrecalculateShifts();

  // Feature-based Expectimax search.
  if (!strcmp(argv[1], "load")) {
    ifstream fin(argv[2]);
    for (int i = 0; i < sizeof(FeatureScore)/sizeof(double); i++) fin >> FeatureScore[i];
    cout << "Loaded." << endl;
  } else {
    cout << "Training..." << endl;
    for (int ntrain = 100000000; ntrain > 0;) {
      Board b;
      for (;;) {
        pair<int, double> res = ExpectimaxSearch(b, TwelvelockScore, 1);
        if (res.first == -1) break;
        TrainFeatureScore(b, res.second);
        ntrain--;
        if (!b.player_move(res.first)) continue;
        b.computer_move_random(res.first);
      }
    }
    cout << "Trained." << endl;
  }

  /*{
    Board b;
    b.set_value(0, 0, 2);
    b.set_value(1, 0, 5);
    b.set_value(2, 0, 1);
    b.set_value(3, 0, 2);
    b.set_value(0, 1, 3);
    b.set_value(1, 1, 5);
    b.set_value(2, 1, 3);
    b.set_value(3, 1, 5);
    b.set_value(0, 2, 5);
    b.set_value(1, 2, 3);
    b.set_value(2, 2, 5);
    b.set_value(3, 2, 3);
    b.set_value(0, 3, 3);
    b.set_value(1, 3, 5);
    b.set_value(2, 3, 8);
    b.set_value(3, 3, 5);
    b.set_next(1);
    cout << b;
    pair<int, double> res = ExpectimaxSearch(b, TwelvelockFeatureScore, 4);
    cout << dirs[res.first] << ' ' << res.second << endl;
    return 0;
  }*/

  int fail = 0, succ = 0;
  for (;;) {
    if ((fail+succ) % 1000 == 999) {
      ofstream fout(argv[2]);
      for (int i = 0; i < sizeof(FeatureScore)/sizeof(double); i++) {
        fout << FeatureScore[i] << endl;
      }
    }
    Board b;
    int nmoves = 0;
    while (!b.dead()) {
      nmoves++;
      //if (nmoves > 150) break;

      double t1 = clock();
      SearchedInternalNodes = 0;
      pair<int, double> res = ExpectimaxSearch(b, TwelvelockFeatureScore, 3);
      TrainFeatureScore(b, ExpectimaxSearch(b, TwelvelockScore, 3).second);
      t1 = (clock() - t1) / CLOCKS_PER_SEC;

      //double t2 = clock();
      //SmartExpectimaxNode::NodesExpanded = 0;
      //pair<int, double> res2 = SmartExpectimaxSearch(b, TwelvelockFeatureScore, 4);
      //t2 = (clock() - t2) / CLOCKS_PER_SEC;

      //cout << res.first << ' ' << SearchedInternalNodes << "  " << res.second << "  " << t1 << endl;
      //cout << res2.first << ' ' << SmartExpectimaxNode::NodesExpanded << "  " << res2.second << "  " << t2 << endl;
      //if (res.first != res2.first) cout << b << endl;
      //cout << endl;

      b.player_move(res.first);
      b.computer_move_random(res.first);
    }
    cout << FeatureScore[2*(1<<16) + 0x3132] << endl;
    cout << FeatureScore[2*(1<<16) + 0x3312] << endl;
    cout << nmoves << " moves: " << TwelvelockScore(b) << endl;
    vector<int> f = b.features();
    for (auto x : f) printf(" %x %lf\n", x, FeatureScore[x]);
    if (TwelvelockScore(b) == 30) succ++; else fail++;
    cout << succ << " success, " << fail << " fail (" << succ*100.0/(succ+fail) << "%)" << endl;
    cout << b << endl;
  }


  /*// Naive Expectimax search.
  int fail = 0, succ = 0;
  for (;;) {
    Board b;
    int nmoves = 0;
    while (!b.dead()) {
      nmoves++;
      if (nmoves > 200) break;
      int d = ExpectimaxSearch(b, score_func, 3).first;
      b.player_move(d);
      b.computer_move_random(d);
    }
    cout << nmoves << " moves: " << score_func(b) << endl;
    if (score_func(b) == 10) succ++; else fail++;
    cout << succ << " success, " << fail << " fail (" << succ*100.0/(succ+fail) << "%)" << endl;
    cout << b << endl;
  }*/
}
