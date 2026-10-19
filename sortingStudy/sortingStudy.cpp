#include <iostream>
#include <vector>
#include <utility>
#include <chrono>
#include <cstdlib>
#include <iomanip>
using namespace std;


// Required signature: checks if the vector is sorted in non-decreasing order
bool isSorted(const vector<int>& values){
    for (int i = 0; i < (int)values.size() - 1; i++){
        if (values[i] > values[i + 1]){
            return false;
        }
    }
    return true;
}


// bubblesort
// Worst case O(n^2) on reverse-sorted input every pass 
void bubbleSort(vector<int>& values){
    for (int i = 0; i < (int)values.size() - 1; i++){
        for (int j = 1; j < (int)values.size() - i; j++){
            if (values[j] < values[j - 1]){
                swap(values[j], values[j - 1]);
            }
        }
    }
}


// selectionsort
// Always O(n^2) comparisons regardless of input order
void selectionSort(vector<int>& values){
    for (int i = 0; i < (int)values.size() - 1; i++){
        int minIndex = i;
        for (int j = i + 1; j < (int)values.size(); j++){
            if (values[j] < values[minIndex]){
                minIndex = j;
            }
        }
        swap(values[i], values[minIndex]);
    }
}


// insertionsort
// Best case O(n) on already-sorted input. Worst case O(n^2) on reverse-sorted input
void insertionSort(vector<int>& values){
    for (int i = 1; i < (int)values.size(); i++){
        int key = values[i];
        int j = i - 1;
        while (j >= 0 && values[j] > key){
            values[j + 1] = values[j];
            j--;
        }
        values[j + 1] = key;
    }
}


// quicksort recursive function
// Uses the middle element as the pivot. Picking the middle element means
// sorted/reverse-sorted data partitions roughly in half, the time complexity is O(n log(n)).
void quickSort(vector<int>& values, int low, int high){
    if (low < high){
        int mid = low + (high - low) / 2;
        swap(values[mid], values[high]);   // move the middle element to the end so it becomes the pivot
        int pivot = values[high];
        int i = low - 1;
        for (int j = low; j < high; j++){
            if (values[j] < pivot){
                i++;
                swap(values[i], values[j]);
            }
        }
        swap(values[i + 1], values[high]);
        int pi = i + 1;
        quickSort(values, low, pi - 1);
        quickSort(values, pi + 1, high);
    }
}


// ---------------------------------------------------------------------
// Input generators
// ---------------------------------------------------------------------

// Builds a vector of size n with values in random order.
vector<int> makeRandom(int n){
    vector<int> v(n);
    for (int i = 0; i < n; i++){
        v[i] = rand() % 1000000 + 1;
    }
    return v;
}

// Builds an already-sorted vector of size n: 0, 1, 2, ... n-1
vector<int> makeSorted(int n){
    vector<int> v(n);
    for (int i = 0; i < n; i++){
        v[i] = i;
    }
    return v;
}

// Builds a reverse-sorted vector of size n: n, n-1, ... 1
vector<int> makeReverseSorted(int n){
    vector<int> v(n);
    for (int i = 0; i < n; i++){
        v[i] = n - i;
    }
    return v;
}


