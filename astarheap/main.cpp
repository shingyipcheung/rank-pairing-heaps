#include <iostream>
#include <fstream>
#include <deque> // hold the result path
#include <chrono>
#include <unordered_map> // hash map coordinate to iterator

#define TYPE1_RANK_REDUCTION

#include "rp_heap.h"
#include "AstarNode.h"
#include <cmath>


const double SQRT2 = std::sqrt(2.0);


// Custom hash function for Point2D.
struct Point2DHash {
    size_t operator()(const Point2D &point) const {
        return 51 + std::hash<int>()(point.x) * 51 + std::hash<int>()(point.y);
    }
};

inline double heuristic(int x1, int y1, int x2, int y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2); // Manhattan distance
    // Other distance metrics can be swapped in here as needed
}

bool compare(const AstarNode *left, const AstarNode *right) {
    return *left < *right; // Assuming AstarNode has an overloaded < operator
}

std::deque<Node> shortest_path_a_star(const std::vector<std::vector<unsigned char>> &map, int L, int W, const Node &s, const Node &g) {
    typedef rp_heaps<AstarNode *, decltype(&compare)>::const_iterator iterator;
    std::unordered_map<Point2D, iterator, Point2DHash> open_set, closed_set;
    rp_heaps<AstarNode *, decltype(&compare)> heap(&compare);
    std::deque<Node> result_path;
    std::deque<AstarNode> node_list;

    node_list.emplace_back(s.x, s.y, 0, heuristic(s.x, s.y, g.x, g.y));
    open_set[s] = heap.push(&node_list.back());

    const int DIRECTIONS = 8;
    const int dx[DIRECTIONS] = {0, 1, 0, -1, -1, 1, 1, -1};
    const int dy[DIRECTIONS] = {-1, 0, 1, 0, -1, -1, 1, 1};

    while (!open_set.empty()) {
        AstarNode *current_node;
        heap.pop(current_node);

        if (*current_node == g) {
            std::cout << "Total distance: " << current_node->g << '\n';
            Node *curr = current_node;
            while (curr) {
                result_path.push_front(*curr);
                curr = curr->prev;
            }
            heap.clear();
            return result_path;
        }

        closed_set[*current_node] = open_set[*current_node];
        open_set.erase(*current_node);

        for (int i = 0; i < DIRECTIONS; ++i) {
            int next_x = current_node->x + dx[i];
            int next_y = current_node->y + dy[i];
            Point2D neighbor_point(next_x, next_y);

            if (next_y >= 0 && next_y < W && next_x >= 0 && next_x < L && map[next_y][next_x] == 0) {
                /*
                 *if the current move being checked is a diagonal one, and there is an obstacle in the path of the diagonal move,
                 then continue
                 */
                if ((dx[i] & dy[i]) && (map[next_y][current_node->x] == 1 || map[current_node->y][next_x] == 1)) {
                    continue;
                }
                if (closed_set.find(neighbor_point) != closed_set.end()) {
                    continue;
                }

                double g_score = current_node->g + ((dx[i] & dy[i]) == 0 ? 1 : SQRT2);

                if (open_set.find(neighbor_point) != open_set.end()) {
                    AstarNode *neighbor = *open_set[neighbor_point];
                    if (g_score < neighbor->g) {
                        neighbor->prev = current_node;
                        neighbor->g = g_score;
                        neighbor->f = g_score + neighbor->h;
                        heap.decrease(open_set[neighbor_point], neighbor);
                    }
                } else {
                    node_list.emplace_back(next_x, next_y, g_score, heuristic(next_x, next_y, g.x, g.y), current_node);
                    open_set[neighbor_point] = heap.push(&node_list.back());
                }
            }
        }
    }
    return result_path;
}

std::deque<Node> shortest_path_bfs(const std::vector<std::vector<unsigned char>> &map, int L, int W, const Node &s, const Node &g) {
    int ax = s.x;
    int ay = s.y;
    int bx = g.x;
    int by = g.y;
    std::deque<Node> resultPath;
    std::vector<std::vector<bool> > visited = std::vector<std::vector<bool> >(W, std::vector<bool>(L, false));
    std::deque<Node> q;

    const int DIRECTIONS = 8;
    const int dx[DIRECTIONS] = {0, 0, 1, -1};
    const int dy[DIRECTIONS] = {1, -1, 0, 0};
    int head = 0;
    int tail = 0;
    q.push_back(s);
    visited[ay][ax] = true;
    while (head <= tail) {
        for (int i = 0; i < DIRECTIONS; i++) {
            int x = q[head].x + dx[i];
            int y = q[head].y + dy[i];
            if (y >= 0 && y < W && x >= 0 && x < L && map[y][x] == 0 && !visited[y][x]) {
                tail++;
                visited[y][x] = true;
                q.emplace_back(x, y, &q[head]);
                if (x == bx && y == by) {
                    int count = 0;
                    Node *curr = &q[tail];
                    Node *prev;
                    do {
                        resultPath.push_front(Node(curr->x, curr->y));
                        prev = curr->prev;
                        count++;
                        curr = prev;
                    } while (curr != nullptr);
                    return resultPath;
                }
            }
        }
        head++;
    }
    return resultPath;
}


int main() {
    const std::string file_path = "./map/102_000_00033.bin";
    std::ifstream file(file_path, std::ios::in | std::ios::binary);

    if (!file) {
        std::cerr << "Unable to open file: " << file_path << std::endl;
        return 1;
    }

    // Read map dimensions
    unsigned char length = 0;
    unsigned char width = 0;
    file.read((char *) (&length), sizeof(length));
    file.read((char *) (&width), sizeof(width));

    if (!file) {
        std::cerr << "Error reading file dimensions." << std::endl;
        return 1;
    }

    // Read map data
    std::vector<std::vector<unsigned char>> map_data(width, std::vector<unsigned char>(length));
    for (int y = 0; y < width; ++y) {
        file.read((char *) (map_data[y].data()), length);

        if (!file) {
            std::cerr << "Error reading map data." << std::endl;
            return 1;
        }
    }

    Node start_node(62, 146);
    Node goal_node(100, 31 + 19);

    // Measure execution time for A* algorithm
    const auto start_time = std::chrono::high_resolution_clock::now();
    auto result_path = shortest_path_a_star(map_data, length, width, start_node, goal_node);
    const auto end_time = std::chrono::high_resolution_clock::now();

    const auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "It took " << elapsed_time.count() << "ms" << std::endl;

    return 0;
}

