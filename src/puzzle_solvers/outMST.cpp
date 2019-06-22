#include <memory>
#include <string>
#include <queue>
#include <functional>
#include <iostream>
#include <climits>
#include <map>
#include <vector>
#include <set>

#include "puzzle.h"
#include "fill_polygon.h"
#include "solver_registry.h"

namespace
{
struct XorShift {
	unsigned x, y, z, w;
	XorShift() { x = 123456789; y = 362436069; z = 521288629; w = 88675123; }
	XorShift(int _w) { x = 123456789; y = 362436069; z = 521288629; w = _w; }
	void setSeed() { x = 123456789; y = 362436069; z = 521288629; w = 88675123; }
	void setSeed(int _w) { x = 123456789; y = 362436069; z = 521288629; w = _w; }
	unsigned nextUInt() {
		unsigned t = (x ^ (x << 11));
		x = y; y = z; z = w;
		return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
	}
	unsigned nextUInt(unsigned mod) {
		unsigned t = (x ^ (x << 11));
		x = y; y = z; z = w;
		w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
		return w % mod;
	}
	unsigned nextUInt(unsigned l, unsigned r) {
		unsigned t = (x ^ (x << 11));
		x = y; y = z; z = w;
		w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
		return w % (r - l + 1) + l;
	}
	double nextDouble() {
		return double(nextUInt()) / UINT_MAX;
	}
} rnd;

class DisjointSet
{
public:
  std::vector<unsigned> par;
  std::vector<unsigned> rank;
  std::vector<unsigned> sz;
  size_t set_sz;

  DisjointSet() : set_sz(0) {}

  DisjointSet(size_t n) : par(n), rank(n, 0), sz(n, 1), set_sz(n)
  {
    for (size_t i = 0; i < n; i++)
      par[i] = i;
  }

  size_t set_size() { return set_sz; }
  size_t size(size_t v) { return sz[find(v)]; }

  size_t find(size_t x)
  {
    if (par[x] == x)
      return x;
    else
      return par[x] = find(par[x]);
  }

  void unite(size_t x, size_t y)
  {
    x = find(x);
    y = find(y);
    if (x == y)
      return;

    if (rank[x] < rank[y])
      par[x] = y;
    else
    {
      par[y] = x;
      if (rank[x] == rank[y])
        rank[x]++;
    }
    sz[x] += sz[y];
    sz[y] = sz[x];
    set_sz--;
  }

  bool same(size_t x, size_t y)
  {
    return find(x) == find(y);
  }
};

template <typename T>
class Edge
{
public:
  int from, to;
  T cost;

  Edge() : from(0), to(0), cost(0) {}
  Edge(int from, int to) : from(from), to(to), cost(1) {}
  Edge(int from, int to, double cost) : from(from), to(to), cost(cost) {}
};
template <typename T>
std::ostream &operator<<(std::ostream &os, const Edge<T> &e)
{
  os << "{from:" + to_string(e.from) + ",to:" + to_string(e.to) + ",cost:" + to_string(e.cost) + "}";
  return os;
}

template <typename T>
class Kruskal
{
  double MSTcost;
  std::vector<Edge<T>> edges;

public:
  Kruskal(std::vector<Edge<T>> es, int V)
  {
    int E = es.size();
    sort(es.begin(), es.end(), [&](const Edge<T> &e1, const Edge<T> &e2) {
      return e1.cost < e2.cost;
    });
    DisjointSet ds(V);
    MSTcost = 0;

    for (int i = 0; i < E; i++)
    {
      auto &e = es[i];
      if (!ds.same(e.from, e.to))
      {
        ds.unite(e.from, e.to);
        edges.push_back(e);
        MSTcost += e.cost;
      }
    }
  }
  double getCost() { return MSTcost; }
  std::vector<Edge<T>> getEdges() { return edges; }
};

class Dijkstra
{
  using pii = std::pair<int, int>;
  static const int INF = INT_MAX >> 2;
  std::vector<int> d;
  std::vector<int> prev;

public:
  Dijkstra(const std::vector<std::vector<Edge<int>>> &G, int V, int s)
      : d(V, INF), prev(V, -1)
  {
    std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pq;

    d[s] = 0;
    pq.push(pii(0, s));

    while (!pq.empty())
    {
      pii p = pq.top();
      pq.pop();
      int v = p.second;
      if (d[v] < p.first)
        continue;
      for (int i = 0; i < G[v].size(); i++)
      {
        auto &e = G[v][i];
        if (d[e.to] > d[v] + e.cost)
        {
          d[e.to] = d[v] + e.cost;
          prev[e.to] = v;
          pq.push(pii(d[e.to], e.to));
        }
      }
    }
  }

  std::vector<int> shortest_path_cost() { return d; }
  int shortest_path_cost(int v) { return d[v]; }

