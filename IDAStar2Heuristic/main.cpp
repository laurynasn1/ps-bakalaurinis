#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <set>
#include <cstring>
#include <cassert>
#include <chrono>
#include <mpi.h>
#include <windows.h>
#include <psapi.h>
#define x first
#define y second

#define ID(x, y) ((x) * N + (y))
#define INF INT_MAX

using namespace std;

const int MAXN = 8;
int id, numProcs;
int N;
unordered_map<string, int> closedList;
pair<int, int> d[] = { make_pair(-1, 0), make_pair(0, 1), make_pair(1, 0), make_pair(0, -1) };
char dir[] = {               '^'       ,       '>'      ,       'v'      ,        '<'       };
bool visited[MAXN][MAXN];

int set1[] = { 1, 18, 20, 15, 16, 11, 12, 5 };
int set2[] = { 2, 18, 20, 13, 14, 9, 10, 6 };
int set3[] = { 3, 9, 13, 11, 15, 17, 19, 7 };
int set4[] = { 4, 10, 14, 12, 16, 17, 19, 8 };

int sets[][8] = {
    { 1, 18, 20, 15, 16, 11, 12, 5 },
    { 2, 18, 20, 13, 14, 9, 10, 6 },
    { 3, 9, 13, 11, 15, 17, 19, 7 },
    { 4, 10, 14, 12, 16, 17, 19, 8 },
};

int sdir[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 2 + 3, 2 + 4, 1 + 3, 1 + 4, 2 + 3, 2 + 4, 1 + 3, 1 + 4, 3 + 4, 1 + 2, 3 + 4, 1 + 2, 0 };

pair<int, int> mv[] = { {0, 0}, {-1, 0}, {1, 0}, {0, 1}, {0, -1} };

bool connectedUp[22][22];
bool connectedDown[22][22];
bool connectedLeft[22][22];
bool connectedRight[22][22];

int nodesVisited;
chrono::steady_clock::time_point _t;

void calculateConnectedMatrix()
{
    for (int i : set1)
        for (int j : set2)
            connectedUp[i][j] = connectedDown[j][i] = true;
    for (int i : set3)
        for (int j : set4)
            connectedRight[i][j] = connectedLeft[j][i] = true;
}

bool boundaries (int x, int y)
{
    return (x >= 0 && x < N && y >= 0 && y < N);
}

bool valid (int x, int y, string &s)
{
    return (boundaries(x, y) && s[ID(x, y)] == 0);
}

int have[22];
map<int, vector<pair<int, int>>> paths;
int _cnt = 0;
int localBest = INT_MAX;
int sums[22];

set<pair<int, map<int, vector<pair<int, int>>>>> initialBoards;

bool notMovable(string & BOARD, int &x, int &y)
{
    return (BOARD[ID(x, y)] >= 13 && BOARD[ID(x, y)] <= 16) || BOARD[ID(x, y)] == 19 || BOARD[ID(x, y)] == 20;
}

bool movable(int b)
{
    return (b >= 9 && b <= 12) || b == 17 || b == 18;
}

int hungarian(int n, int m, vector<vector<int>> & a)
{
    vector<int> u (n+1), v (m+1), p (m+1), way (m+1);
    for (int i=1; i<=n; ++i) {
        p[0] = i;
        int j0 = 0;
        vector<int> minv (m+1, INF);
        vector<char> used (m+1, false);
        do {
            used[j0] = true;
            int i0 = p[j0],  delta = INF,  j1;
            for (int j=1; j<=m; ++j)
                if (!used[j]) {
                    int cur = a[i0][j]-u[i0]-v[j];
                    if (cur < minv[j])
                        minv[j] = cur,  way[j] = j0;
                    if (minv[j] < delta)
                        delta = minv[j],  j1 = j;
                }
            for (int j=0; j<=m; ++j)
                if (used[j])
                    u[p[j]] += delta,  v[j] -= delta;
                else
                    minv[j] -= delta;
            j0 = j1;
        } while (p[j0] != 0);
        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0);
    }
    return -v[0];
}

