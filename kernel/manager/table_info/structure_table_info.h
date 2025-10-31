#pragma once
#include "kernel/manager/table_info/table_info.h"

namespace kernel {
namespace manager {

class StructureTableInfo : public TableInfo {
public:
    explicit StructureTableInfo(DataConfig* data_config);
    virtual ~StructureTableInfo();

    bool ParseTableConfig(const TiXmlElement* table_xml) override;
    bool ParseTableConfig(const rapidjson::Value& table) override;
    bool ParseFieldDesc(const TiXmlElement* descriptor_xml) override;

private:
    bool ParseStructureTableConfig(const TiXmlElement* table_xml);
    bool CheckStructureFieldDesc();

private:
    LOG_DECLARE;
};

} // namespace manager
} // namespace kernel