#include <iostream>
#include <string>
using namespace std;

struct Node {
    string name; //name of property
    int cost;    //cost of property
    string owner;   // who owns it "" means unowned
    Node* next;    //next pointer

    Node(string n, int c) : name(n), cost(c), owner(""), next(nullptr) {}  //constructor
};

class MonopolyBoard {
private:
    Node* tail;     // last node, private so it doesnt change
    int size;       //size of the board should not change

public:
    MonopolyBoard() : tail(nullptr), size(0) {} //constructor for the board
    ~MonopolyBoard(){};                           // frees every node, destructor

    void insert(string name, int cost){
        Node* newNode = new Node(name, cost);
        if (tail == nullptr) {
            tail = newNode;
            tail->next = tail; // circular link, points to itself
        } else {
            newNode->next = tail->next;  //makes new node point to the first node
            tail->next = newNode;        //makes the old last point to the new last
            tail = newNode; // update tail to the new node
        }
        size++;
    }         // add at end of board


    Node* search(string name);                  // nullptr if not found

    bool remove(string name);                   // true if removed

    void display(){
        if (tail == nullptr) {
        cout << "Board is empty" << endl;
        return;
    }
        Node* current = tail->next;   // start at the first square
        int position = 0;
        
         do {
        cout << position << ": " << current->name
             << " ($" << current->cost << ") - ";

        if (current->owner.empty()) {
            cout << "unowned";
        } else {
            cout << "owned by " << current->owner;
        }
        cout << endl;

        current = current->next;  // step to the next square
        position++;
      } while (current != tail->next);

    }                            // full traversal


    Node* move(Node* start, int steps);         // advance + wrap
    bool purchase(Node* space, string player);  // fails if already owned
    Node* getStart() { return tail ? tail->next : nullptr; }
};

int main(){
    MonopolyBoard board;
board.display();                 // should say "Board is empty"
board.insert("GO", 0);
board.insert("Baltic", 60);
board.insert("Reading RR", 200);
board.display();
    return 0;
}