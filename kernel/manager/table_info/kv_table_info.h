#pragma once
#include <string>
#include "kernel/manager/table_info/table_info.h"

namespace kernel {
namespace manager {

class KvTableInfo : public TableInfo {
public:
    explicit KvTableInfo(DataConfig* data_config);
    virtual ~KvTableInfo();

    bool ParseTableConfig(const TiXmlElement* table_xml) override;
    bool ParseFieldDesc(const TiXmlElement* descriptor_xml) override;

private:
    bool ParseKvTableConfig(const TiXmlElement* table_xml);
    bool CheckKvFieldDesc();

private:
    LOG_DECLARE;
};

} // namespace manager
} // namespace kernel