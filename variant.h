#pragma once

#include <variant>

using std::variant;
using std::in_place_type_t;
using std::in_place_index_t;
using std::variant_size;
using std::variant_size_v;
using std::variant_alternative;
using std::variant_alternative_t;
using std::get;
using std::get_if;
using std::holds_alternative;
using std::bad_variant_access;
using std::in_place_index;
using std::in_place_type;
using std::visit;
using std::variant_npos;
