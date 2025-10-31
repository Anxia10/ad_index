#pragma once
#include "kernel/manager/table_info/table_info.h"

namespace kernel {
namespace manager {

class ViewTableInfo : public TableInfo {
public:
    explicit ViewTableInfo(DataConfig* data_config);
    virtual ~ViewTableInfo();

private:
    LOG_DECLARE;
};

} // namespace manager
} // namespace kernel