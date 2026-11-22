// Smart Water Management System
// OOP Mini Project - Object-Oriented Programming Using C++ (SDG 6)
// Compile: g++ -std=c++17 -Wall -Wextra -pedantic water_management.cpp -o water_tracker

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const string DATA_FILE = "water_records.txt";
const int DAYS_IN_MONTH = 30;
const double BENCHMARK_LPCD = 135.0;  // litres per person per day

// ---------------------------------------------------------------------------
// Abstract base class: one water-using activity of the household
// ---------------------------------------------------------------------------
class WaterUsage {
protected:
    string name;
    double litresPerUse;
    double usesPerDay;

public:
    WaterUsage(const string& n, double l, double u)
        : name(n), litresPerUse(l), usesPerDay(u) {}
    virtual ~WaterUsage() = default;

    virtual string type() const = 0;  // category name
    virtual string tip() const = 0;   // category-specific saving tip

    const string& getName() const { return name; }
    double getLitresPerUse() const { return litresPerUse; }
    double getUsesPerDay() const { return usesPerDay; }
    double dailyLitres() const { return litresPerUse * usesPerDay; }
    double monthlyLitres() const { return dailyLitres() * DAYS_IN_MONTH; }
};

class Bathroom : public WaterUsage {
public:
    using WaterUsage::WaterUsage;
    string type() const override { return "Bathroom"; }
    string tip() const override {
        return "Use a bucket or low-flow shower head and fix leaking taps.";
    }
};

class Kitchen : public WaterUsage {
public:
    using WaterUsage::WaterUsage;
    string type() const override { return "Kitchen"; }
    string tip() const override {
        return "Wash vegetables in a bowl and reuse that water for plants.";
    }
};

class Laundry : public WaterUsage {
public:
    using WaterUsage::WaterUsage;
    string type() const override { return "Laundry"; }
    string tip() const override {
        return "Run the washing machine only with full loads.";
    }
};

class Garden : public WaterUsage {
public:
    using WaterUsage::WaterUsage;
    string type() const override { return "Garden"; }
    string tip() const override {
        return "Water plants early morning or evening and use drip watering.";
    }
};

// ---------------------------------------------------------------------------
// Manager class: stores objects, calculates summaries, handles files
// ---------------------------------------------------------------------------
class WaterManager {
private:
    vector<unique_ptr<WaterUsage>> usages;
    double monthlyTargetKL = 15.0;  // kilolitres per month
    int members = 4;

    static unique_ptr<WaterUsage> create(int type, const string& n,
                                         double l, double u) {
        switch (type) {
            case 1: return make_unique<Bathroom>(n, l, u);
            case 2: return make_unique<Kitchen>(n, l, u);
            case 3: return make_unique<Laundry>(n, l, u);
            case 4: return make_unique<Garden>(n, l, u);
            default: return nullptr;
        }
    }

    // Slab-wise bill: 0-10 kL @ Rs 5, 10-20 kL @ Rs 8, above 20 kL @ Rs 12
    // plus a fixed service charge of Rs 40.
    static double estimateBill(double kl) {
        double bill = 40.0;
        double first = min(kl, 10.0);
        bill += first * 5.0;
        if (kl > 10.0) bill += (min(kl, 20.0) - 10.0) * 8.0;
        if (kl > 20.0) bill += (kl - 20.0) * 12.0;
        return bill;
    }

public:
    bool add(int type, const string& n, double l, double u) {
        auto p = create(type, n, l, u);
        if (!p) return false;
        usages.push_back(move(p));
        return true;
    }

    void setTarget(double kl) { monthlyTargetKL = kl; }
    void setMembers(int m) { members = m; }

    void viewAll() const {
        if (usages.empty()) {
            cout << "No water usage records yet.\n";
            return;
        }
        cout << left << setw(5) << "No." << setw(12) << "Category"
             << setw(26) << "Activity" << setw(10) << "L/use"
             << setw(10) << "Uses/day" << "kL/month\n";
        cout << string(71, '-') << "\n";
        int i = 1;
        for (const auto& u : usages) {  // runtime polymorphism via type()
            cout << left << setw(5) << i++ << setw(12) << u->type()
                 << setw(26) << u->getName() << fixed << setprecision(1)
                 << setw(10) << u->getLitresPerUse() << setw(10)
                 << u->getUsesPerDay() << setprecision(2)
                 << u->monthlyLitres() / 1000.0 << "\n";
        }
    }

