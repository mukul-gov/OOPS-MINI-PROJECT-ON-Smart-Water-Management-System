# Smart Water Management System (SDG 6)

C++17 console application for recording household water usage,
estimating monthly consumption and bill, and getting water-saving tips.

## Build and run
    g++ -std=c++17 -Wall -Wextra -pedantic water_management.cpp -o water_tracker
    ./water_tracker

## OOP concepts used
- Abstraction: abstract class `WaterUsage`
- Inheritance: `Bathroom`, `Kitchen`, `Laundry`, `Garden`
- Polymorphism: virtual `type()` and `tip()`
- Encapsulation: protected data, public methods
- STL: `vector<unique_ptr<WaterUsage>>`, `map`
- File handling: records saved to / loaded from `water_records.txt`
