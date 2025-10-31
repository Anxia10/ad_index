#include <string>
#include "kernel/manager/table_info/view_table_info.h"

namespace kernel {
namespace manager {

LOG_SETUP("kernel", ViewTableInfo);

ViewTableInfo::ViewTableInfo(DataConfig* data_config) :
    TableInfo(data_config) {
    type_ = TableType::VIEW;
}

ViewTableInfo::~ViewTableInfo() {
}

} // namespace manager
} // namespace kernel