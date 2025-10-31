#include "kernel/manager/table_info/structure_table_info.h"
#include <string>

namespace kernel {
namespace manager {

LOG_SETUP("kernel", StructureTableInfo);

StructureTableInfo::StructureTableInfo(DataConfig* data_config) :
    TableInfo(data_config) {
    type_ = TableType::STRUCTURE;
}

StructureTableInfo::~StructureTableInfo() {
}

bool StructureTableInfo::ParseTableConfig(const rapidjson::Value& table) {
    if (!TableInfo::ParseTableConfig(table)) {
        LOG_ERROR("Table info parse table config fail.");
        return false;
    }
    if (table.HasMember(kStructureTableKeyXmlStr)) {
        std::string key = table[kStructureTableKeyXmlStr].GetString();
        table_config_[kStructureTableKeyXmlStr] = key;
    }
    const std::string& key_name =
        table_config_.find(kStructureTableKeyXmlStr)->second;
    if (field_desc_.find(key_name) == field_desc_.end()) {
        LOG_ERROR("Not found key field for structure table [%s].",
                  GetTableName().c_str());
        return false;
    }
    return true;
}

bool StructureTableInfo::ParseTableConfig(const TiXmlElement* table_xml) {
    if (!TableInfo::ParseTableConfig(table_xml)) {
        LOG_ERROR("Table info parse table config fail.");
        return false;
    }
    return ParseStructureTableConfig(table_xml);
}

bool StructureTableInfo::ParseStructureTableConfig(
    const TiXmlElement* table_xml) {
    const char* tmp = nullptr;
    tmp = table_xml->Attribute(kStructureTableKeyXmlStr);
    if (tmp == nullptr) {
        LOG_ERROR("Not found key for structure table.");
        return false;
    }
    table_config_[kStructureTableKeyXmlStr] = tmp;
    return true;
}

bool StructureTableInfo::ParseFieldDesc(
    const TiXmlElement* descriptor_xml) {
    if (!TableInfo::ParseFieldDesc(descriptor_xml)) {
        LOG_ERROR("Table info parse field desc fail.");
        return false;
    }
    return CheckStructureFieldDesc();
}

bool StructureTableInfo::CheckStructureFieldDesc() {
    const std::string& key_name =
        table_config_.find(kStructureTableKeyXmlStr)->second;
    if (field_desc_.find(key_name) == field_desc_.end()) {
        LOG_ERROR("Not found key field for structure table [%s].",
                  GetTableName().c_str());
        return false;
    }
    return true;
}

} // namespace manager
} // namespace kernel