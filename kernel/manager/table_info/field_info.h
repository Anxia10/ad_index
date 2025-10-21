#pragma once
#include <map>
#include <string>
#include "kernel/common/log/log.h"
#include "rapidjson/document.h"

namespace kernel {
namespace manager {

enum FieldType {
  Int8 = 0,
  Int16 = 1,
  Int32 = 2,
  Int64 = 3,
  Float = 4,
  Double = 5,
  String = 6,
  Tuple = 7,
  Reference = 8,
  UnigString = 9,
  UnknownField = 10,
};

struct IndexInfo;
struct TableInfo;
struct FieldInfo {
  std::string name;
  FieldType type;
  int32_t count;
  int32_t offset;
  int32_t size;
  int32_t element_size;
  IndexInfo* index_info;
  std::map<std::string, FieldInfo> tuple_field_map;
  std::string clazz_name;
  std::string related_field_name;
  std::string related_table_name;
  TableInfo* related_table_info;

  static FieldInfo* GetFieldInfoByUnionName(
    const std::string& name,
    std::map<std::string, FieldInfo>* field_desc);
  static int32_t GetFieldSize(const FieldInfo& field_info);
  static int32_t GetFieldElementSize(const FieldInfo& field_info);
  static FieldType GetFieldType(const std::string& name);

  FieldInfo();
  explicit FieldInfo(const std::string& n,
    FieldType t,
    int32_t c = 1, int32_t off = -1);
  ~FieldInfo();
  void Clear();

  bool IsReference() const {
    return type == FieldType::Reference;
  }
  bool IsRelated() const {
    return related_table_info != nullptr;
  }
  bool IsInverted() const {
    return index_info != nullptr;
  }
  bool IsSingle() const {
    return count == 1;
  }
  bool IsMultiFix() const {
    return count > 1;
  }
  bool IsFix() const {
    return IsSingle() || IsMultiFix();
  }
  bool IsMultiVar() const {
    return count == 0;
  }
  bool IsMulti() const {
    return IsMultiFix() || IsMultiVar();
  }
#define TYPE_CHECK_FUNC(TypeName) \
bool Is##TypeName() const { \
return type == FieldType::TypeName; \
}
  TYPE_CHECK_FUNC(Int8);
  TYPE_CHECK_FUNC(Int16);
  TYPE_CHECK_FUNC(Int32);
  TYPE_CHECK_FUNC(Int64);
  TYPE_CHECK_FUNC(Float);
  TYPE_CHECK_FUNC(Double);
  TYPE_CHECK_FUNC(String);
  TYPE_CHECK_FUNC(UnigString);
  TYPE_CHECK_FUNC(Tuple);
#undef TYPE_CHECK_FUNC
  bool IsUnknown() const {
    return type == FieldType::UnknownField;
  }
  int32_t GetSize() const;
  int32_t GetElementSize() const;
  bool ConvertJsonFieldToString(const rapidjson::Value& value,
    std::string* result) const;
  bool ConvertJsonSingleFieldToString(const rapidjson::Value& value,
    std::string* result) const;
  bool ConvertStoreFormat(const std::string& str_value,
    std::string* store) const;
  const char* GetJniSignature() const {
    switch (type) {
    case Int8:
      return IsSingle() ? "B" : "[B";
    case Int16:
      return IsSingle() ? "S" : "[S";
    case Int32:
      return IsSingle() ? "I" : "[I";
    case Int64:
      return IsSingle() ? "J" : "[J";
    case Float:
      return IsSingle() ? "F" : "[F";
    case Double:
      return IsSingle() ? "D" : "[D";
    case String:
      return IsSingle() ? "Ljava/lang/String;" : "Ljava/util/List;";
    default:
      break;
    }
    return nullptr;
  }

private:
  LOG_DECLARE;
};

typedef std::map<std::string, FieldInfo> FieldDesc;

} // namespace manager
} // namespace kernel