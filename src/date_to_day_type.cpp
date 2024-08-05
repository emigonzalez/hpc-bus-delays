#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

// Enum to represent day types
enum DayType {
    WEEKDAY = 1,
    SATURDAY,
    SUNDAY
};

// Function to get the day of the week from a date string (format: yyyy-mm-dd)
int get_day_of_week(const std::string& date_str) {
    std::tm tm_date = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm_date, "%Y-%m-%d");
    if (ss.fail()) {
        std::cerr << "Error parsing date: " << date_str << std::endl;
        return -1; // Error code for parsing failure
    }
    // Use mktime to convert tm structure to time_t and normalize the tm structure
    std::time_t time = std::mktime(&tm_date);
    if (time == -1) {
        std::cerr << "Error converting date to time_t: " << date_str << std::endl;
        return -1; // Error code for conversion failure
    }
    // Return the day of the week (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
    return tm_date.tm_wday;
}

// Function to get the day type given a date string
DayType get_day_type(const std::string& date_str) {
    int day_of_week = get_day_of_week(date_str);
    if (day_of_week == -1) {
        return static_cast<DayType>(-1); // Error code for invalid date
    }
    switch (day_of_week) {
        case 0: return SUNDAY;    // Sunday
        case 6: return SATURDAY;  // Saturday
        default: return WEEKDAY;  // Weekday (Monday to Friday)
    }
}

// Function to print the day type as a string
int date_to_date_type(const std::string& date_str) {
    DayType day_type = get_day_type(date_str);
    if (day_type == static_cast<DayType>(-1)) {
        std::cerr << "Invalid date provided" << std::endl;
        return -1; // Error code for invalid day type
    }

    return static_cast<int>(day_type);
}
