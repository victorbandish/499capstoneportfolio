#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>  // For std::sort

// Define the Course class that will hold course details
class Course {
public:
    std::string courseNumber;   // The course number 
    std::string title;          // The title of the course 
    std::vector<std::string> prerequisites;  // List of prerequisites for the course

    // Constructor to initialize the course with its course number and title
    Course(std::string number, std::string courseTitle) : courseNumber(number), title(courseTitle) {}

    // Method to add prerequisites to the course
    void addPrerequisite(const std::string& prereq) {
        prerequisites.push_back(prereq);
    }
};

// Function to open the file and check if it is successfully opened
std::ifstream openFile(const std::string& fileName) {
    std::ifstream file(fileName);  // Try to open the file
    if (!file.is_open()) {  // If the file cannot be opened, print error and exit
        std::cerr << "Error: File not found" << std::endl;
        exit(1);  // Exit the program
    }
    return file;  // Return the opened file
}

// Function to parse the course data from the file and store it in a vector of Course objects
std::vector<Course> parseFile(std::ifstream& file) {
    std::vector<Course> courseList;  // Vector to store Course objects
    std::string line;

    // Read the file line by line
    while (getline(file, line)) {
        std::istringstream ss(line);  // Create a stringstream for each line
        std::string courseNumber, title;

        // Split the line by commas and store the course number and title
        getline(ss, courseNumber, ',');
        getline(ss, title, ',');

        // Create a Course object and initialize it with course number and title
        Course course(courseNumber, title);

        // Parse the prerequisites (if any) by reading the remaining comma-separated values
        std::string prereq;
        while (getline(ss, prereq, ',')) {
            course.addPrerequisite(prereq);  // Add each prerequisite to the course object
        }

        courseList.push_back(course);  // Add the course object to the list
    }

    return courseList;  // Return the vector containing all courses
}

// Function to print the sorted list of courses
void printSortedCourses(const std::vector<Course>& courses) {
    std::vector<Course> sortedCourses = courses;  // Create a copy of the courses vector to sort
    std::sort(sortedCourses.begin(), sortedCourses.end(), [](const Course& a, const Course& b) {
        // Compare course numbers to sort the courses in ascending order
        return a.courseNumber < b.courseNumber;
        });

    // Print the sorted list of courses
    for (const auto& course : sortedCourses) {
        std::cout << course.courseNumber << ", " << course.title << std::endl;  // Print course number and title
    }
}

// Function to search for a specific course by its course number
void searchCourse(const std::string& courseNumber, const std::vector<Course>& courses) {
    for (const auto& course : courses) {
        if (course.courseNumber == courseNumber) {  // If the course number matches exactly
            std::cout << course.courseNumber << ", " << course.title << std::endl;  // Print the course details
            std::cout << "Prerequisites: ";
            if (course.prerequisites.empty()) {  // If no prerequisites, print "None"
                std::cout << "None" << std::endl;
            }
            else {  // Otherwise, print each prerequisite
                for (const auto& prereq : course.prerequisites) {
                    std::cout << prereq << " ";
                }
                std::cout << std::endl;
            }
            return;  // Exit the function once the course is found
        }
    }
    std::cout << "Error: Course not found" << std::endl;  // If no course is found, print an error message
}

// Function to display the main menu and prompt the user for their choice
int displayMenu() {
    // Print the available options to the user
    std::cout << "1. Load Data Structure." << std::endl;
    std::cout << "2. Print Course List." << std::endl;
    std::cout << "3. Print Course." << std::endl;
    std::cout << "9. Exit" << std::endl;
    std::cout << "What would you like to do? ";  // Prompt user for input

    int choice;
    std::cin >> choice;  // Read the user's choice
    return choice;  // Return the user's choice
}

int main() {
    std::string filename = "CS 300 ABCU_Advising_Program_Input.csv";  // Default file path to the course data CSV file
    std::vector<Course> courses;  // Vector to store courses
    std::ifstream file = openFile(filename);  // Open the file

    courses = parseFile(file);  // Parse the file and load courses into the vector

    std::cout << "Welcome to the course planner." << std::endl;

    // Main program loop to keep displaying the menu until the user exits
    while (true) {
        int choice = displayMenu();  // Display the menu and get the user's choice

        switch (choice) {
        case 1:  // Option to load a new file
            std::cout << "Enter file name: ";
            std::cin >> filename;  // Prompt the user to enter the file name
            file = openFile(filename);  // Open the new file
            courses = parseFile(file);  // Parse the new file and load the courses
            break;
        case 2:  // Option to print the sorted course list
            std::cout << "Here is a sample schedule:" << std::endl;
            printSortedCourses(courses);  // Print the sorted course list
            break;
        case 3: {  // Option to search for a specific course
            std::string courseNumber;
            std::cout << "What course do you want to know about? ";
            std::cin >> courseNumber;  // Prompt the user to enter the course number
            searchCourse(courseNumber, courses);  // Search for the course and print the details
            break;
        }
        case 9:  // Option to exit the program
            std::cout << "Thank you for using the course planner!" << std::endl;
            return 0;  // Exit the program
        default:  // Handle invalid input from the user
            std::cout << choice << " is not a valid option." << std::endl;
        }
    }
}