#pragma once
#include <string>
#include <map>
#include <set>
#include <unordered_set>
#include "kernel/common/log/log.h"
#include "tinyxml.h"
#include "rapidjson/document.h"
#include "kernel/manager/table_info/field_info.h"
#include "kernel/manager/table_info/index_info.h"
#include <google/protobuf/descriptor.h>

namespace kernel {
namespace manager {

// v2
extern const char kWorkerDepsPath[];
extern const char kUnitTestDepsPath[];
extern const char HanguFullDataPath[];
extern const char kTables[];
extern const char kKey[];
extern const char kName[];
extern const char kClazz[];
extern const char kDataPath[];
extern const char kProtoFile[];
extern const char kPartitionNum[];
extern const char kRelated[];
extern const char kInc[];
extern const char kReloadType[];
extern const char kBuildOption[];
extern const char kAliasFileName[];
extern const char kLogConfig[];
extern const char kRoaringBitmapStr[];

// table
extern const char kTableNameXmlStr[];
extern const char kTableClazzNameXmlStr[];
extern const char kTableTypeXmlStr[];
extern const char kFieldDescriptorXmlStr[];
extern const char kKernelConfigXmlStr[];

// kkv table
extern const char kKkvTableTypeStr[];
extern const char kKkvTablePkeyXmlStr[];
extern const char kKkvTableSkeyXmlStr[];
extern const char kKkvTableValueXmlStr[];

// kv table
extern const char kKvTableTypeStr[];
extern const char kKvTableKeyXmlStr[];
extern const char kKvTableValueXmlStr[];

// structure table
extern const char kStructureTableTypeStr[];
extern const char kStructureTableKeyXmlStr[];
extern const char kStructureTableRelatedXmlStr[];
extern const char kStructureTableFieldNameXmlStr[];

// view table
extern const char kViewTableTypeStr[];

// field
extern const char kFieldXmlStr[];
extern const char kFieldNameXmlStr[];
extern const char kFieldTypeXmlStr[];
extern const char kFieldCountXmlStr[];
extern const char kFieldClazzNameXmlStr[];

// forward
extern const char kForwardXmlStr[];

// inverted
extern const char kInvertedXmlStr[];
extern const char kIndexXmlStr[];
extern const char kIndexNameXmlStr[];
extern const char kIndexTypeXmlStr[];
extern const char kIndexPayloadXmlStr[];
extern const char kIndexSegmentLevelXmlStr[];

// inverted belong faiss
extern const char kMetric[];
extern const char kIndexSecondaryType[];
extern const char kNcentroids[];
extern const char kBucket[];
extern const char kDimension[];
extern const char kTypeIndexThreshold[];
extern const char kTypeIndexPQ[];
extern const char kTypeIndexIVFFlat[];
extern const char kTypeIndexFaissHnsw[];
extern const char kTypeIndexHnswlib[];
extern const char kTypeIndexFaiss[];

// field type
extern const char kForeign[];
extern const char kReference[];

typedef std::map<std::string, std::string> TableConfig;
typedef std::map<std::string, std::string> KernelConfig;
typedef std::map<std::string, FieldInfo*> RelatedFieldMap;

class DataConfig;

enum TableType {
  KKV = 0,
  KV = 1,
  STRUCTURE = 2,
  VIEW = 3,
  Unknown = 4
};

class TableInfo {
public:
  static int32_t CalculateBlockSize(const FieldDesc& field_desc,
    const std::set<std::string>& check_list = std::set<std::string>());
  explicit TableInfo(DataConfig* data_config);
  virtual ~TableInfo();

  TableConfig& GetTableConfig() {
    return table_config_;
  }
  FieldDesc& GetFieldDesc() {
    return field_desc_;
  }
  KernelConfig& GetKernelConfig() {
    return kernel_config_;
  }
  IndexDesc& GetIndexDesc() {
    return index_desc_;
  }
  void SetTableName(const std::string& name) {
    name_ = name;
  }
  const std::string& GetTableName() const {
    return name_;
  }
  void SetClazzName(const std::string& name) {
    clazz_name_ = name;
  }
  const std::string& GetClazzName() const {
    return clazz_name_;
  }
  TableType GetTableType() {
    return type_;
  }
  RelatedFieldMap& GetRelatedFieldMap() {
    return related_field_map_;
  }

  virtual bool ParseTableConfig(const rapidjson::Value& table);
  virtual bool ParseKernelConfig(const rapidjson::Value& kernel_config);
  virtual int32_t ParseFieldDescriptor(const google::protobuf::FieldDescriptor* field,
    FieldInfo* field_info, int32_t& total_size);
  virtual bool ParseTableConfig(const TiXmlElement* table_xml);
  virtual bool ParseFieldDescriptor(const TiXmlElement* descriptor_xml);
  virtual bool ParseInverted(const TiXmlElement* inverted_xml);
  virtual bool ParseInverted(const rapidjson::Value& table);
  virtual bool ParseKernelConfig(const TiXmlElement* kernel_xml);


    protected:
  int32_t BuildFieldInfo(const TiXmlElement* field_xml,
    FieldInfo* field_info);
  int32_t BuildTupleInfo(const TiXmlElement* field_xml,
    FieldInfo* field_info);

private:
  bool BuildBasicField(const TiXmlElement* field_xml, FieldInfo* field_info);
  bool BuildForeignField(const TiXmlElement* field_xml, FieldInfo* field_info);
  bool BuildReferenceField(const TiXmlElement* field_xml,
    FieldInfo* field_info);
  bool IsFaiss(const std::string& type);

protected:
  std::string name_;
  std::string clazz_name_;
  TableType type_;
  TableConfig table_config_;
  FieldDesc field_desc_;
  IndexDesc index_desc_;
  KernelConfig kernel_config_;
  DataConfig* data_config_;
  std::map<std::string, FieldInfo*> related_field_map_;
  std::unordered_set<std::string> faiss_index_set_;

private:
  std::vector<google::protobuf::Descriptor*> descriptor_;
  LOG_DECLARE;
};

} // namespace manager
} // namespace kernel