void recalculateH(string & BOARD, int b)
{
    vector<pair<int, int>> coords;
    for (int i = 0; i < BOARD.size(); i++)
        if (BOARD[i] == b)
            coords.emplace_back(make_pair(i / N, i % N));

    vector<int> row(coords.size() + 1, 0);
    vector<vector<int>> a(paths[b].size() + 1, row);
    for (int j = 0; j < paths[b].size(); j++)
        for (int k = 0; k < coords.size(); k++)
            a[j + 1][k + 1] = abs(paths[b][j].x - coords[k].x) + abs(paths[b][j].y - coords[k].y);

    sums[b] = hungarian(paths[b].size(), coords.size(), a);
}

int assignH(string & BOARD)
{
    int ret = 0;
    map<int, vector<pair<int, int>>> coords;
    for (int i = 0; i < BOARD.size(); i++)
        if (movable(BOARD[i]))
            coords[BOARD[i]].emplace_back(make_pair(i / N, i % N));

    for (int i = 9; i <= 12; i++)
    {
        vector<int> row(coords[i].size() + 1, 0);
        vector<vector<int>> a(paths[i].size() + 1, row);
        for (int j = 0; j < paths[i].size(); j++)
            for (int k = 0; k < coords[i].size(); k++)
                a[j + 1][k + 1] = abs(paths[i][j].x - coords[i][k].x) + abs(paths[i][j].y - coords[i][k].y);
        int h = hungarian(paths[i].size(), coords[i].size(), a);
        ret += h;
    }

    for (int i = 17; i <= 18; i++)
    {
        vector<int> row(coords[i].size() + 1, 0);
        vector<vector<int>> a(paths[i].size() + 1, row);
        for (int j = 0; j < paths[i].size(); j++)
            for (int k = 0; k < coords[i].size(); k++)
                a[j + 1][k + 1] = abs(paths[i][j].x - coords[i][k].x) + abs(paths[i][j].y - coords[i][k].y);
        ret += hungarian(paths[i].size(), coords[i].size(), a);
    }
    return ret;
}

int newH()
{
    int ret = 0;
    for (int i = 9; i <= 12; i++)
        ret += sums[i];
    for (int i = 17; i <= 18; i++)
        ret += sums[i];
    return ret;
}

int initH(string & BOARD)
{
    for (int i = 9; i <= 12; i++)
        recalculateH(BOARD, i);
    for (int i = 17; i <= 18; i++)
        recalculateH(BOARD, i);
    return newH();
}

void moveDfs(string & BOARD, int lx, int ly, int lastDir, int &endX, int &endY)
{
    int x = lx + mv[lastDir].x;
    int y = ly + mv[lastDir].y;
    int dir = lastDir > 2 ? 7 - lastDir : 3 - lastDir;

    if (x == endX && y == endY)
    {
        if (BOARD[ID(x, y)] - 4 == dir)
        {
            _cnt++;
            initialBoards.insert(make_pair(assignH(BOARD), paths));
        }
        return;
    }

    if (!boundaries(x, y) || visited[x][y] || (BOARD[ID(x, y)] >= 1 && BOARD[ID(x, y)] <= 4)) return;


    if (notMovable(BOARD, x, y))
    {
        for (int p : sets[dir - 1])
        {
            if (BOARD[ID(x, y)] == p)
            {
                paths[BOARD[ID(x, y)]].emplace_back(make_pair(x, y));
                visited[x][y] = true;
                moveDfs(BOARD, x, y, sdir[BOARD[ID(x, y)]] - dir, endX, endY);
                visited[x][y] = false;
                paths[BOARD[ID(x, y)]].pop_back();
                break;
            }
        }
        return;
    }

    for (int p : sets[dir - 1])
    {
        if (movable(p) && have[p] > 0)
        {
            int ndir = sdir[p] - dir;
            have[p]--;
            paths[p].emplace_back(make_pair(x, y));
            visited[x][y] = true;
            moveDfs(BOARD, x, y, ndir, endX, endY);
            visited[x][y] = false;
            paths[p].pop_back();
            have[p]++;
        }
    }
}

