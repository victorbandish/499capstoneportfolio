#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/*
========================================================
Milestone Three: Algorithms and Data Structures Enhancement
--------------------------------------------------------
This version of the course planner was enhanced to better
demonstrate algorithmic principles and data structure usage.

Key enhancements:
- Replaced linear storage with a Binary Search Tree (BST)
- Implemented BST insert, search, and in-order traversal
- Used in-order traversal to produce sorted output
- Normalized input data to ensure consistent searching
- Emphasized time/space trade-offs in data structure choice

These changes align this artifact with the Algorithms and
Data Structures category of the CS-499 ePortfolio.
========================================================
*/

// Represents a single course and its prerequisites
struct Course {
    std::string courseNumber;
    std::string title;
    std::vector<std::string> prerequisites;
};

/*
--------------------------------------------------------
String normalization helpers
--------------------------------------------------------
These functions ensure consistent comparisons during
BST insert and search operations by removing whitespace
and standardizing case.
*/
static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

static std::string toUpper(std::string s) {
    for (char& ch : s) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return s;
}

static std::string normalizeCourseNumber(const std::string& s) {
    return toUpper(trim(s));
}

/*
--------------------------------------------------------
Binary Search Tree Implementation
--------------------------------------------------------
The BST is used as the primary data structure to store
courses. This allows:
- Average O(log n) insert
- Average O(log n) search
- O(n) in-order traversal for sorted output

This directly demonstrates algorithmic reasoning and
data structure trade-offs.
*/
class CourseBST {
private:
    struct Node {
        Course data;
        Node* left;
        Node* right;

        explicit Node(const Course& c) : data(c), left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;

    // Recursively deletes nodes to prevent memory leaks
    static void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    // Recursive BST insert algorithm
    static Node* insert(Node* node, const Course& course) {
        if (!node) {
            return new Node(course);
        }

        if (course.courseNumber < node->data.courseNumber) {
            node->left = insert(node->left, course);
        }
        else if (course.courseNumber > node->data.courseNumber) {
            node->right = insert(node->right, course);
        }
        else {
            // Duplicate keys overwrite existing data
            node->data = course;
        }
        return node;
    }

    // Recursive BST search algorithm
    static const Course* search(Node* node, const std::string& key) {
        if (!node) return nullptr;

        if (key == node->data.courseNumber) {
            return &node->data;
        }
        if (key < node->data.courseNumber) {
            return search(node->left, key);
        }
        return search(node->right, key);
    }

    // In-order traversal prints courses in sorted order
    static void inOrderPrint(Node* node) {
        if (!node) return;
        inOrderPrint(node->left);
        std::cout << node->data.courseNumber << ", " << node->data.title << "\n";
        inOrderPrint(node->right);
    }

public:
    ~CourseBST() {
        destroy(root);
    }

    void clear() {
        destroy(root);
        root = nullptr;
    }

    void insert(const Course& course) {
        root = insert(root, course);
    }

    const Course* search(const std::string& courseNumber) const {
        return search(root, courseNumber);
    }

    void printInOrder() const {
        inOrderPrint(root);
    }
};

/*
--------------------------------------------------------
CSV Loading Logic
--------------------------------------------------------
Reads course data from a CSV file and inserts each course
into the BST. Data normalization ensures correct ordering
and searching within the tree.
*/
static bool loadCoursesFromCsv(const std::string& fileName, CourseBST& bstOut) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        return false;
    }

    bstOut.clear();

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string courseNumber, title;

        if (!std::getline(ss, courseNumber, ',')) continue;
        if (!std::getline(ss, title, ',')) continue;

        Course c;
        c.courseNumber = normalizeCourseNumber(courseNumber);
        c.title = trim(title);

        std::string prereq;
        while (std::getline(ss, prereq, ',')) {
            prereq = normalizeCourseNumber(prereq);
            if (!prereq.empty()) {
                c.prerequisites.push_back(prereq);
            }
        }

        bstOut.insert(c);
    }

    return true;
}

/*
--------------------------------------------------------
Course Detail Output
--------------------------------------------------------
Uses BST search to retrieve a specific course in
average O(log n) time.
*/
static void printCourseDetails(const CourseBST& bst, std::string courseNumber) {
    courseNumber = normalizeCourseNumber(courseNumber);

    const Course* c = bst.search(courseNumber);
    if (!c) {
        std::cout << "Error: Course not found\n";
        return;
    }

    std::cout << c->courseNumber << ", " << c->title << "\n";
    std::cout << "Prerequisites: ";

    if (c->prerequisites.empty()) {
        std::cout << "None\n";
        return;
    }

    for (size_t i = 0; i < c->prerequisites.size(); ++i) {
        std::cout << c->prerequisites[i];
        if (i + 1 < c->prerequisites.size()) std::cout << ", ";
    }
    std::cout << "\n";
}

/*
--------------------------------------------------------
User Interface
--------------------------------------------------------
The menu logic remains simple to keep the focus on
algorithmic behavior rather than UI complexity.
*/
static int displayMenu() {
    std::cout << "\n1. Load Data Structure.\n";
    std::cout << "2. Print Course List.\n";
    std::cout << "3. Print Course.\n";
    std::cout << "9. Exit\n";
    std::cout << "What would you like to do? ";

    std::string input;
    std::getline(std::cin, input);
    input = trim(input);

    if (input.empty()) return -1;

    try {
        return std::stoi(input);
    }
    catch (...) {
        return -1;
    }
}

int main() {
    CourseBST bst;
    bool dataLoaded = false;

    std::cout << "Welcome to the course planner.\n";

    while (true) {
        int choice = displayMenu();

        switch (choice) {
        case 1: {
            std::cout << "Enter file name: ";
            std::string filename;
            std::getline(std::cin, filename);

            if (!loadCoursesFromCsv(filename, bst)) {
                std::cout << "Error: File not found or could not be opened\n";
                dataLoaded = false;
            }
            else {
                std::cout << "Data loaded successfully.\n";
                dataLoaded = true;
            }
            break;
        }
        case 2:
            if (!dataLoaded) {
                std::cout << "Please load data first using option 1.\n";
                break;
            }
            std::cout << "Here is a sample schedule:\n";
            bst.printInOrder();  // In-order traversal guarantees sorted output
            break;

        case 3: {
            if (!dataLoaded) {
                std::cout << "Please load data first using option 1.\n";
                break;
            }
            std::cout << "What course do you want to know about? ";
            std::string courseNumber;
            std::getline(std::cin, courseNumber);
            printCourseDetails(bst, courseNumber);
            break;
        }
        case 9:
            std::cout << "Thank you for using the course planner!\n";
            return 0;

        default:
            std::cout << choice << " is not a valid option.\n";
        }
    }
}
