/**
 Copyright [2021-1-28] <xuyao>
*/
#include <string>
#include <algorithm>
#include "kernel/manager/table_info/table_info_factory.h"
#include "kernel/manager/table_info/kkv_table_info.h"
#include "kernel/manager/table_info/kv_table_info.h"
#include "kernel/manager/table_info/structure_table_info.h"
#include "kernel/manager/table_info/view_table_info.h"

namespace kernel {
namespace manager {

LOG_SETUP("kernel", TableInfoFactory);

TableInfoFactory::TableInfoFactory() {
}

TableInfoFactory::~TableInfoFactory() {
}

TableInfo* TableInfoFactory::CreateTableInfo(
    const rapidjson::Value& table,
    DataConfig* data_config) {
    std::string name = table["table_name"].GetString();
    TableInfo* ret = nullptr;
    std::string table_type = kStructureTableTypeStr;
    if (table.HasMember("table_type")) {
        std::string table_type_str = table["table_type"].GetString();
        if(!table_type_str.empty()){
            table_type = table_type_str;
        }
    }
    if (table_type == kKkvTableTypeStr) {
        ret = new KkvTableInfo(data_config);
    } else if (table_type == kKvTableTypeStr) {
        ret = new KvTableInfo(data_config);
    } else if (table_type == kStructureTableTypeStr) {
        ret = new StructureTableInfo(data_config);
    } else if (table_type == kViewTableTypeStr) {
        ret = new ViewTableInfo(data_config);
    } else {
        LOG_ERROR("Unsupport table type [%s] in table [%s].",
                  table_type.c_str(), name.c_str());
        return nullptr;
    }
    TableConfig& table_config = ret->GetTableConfig();
    table_config[kTableNameXmlStr] = name;
    table_config[kTableTypeXmlStr] = table_type;
    ret->SetTableName(name);
    if (table.HasMember("clazz")) {
        std::string clazz_name = table["clazz"].GetString();
        ret->SetClazzName(clazz_name);
    }
    return ret;
}

TableInfo* TableInfoFactory::CreateTableInfo(
    const TiXmlElement* table_xml,
    DataConfig* data_config) {
    const char* tmp = nullptr;
    // name
    tmp = table_xml->Attribute(kTableNameXmlStr);
    if (tmp == nullptr) {
        LOG_ERROR("Not found table name.");
        return nullptr;
    }
    std::string table_name = tmp;
    // type
    tmp = table_xml->Attribute(kTableTypeXmlStr);
    if (tmp == nullptr) {
        LOG_ERROR("Not found table type for table.");
        return nullptr;
    }
    std::string table_type = tmp;
    TableInfo* ret = nullptr;
    if (table_type == kKkvTableTypeStr) {
        ret = new KkvTableInfo(data_config);
    } else if (table_type == kKvTableTypeStr) {
        ret = new KvTableInfo(data_config);
    } else if (table_type == kStructureTableTypeStr) {
        ret = new StructureTableInfo(data_config);
    } else if (table_type == kViewTableTypeStr) {
        ret = new ViewTableInfo(data_config);
    } else {
        LOG_ERROR("Unsupport table type [%s] in table [%s].",
                  table_type.c_str(), table_name.c_str());
        return nullptr;
    }
    TableConfig& table_config = ret->GetTableConfig();
    table_config[kTableNameXmlStr] = table_name;
    table_config[kTableTypeXmlStr] = table_type;
    ret->SetTableName(table_name);
    tmp = table_xml->Attribute(kTableClazzNameXmlStr);
    if (tmp != nullptr) {
        std::string clazz_name(tmp);
        std::replace(clazz_name.begin(), clazz_name.end(), '.', '/');
        ret->SetClazzName(clazz_name);
    }
    return ret;
}

} // namespace manager
} // namespace kernel