#include "records.hpp"

#include <nlohmann/json.hpp>

namespace joque
{

void to_json( nlohmann::json& j, const output_chunk& rec );

void to_json( nlohmann::json& j, const run_record& rec );

void to_json( nlohmann::json& j, const exec_record& rec );

}  // namespace joque