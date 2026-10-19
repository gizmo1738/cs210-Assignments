#include <iostream>
#include <stdexcept>
using namespace std;

// ===== Stacks =====
class ArrayStack {
    private:
        int* data;
        int capacity;
        int count;

        //grows the stacks capacity when it is full, O(n)
        void grow(){
            int newCapacity = capacity * 2;
            int* newData = new int[newCapacity];
            for (int i = 0; i < count; i++) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity = newCapacity;
            cout << "  (resized to capacity " << capacity << ")\n";

        }
    public:
        //constructor
        ArrayStack(int initialCap = 2)
        : data(new int[initialCap]), capacity(initialCap), count(0) {}

        //destructor
        ~ArrayStack() { delete[] data; }



        void push(int x) {                 // O(1) amortized, O(n) when it grows
        if (count == capacity) grow();
        data[count] = x;
        count++;
    }

    int pop() {                        // O(1)
        if (isEmpty()) throw out_of_range("pop from empty stack");
        count--;
        return data[count];
    }

    int top() const {                  // O(1)
        if (isEmpty()) throw out_of_range("top of empty stack");
        return data[count - 1];
    }

    bool isEmpty() const { return count == 0; }
    int size() const { 
        return count; }
};


// ===== Linked Stack =====
class LinkedStack {
private:
    struct Node {
        int value;
        Node* next;
    };

    Node* head;   // top of the stack
    int count;

public:
    LinkedStack() : head(nullptr), count(0) {}

    ~LinkedStack() {
        while (!isEmpty()) {
            pop();                 // pop deletes each node
        }
    }

    LinkedStack(const LinkedStack&) = delete;
    LinkedStack& operator=(const LinkedStack&) = delete;

    void push(int x) {             // O(1), always
        head = new Node{x, head};  // new node points at old top
        count++;
    }

    int pop() {                    // O(1)
        if (isEmpty()) throw out_of_range("pop from empty stack");
        Node* oldTop = head;
        int val = oldTop->value;
        head = head->next;         // move top down one
        delete oldTop;             // free the removed node
        count--;
        return val;
    }

    int top() const {              // O(1)
        if (isEmpty()) throw out_of_range("top of empty stack");
        return head->value;
    }

    bool isEmpty() const { return head == nullptr; }
    int size() const { return count; }
};

// ===== Array Queue (circular buffer) =====
class ArrayQueue {
private:
    int* data;
    int capacity;
    int frontIdx;   // index of the front item
    int count;      // number of items

    // O(n): copy items IN QUEUE ORDER into a bigger array
    void grow() {
        int newCap = capacity * 2;
        int* bigger = new int[newCap];
        for (int i = 0; i < count; i++) {
            bigger[i] = data[(frontIdx + i) % capacity];
        }
        delete[] data;
        data = bigger;
        capacity = newCap;
        frontIdx = 0;   // front is now at the start of the new array
        cout << "  (resized to capacity " << capacity << ")\n";
    }

public:
    ArrayQueue(int initialCap = 4)
        : data(new int[initialCap]), capacity(initialCap), frontIdx(0), count(0) {}

    ~ArrayQueue() { delete[] data; }

    ArrayQueue(const ArrayQueue&) = delete;
    ArrayQueue& operator=(const ArrayQueue&) = delete;

    void enqueue(int x) {              // O(1) amortized, O(n) on resize
        if (count == capacity) grow();
        int backIdx = (frontIdx + count) % capacity;
        data[backIdx] = x;
        count++;
    }

    int dequeue() {                    // O(1)
        if (isEmpty()) throw out_of_range("dequeue from empty queue");
        int val = data[frontIdx];
        frontIdx = (frontIdx + 1) % capacity;
        count--;
        return val;
    }

    int front() const {                // O(1)
        if (isEmpty()) throw out_of_range("front of empty queue");
        return data[frontIdx];
    }

