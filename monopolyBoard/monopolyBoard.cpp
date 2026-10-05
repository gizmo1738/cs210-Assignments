#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

struct Node
{
    string name;  // name of property
    int cost;     // cost of property
    string owner; // who owns it "" means unowned
    Node *next;   // next pointer

    Node(string n, int c) : name(n), cost(c), owner(""), next(nullptr) {} // constructor
};

class MonopolyBoard //circular linked list
{
private:
    Node *tail;
    int size;

public:
    MonopolyBoard() : tail(nullptr), size(0) {} // constructor for the board
    ~MonopolyBoard()
    {
        if (tail == nullptr)
        {
            return; // empty board, nothing to free
        }

        Node *current = tail->next; // start at the first square
        tail->next = nullptr;       // break the circle so the walk has an end, no longer circular

        while (current != nullptr)
        {
            Node *nextNode = current->next; // save where to go next
            delete current;                 // safe to free this one
            current = nextNode;             // step forward
        }
        tail = nullptr;
    } // frees every node, destructor

    void insert(string name, int cost)
    {
        Node *newNode = new Node(name, cost);
        if (tail == nullptr)
        {
            tail = newNode;
            tail->next = tail; // circular link, points to itself
        }
        else
        {
            newNode->next = tail->next; // makes new node point to the first node
            tail->next = newNode;       // makes the old last point to the new last
            tail = newNode;             // update tail to the new node
        }
        size++;
    } // add at end of board

    Node *search(string name)
    {
        if (tail == nullptr)
        {
            return nullptr; // empty board, nothing to find
        }

        Node *current = tail->next; // start at the first square
        do
        {
            if (current->name == name)
            {
                return current; // found it, stop here
            }
            current = current->next; // step forward
        } while (current != tail->next);

        return nullptr; // went all the way around, not found
    }

    bool remove(string name)
    {
        if (tail == nullptr)
        {
            return false; // empty board, nothing to remove
        }

        Node *prev = tail;          // square behind the first square
        Node *current = tail->next; // start at the first square

        do
        {
            if (current->name == name)
            {
                if (current == prev)
                {
                    tail = nullptr; // case 1: only square, board is now empty
                }
                else
                {
                    prev->next = current->next; // skip over current
                    if (current == tail)
                    {
                        tail = prev; // case 2: removed the last square
                    }
                }
                delete current; // free the memory
                size--;
                return true;
            }
            prev = current; // both pointers step forward together
            current = current->next;
        } while (current != tail->next);

        return false; // went all the way around, not found
    }

    void display()
    {
        if (tail == nullptr)
        {
            cout << "Board is empty" << endl;
            return;
        }
        Node *current = tail->next; // start at the first square
        int position = 0;

        do
        {
            cout << position << ": " << current->name
                 << " ($" << current->cost << ") - ";

            if (current->owner.empty())
            {
                cout << "unowned";
            }
            else
            {
                cout << "owned by " << current->owner;
            }
            cout << endl;

            current = current->next; // step to the next square
            position++;
        } while (current != tail->next);

    } // full traversal

    Node *move(Node *start, int steps)
    {
        if (start == nullptr)
        {
            return nullptr; // no square to move from
        }

        Node *current = start;
        for (int i = 0; i < steps; i++)
        {
            current = current->next; // one step forward, wraps on its own
        }
        return current; // the square the player lands on
    }

    bool purchase(Node *space, string player)
    {
        if (space == nullptr)
        {
            return false; // no square given
        }
        if (!space->owner.empty())
        {
            return false; // already owned, can't buy
        }
        space->owner = player;
        return true;
    }

    Node *getStart()
    {
        if (tail != nullptr)
        {
            return tail->next; // the first square (GO)
        }
        else
        {
            return nullptr; // empty board, no first square
        }
    }
};

struct Player
{
    string name;
    Node *position;
    int money;
};

int main()
{
    srand(42); // fixed seed for rand

    MonopolyBoard board;
    board.insert("GO", 0);
    board.insert("Clubhouse Drive", 60);
    board.insert("Heritage Road", 60);
    board.insert("The Kebab Shop", 200);
    board.insert("Chipotle", 100);
    board.insert("Panda Express", 100);
    board.insert("St. Charles Place", 140);
    board.insert("Electric Company", 150);
    board.insert("Tacos el Gordo", 140);
    board.insert("Petco Park", 350);
    board.insert("Boardwalk", 400);

    cout << "=== Starting board ===" << endl;
    board.display();

    Player players[2] = {
        {"Ismael", board.getStart(), 1500},
        {"Sofia", board.getStart(), 1500}};

    const int TURNS = 12;
    for (int turn = 1; turn <= TURNS; turn++)
    {
        Player &p = players[(turn - 1) % 2]; // alternate players
        int roll = rand() % 6 + 1;           // dice roll 1-6

        p.position = board.move(p.position, roll);
        Node *space = p.position;

        cout << "\nTurn " << turn << ": " << p.name << " rolls " << roll
             << " and lands on " << space->name << endl;

        if (space->cost == 0)
        {
            cout << "  Nothing to buy here." << endl;
        }
        else if (space->owner == p.name)
        {
            cout << "  You already own this." << endl;
        }
        else if (!space->owner.empty())
        {
            cout << "  Already owned by " << space->owner << " - can't buy." << endl;
        }
        else if (p.money < space->cost)
        {
            cout << "  Can't afford it ($" << p.money << " left)." << endl;
        }
        else if (board.purchase(space, p.name))
        {
            p.money -= space->cost;
            cout << "  Bought it for $" << space->cost
                 << ". Money left: $" << p.money << endl;
        }
    }

    cout << "\n=== Final board ===" << endl;
    board.display();

    cout << "\n=== Final players ===" << endl;
    for (int i = 0; i < 2; i++)
    {
        cout << players[i].name << " is on " << players[i].position->name
             << " with $" << players[i].money << endl;
    }

    return 0;
}