#pragma once

#include "records.hpp"

#include <filesystem>
#include <ostream>
#include <string>

namespace joque
{

/// Generates JUnit XML file, expects that execution record represents some form
/// of test execution.
///
/// \param p output file to which the XML should be generated
/// \param ts_name name of the test suite
/// \param exec_rec execution record
void generate_junit_xml(
    const std::filesystem::path& p,
    const std::string&           ts_name,
    const exec_record&           exec_rec );

/// Overload of `generate_junit_xml` that uses ostream instead of filename.
///
/// \param os output stream to which the XML should be generated
/// \param ts_name name of the test suite
/// \param exec_rec execution record
void generate_junit_xml(
    std::ostream&      os,
    const std::string& ts_name,
    const exec_record& exec_rec );

}  // namespace joque