    bool isEmpty() const { return count == 0; }
    int size() const { return count; }
};


// ===== Linked Queue =====
class LinkedQueue {
private:
    struct Node {
        int value;
        Node* next;
    };

    Node* head;   // front: dequeue here
    Node* tail;   // back: enqueue here
    int count;

public:
    LinkedQueue() : head(nullptr), tail(nullptr), count(0) {}

    ~LinkedQueue() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    LinkedQueue(const LinkedQueue&) = delete;
    LinkedQueue& operator=(const LinkedQueue&) = delete;

    void enqueue(int x) {              // O(1)
        Node* n = new Node{x, nullptr};  // new back node points to nothing
        if (isEmpty()) {
            head = n;                  // only node is both front and back
        } else {
            tail->next = n;            // link old back to new node
        }
        tail = n;
        count++;
    }

    int dequeue() {                    // O(1)
        if (isEmpty()) throw out_of_range("dequeue from empty queue");
        Node* oldFront = head;
        int val = oldFront->value;
        head = head->next;
        if (head == nullptr) {
            tail = nullptr;            // queue is now empty
        }
        delete oldFront;
        count--;
        return val;
    }

    int front() const {                // O(1)
        if (isEmpty()) throw out_of_range("front of empty queue");
        return head->value;
    }

    bool isEmpty() const { return head == nullptr; }
    int size() const { return count; }
};

// ===== Array Deque (circular buffer) =====
class ArrayDeque {
private:
    int* data;
    int capacity;
    int frontIdx;
    int count;

    // O(n): same unwrap as the array queue
    void grow() {
        int newCap = capacity * 2;
        int* bigger = new int[newCap];
        for (int i = 0; i < count; i++) {
            bigger[i] = data[(frontIdx + i) % capacity];
        }
        delete[] data;
        data = bigger;
        capacity = newCap;
        frontIdx = 0;
        cout << "  (resized to capacity " << capacity << ")\n";
    }

    int backIdx() const {              // index of the last item
        return (frontIdx + count - 1) % capacity;
    }

public:
    ArrayDeque(int initialCap = 4)
        : data(new int[initialCap]), capacity(initialCap), frontIdx(0), count(0) {}

    ~ArrayDeque() { delete[] data; }

    ArrayDeque(const ArrayDeque&) = delete;
    ArrayDeque& operator=(const ArrayDeque&) = delete;

    void pushFront(int x) {            // O(1) amortized, O(n) on resize
        if (count == capacity) grow();
        frontIdx = (frontIdx - 1 + capacity) % capacity;   // step back, wrapping
        data[frontIdx] = x;
        count++;
    }

    void pushBack(int x) {             // O(1) amortized, O(n) on resize
        if (count == capacity) grow();
        data[(frontIdx + count) % capacity] = x;
        count++;
    }

    int popFront() {                   // O(1)
        if (isEmpty()) throw out_of_range("popFront from empty deque");
        int val = data[frontIdx];
        frontIdx = (frontIdx + 1) % capacity;
        count--;
        return val;
    }

    int popBack() {                    // O(1)
        if (isEmpty()) throw out_of_range("popBack from empty deque");
        int val = data[backIdx()];
        count--;                       // frontIdx doesn't move
        return val;
    }

    int front() const {
        if (isEmpty()) throw out_of_range("front of empty deque");
        return data[frontIdx];
    }

    int back() const {
        if (isEmpty()) throw out_of_range("back of empty deque");
        return data[backIdx()];
    }

    bool isEmpty() const { return count == 0; }
    int size() const { return count; }
};

// ===== Linked Deque (doubly linked) =====
class LinkedDeque {
private:
    struct Node {
        int value;
        Node* prev;
        Node* next;
    };

    Node* head;
    Node* tail;
    int count;

public:
    LinkedDeque() : head(nullptr), tail(nullptr), count(0) {}

    ~LinkedDeque() {
        while (!isEmpty()) {
            popFront();
        }
    }

