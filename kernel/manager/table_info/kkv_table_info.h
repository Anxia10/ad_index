#pragma once
#include <string>
#include "kernel/manager/table_info/table_info.h"

namespace kernel {
namespace manager {

class KkvTableInfo : public TableInfo {
public:
    explicit KkvTableInfo(DataConfig* data_config);
    virtual ~KkvTableInfo();

    bool ParseTableConfig(const TiXmlElement* table_xml) override;
    bool ParseFieldDesc(const TiXmlElement* descriptor_xml) override;

private:
    bool ParseKkvTableConfig(const TiXmlElement* table_xml);
    bool CheckKkvFieldDesc();

private:
    LOG_DECLARE;
};

} // namespace manager
} // namespace kernel