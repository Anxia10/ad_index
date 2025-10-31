#include <utility>
#include <set>
#include <string>
#include "kernel/manager/table_info/kv_table_info.h"
#include "kernel/common/utils.h"

namespace kernel {
namespace manager {

LOG_SETUP("kernel", KvTableInfo);

KvTableInfo::KvTableInfo(DataConfig* data_config) :
    TableInfo(data_config) {
    type_ = TableType::KV;
}

KvTableInfo::~KvTableInfo() {
}

bool KvTableInfo::ParseTableConfig(const TiXmlElement* table_xml) {
    if (!TableInfo::ParseTableConfig(table_xml)) {
        LOG_ERROR("Table info parse table config fail.");
        return false;
    }
    return ParseKvTableConfig(table_xml);
}

bool KvTableInfo::ParseKvTableConfig(const TiXmlElement* table_xml) {
    const char* tmp = nullptr;
    tmp = table_xml->Attribute(kKvTableKeyXmlStr);
    if (tmp == nullptr) {
        LOG_ERROR("Not found key for kv table.");
        return false;
    }
    table_config_.insert(
        std::pair<std::string, std::string>(
            kKvTableKeyXmlStr, tmp));

    tmp = table_xml->Attribute(kKvTableValueXmlStr);
    if (tmp == nullptr) {
        LOG_ERROR("Not found value for kv table.");
        return false;
    }
    table_config_.insert(
        std::pair<std::string, std::string>(
            kKvTableValueXmlStr, tmp));
    return true;
}

bool KvTableInfo::ParseFieldDesc(
    const TiXmlElement* descriptor_xml) {
    if (!TableInfo::ParseFieldDesc(descriptor_xml)) {
        LOG_ERROR("Table info parse field desc fail.");
        return false;
    }
    return CheckKvFieldDesc();
}

bool KvTableInfo::CheckKvFieldDesc() {
    const std::string& key_name =
        table_config_.find(kKvTableKeyXmlStr)->second;
    const std::string& value_name =
        table_config_.find(kKvTableValueXmlStr)->second;
    if (field_desc_.find(key_name) == field_desc_.end()) {
        LOG_ERROR("Not found key field for kv table.");
        return false;
    }
    if (field_desc_.find(value_name) == field_desc_.end()) {
        LOG_ERROR("Not found value field for kv table.");
        return false;
    }
    return true;
}

} // namespace manager
} // namespace kernel