    LinkedDeque(const LinkedDeque&) = delete;
    LinkedDeque& operator=(const LinkedDeque&) = delete;

    void pushFront(int x) {            // O(1)
        Node* n = new Node{x, nullptr, head};   // prev = none, next = old head
        if (isEmpty()) {
            tail = n;
        } else {
            head->prev = n;            // old head points back to new node
        }
        head = n;
        count++;
    }

    void pushBack(int x) {             // O(1)
        Node* n = new Node{x, tail, nullptr};   // prev = old tail, next = none
        if (isEmpty()) {
            head = n;
        } else {
            tail->next = n;            // old tail points forward to new node
        }
        tail = n;
        count++;
    }

    int popFront() {                   // O(1)
        if (isEmpty()) throw out_of_range("popFront from empty deque");
        Node* old = head;
        int val = old->value;
        head = head->next;
        if (head == nullptr) {
            tail = nullptr;            // deque is now empty
        } else {
            head->prev = nullptr;      // new head has nothing before it
        }
        delete old;
        count--;
        return val;
    }

    int popBack() {                    // O(1), thanks to prev
        if (isEmpty()) throw out_of_range("popBack from empty deque");
        Node* old = tail;
        int val = old->value;
        tail = tail->prev;
        if (tail == nullptr) {
            head = nullptr;            // deque is now empty
        } else {
            tail->next = nullptr;      // new tail has nothing after it
        }
        delete old;
        count--;
        return val;
    }

    int front() const {
        if (isEmpty()) throw out_of_range("front of empty deque");
        return head->value;
    }

    int back() const {
        if (isEmpty()) throw out_of_range("back of empty deque");
        return tail->value;
    }

    bool isEmpty() const { return head == nullptr; }
    int size() const { return count; }
};


// ===== Demos =====

