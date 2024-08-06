#define _XOPEN_SOURCE 700 // to suppress warning about strptime

#include <stdio.h>
#include <stdlib.h> // Include this header for EXIT_FAILURE
#include <time.h>

// Enum to represent day types
typedef enum {
    WEEKDAY,
    SATURDAY,
    SUNDAY
} DayType;

// Function to get the day of the week from a date string (format: yyyy-mm-dd)
int get_day_of_week(const char* date_str) {
    struct tm tm_date = {0};
    if (strptime(date_str, "%Y-%m-%d", &tm_date) == NULL) {
        fprintf(stderr, "Error parsing date: %s\n", date_str);
        return -1;
    }
    // Use mktime to convert tm structure to time_t and normalize the tm structure
    time_t time = mktime(&tm_date);
    if (time == -1) {
        fprintf(stderr, "Error converting date to time_t: %s\n", date_str);
        return -1;
    }
    // Return the day of the week (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
    return tm_date.tm_wday;
}

// Function to get the day type given a date string
DayType get_day_type(const char* date_str) {
    int day_of_week = get_day_of_week(date_str);
    switch (day_of_week) {
        case 0: return SUNDAY;    // Sunday
        case 6: return SATURDAY;  // Saturday
        default: return WEEKDAY;  // Weekday (Monday to Friday)
    }
}

// Function to print the day type as a string
int date_to_date_type(const char* date_str) {
    DayType day_type = get_day_type(date_str);

    switch (day_type) {
        case WEEKDAY: return 1;
        case SATURDAY: return 2;
        case SUNDAY: return 3;
        default: return 0;
    }
}
