#pragma once

#include "joque/exec.hpp"

namespace joque
{

void render_junit_xml( std::ostream& os, const std::string& ts_name, const exec_record& exec_rec );

}