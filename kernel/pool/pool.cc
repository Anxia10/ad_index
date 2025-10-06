#include "kernel/pool/pool.h"

namespace kernel {
namespace pool {
LOG_SETUP("kernel", Pool);

Pool::Pool() :
    store_(nullptr) {
}

Pool::~Pool() {
    Release();
}

}
}