int main(){

    srand(42);   // fixed seed 

    int sizes[3] = {500, 2000, 5000};
    string labels[3] = {"Small", "Medium", "Large"};

    //Formatting
    for (int s = 0; s < 3; s++){
        int n = sizes[s];

        cout << "=== " << labels[s] << " Size (n = " << n << ") ===" << endl;
        cout << left
             << setw(16) << "Algorithm"
             << setw(15) << "Random"
             << setw(15) << "Sorted"
             << setw(15) << "Reverse" << endl;
        cout << string(61, '-') << endl;

        // --- Bubble Sort ---
        vector<int> bubbleRandom = makeRandom(n);
        auto startBR = chrono::high_resolution_clock::now();
        bubbleSort(bubbleRandom);
        auto endBR = chrono::high_resolution_clock::now();
        double bubbleRandomTime = chrono::duration<double, milli>(endBR - startBR).count();
        if (!isSorted(bubbleRandom)) cout << "ERROR: Bubble Sort failed on random data!" << endl;

        vector<int> bubbleSorted = makeSorted(n);
        auto startBS = chrono::high_resolution_clock::now();
        bubbleSort(bubbleSorted);
        auto endBS = chrono::high_resolution_clock::now();
        double bubbleSortedTime = chrono::duration<double, milli>(endBS - startBS).count();
        if (!isSorted(bubbleSorted)) cout << "ERROR: Bubble Sort failed on sorted data!" << endl;

        vector<int> bubbleReverse = makeReverseSorted(n);
        auto startBRv = chrono::high_resolution_clock::now();
        bubbleSort(bubbleReverse);
        auto endBRv = chrono::high_resolution_clock::now();
        double bubbleReverseTime = chrono::duration<double, milli>(endBRv - startBRv).count();
        if (!isSorted(bubbleReverse)) cout << "ERROR: Bubble Sort failed on reverse-sorted data!" << endl;

        cout << left << setw(16) << "Bubble Sort"
             << fixed << setprecision(3)
             << setw(15) << bubbleRandomTime
             << setw(15) << bubbleSortedTime
             << setw(15) << bubbleReverseTime << endl;


             
        // --- Selection Sort ---
        vector<int> selectionRandom = makeRandom(n);
        auto startSR = chrono::high_resolution_clock::now();
        selectionSort(selectionRandom);
        auto endSR = chrono::high_resolution_clock::now();
        double selectionRandomTime = chrono::duration<double, milli>(endSR - startSR).count();
        if (!isSorted(selectionRandom)) cout << "ERROR: Selection Sort failed on random data!" << endl;

        vector<int> selectionSorted = makeSorted(n);
        auto startSS = chrono::high_resolution_clock::now();
        selectionSort(selectionSorted);
        auto endSS = chrono::high_resolution_clock::now();
        double selectionSortedTime = chrono::duration<double, milli>(endSS - startSS).count();
        if (!isSorted(selectionSorted)) cout << "ERROR: Selection Sort failed on sorted data!" << endl;

        vector<int> selectionReverse = makeReverseSorted(n);
        auto startSRv = chrono::high_resolution_clock::now();
        selectionSort(selectionReverse);
        auto endSRv = chrono::high_resolution_clock::now();
        double selectionReverseTime = chrono::duration<double, milli>(endSRv - startSRv).count();
        if (!isSorted(selectionReverse)) cout << "ERROR: Selection Sort failed on reverse-sorted data!" << endl;

        cout << left << setw(16) << "Selection Sort"
             << fixed << setprecision(3)
             << setw(15) << selectionRandomTime
             << setw(15) << selectionSortedTime
             << setw(15) << selectionReverseTime << endl;

        // --- Insertion Sort ---
        vector<int> insertionRandom = makeRandom(n);
        auto startIR = chrono::high_resolution_clock::now();
        insertionSort(insertionRandom);
        auto endIR = chrono::high_resolution_clock::now();
        double insertionRandomTime = chrono::duration<double, milli>(endIR - startIR).count();
        if (!isSorted(insertionRandom)) cout << "ERROR: Insertion Sort failed on random data!" << endl;

        vector<int> insertionSorted = makeSorted(n);
        auto startIS = chrono::high_resolution_clock::now();
        insertionSort(insertionSorted);
        auto endIS = chrono::high_resolution_clock::now();
        double insertionSortedTime = chrono::duration<double, milli>(endIS - startIS).count();
        if (!isSorted(insertionSorted)) cout << "ERROR: Insertion Sort failed on sorted data!" << endl;

        vector<int> insertionReverse = makeReverseSorted(n);
        auto startIRv = chrono::high_resolution_clock::now();
        insertionSort(insertionReverse);
        auto endIRv = chrono::high_resolution_clock::now();
        double insertionReverseTime = chrono::duration<double, milli>(endIRv - startIRv).count();
        if (!isSorted(insertionReverse)) cout << "ERROR: Insertion Sort failed on reverse-sorted data!" << endl;

        cout << left << setw(16) << "Insertion Sort"
             << fixed << setprecision(3)
             << setw(15) << insertionRandomTime
             << setw(15) << insertionSortedTime
             << setw(15) << insertionReverseTime << endl;

        // --- Quick Sort ---
        vector<int> quickRandom = makeRandom(n);
        auto startQR = chrono::high_resolution_clock::now();
        quickSort(quickRandom, 0, (int)quickRandom.size() - 1);
        auto endQR = chrono::high_resolution_clock::now();
        double quickRandomTime = chrono::duration<double, milli>(endQR - startQR).count();
        if (!isSorted(quickRandom)) cout << "ERROR: Quick Sort failed on random data!" << endl;

        vector<int> quickSorted = makeSorted(n);
        auto startQS = chrono::high_resolution_clock::now();
        quickSort(quickSorted, 0, (int)quickSorted.size() - 1);
        auto endQS = chrono::high_resolution_clock::now();
        double quickSortedTime = chrono::duration<double, milli>(endQS - startQS).count();
        if (!isSorted(quickSorted)) cout << "ERROR: Quick Sort failed on sorted data!" << endl;

        vector<int> quickReverse = makeReverseSorted(n);
        auto startQRv = chrono::high_resolution_clock::now();
        quickSort(quickReverse, 0, (int)quickReverse.size() - 1);
        auto endQRv = chrono::high_resolution_clock::now();
        double quickReverseTime = chrono::duration<double, milli>(endQRv - startQRv).count();
        if (!isSorted(quickReverse)) cout << "ERROR: Quick Sort failed on reverse-sorted data!" << endl;

        cout << left << setw(16) << "Quick Sort"
             << fixed << setprecision(3)
             << setw(15) << quickRandomTime
             << setw(15) << quickSortedTime
             << setw(15) << quickReverseTime << endl;

        cout << endl;
    }

    return 0;
}