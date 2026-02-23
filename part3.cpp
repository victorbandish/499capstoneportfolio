#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "sqlite3.h"

/*
========================================================
CS 499 – Milestone Four: Databases Enhancement
--------------------------------------------------------
This version of the CS 300 Analysis and Design course
planner has been enhanced to demonstrate database
competency using SQLite.

Key database enhancements include:
- Replacing in-memory data storage with a SQLite database
- Designing relational tables for courses and prerequisites
- Loading CSV data into database tables
- Using SQL queries with ORDER BY for sorted output
- Using parameterized queries to safely retrieve data
- Maintaining data consistency through normalization

This artifact aligns with the Databases category of the
CS 499 ePortfolio.
========================================================
*/

/*
--------------------------------------------------------
String normalization helpers
--------------------------------------------------------
These functions ensure consistent storage and querying
of course numbers by trimming whitespace and converting
input to uppercase.
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
SQLite helper class
--------------------------------------------------------
Encapsulates opening, closing, and executing SQL
statements against the SQLite database.
*/
class Database {
public:
    Database() : db(nullptr) {}
    ~Database() { close(); }

    bool open(const std::string& filename) {
        if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
            std::cout << "Error opening database: " << sqlite3_errmsg(db) << "\n";
            return false;
        }
        return true;
    }

    void close() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    bool execute(const std::string& sql) {
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cout << "SQL error: " << errMsg << "\n";
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    sqlite3* get() { return db; }

private:
    sqlite3* db;
};

/*
--------------------------------------------------------
Database schema creation
--------------------------------------------------------
Creates relational tables for courses and prerequisites.
Foreign keys enforce data integrity.
Indexes improve query performance.
*/
static bool createSchema(Database& db) {
    const char* schemaSQL = R"SQL(
        PRAGMA foreign_keys = ON;

        CREATE TABLE IF NOT EXISTS courses (
            course_number TEXT PRIMARY KEY,
            title TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS prerequisites (
            course_number TEXT NOT NULL,
            prereq_number TEXT NOT NULL,
            PRIMARY KEY (course_number, prereq_number),
            FOREIGN KEY (course_number)
                REFERENCES courses(course_number)
                ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_prereq_course
            ON prerequisites(course_number);
    )SQL";

    return db.execute(schemaSQL);
}

/*
--------------------------------------------------------
CSV loader
--------------------------------------------------------
Reads course data from a CSV file and inserts it into
the database using SQL INSERT statements.
*/
static bool loadCoursesFromCSV(const std::string& filename, Database& db) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: CSV file not found\n";
        return false;
    }

    db.execute("DELETE FROM prerequisites;");
    db.execute("DELETE FROM courses;");

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string courseNum, title;

        if (!std::getline(ss, courseNum, ',')) continue;
        if (!std::getline(ss, title, ',')) continue;

        courseNum = normalizeCourseNumber(courseNum);
        title = trim(title);

        std::string insertCourse =
            "INSERT OR REPLACE INTO courses VALUES ('" +
            courseNum + "', '" + title + "');";

        if (!db.execute(insertCourse)) return false;

        std::string prereq;
        while (std::getline(ss, prereq, ',')) {
            prereq = normalizeCourseNumber(prereq);
            if (prereq.empty()) continue;

            std::string insertPrereq =
                "INSERT INTO prerequisites VALUES ('" +
                courseNum + "', '" + prereq + "');";

            if (!db.execute(insertPrereq)) return false;
        }
    }

    return true;
}

/*
--------------------------------------------------------
Query functions
--------------------------------------------------------
*/
static void printCourseList(Database& db) {
    const char* sql =
        "SELECT course_number, title FROM courses ORDER BY course_number;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout
            << sqlite3_column_text(stmt, 0)
            << ", "
            << sqlite3_column_text(stmt, 1)
            << "\n";
    }

    sqlite3_finalize(stmt);
}

static void printCourseDetails(Database& db, std::string courseNum) {
    courseNum = normalizeCourseNumber(courseNum);

    const char* courseSQL =
        "SELECT title FROM courses WHERE course_number = ?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db.get(), courseSQL, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, courseNum.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        std::cout << "Course not found\n";
        sqlite3_finalize(stmt);
        return;
    }

    std::cout << courseNum << ", " << sqlite3_column_text(stmt, 0) << "\n";
    sqlite3_finalize(stmt);

    const char* prereqSQL =
        "SELECT prereq_number FROM prerequisites WHERE course_number = ?;";

    sqlite3_prepare_v2(db.get(), prereqSQL, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, courseNum.c_str(), -1, SQLITE_TRANSIENT);

    std::cout << "Prerequisites: ";
    bool found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << sqlite3_column_text(stmt, 0) << " ";
        found = true;
    }

    if (!found) std::cout << "None";
    std::cout << "\n";

    sqlite3_finalize(stmt);
}

/*
--------------------------------------------------------
Menu and program entry point
--------------------------------------------------------
*/
static int menu() {
    std::cout << "\n1. Load Courses\n2. Print Course List\n3. Print Course\n9. Exit\nChoice: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    return choice;
}

int main() {
    Database db;
    db.open("courses.db");
    createSchema(db);

    bool loaded = false;

    while (true) {
        int choice = menu();

        if (choice == 1) {
            loaded = loadCoursesFromCSV("courses.csv", db);
            if (loaded) std::cout << "Courses loaded successfully.\n";
        }
        else if (choice == 2 && loaded) {
            printCourseList(db);
        }
        else if (choice == 3 && loaded) {
            std::string course;
            std::cout << "Enter course number: ";
            std::getline(std::cin, course);
            printCourseDetails(db, course);
        }
        else if (choice == 9) {
            break;
        }
        else {
            std::cout << "Invalid option or data not loaded.\n";
        }
    }

    return 0;
}
