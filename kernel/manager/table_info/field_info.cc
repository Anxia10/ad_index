#include <vector>
#include <string>
#include "kernel/manager/table_info/field_info.h"
#include "kernel/common/utils/utils.h"
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace manager {

LOG_SETUP("kernel", FieldInfo);

// field type
static const char kInt8[] = "int8";
static const char kInt16[] = "int16";
static const char kInt32[] = "int32";
static const char kInt64[] = "int64";
static const char kFloat[] = "float";
static const char kDouble[] = "double";
static const char kString[] = "string";
static const char kUnigString[] = "unig_string";
static const char kTuple[] = "tuple";

FieldInfo::FieldInfo() : type(FieldType::UnknownField), count(1), offset(-1),
  index_info(nullptr), related_table_info(nullptr) {
}

FieldInfo::FieldInfo(const std::string& n, FieldType t,
  int32_t c, int32_t off) :
  name(n), type(t), count(c), offset(off),
  index_info(nullptr), related_table_info(nullptr) {
}

FieldInfo::~FieldInfo() {
  Clear();
}


void FieldInfo::Clear() {
  name.clear();
  type = FieldType::UnknownField;
  count = 1;
  offset = -1;
  tuple_field_map.clear();
}

int32_t FieldInfo::GetFieldElementSize(const FieldInfo& field_info) {
  int32_t size = 0;
  switch (field_info.type) {
  case FieldType::Int8:
    size = sizeof(int8_t);
    break;
  case FieldType::Int16:
    size = sizeof(int16_t);
    break;
  case FieldType::Int32:
    size = sizeof(int32_t);
    break;
  case FieldType::Int64:
    size = sizeof(int64_t);
    break;
  case FieldType::Float:
    size = sizeof(float);
    break;
  case FieldType::Double:
    size = sizeof(double);
    break;
  case FieldType::String:
    size = sizeof(int64_t);
    break;
  case FieldType::UnigString:
    size = sizeof(int64_t);
    break;
  case FieldType::Tuple: {
    std::map<std::string, FieldInfo>::const_iterator it
      = field_info.tuple_field_map.begin();
    while (it != field_info.tuple_field_map.end()) {
      int32_t local_size = GetFieldSize(it->second);
      if (-1 == local_size) {
        return -1;
      }
      size += local_size;
      it++;
    }
    break;
  }
  case FieldType::Reference: {
    size = 0;
    break;
  }
  case FieldType::UnknownField:
    return -1;
  }
  return size;
}

int32_t FieldInfo::GetFieldSize(const FieldInfo& field_info) {
  if (field_info.IsMultiVar()) {
    return sizeof(int64_t);
  }
  int32_t size = GetFieldElementSize(field_info);
  if (unlikely(0 > size)) {
    return -1;
  }
  size *= field_info.count;
  return size;
}

int32_t FieldInfo::GetSize() const {
  return size;
}

int32_t FieldInfo::GetElementSize() const {
  return element_size;
}

FieldInfo* FieldInfo::GetFieldInfoByUnionName(
  const std::string& name,
  std::map<std::string, FieldInfo>* field_desc) {
  std::vector<data::Data> section_name;
  Utils::Split(name, ".", &section_name);
  FieldDesc* current_field_desc = field_desc;
  FieldDesc::iterator it = current_field_desc->end();
  for (const data::Data& section : section_name) {
    it = current_field_desc->find(section.ToString());
    if (it == current_field_desc->end()) {
      LOG_ERROR("Not found section [%s].", section.ToString().c_str());
      return nullptr;
    }
    current_field_desc = &(it->second.tuple_field_map);
  }
  return &(it->second);
}

FieldType FieldInfo::GetFieldType(const std::string& name) {
  FieldType type = FieldType::UnknownField;
  if (name == kInt8) {
    type = FieldType::Int8;
  } else if (name == kInt16) {
    type = FieldType::Int16;
  } else if (name == kInt32) {
    type = FieldType::Int32;
  } else if (name == kInt64) {
    type = FieldType::Int64;
  } else if (name == kFloat) {
    type = FieldType::Float;
  } else if (name == kDouble) {
    type = FieldType::Double;
  } else if (name == kString) {
    type = FieldType::String;
  } else if (name == kUnigString) {
    type = FieldType::UnigString;
  } else if (name == kTuple) {
    type = FieldType::Tuple;
  }
  return type;
}

bool FieldInfo::ConvertJsonFieldToString(
  const rapidjson::Value& value, std::string* result) const {
  result->clear();
  bool is_single = IsSingle() && !value.IsArray();
  bool is_multi_fix = IsMultiFix() && value.IsArray() &&
    static_cast<int32_t>(value.GetArray().Size()) == count;
  bool is_multi_var = IsMultiVar() && value.IsArray();
  if (is_single) {
    if (unlikely(!ConvertJsonSingleFieldToString(value, result))) {
      LOG_ERROR("Field [%s] convert json single field to string fail.",
        name.c_str());
      return false;
    }
  } else if (is_multi_fix || is_multi_var) {
    rapidjson::Value::ConstArray values = value.GetArray();
    for (size_t i = 0; i < values.Size(); i++) {
      if (unlikely(!ConvertJsonSingleFieldToString(values[i],
        result))) {
        LOG_ERROR("Field [%s] convert json single field to string fail.",
          name.c_str());
        return false;
      }
    }
  } else {
    LOG_ERROR("Unsupport convert condition.");
    return false;
  }
  return true;
}

bool FieldInfo::ConvertJsonSingleFieldToString(
  const rapidjson::Value& value, std::string* result) const {
  int64_t tmp = 0L;
  float tmp_float = 0.0;
  double tmp_double = 0.0;
  switch (type) {
  case FieldType::Int8:
    if (unlikely(!value.IsInt())) {
      return false;
    }
    tmp = value.GetInt();
    result->append(reinterpret_cast<const char*>(&tmp), sizeof(int8_t));
    break;
  case FieldType::Int16:
    if (unlikely(!value.IsInt())) {
      return false;
    }
    tmp = value.GetInt();
    result->append(reinterpret_cast<const char*>(&tmp), sizeof(int16_t));
    break;
  case FieldType::Int32:
    if (unlikely(!value.IsInt())) {
      return false;
    }
    tmp = value.GetInt();
    result->append(reinterpret_cast<const char*>(&tmp), sizeof(int32_t));
    break;
  case FieldType::Int64:
    if (unlikely(!value.IsInt64())) {
      return false;
    }
    tmp = value.GetInt64();
    result->append(reinterpret_cast<const char*>(&tmp), sizeof(int64_t));
    break;
  case FieldType::Float:
    if (unlikely(!value.IsFloat())) {
      return false;
    }
    tmp_float = value.GetFloat();
    result->append(reinterpret_cast<const char*>(&tmp_float), sizeof(float));
    break;
  case FieldType::Double:
    if (unlikely(!value.IsDouble())) {
      return false;
    }
    tmp_double = value.GetDouble();
    result->append(reinterpret_cast<const char*>(&tmp_double), sizeof(double));
    break;
  case FieldType::String:
    if (unlikely(!value.IsString())) {
      return false;
    }
    result->append(value.GetString());
    break;
  case FieldType::UnigString:
    if (unlikely(!value.IsString())) {
      return false;
    }
    result->append(value.GetString());
    break;
  case FieldType::Tuple:
  case FieldType::Reference:
  case FieldType::UnknownField:
    return false;
  }
  return true;
}

bool FieldInfo::ConvertStoreFormat(
  const std::string& str_value, std::string* store) const {
  store->clear();
  switch (type) {
  case FieldType::String: {
    store->assign(str_value);
    break;
  }
  case FieldType::UnigString: {
    store->assign(str_value);
    break;
  }
  case FieldType::Int8: {
    int8_t tmp = atol(str_value.c_str());
    store->assign(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
    break;
  }
  case FieldType::Int16: {
    int16_t tmp = atol(str_value.c_str());
    store->assign(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
    break;
  }
  case FieldType::Int32: {
    int32_t tmp = atol(str_value.c_str());
    store->assign(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
    break;
  }
  case FieldType::Int64: {
    int64_t tmp = atol(str_value.c_str());
    store->assign(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
    break;
  }
  case FieldType::Float: {
    float tmp = atof(str_value.c_str());
    store->assign(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
    break;
  }
  case FieldType::Double: {
    double tmp = atof(str_value.c_str());
    store->assign(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
    break;
  }
  case FieldType::Tuple:
  case FieldType::Reference:
  case FieldType::UnknownField:
    return false;
  }
  return true;
}

}
}