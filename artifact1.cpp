#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// -----------------------------
// Software Design Enhancements
// -----------------------------
// 1) Do not auto-load data at startup. User must choose menu option 1.
// 2) No exit() inside helpers. Return success/failure and handle in main.
// 3) Separation of concerns: parsing, printing, lookup, and UI are separated.
// 4) Robust input handling using getline so filenames with spaces work.
// 5) Data normalization (trim + uppercase course numbers) to reduce input defects.
// 6) Use unordered_map keyed by courseNumber for scalable lookups.
//    Keep sorted printing by sorting keys rather than sorting the container.

// Holds course details
struct Course {
    std::string courseNumber;
    std::string title;
    std::vector<std::string> prerequisites;
};

// Trim whitespace from both ends of a string
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

// Uppercase helper for consistent matching
static std::string toUpper(std::string s) {
    for (char& ch : s) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return s;
}

// Parse CSV file into an unordered_map keyed by course number
// Returns true if file opened and parsed, false if file open fails.
static bool loadCoursesFromCsv(
    const std::string& fileName,
    std::unordered_map<std::string, Course>& coursesOut
) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        return false;
    }

    // Load into a temp container first so we only overwrite on success.
    std::unordered_map<std::string, Course> temp;

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue; // skip blank lines
        }

        std::istringstream ss(line);
        std::string courseNumber, title;

        if (!std::getline(ss, courseNumber, ',')) {
            continue; // malformed line
        }
        if (!std::getline(ss, title, ',')) {
            continue; // malformed line
        }

        courseNumber = toUpper(trim(courseNumber));
        title = trim(title);

        if (courseNumber.empty() || title.empty()) {
            continue; // invalid record
        }

        Course c;
        c.courseNumber = courseNumber;
        c.title = title;

        std::string prereq;
        while (std::getline(ss, prereq, ',')) {
            prereq = toUpper(trim(prereq));
            if (!prereq.empty()) {
                c.prerequisites.push_back(prereq);
            }
        }

        // If duplicates exist, later records overwrite earlier ones.
        temp[courseNumber] = c;
    }

    coursesOut.swap(temp);
    return true;
}

// Print a sorted list of courses (sorted by course number)
static void printCourseList(const std::unordered_map<std::string, Course>& courses) {
    std::vector<std::string> keys;
    keys.reserve(courses.size());

    for (const auto& kv : courses) {
        keys.push_back(kv.first);
    }

    std::sort(keys.begin(), keys.end());

    for (const auto& key : keys) {
        const Course& c = courses.at(key);
        std::cout << c.courseNumber << ", " << c.title << '\n';
    }
}

// Print a single course and its prerequisites
static void printCourseDetails(const std::unordered_map<std::string, Course>& courses,
    std::string courseNumber) {
    courseNumber = toUpper(trim(courseNumber));

    const auto it = courses.find(courseNumber);
    if (it == courses.end()) {
        std::cout << "Error: Course not found\n";
        return;
    }

    const Course& c = it->second;
    std::cout << c.courseNumber << ", " << c.title << '\n';

    std::cout << "Prerequisites: ";
    if (c.prerequisites.empty()) {
        std::cout << "None\n";
        return;
    }

    for (size_t i = 0; i < c.prerequisites.size(); ++i) {
        std::cout << c.prerequisites[i];
        if (i + 1 < c.prerequisites.size()) {
            std::cout << ", ";
        }
    }
    std::cout << '\n';
}

// Display the menu and return a validated integer choice
static int displayMenu() {
    std::cout << "\n1. Load Data Structure.\n";
    std::cout << "2. Print Course List.\n";
    std::cout << "3. Print Course.\n";
    std::cout << "9. Exit\n";
    std::cout << "What would you like to do? ";

    std::string input;
    std::getline(std::cin, input);

    input = trim(input);
    if (input.empty()) {
        return -1;
    }

    try {
        return std::stoi(input);
    }
    catch (...) {
        return -1;
    }
}

int main() {
    std::unordered_map<std::string, Course> courses;
    bool dataLoaded = false;

    std::cout << "Welcome to the course planner.\n";

    while (true) {
        int choice = displayMenu();

        switch (choice) {
        case 1: {
            std::cout << "Enter file name: ";
            std::string filename;
            std::getline(std::cin, filename);

            if (!loadCoursesFromCsv(filename, courses)) {
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
            printCourseList(courses);
            break;

        case 3: {
            if (!dataLoaded) {
                std::cout << "Please load data first using option 1.\n";
                break;
            }
            std::cout << "What course do you want to know about? ";
            std::string courseNumber;
            std::getline(std::cin, courseNumber);
            printCourseDetails(courses, courseNumber);
            break;
        }

        case 9:
            std::cout << "Thank you for using the course planner!\n";
            return 0;

        default:
            std::cout << choice << " is not a valid option.\n";
            break;
        }
    }
}

