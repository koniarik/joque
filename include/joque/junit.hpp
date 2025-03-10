/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
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
