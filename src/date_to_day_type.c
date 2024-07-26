#include <stdio.h>
#include <stdlib.h> // Include this header for EXIT_FAILURE

// Enum to represent day types
typedef enum {
    WEEKDAY,
    SATURDAY,
    SUNDAY
} DayType;

// Array to store the day types for June 2024
const DayType june_2024_day_types[] = {
    SATURDAY,  // 1 June 2024 is a Saturday
    SUNDAY,   // 2 June 2024
    WEEKDAY,  // 3 June 2024
    WEEKDAY,  // 4 June 2024
    WEEKDAY,  // 5 June 2024
    WEEKDAY,  // 6 June 2024
    WEEKDAY,  // 7 June 2024
    SATURDAY, // 8 June 2024
    SUNDAY,   // 9 June 2024
    WEEKDAY,  // 10 June 2024
    WEEKDAY,  // 11 June 2024
    WEEKDAY,  // 12 June 2024
    WEEKDAY,  // 13 June 2024
    WEEKDAY,  // 14 June 2024
    SATURDAY, // 15 June 2024
    SUNDAY,   // 16 June 2024
    WEEKDAY,  // 17 June 2024
    WEEKDAY,  // 18 June 2024
    SUNDAY,   // 19 June 2024 is a Holiday
    WEEKDAY,  // 20 June 2024
    WEEKDAY,  // 21 June 2024
    SATURDAY, // 22 June 2024
    SUNDAY,   // 23 June 2024
    WEEKDAY,  // 24 June 2024
    WEEKDAY,  // 25 June 2024
    WEEKDAY,  // 26 June 2024
    WEEKDAY,  // 27 June 2024
    WEEKDAY,  // 28 June 2024
    SATURDAY, // 29 June 2024
    SUNDAY    // 30 June 2024
};

// Function to get the day type given a day of June 2024
DayType get_day_type(int day) {
    if (day < 1 || day > 30) {
        fprintf(stderr, "Invalid day: %d\n", day);
        exit(EXIT_FAILURE);
    }
    return june_2024_day_types[day - 1];
}

// Function to print the day type as a string
const char* day_type_to_string(DayType day_type) {
    switch (day_type) {
        case WEEKDAY: return "Weekday";
        case SATURDAY: return "Saturday";
        case SUNDAY: return "Sunday";
        default: return "Unknown";
    }
}

// Function to print the day type as a string
const char* day_to_string(int day) {
    DayType day_type = get_day_type(day);

    switch (day_type) {
        case WEEKDAY: return "Weekday";
        case SATURDAY: return "Saturday";
        case SUNDAY: return "Sunday";
        default: return "Unknown";
    }
}

int main() {
    int day = 19; // Example day

    printf("Day %d of June 2024 is a %s\n", day, day_to_string(day));

    return 0;
}