  std::vector<int> get_path(int t)
  {
    std::vector<int> path;
    for (; t != -1; t = prev[t])
      path.push_back(t);
    reverse(path.begin(), path.end());
    return path;
  }
};

std::vector<std::vector<Edge<int>>> createGridGraph(int H, int W)
{
  auto IJ = [&](int i, int j) {
    return i * W + j;
  };
  std::vector<std::vector<Edge<int>>> G(H * W);
  for (int i = 0; i < H; i++)
  {
    for (int j = 0; j < W - 1; j++)
    {
      int u = IJ(i, j), v = IJ(i, j + 1);
      G[u].push_back(Edge<int>(-1, v, 1));
      G[v].push_back(Edge<int>(-1, u, 1));
    }
  }
  for (int j = 0; j < W; j++)
  {
    for (int i = 0; i < H - 1; i++)
    {
      int u = IJ(i, j), v = IJ(i + 1, j);
      G[u].push_back(Edge<int>(-1, v, 1));
      G[v].push_back(Edge<int>(-1, u, 1));
    }
  }
  return G;
}

} //namespace

PuzzleSolution outMST(PuzzleSolverParam param, Puzzle puzzle)
{
  int tSize = puzzle.tSize, H = tSize, W = tSize;
  auto &iSqs = puzzle.iSqs;
  auto oSqs = puzzle.oSqs;

  std::map<int, int> ij2v;
  std::map<int, int> v2ij;
  std::vector<std::vector<bool>> poly2d(H, std::vector<bool>(W, true));
  //std::vector<std::vector<bool>> in2d(H, std::vector<bool>(W, false));
  std::vector<std::vector<bool>> out2d(H, std::vector<bool>(W, false));
  // for(const Point& in : iSqs){
  //   in2d[in.y][in.x] = true;
  // }
  for (const Point &out : oSqs)
  {
    out2d[out.y][out.x] = true;
  }

  auto IJ = [&](int i, int j) {
    return i * tSize + j;
  };

  auto I = [&](int ij) {
    return ij / tSize;
  };

  auto J = [&](int ij) {
    return ij % tSize;
  };

  int V = 0;
  if (!out2d[0][0])
    oSqs.emplace_back(0, 0);
  for (const Point &p : oSqs)
  {
    int ij = IJ(p.y, p.x);
    ij2v[ij] = V;
    v2ij[V] = ij;
    V++;
  }

  std::vector<Edge<double>> edges;
  for (int u = 0; u < V - 1; u++)
  {
    for (int v = u + 1; v < V; v++)
    {
      int ui = I(v2ij[u]), uj = J(v2ij[u]);
      int vi = I(v2ij[v]), vj = J(v2ij[v]);
      //double cost = sqrt(pow(ui - vi, 2) + pow(uj - vj, 2));
      double cost = abs(ui - vi) + abs(uj - vj);
      edges.emplace_back(u, v, cost);
    }
  }

  Kruskal<double> mst(edges, V);
  std::vector<Edge<double>> mstEdges = mst.getEdges();

  std::set<int> pset;
  std::vector<std::vector<Edge<int>>> G = createGridGraph(H, W);
  for (const Point &p : iSqs)
  {
    G[IJ(p.y, p.x)].clear();
  }
  for (const auto &e : mstEdges)
  {
    int uij = v2ij[e.from], vij = v2ij[e.to];
    Dijkstra djk(G, G.size(), uij);
    auto path = djk.get_path(vij);
    for (int v : path)
    {
      if (pset.find(v) != pset.end())
        continue;
      pset.insert(v);
      poly2d[I(v)][J(v)] = false;
    }
  }

  constexpr int R = 1;
  Map2D map2d(H, W);
  for (int i = 0; i < H; i++)
  {
    for (int j = 0; j < W; j++)
    {
      if (poly2d[i][j])
        map2d.data[IJ(i, j)] = R;
    }
  }

  Polygon fine_polygon;
  assert(parsePolygon(fine_polygon, map2d, R));

  PuzzleSolution solution;
  solution.wall = simplifyPolygon(fine_polygon);

  // // character representation of map ======================================
  // static const char NON_WRAPPED = '.';
  // static const char WRAPPED = ' ';
  // static const char WRAPPY = '@';
  // static const char BOOSTER_MANIPULATOR = 'B';
  // static const char BOOSTER_FAST_WHEEL = 'F';
  // static const char BOOSTER_DRILL = 'L';
  // static const char BOOSTER_TELEPORT = 'R';
  // static const char BOOSTER_CLONING = 'C';
  // static const char WALL = '#';
  // static const char SPAWN_POINT = 'X';

  auto findUnwrapped = [&](){
    int i, j;
    do{
      i = rnd.nextUInt(H);
      j = rnd.nextUInt(W);
    } while(!poly2d[i][j]);
    poly2d[i][j] = false;
    return Point(j, i);
  };

  {
    int bNum = puzzle.bNum;
    int fNum = puzzle.fNum;
    int lNum = puzzle.dNum;
    int rNum = puzzle.rNum;
    int cNum = puzzle.cNum;
    int xNum = puzzle.xNum;
    for(int i = 0; i < bNum; i++) solution.Bs.push_back(findUnwrapped());
    for(int i = 0; i < fNum; i++) solution.Fs.push_back(findUnwrapped());
    for(int i = 0; i < lNum; i++) solution.Ls.push_back(findUnwrapped());
    for(int i = 0; i < rNum; i++) solution.Rs.push_back(findUnwrapped());
    for(int i = 0; i < cNum; i++) solution.Cs.push_back(findUnwrapped());
    for(int i = 0; i < xNum; i++) solution.Xs.push_back(findUnwrapped());
    solution.wrapper = findUnwrapped();
  }
  return solution;
}

REGISTER_PUZZLE_SOLVER("outMST", outMST);
