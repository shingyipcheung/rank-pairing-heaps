#include <iostream>
#include <fstream>
#include <windows.h>
#include <deque> // hold the result path
#include <ctime>
#include <unordered_map> // hash map coordinate to iterator
#define TYPE1_RANK_REDUCTION
#include "rp_heap.h"
#include "AstarNode.h"

using namespace std; // for demo only

#define BLACK 0
#define DARKYELLOW 6
#define GRAY 8
#define BLUE 9
#define GREEN 10
#define SKYBLUE 11
#define RED 12 
#define PURPLE 13
#define YELLOW 14
#define WHITE 15

const double SQRT2 = sqrt(2); 

void changeConsoleColor(WORD color)
{
	static WORD current_color = WHITE;
	if (current_color != color)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
		current_color = color;
	}
}

inline double heuristic(int x1, int y1, int x2, int y2) 
{
	int xDiff = x1 - x2;
	int yDiff = y1 - y2;
	//return sqrt(xDiff * xDiff + yDiff * yDiff); //Euclidean distance
	return abs(xDiff) + abs(yDiff); //Manhattan distance
	//return max(abs(xDiff) , abs(yDiff)); //Chebyshev distance
}

namespace std {
	template <>
	class hash<Point2D>
	{
	public :
		size_t operator()(const Point2D &coor) const
		{
			return 51 + hash<int>()(coor.x) * 51 + hash<int>()(coor.y);
		}
	};
};

bool compare(AstarNode* left, AstarNode* right)
{
	return *left < *right;
}

deque<Node> AstarAlgorithm(const vector<vector<unsigned char>>& map, int L, int W, const Node& s, const Node& g)
{
	typedef rp_heap<AstarNode*, decltype(&compare)>::const_iterator iterator;
	unordered_map<Point2D, iterator> open_set, closed_set;
	rp_heap<AstarNode*, decltype(&compare)> heap(&compare);
	deque<Node> resultPath;
	deque<AstarNode> node_list;
	node_list.emplace_back(s.x, s.y, 0, heuristic(s.x, s.y, g.x, g.y));
	open_set[s] = heap.push(&node_list.back());
	int c[8] = {0, 1, 0, -1, -1, 1, 1, -1};
	int d[8] = {-1, 0, 1, 0, -1, -1, 1, 1};
	while (!open_set.empty()) 
	{
		AstarNode* min_node;
		heap.pop(min_node);
		if (*min_node == g)
		{
			cout << "Total distance: " << min_node->g << '\n';
			Node* curr = min_node;
			do
			{
				resultPath.push_front(*curr);
				curr = curr->prev;
			} while(curr);
			heap.clear();
			return resultPath;
		}
		closed_set.insert(std::make_pair(*min_node, open_set[*min_node]));
		open_set.erase(*min_node);
		for (int i = 0; i < 8; i++) 
		{
			int x = min_node->x + c[i];
			int y = min_node->y + d[i];
			Point2D node(x, y);
			if (y >= 0 && y < W && x >= 0 && x < L && map[y][x] == 0) 
			{
				if ((c[i] & d[i]) && (map[y][min_node->x] == 1 || map[min_node->y][x] == 1))
					continue;
				if (closed_set.find(node) != closed_set.end())
					continue;
				else
				{
					double g_score = min_node->g + ((c[i] & d[i]) == 0 ? 1 : SQRT2);
					if (open_set.find(node) != open_set.end()) 
					{
						AstarNode* neighbor = *open_set[node];
						if (g_score < neighbor->g) 
						{
							neighbor->prev = min_node;
							neighbor->g = g_score;
							neighbor->f = g_score + neighbor->h;
							heap.decrease(open_set[node], neighbor);
						}
					}
					else 
					{
						node_list.emplace_back(x, y, g_score, heuristic(x, y, g.x, g.y), min_node);
						open_set.insert(make_pair(node, heap.push(&node_list.back())));
					}
				}
			}
		}
	}
	return resultPath;
}

deque<Node> shortestPathBFS(const vector<vector<unsigned char>>& map, int L, int W, const Node& s, const Node& g) 
{
	int ax = s.x;
	int ay = s.y; 
	int bx = g.x; 
	int by = g.y;
	deque<Node> resultPath;
	deque<deque<bool> > visited = deque< deque<bool> >(W, deque<bool>(L, false));
	deque<Node> q;
	int c[4] = {0, 0, 1, -1};
	int d[4] = {1, -1, 0, 0};
	int head = 0;
	int tail = 0;
	q.push_back(s);
	visited[ay][ax] = true;
	while (head <= tail) 
	{
		for (int i = 0; i < 4; i++)
		{
			int x = q[head].x + c[i];
			int y = q[head].y + d[i];
			if (y >= 0 && y < W && x >=0 && x < L && map[y][x] == 0 && visited[y][x] == false) 
			{
				tail++;
				visited[y][x] = true;
				q.push_back(Node(x, y, &q[head]));
				if (x == bx && y == by) {
					int count = 0;
					Node* curr = &q[tail];
					Node* prev;
					do
					{
						resultPath.push_front(Node(curr->x, curr->y));
						prev = curr->prev;
						count++;
						curr = prev;
					} while(curr != nullptr);
					return resultPath;
				}
			}
		}
		head++;
	}
	return resultPath;
}

int main () 
{
	ifstream file("102_000_00033.bin", ios::in|ios::binary);
	if (file.is_open())
	{
		streampos size = file.tellg();
		file.seekg(0, ios::beg);
		unsigned char length = 0;
		unsigned char width = 0;
		file.read((char*)&length, 1);
		file.read((char*)&width, 1);
		vector< vector<unsigned char> > map(width, vector<unsigned char>(length));;
		for (int y = 0; y < width; y++) 
			for (int x = 0; x < length; x++)
				file.read((char*)&map[y][x], 1); // 1 byte
		Node s(62, 146);
		Node g(100, 31 + 19);
		clock_t startTime = clock();
		auto resultPath = AstarAlgorithm(map, length, width, s, g);
		clock_t endTime = clock();
		cout << "Runtime: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << '\n';
		for (unsigned int i = 0; i < resultPath.size(); i++)
			for (auto it = resultPath.begin(); it != resultPath.end(); it++)
				map[it->y][it->x] = 2;
		changeConsoleColor(WHITE);
		for (int y = 0; y < width; y++) 
		{
			for (int x = 0; x < length; x++) 
			{
				changeConsoleColor(WHITE);
				if (map[y][x] == 0)
				{
					cout << "□";
				}
				else if (map[y][x] == 1)
				{
					cout << "■";
				}
				else if (map[y][x] == 2) 
				{
					changeConsoleColor(BLUE);
					cout << "■";
				} 
			}
			cout << '\n';
		}
	}
	else 
		cout << "Unable to open file";
	system("pause");
	return 0;
}

