#pragma once

#include <map>
#include <string>

class StudentList final {
private:
  std::map<std::string, int> data_;

public:
  using const_iterator = std::map<std::string, int>::const_iterator;

  StudentList() = default;

  /**
   * @brief Return constant end iterator of data
   *
   * @return auto const iterator
   */
  auto End() const { return data_.cend(); }

  /**
   * @brief Read student list from file(name, grade)
   *  Input should be with no error, data format is "name grade", no name
   * duplicates
   * @throws std::runtime_error if input error or bad format
   * @param is
   */
  void Read(std::istream &is);

  /**
   * @brief Output student list sorted by name
   * @throws std::runtime_error on failed output
   * @param os
   */
  void Output(std::ostream &os) const;

  /**
   * @brief Get the Max Grade Student
   *
   * @return std::string
   */
  const_iterator GetMaxGradeStudent() const;

  /**
   * @brief Get the Avg Grade
   *
   * @return int
   */
  double GetAvgGrade() const;
};