    void summary() const {
        if (usages.empty()) {
            cout << "Add at least one activity first.\n";
            return;
        }
        double daily = 0;
        map<string, double> byCategory;
        for (const auto& u : usages) {
            daily += u->dailyLitres();
            byCategory[u->type()] += u->monthlyLitres() / 1000.0;
        }
        double monthlyKL = daily * DAYS_IN_MONTH / 1000.0;
        double perPerson = daily / members;

        cout << fixed << setprecision(2);
        cout << "Daily consumption   : " << daily << " litres\n";
        cout << "Monthly consumption : " << monthlyKL << " kL\n";
        for (const auto& c : byCategory)
            cout << "  " << left << setw(10) << c.first << ": " << c.second
                 << " kL\n";
        cout << "Household members   : " << members << "\n";
        cout << "Per person per day  : " << perPerson << " litres (benchmark "
             << BENCHMARK_LPCD << ")\n";
        cout << "Estimated bill      : Rs " << estimateBill(monthlyKL)
             << " per month\n";
        cout << "Monthly target      : " << monthlyTargetKL << " kL\n";
        if (monthlyKL > monthlyTargetKL)
            cout << "Status: OVER target by " << monthlyKL - monthlyTargetKL
                 << " kL.\n";
        else
            cout << "Status: WITHIN target by " << monthlyTargetKL - monthlyKL
                 << " kL.\n";

        cout << "Water-saving tips:\n";
        map<string, string> tips;
        for (const auto& u : usages) tips[u->type()] = u->tip();
        for (const auto& t : tips) cout << "- " << t.second << "\n";
    }

    bool save() const {
        ofstream out(DATA_FILE);
        if (!out) return false;
        out << monthlyTargetKL << " " << members << "\n";
        for (const auto& u : usages) {
            int code = (u->type() == "Bathroom") ? 1
                     : (u->type() == "Kitchen")  ? 2
                     : (u->type() == "Laundry")  ? 3 : 4;
            out << code << "|" << u->getName() << "|" << u->getLitresPerUse()
                << "|" << u->getUsesPerDay() << "\n";
        }
        return true;
    }

    void load() {
        ifstream in(DATA_FILE);
        if (!in) return;
        string line;
        if (getline(in, line)) {
            istringstream head(line);
            double t;
            int m;
            if (head >> t >> m && t > 0 && m > 0) {
                monthlyTargetKL = t;
                members = m;
            }
        }
        while (getline(in, line)) {
            istringstream ss(line);
            string code, n, l, u;
            if (getline(ss, code, '|') && getline(ss, n, '|') &&
                getline(ss, l, '|') && getline(ss, u, '|')) {
                try {
                    add(stoi(code), n, stod(l), stod(u));
                } catch (...) {
                    // skip damaged line
                }
            }
        }
    }
};

// ---------------------------------------------------------------------------
// Input validation helpers
// ---------------------------------------------------------------------------
double readDouble(const string& prompt, double lo, double hi) {
    double v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= lo && v <= hi) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return v;
        }
        if (cin.eof()) exit(0);
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Enter a value between " << lo << " and "
             << hi << ".\n";
    }
}

int readInt(const string& prompt, int lo, int hi) {
    return static_cast<int>(readDouble(prompt, lo, hi));
}

string readLine(const string& prompt) {
    string s;
    while (true) {
        cout << prompt;
        if (!getline(cin, s)) exit(0);
        // remove '|' because it is the file separator
        s.erase(remove(s.begin(), s.end(), '|'), s.end());
        if (!s.empty()) return s;
        cout << "Name cannot be empty.\n";
    }
}

int main() {
    WaterManager manager;
    manager.load();

    while (true) {
        cout << "\n=== SMART WATER MANAGEMENT SYSTEM ===\n"
             << "1 Add water usage\n"
             << "2 View all usage\n"
             << "3 Water summary and bill estimate\n"
             << "4 Set monthly target and household members\n"
             << "5 Save and exit\n";
        int choice = readInt("Choice: ", 1, 5);

        if (choice == 1) {
            int type = readInt(
                "Type (1 Bathroom, 2 Kitchen, 3 Laundry, 4 Garden): ", 1, 4);
            string name = readLine("Activity name: ");
            double l = readDouble("Water per use (litres): ", 0.1, 10000);
            double u = readDouble("Uses per day: ", 0.1, 100);
            manager.add(type, name, l, u);
            cout << "Usage added.\n";
        } else if (choice == 2) {
            manager.viewAll();
        } else if (choice == 3) {
            manager.summary();
        } else if (choice == 4) {
            manager.setTarget(readDouble("Monthly target (kL): ", 1, 1000));
            manager.setMembers(readInt("Household members: ", 1, 50));
            cout << "Target and members updated.\n";
        } else {
            if (manager.save())
                cout << "Records saved. Goodbye!\n";
            else
                cout << "Could not save records.\n";
            break;
        }
    }
    return 0;
}