void demoArrayStack() {
    cout << "=== Array Stack ===\n";
    ArrayStack s;   // capacity 2, so resizing shows up

    try { s.pop(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }

    for (int i = 1; i <= 5; i++) {
        s.push(i * 10);
        cout << "push " << i * 10 << " -> top = " << s.top()
             << ", size = " << s.size() << "\n";
    }
    while (!s.isEmpty()) {
        cout << "pop -> " << s.pop() << "\n";
    }

    try { s.top(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }
}

void demoLinkedStack() {
    cout << "\n=== Linked Stack ===\n";
    LinkedStack s;

    try { s.pop(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }

    for (int i = 1; i <= 5; i++) {
        s.push(i * 10);
        cout << "push " << i * 10 << " -> top = " << s.top()
             << ", size = " << s.size() << "\n";
    }
    while (!s.isEmpty()) {
        cout << "pop -> " << s.pop() << "\n";
    }

    try { s.top(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }
}

void demoArrayQueue() {
    cout << "\n=== Array Queue ===\n";
    ArrayQueue q;   // capacity 4

    try { q.dequeue(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }

    for (int i = 1; i <= 3; i++) {        // enqueue 10, 20, 30
        q.enqueue(i * 10);
        cout << "enqueue " << i * 10 << " -> front = " << q.front()
             << ", size = " << q.size() << "\n";
    }
    cout << "dequeue -> " << q.dequeue() << "\n";   // 10
    cout << "dequeue -> " << q.dequeue() << "\n";   // 20

    for (int i = 4; i <= 6; i++) {        // enqueue 40, 50, 60 (wraps around)
        q.enqueue(i * 10);
        cout << "enqueue " << i * 10 << " -> front = " << q.front()
             << ", size = " << q.size() << "\n";
    }

    q.enqueue(70);                        // array is full, triggers grow()
    cout << "enqueue 70 -> front = " << q.front()
         << ", size = " << q.size() << "\n";

    while (!q.isEmpty()) {
        cout << "dequeue -> " << q.dequeue() << "\n";   // 30 40 50 60 70
    }

    try { q.front(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }
}

void demoLinkedQueue() {
    cout << "\n=== Linked Queue ===\n";
    LinkedQueue q;

    try { q.dequeue(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }

    for (int i = 1; i <= 5; i++) {        // enqueue 10..50
        q.enqueue(i * 10);
        cout << "enqueue " << i * 10 << " -> front = " << q.front()
             << ", size = " << q.size() << "\n";
    }
    cout << "dequeue -> " << q.dequeue() << "\n";   // 10
    cout << "dequeue -> " << q.dequeue() << "\n";   // 20

    q.enqueue(60);
    cout << "enqueue 60 -> front = " << q.front()
         << ", size = " << q.size() << "\n";

    while (!q.isEmpty()) {
        cout << "dequeue -> " << q.dequeue() << "\n";   // 30 40 50 60
    }

    // Reuse after emptying: tests the tail reset
    q.enqueue(99);
    cout << "enqueue 99 after emptying -> front = " << q.front() << "\n";
    cout << "dequeue -> " << q.dequeue() << "\n";

    try { q.front(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }
}

void demoArrayDeque() {
    cout << "\n=== Array Deque ===\n";
    ArrayDeque d;   // capacity 4

    try { d.popFront(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }

    d.pushBack(10);
    cout << "pushBack 10  -> front = " << d.front() << ", back = " << d.back() << "\n";
    d.pushBack(20);
    cout << "pushBack 20  -> front = " << d.front() << ", back = " << d.back() << "\n";
    d.pushFront(5);   // wraps backward from index 0 to 3
    cout << "pushFront 5  -> front = " << d.front() << ", back = " << d.back() << "\n";
    d.pushFront(1);   // array is now full
    cout << "pushFront 1  -> front = " << d.front() << ", back = " << d.back() << "\n";
    d.pushBack(30);   // triggers grow()
    cout << "pushBack 30  -> front = " << d.front() << ", back = " << d.back()
         << ", size = " << d.size() << "\n";

    cout << "popFront -> " << d.popFront() << "\n";   // 1
    cout << "popBack  -> " << d.popBack()  << "\n";   // 30
    cout << "popFront -> " << d.popFront() << "\n";   // 5
    cout << "popBack  -> " << d.popBack()  << "\n";   // 20
    cout << "popBack  -> " << d.popBack()  << "\n";   // 10

    try { d.back(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }
}

void demoLinkedDeque() {
    cout << "\n=== Linked Deque ===\n";
    LinkedDeque d;

    try { d.popBack(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }

    d.pushBack(10);
    cout << "pushBack 10  -> front = " << d.front() << ", back = " << d.back() << "\n";
    d.pushFront(5);
    cout << "pushFront 5  -> front = " << d.front() << ", back = " << d.back() << "\n";
    d.pushBack(20);
    cout << "pushBack 20  -> front = " << d.front() << ", back = " << d.back() << "\n";
    d.pushFront(1);
    cout << "pushFront 1  -> front = " << d.front() << ", back = " << d.back()
         << ", size = " << d.size() << "\n";

    cout << "popBack  -> " << d.popBack()  << "\n";   // 20
    cout << "popFront -> " << d.popFront() << "\n";   // 1
    cout << "popBack  -> " << d.popBack()  << "\n";   // 10
    cout << "popBack  -> " << d.popBack()  << "\n";   // 5, now empty

    // Reuse after emptying from the back: tests the head reset
    d.pushFront(99);
    cout << "pushFront 99 after emptying -> front = " << d.front()
         << ", back = " << d.back() << "\n";
    cout << "popFront -> " << d.popFront() << "\n";

    try { d.front(); }
    catch (const out_of_range& e) { cout << "Caught: " << e.what() << "\n"; }
}

int main() {
    demoArrayStack();
    demoLinkedStack();
    demoArrayQueue();
    demoLinkedQueue();
    demoArrayDeque();
    demoLinkedDeque();
    return 0;
}