void allMoves(string & BOARD)
{
    for (char c : BOARD)
        have[c]++;

    int startX, startY, endX, endY;
    for (int i = 0; i < BOARD.size(); i++)
    {
        if (BOARD[i] >= 1 && BOARD[i] <= 4)
        {
            startX = i / N;
            startY = i % N;
        }
        else if (BOARD[i] >= 5 && BOARD[i] <= 8)
        {
            endX = i / N;
            endY = i % N;
        }
    }

    moveDfs(BOARD, startX, startY, BOARD[ID(startX, startY)], endX, endY);
}

vector<pair<pair<int, int>, char>> path;
vector<pair<int, int>> which;

vector<pair<pair<int, int>, int>> generateMovables(string & BOARD, int &startX, int &startY, int &endX, int &endY, int lastX, int lastY)
{
    vector<pair<pair<int, int>, int>> ret;
    ret.reserve(N * N);
    memset(visited, false, sizeof(visited));
    queue<pair<int, int>> q;

    q.push(make_pair(lastX, lastY));
    q.push(make_pair(startX, startY));
    q.push(make_pair(endX, endY));

    visited[lastX][lastY] = true;
    visited[startX][startY] = true;
    visited[endX][endY] = true;

    while (!q.empty())
    {
        pair<int, int> & p = q.front();
        q.pop();

        for (int k = 0; k < 4; k++)
        {
            pair<int, int> newP = make_pair(p.x + d[k].x, p.y + d[k].y);
            if (boundaries(newP.x, newP.y) && !visited[newP.x][newP.y])
            {
                q.push(newP);
                visited[newP.x][newP.y] = true;

            }
            if ((movable(BOARD[ID(p.x, p.y)]) || BOARD[ID(p.x, p.y)] == 21) && valid(newP.x, newP.y, BOARD))
                ret.push_back(make_pair(p, k));
        }
    }
    return ret;
}
int idaNodes = 0;

