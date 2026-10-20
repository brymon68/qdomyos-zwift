#include "../../src/zwiftinclinegrade.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

static void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

static void requireGrade(const std::optional<double> &grade, double expected) {
    require(grade.has_value(), "Expected an incline command.");
    require(std::abs(*grade - expected) < 0.000001, "Unexpected calibrated incline.");
}

int main() {
    try {
        ZwiftInclineGrade terrain;
        require(!terrain.update(100, 1000, true, 0.5, 0.1, false), "First sample must not command incline.");
        requireGrade(terrain.update(110, 1010, true, 0.5, 0.1, false), 0.6);
        requireGrade(terrain.update(120, 1000, true, 0.5, 0.1, false), -0.4);
        requireGrade(terrain.update(130, 990, true, 0.5, 0.1, true), -0.9);

        require(!terrain.update(130, 990, true, 0.5, 0, false), "Stationary sample must not command incline.");
        require(!terrain.update(131, 991, true, 0.5, 0, false), "A one-unit distance step must be ignored.");
        require(!terrain.update(120, 980, true, 0.5, 0, false), "Backward distance must not command incline.");
        requireGrade(terrain.update(130, 990, true, 0.5, 0, false), 0.5);

        require(!terrain.update(140, 1010, false, 0.5, 0, false), "Disabled auto-incline must not issue a command.");
        requireGrade(terrain.update(150, 1010, true, 0.5, 0, false), 0.0);
        requireGrade(terrain.update(160, 1020, true, 0, 0.25, true), 0.25);

        ZwiftInclineGrade restarted;
        require(!restarted.update(0, 100, true, 0.5, 0, false), "Zero distance must establish history only.");
        require(!restarted.update(10, 120, true, 0.5, 0, false), "Previous zero distance must not command incline.");
        requireGrade(restarted.update(20, 130, true, 0.5, 0, false), 0.5);
        std::cout << "Zwift incline tests passed: history, grade calibration, negative scaling and enable gate.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
