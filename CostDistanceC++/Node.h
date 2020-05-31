#include <iostream>

class Node
{

	private:
    int mRow, mCol, mLayer;                                 //data fields
		double mcost;

  public:
    Node() {};                                      //default constructor
    Node(int r, int c,int l, double h) {									//constructor
		mRow = r; mCol = c; mLayer = l; mcost = h;
		}    
    
    int row() const { return mRow; }             //accessor methods
    int col() const { return mCol; }
	int layer() const { return mLayer; }
		double cost() const { return mcost; }

		// idxKey
		long idxKey() {
			return mRow*5+mCol*5+mLayer;
		}

		// overload the << operator so heap knows how to print the node
		friend ostream& operator<<(ostream& os, const Node& n);

		/* overload the less-than operator so heap knows how to compare two Node objects */
		bool operator<(const Node&) const;              //overloaded < operator

		/* overload the less-than operator so heap knows how to compare two Node objects */
		bool operator>(const Node&) const;              //overloaded > operator
};

ostream& operator << (ostream &os, const Node& aNode)
{
	os << "Node: row = " << aNode.row() << "; col = " << aNode.col() << "; layer = " << aNode.layer() << "; cost = " << aNode.cost();
	return os;
}

bool Node::operator<(const Node& right) const {
	return mcost < right.cost();
}

bool Node::operator>(const Node& right) const {
	return mcost > right.cost();
}