int IDA_Search(string & BOARD, int g, int threshold, int &startX, int &startY, int &endX, int &endY, int lastX, int lastY)
{
    int h = newH();
    if (h == 0) return -1;
    int f = g + h;

    if (f >= localBest)
        return INT_MAX;
    if (f > threshold)
        return f;

    int minC = INT_MAX;
    idaNodes++;
    if (idaNodes % 1000 == 0)
    {
        int flag;
        MPI_Iprobe(MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
        if (flag)
        {
            int newBest;
            MPI_Recv(&newBest, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            localBest = min(localBest, newBest);
        }
    }
    auto movables = generateMovables(BOARD, startX, startY, endX, endY, lastX, lastY);
    set<pair<int, int>> moves;
    for (int i = 0; i < movables.size(); i++)
    {
        const auto & mvv = movables[i];
        int newX = mvv.x.x + d[mvv.y].x;
        int newY = mvv.x.y + d[mvv.y].y;

        swap(BOARD[ID(mvv.x.x, mvv.x.y)], BOARD[ID(newX, newY)]);
        assert(movable(BOARD[ID(newX, newY)]) || BOARD[ID(newX, newY)] == 21);
        if (movable(BOARD[ID(newX, newY)]))
            recalculateH(BOARD, BOARD[ID(newX, newY)]);
        moves.insert(make_pair(newH(), i));
        swap(BOARD[ID(mvv.x.x, mvv.x.y)], BOARD[ID(newX, newY)]);
        assert(movable(BOARD[ID(mvv.x.x, mvv.x.y)]) || BOARD[ID(mvv.x.x, mvv.x.y)] == 21);
        if (movable(BOARD[ID(mvv.x.x, mvv.x.y)]))
            recalculateH(BOARD, BOARD[ID(mvv.x.x, mvv.x.y)]);
    }

    for (const auto & [h, id] : moves)
    {
        const auto & mvv = movables[id];
        int newX = mvv.x.x + d[mvv.y].x;
        int newY = mvv.x.y + d[mvv.y].y;
        swap(BOARD[ID(mvv.x.x, mvv.x.y)], BOARD[ID(newX, newY)]);
        assert(movable(BOARD[ID(newX, newY)]) || BOARD[ID(newX, newY)] == 21);
        if (movable(BOARD[ID(newX, newY)]))
            recalculateH(BOARD, BOARD[ID(newX, newY)]);
        if (!closedList.count(BOARD) || g + 1 < closedList[BOARD])
        {
            closedList[BOARD] = g + 1;
            path.push_back(make_pair(make_pair(mvv.x.x, mvv.x.y), dir[mvv.y]));
            int t = IDA_Search(BOARD, g + 1, threshold, startX, startY, endX, endY, mvv.x.x, mvv.x.y);
            if (t == -1)
                return -1;
            if (t < minC)
                minC = t;
            path.pop_back();
        }
        swap(BOARD[ID(mvv.x.x, mvv.x.y)], BOARD[ID(newX, newY)]);
        assert(movable(BOARD[ID(mvv.x.x, mvv.x.y)]) || BOARD[ID(mvv.x.x, mvv.x.y)] == 21);
        if (movable(BOARD[ID(mvv.x.x, mvv.x.y)]))
            recalculateH(BOARD, BOARD[ID(mvv.x.x, mvv.x.y)]);
    }
    return minC;
}

void IDA_Star(string BOARD)
{
    int startX, startY, endX, endY;
    for (int i = 0; i < BOARD.size(); i++)
    {
        if (BOARD[i] >= 1 && BOARD[i] <= 4)
        {
            startX = i / N;
            startY = i % N;
        }
        else if (BOARD[i] >= 5 && BOARD[i] <= 8)
        {
            endX = i / N;
            endY = i % N;
        }
    }

    closedList[BOARD] = 1;
    int threshold = initH(BOARD);

    while (true)
    {
        int t = IDA_Search(BOARD, 0, threshold, startX, startY, endX, endY, startX, startY);
        if (t == -1)
        {
            cout << "Solution of length " << path.size() << " found in " << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - _t).count() << " secs" << endl;
            for (const auto & p : path)
                cout << p.x.y + 1 << " " << p.x.x + 1 << " " << p.y << "\n";

            localBest = path.size();
            for (int i = 0; i < numProcs; i++)
            {
                if (i == id) continue;
                MPI_Request req;
                MPI_Isend(&localBest, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &req);
            }
            return;
        }
        if (t == INT_MAX)
        {
            return;
        }
        threshold = t;
        idaNodes = 0;
        closedList.clear();
    }
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    ifstream INPUT ("INPUT.txt");

    INPUT >> N;

    string START (N * N, 0);

    for (int i = 0; i < N; i++)
        for (int j = 0, a; j < N; j++)
        {
            INPUT >> a;
            START[ID(i, j)] = a;
        }

    calculateConnectedMatrix();

    allMoves(START);
    cout << "Initial board count: " << initialBoards.size() << "\n";

    vector<pair<int, map<int, vector<pair<int, int>>>>> threadBoards[numProcs];

    int _i = 0;
    for (const auto & init : initialBoards)
    {
        threadBoards[_i % numProcs].emplace_back(init);
        _i++;
    }

    _i = 0;
    _t = chrono::steady_clock::now();
    for (const auto & [h, init] : threadBoards[id])
    {
        if (_i < 5) cout << "Thread " << id << " out of " << numProcs << " analyzing board with starting H: " << h << ", local best " << localBest << ", progress: " << _i << "/" << threadBoards[id].size() << "\n";
        paths = init;
        IDA_Star(START);
        _i++;
        closedList.clear();
        path.clear();
    }

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    cout << "Peak committed memory usage: " << pmc.PeakPagefileUsage << " bytes, Peak physical (RAM) memory usage: " << pmc.PeakWorkingSetSize << " bytes." << endl;

    MPI_Finalize();
    return 0;
}
