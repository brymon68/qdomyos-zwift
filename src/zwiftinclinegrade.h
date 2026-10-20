#ifndef ZWIFTINCLINEGRADE_H
#define ZWIFTINCLINEGRADE_H

#include <optional>

// Zwift altitude and distance retain their existing API units here.
class ZwiftInclineGrade {
  public:
    std::optional<double> update(float distance, float altitude, bool enabled,
                                 double gain, double offset, bool doubleNegative) {
        std::optional<double> result;
        if (previousDistance > 0) {
            const float delta = distance - previousDistance;
            if (delta > 1) {
                const float incline = (altitude - previousAltitude) / delta;
                double grade = incline * gain + offset;
                if (doubleNegative && incline < 0)
                    grade = (incline * 2.0) * gain + offset;
                if (enabled)
                    result = grade;
            }
        }
        previousDistance = distance;
        previousAltitude = altitude;
        return result;
    }

  private:
    float previousDistance = 0;
    float previousAltitude = 0;
};

#endif // ZWIFTINCLINEGRADE_H
