#pragma once
#include "sophon/manager/table_info/table_info.h"

namespace kernel {
namespace manager {

class TableInfoFactory {
public:
    static TableInfo* CreateTableInfo(const TiXmlElement* table_xml,
                                     DataConfig* data_config);
    static TableInfo* CreateTableInfo(const rapidjson::Value& table,
                                     DataConfig* data_config);

private:
    TableInfoFactory();
    ~TableInfoFactory();

private:
    LOG_DECLARE;
};

} // namespace manager
} // namespace kernel

/* vim:set filetype=cpp: */