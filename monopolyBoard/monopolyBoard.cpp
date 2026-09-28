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


    Node* search(string name){
     if (tail == nullptr) {
         return nullptr;              // empty board, nothing to find
     }

     Node* current = tail->next;      // start at the first square
     do {
        if (current->name == name) {
            return current;          // found it, stop here
        }
        current = current->next;     // step forward
        } while (current != tail->next);

        return nullptr;                  // went all the way around, not found
    }                  


    bool remove(string name){
            if (tail == nullptr) {
        return false;                    // empty board, nothing to remove
    }

        Node* prev = tail;                   // square behind the first square
        Node* current = tail->next;          // start at the first square

        do {
            if (current->name == name) {
                if (current == prev) {
                    tail = nullptr;          // case 1: only square, board is now empty
                } else {
                prev->next = current->next;  // skip over current
                if (current == tail) {
                    tail = prev;         // case 2: removed the last square
                }
            }
            delete current;              // free the memory
            size--;
            return true;
            }
            prev = current;                  // both pointers step forward together
            current = current->next;
        } while (current != tail->next);

    return false;                        // went all the way around, not found
    }                   

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


    Node* move(Node* start, int steps);        // advance + wrap
    bool purchase(Node* space, string player);  // fails if already owned
    Node* getStart() { return tail ? tail->next : nullptr; }
};

int main(){
  MonopolyBoard board;

    board.insert("GO", 0);
    board.insert("Baltic", 60);
    board.insert("Reading RR", 200);
    board.display();

    cout << "--- remove Baltic (middle) ---" << endl;
    board.remove("Baltic");
    board.display();

    cout << "--- remove Reading RR (last) ---" << endl;
    board.remove("Reading RR");
    board.display();

    cout << "--- insert Boardwalk (tests tail) ---" << endl;
    board.insert("Boardwalk", 400);
    board.display();

    cout << "--- remove Park Place (not there) ---" << endl;
    cout << (board.remove("Park Place") ? "removed" : "not found") << endl;

    cout << "--- remove GO, then Boardwalk ---" << endl;
    board.remove("GO");
    board.remove("Boardwalk");
    board.display();

    return 0;
}