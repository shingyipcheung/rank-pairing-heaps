#ifndef ASTARNODE_H
#define ASTARNODE_H

class Point2D
{
public:
	int x;
	int y;
	Point2D(int x, int y) : x(x), y(y) {}
    bool operator==(const Point2D &right) const 
	{ 
        return this->x == right.x && this->y == right.y; 
    }
    bool operator<(const Point2D &right) const 
	{
        if (x == right.x) 
            return y < right.y;
        return x < right.x;
    }
};

class Node : public Point2D
{
public:
    Node* prev;
	Node(int x, int y) : Point2D(x, y)
	{
        prev = nullptr;
    }
    Node(int x, int y, Node* prev) : Point2D(x, y)
	{
        this->prev = prev;
    }
};

class AstarNode : public Node 
{
public:
    double g; //actual cost from start
    double h; //estimated cost to goal
    double f;
    AstarNode(int x = 0, int y = 0, double g = 0, double h = 0, AstarNode* parent = nullptr) : Node(x, y, parent) 
	{
        this->g = g;
        this->h = h;
        this->f = g + h;
    }
    bool operator<(const AstarNode &right) const 
	{
        return f < right.f;
    }
};

#endif /* ASTARNODE_H */