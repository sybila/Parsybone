#ifndef MEASUREMENTS_CONTENT_HPP
#define MEASUREMENTS_CONTENT_HPP

class MeasurementsContent {
   vector<string> measurements;

public:
   void addMeasurement(const string & measurement) {
      measurements.push_back(measurement);
   }

   const vector<string> & getMeasurements() {
      return measurements;
   }

   const string & getMeasurement(int measurement_no) {
      return measurements[measurement_no];
   }
};

#endif // MEASUREMENTS_CONTENT_HPP
