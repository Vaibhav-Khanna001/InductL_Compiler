#ifndef VERIFIER_HPP
#define VERIFIER_HPP

#include <string>
#include <map>
#include <algorithm>
#include <iostream>

// Represents the mathematical boundaries of a variable
struct ValueRange {
    double min;
    double max;

    // Default to "any real number"
    ValueRange(double lo = -1e18, double hi = 1e18) : min(lo), max(hi) {}

    // Helper to check if a range is strictly within another (Verification)
    bool isWithin(const ValueRange& other) const {
        return (this->min >= other.min && this->max <= other.max);
    }
};

class VerifierState {
    std::map<std::string, ValueRange> constraints;
public:
    void setConstraint(std::string name, double lo, double hi) {
        constraints[name] = ValueRange(lo, hi);
    }

    ValueRange getConstraint(std::string name) {
        if (constraints.find(name) != constraints.end()) return constraints[name];
        return ValueRange(); // Return unknown range if not found
    }

    void dump() {
        std::cout << "--- Static Verification State ---" << std::endl;
        for (auto const& [name, range] : constraints) {
            std::cout << name << " : [" << range.min << ", " << range.max << "]" << std::endl;
        }
    }
};

#endif