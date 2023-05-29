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
#include <windows.h>
#include <psapi.h>
#define x first
#define y second

#define ID(x, y) ((x) * N + (y))

using namespace std;

const int MAXN = 8;
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

void travel (int x, int y, int &endX, int &endY, string &BOARD, int &currX, int &currY, int &dr)
{
    currX = x;
    currY = y;
    visited[x][y] = true;

    if (currX == endX && currY == endY)
        return;

    if (connectedUp[BOARD[ID(x, y)]][BOARD[ID(x - 1, y)]] && boundaries(x - 1, y) && !visited[x - 1][y])
        dr = 2, travel(x - 1, y, endX, endY, BOARD, currX, currY, dr);

    if (connectedDown[BOARD[ID(x, y)]][BOARD[ID(x + 1, y)]] && boundaries(x + 1, y) && !visited[x + 1][y])
        dr = 1, travel(x + 1, y, endX, endY, BOARD, currX, currY, dr);

    if (connectedRight[BOARD[ID(x, y)]][BOARD[ID(x, y + 1)]] && boundaries(x, y + 1) && !visited[x][y + 1])
        dr = 4, travel(x, y + 1, endX, endY, BOARD, currX, currY, dr);

    if (connectedLeft[BOARD[ID(x, y)]][BOARD[ID(x, y - 1)]] && boundaries(x, y - 1) && !visited[x][y - 1])
        dr = 3, travel(x, y - 1, endX, endY, BOARD, currX, currY, dr);
}

int heur2 (int &currX, int &currY, int &currD, int &goalX, int &goalY, int &goalD)
{
    if (currD == 1)
    {
        if (goalD == 5)
        {
            if (currY == goalY)
                return 4;
            else
                return 2;
        }
        else if (goalD == 6)
        {
            if (currX <= goalX)
                return 4;
            else if (currY == goalY)
                return 0;
            else if (abs(currX - goalX) == 1 && abs(currY - goalY) == 1)
                return 4;
            else
                return 2;
        }
        else if (goalD == 7)
        {
            if (currX == goalX)
                return 3;
            if (currY == goalY)
                return 3;
            if (currX > goalX)
            {
                if (currY > goalY)
                    return 1;
                else
                    return 3;
            }
            else
            {
                if (currY > goalY)
                    return 3;
                else
                    return 3;
            }
        }
        else if (goalD == 8)
        {
            if (currX > goalX && currY < goalY)
                return 1;
            else
                return 3;
        }
    }
    else if (currD == 2)
    {
        if (goalD == 5)
        {
            if (currX >= goalX)
                return 4;
            else if (currY == goalY)
                return 0;
            else if (abs(currX - goalX) == 1 && abs(currY - goalY) == 1)
                return 4;
            else
                return 2;
        }
        else if (goalD == 6)
        {
            if (currY == goalY)
                return 4;
            else
                return 2;
        }
        else if (goalD == 7)
        {
            if (currX < goalX && currY > goalY)
                return 1;
            else
                return 3;
        }
        else if (goalD == 8)
        {
            if (currX < goalX && currY < goalY)
                return 1;
            else
                return 3;
        }
    }
    else if (currD == 3)
    {
        if (goalD == 5)
        {
            if (currX < goalX && currY < goalY)
                return 1;
            else
                return 3;
        }
        else if (goalD == 6)
        {
            if (currX > goalX && currY < goalY)
                return 1;
            else
                return 3;
        }
        else if (goalD == 7)
        {
            if (currX == goalX)
                return 4;
            else
                return 2;
        }
        else if (goalD == 8)
        {
            if (currY >= goalY)
                return 4;
            else if (currX == goalX)
                return 0;
            else if (abs(currX - goalX) == 1 && abs(currY - goalY) == 1)
                return 4;
            else
                return 2;
        }
    }
    else if (currD == 4)
    {
        if (goalD == 5)
        {
            if (currX < goalX && currY > goalY)
                return 1;
            else
                return 3;
        }
        else if (goalD == 6)
        {
            if (currX > goalX && currY > goalY)
                return 1;
            else
                return 3;
        }
        else if (goalD == 7)
        {
            if (currY <= goalY)
                return 4;
            else if (currX == goalX)
                return 0;
            else if (abs(currX - goalX) == 1 && abs(currY - goalY) == 1)
                return 4;
            else
                return 2;
        }
        else if (goalD == 8)
        {
            if (currX == goalX)
                return 4;
            else
                return 2;
        }
    }
}

int heur (int &startX, int &startY, int &endX, int &endY, int dist, string &BOARD)
{
    int currX, currY, dr = 0;
    int goalX, goalY;

    memset(visited, false, sizeof(visited));
    travel(startX, startY, endX, endY, BOARD, currX, currY, dr);

    if (currX == endX && currY == endY) return -1;

    int currD = sdir[BOARD[ID(currX, currY)]];
    if (dr > 0) currD -= dr;
    assert(currD > 0 && currD < 5);

    dr = 0;
    memset(visited, false, sizeof(visited));
    travel(endX, endY, startX, startY, BOARD, goalX, goalY, dr);

    int goalD = sdir[BOARD[ID(goalX, goalY)]];
    if (dr > 0) goalD -= dr - 4;
    assert(goalD > 4 && goalD < 9);

    int h = heur2(currX, currY, currD, goalX, goalY, goalD);
    int mh = max(abs(currX - goalX) + abs(currY - goalY) - 1, h);

    return dist + (h + mh);
}

void A_Star (STATE s)
{
    auto my_cmp = [](const STATE &s1, const STATE &s2) {
        return s1.hValue > s2.hValue;
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

        if (hValue == -1)
        {
            cout << "Solution of length " << moves.size() << " found in " << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - t).count() << " secs" << endl;
            for (int i = 0; i < moves.size(); i++)
                cout << moves[i].x.y + 1 << " " << moves[i].x.x + 1 << " " << moves[i].y << endl;
            return;
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
                                Q.push(STATE{BOARD, nextMoves, heur(startX, startY, endX, endY, nextMoves.size(), BOARD)});
                            }
                            swap (BOARD[ID(new_x, new_y)], BOARD[ID(i, j)]);
                        }
                    }
                }
            }
        }
    }
}

int main()
{
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

    vector<pair<pair<int, int>, char> > vv;
    STATE starting{ START, vv, 0 };
    t = chrono::steady_clock::now();
    A_Star(starting);

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    cout << "Peak committed memory usage: " << pmc.PeakPagefileUsage << " bytes, Peak physical (RAM) memory usage: " << pmc.PeakWorkingSetSize << " bytes." << endl;

    return 0;
}
