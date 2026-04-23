#include "student_list.h"
#include <algorithm>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>

void StudentList::Read(std::istream &is) {
  if (!is) {
    throw std::runtime_error("Cannot read from stream");
  }

  std::string line;
  while (std::getline(is, line)) {
    if (line.empty()) {
      continue;
    }

    std::istringstream iss{line};
    std::string name;
    int grade;

    if (iss >> name >> grade) {
      std::string trash;
      if (iss >> trash) {
        throw std::runtime_error("Extra data in line: " + line);
      }

      if (grade < 0) {
        throw std::runtime_error("Grade can't be negative");
      }

      auto [_, is_inserted] = data_.emplace(name, grade);
      if (!is_inserted) {
        throw std::runtime_error("Duplicate student: " + name);
      }
    } else {
      throw std::runtime_error("Invalid line format");
    }
  }
}

void StudentList::Output(std::ostream &os) const {
  std::ranges::for_each(data_, [&os](const auto &val) {
    os << val.first << " " << val.second << "\n";
  });

  if (!os) {
    throw std::runtime_error("Output failed");
  }
}

StudentList::const_iterator StudentList::GetMaxGradeStudent() const {
  return std::ranges::max_element(data_, {},
                                  [](const auto &val) { return val.second; });
}

double StudentList::GetAvgGrade() const {
  if (data_.empty()) {
    return 0.0;
  }

  int sum = std::transform_reduce(data_.begin(), data_.end(), 0, std::plus{},
                                  [](const auto &val) { return val.second; });

  return static_cast<double>(sum) / data_.size();
}