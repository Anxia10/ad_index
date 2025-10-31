#include <utility>
#include <set>
#include <string>
#include "kernel/manager/table_info/kkv_table_info.h"
#include "kernel/common/utils.h"

namespace kernel {
namespace manager {

LOG_SETUP("kernel", KkvTableInfo);

KkvTableInfo::KkvTableInfo(DataConfig* data_config) :
    TableInfo(data_config) {
    type_ = TableType::KKV;
}

KkvTableInfo::~KkvTableInfo() {
}

bool KkvTableInfo::ParseTableConfig(const TiXmlElement* table_xml) {
    if (!TableInfo::ParseTableConfig(table_xml)) {
        LOG_ERROR("Table info parse table config fail.");
        return false;
    }
    return ParseKkvTableConfig(table_xml);
}

bool KkvTableInfo::ParseKkvTableConfig(const TiXmlElement* table_xml) {
    const char* tmp = nullptr;
    tmp = table_xml->Attribute(kKkvTablePkeyXmlStr);
    if (tmp == nullptr) {
        LOG_ERROR("Not found pkey for kkv table.");
        return false;
    }
    table_config_.insert(
        std::pair<std::string, std::string>(
            kKkvTablePkeyXmlStr, tmp));

    tmp = table_xml->Attribute(kKkvTableSkeyXmlStr);
    if (tmp == nullptr) {
        LOG_ERROR("Not found skey for kkv table.");
        return false;
    }
    table_config_.insert(
        std::pair<std::string, std::string>(
            kKkvTableSkeyXmlStr, tmp));

    tmp = table_xml->Attribute(kKkvTableValueXmlStr);
    if (tmp == nullptr) {
        LOG_ERROR("Not found value for kkv table.");
        return false;
    }
    table_config_.insert(
        std::pair<std::string, std::string>(
            kKkvTableValueXmlStr, tmp));
    return true;
}

bool KkvTableInfo::ParseFieldDesc(
    const TiXmlElement* descriptor_xml) {
    if (!TableInfo::ParseFieldDesc(descriptor_xml)) {
        LOG_ERROR("Table info parse field desc fail.");
        return false;
    }
    return CheckKkvFieldDesc();
}

bool KkvTableInfo::CheckKkvFieldDesc() {
    const std::string& pkey_name =
        table_config_.find(kKkvTablePkeyXmlStr)->second;
    const std::string& skey_name =
        table_config_.find(kKkvTableSkeyXmlStr)->second;
    const std::string& value_name =
        table_config_.find(kKkvTableValueXmlStr)->second;
    if (field_desc_.find(pkey_name) == field_desc_.end()) {
        LOG_ERROR("Not found pkey[%s] field for kkv table.", pkey_name.c_str());
        return false;
    }
    if (field_desc_.find(skey_name) == field_desc_.end()) {
        LOG_ERROR("Not found skey field for kkv table.");
        return false;
    }
    if (field_desc_.find(value_name) == field_desc_.end()) {
        LOG_ERROR("Not found value field for kkv table.");
        return false;
    }
    return true;
}

} // namespace manager
} // namespace kernel