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
chrono::steady_clock::time_point t;

void calculateConnectedMatrix()
{
    for (int i : set1)
        for (int j : set2)
            connectedUp[i][j] = connectedDown[j][i] = true;
    for (int i : set3)
        for (int j : set4)
            connectedRight[i][j] = connectedLeft[j][i] = true;
}

struct STATE
{
    string BOARD;
    vector<pair<pair<int, int>, char> > moves;
    int hValue;
};

bool boundaries (int x, int y)
{
    return (x >= 0 && x < N && y >= 0 && y < N);
}

bool valid (int x, int y, string &s)
{
    return (boundaries(x, y) && s[ID(x, y)] == 0);
}

int assignH(string & BOARD);
int localBest = INT_MAX;

void A_Star (STATE s)
{
    auto my_cmp = [](const STATE &s1, const STATE &s2) {
        return s1.moves.size() + s1.hValue > s2.moves.size() + s2.hValue;
    };
    priority_queue <STATE, vector<STATE>, decltype(my_cmp)> Q{ my_cmp };
    Q.push (s);
    closedList[s.BOARD] = 0;

    int startX, startY, endX, endY;
    for (int i = 0; i < s.BOARD.size(); i++)
    {
        if (s.BOARD[i] >= 1 && s.BOARD[i] <= 4)
        {
            startX = i / N;
            startY = i % N;
        }
        else if (s.BOARD[i] >= 5 && s.BOARD[i] <= 8)
        {
            endX = i / N;
            endY = i % N;
        }
    }

    while (!Q.empty())
    {
        string BOARD = Q.top().BOARD;
        vector<pair<pair<int, int>, char> > moves = Q.top().moves;
        int hValue = Q.top().hValue;
        Q.pop();
        nodesVisited++;

        if (hValue == 0)
        {
            cout << "Solution of length " << moves.size() << " found in " << chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - t).count() << " secs" << endl;
            for (int i = 0; i < moves.size(); i++)
                cout << moves[i].x.y + 1 << " " << moves[i].x.x + 1 << " " << moves[i].y << endl;
            localBest = min(localBest, (int)moves.size());

            for (int i = 0; i < numProcs; i++)
            {
                if (i == id) continue;
                MPI_Request req;
                MPI_Isend(&localBest, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &req);
            }
            return;
        }

        if (moves.size() + hValue >= localBest) continue;

        if (nodesVisited % 10000 == 0)
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

        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                if ((BOARD[ID(i, j)] >= 9 && BOARD[ID(i, j)] <= 12) || BOARD[ID(i, j)] == 17 || BOARD[ID(i, j)] == 18 || BOARD[ID(i, j)] == 21)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        int new_x = i + d[k].x;
                        int new_y = j + d[k].y;

                        if (valid(new_x, new_y, BOARD))
                        {
                            swap (BOARD[ID(new_x, new_y)], BOARD[ID(i, j)]);
                            if (closedList.count (BOARD) == 0 || closedList.at(BOARD) > moves.size() + 1)
                            {
                                vector<pair<pair<int, int>, char> > nextMoves = moves;
                                nextMoves.push_back(make_pair(make_pair(i, j), dir[k]));
                                closedList[BOARD] = nextMoves.size();
                                Q.push(STATE{BOARD, nextMoves, assignH(BOARD)});
                            }
                            swap (BOARD[ID(new_x, new_y)], BOARD[ID(i, j)]);
                        }
                    }
                }
            }
        }
    }
}

int have[22];
map<int, vector<pair<int, int>>> paths;
int _cnt = 0;

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
    t = chrono::steady_clock::now();
    for (const auto & [h, init] : threadBoards[id])
    {
        if (_i < 5) cout << "Thread " << id << " out of " << numProcs << " analyzing board with starting H: " << h << ", local best " << localBest << ", progress: " << _i << "/" << threadBoards[id].size() << "\n";
        paths = init;
        A_Star({START, {}, h});
        _i++;
        closedList.clear();
    }

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    cout << "Peak committed memory usage: " << pmc.PeakPagefileUsage << " bytes, Peak physical (RAM) memory usage: " << pmc.PeakWorkingSetSize << " bytes." << endl;

    MPI_Finalize();
    return 0;
}
