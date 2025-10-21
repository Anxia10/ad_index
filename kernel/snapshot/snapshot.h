#pragma once
#include <functional>
#include <vector>
#include <string>
#include <set>
#include "kernel/common/utils/string_util.h"
#include "kernel/manager/table_info/table_info.h"
// #include "sophon/snapshot/snapshot_segment.h"
// #include "sophon/snapshot/jni_info.h"
// #include "sophon/snapshot/doc.h"

namespace kernel {
namespace manager {
class DataManager;
}
namespace pool {
class Pool;
class MMapPool;
}
namespace index {
class Index;
}
namespace snapshot {

// common
extern const char kTrue[];
extern const char kFalse[];
extern const char kIndex[];
extern const char kStore[];
extern const char kPool[];
extern const char kType[];
extern const char kReadOnWrite[];
extern const char kSegmentLevel[];
extern const char kThreadQueueSize[];
extern const char kSize[];
// kv kkv
extern const char kKv[];
extern const char kKkv[];
extern const char kPrimary[];
extern const char kSecondary[];
// structure
extern const char kStructure[];
extern const char kForward[];
extern const char kInverted[];
extern const char kUnigStr[];
extern const char kTerm[];
extern const char kDocList[];
extern const char kFixStore[];
extern const char kVarStore[];
extern const char kInvertedStore[];
// faiss
extern const char kFaissMetricL2[];
extern const char kFaissIndexIvfFlat[];
extern const int32_t kFaissDefaultNcentroids;
// hash index
extern const char kNurMurHash[];
extern const char kHashBucketNum[];
// sort index
extern const char kSort[];
extern const char kSortBsLevel[];
extern const char kSortInitRecordNum[];
// bplus tree
extern const char kBplusTree[];
// mmap pool
extern const char kFilePool[];
extern const char kMMapPool[];
extern const char kVarLengthPool[];
extern const char kFixLengthPool[];
extern const char kABPool[];
extern const char kMinExpandSize[];
extern const char kReuseTimegap[];
extern const char kFixValueSize[];
extern const char kMMapPreLoad[];
extern const char kMMapMemoryLock[];

/**
 * @brief
 * faiss 搜索输入对象
 */
struct FaissSearchParam {
  int64_t n; // parallel number
  std::vector<float> vec; // query vector
  int64_t k; // top k
  std::vector<float> distances; // distances
  std::vector<int64_t> labels; // ivf ids
  size_t nprobe;
  FaissSearchParam(std::vector<float>& vec) : n(1), vec(vec), k(10), nprobe(4) {}
  FaissSearchParam() : n(1), k(10) {}
  std::string ToString() {
    return "n=" + std::to_string(n)
      + ",k=" + std::to_string(k)
      + ",query=" + kernel::util::VectorJoin(vec, ",").c_str()
      + ",distances=" + kernel::util::VectorJoin(distances, ",").c_str()
      + ",nprobe=" + std::to_string(nprobe)
      + ",labels=" + kernel::util::VectorJoin(labels, ",").c_str();
  }
};

class Snapshot {
public:
  static constexpr uint32_t kSegmentHashSeed = 0x8611B5F3;
  explicit Snapshot(manager::TableInfo* table_info = nullptr,
    manager::DataManager* data_manager = nullptr);
  virtual ~Snapshot();
  virtual bool Init(const std::string& table_name,
    const std::string& dir,
    bool read_only = true,
    const std::string& table_file = std::string()) = 0;
  bool InitJniInfo(JNIEnv* env);
  virtual void Release() = 0;
  int64_t GetTotalDocNum();
  int32_t GetSegmentId(const void* key, int32_t len);
  int64_t MakeDocId(int64_t offset, int32_t segment_id);
  DocStoreInfo GetDocStoreInfo(int64_t doc_id);
  bool ConvertDocToJson(int64_t doc_id, rapidjson::Document* doc);
  const std::string& GetTableName() {
    return table_name_;
  }
  void SetSegmentLevel(int32_t segment_level) {
    segment_level_ = segment_level;
  }
  int32_t GetSegmentLevel() {
    return segment_level_;
  }
  int32_t GetSegmentCount() {
    return (1 << segment_level_);
  }
  std::string GetVersion() {
    return version_;
  }
  void SetVersion(const std::string& version) {
    version_ = version;
  }
  void SetTableInfo(manager::TableInfo* table_info) {
    table_info_ = table_info;
  }
  manager::TableInfo& GetTableInfo() {
    return *table_info_;
  }
  jni::JniInfoMap& GetJniInfoMap() {
    return jni_info_map_;
  }
  jni::JniTableInfo& GetJniTableInfo() {
    return jni_table_info_;
  }
  const SnapshotSegment* GetSegment(int32_t segment_id) const {
    if (segment_id >= static_cast<int32_t>(segment_vec_.size())) {
      return nullptr;
    }
    return segment_vec_[segment_id];
  }
  int64_t GetSegmentIdFromDocId(int64_t doc_id) const {
    if (segment_level_ == 0) return 0L;
    return static_cast<uint64_t>(doc_id) >> (64 - segment_level_);
  }
  int64_t GetOffsetFromDocId(int64_t doc_id) const {
    return (doc_id & (-1UL >> segment_level_));
  }
  const manager::FieldInfo* GetKeyFieldInfo() {
    return key_field_info_;
  }
  manager::DataManager* GetDataManager() {
    return data_manager_;
  }
  void SetFailedCallBack(
    std::function<void(const std::string&,
      const std::string&, const std::string&)>& cb) {
    failed_cb_ = cb;
  }

protected:
  bool InitJniInfo(JNIEnv* env, jclass j_class,
    const kernel::manager::FieldInfo* field_info, jni::JniFieldInfo* jni_info);
  bool TestDirectory(const std::string& dir, bool create);
  index::Index* InitIndex(const std::string& prefix,
    const manager::KernelConfig& kernel_config,
    const std::set<std::string>& check_list = std::set<std::string>());
  pool::Pool* InitPool(const std::string& prefix,
    const manager::KernelConfig& kernel_config,
    const std::set<std::string>& check_list = std::set<std::string>());
  bool ConfigPool(const std::string& prefix,
    const manager::KernelConfig& kernel_config, pool::Pool* pool);
  bool ConvertFieldToJson(const manager::FieldInfo& field_info,
    char* addr, pool::MMapPool* var_store,
    rapidjson::Document::AllocatorType* allocator,
    rapidjson::Value* value);
  bool ConvertSingleFieldToJson(const manager::FieldInfo& field_info,
    char* addr, pool::MMapPool* var_store,
    rapidjson::Document::AllocatorType* allocator,
    rapidjson::Value* value);
  bool ConvertMultiFieldToJson(const manager::FieldInfo& field_info,
    char* addr, pool::MMapPool* var_store,
    rapidjson::Document::AllocatorType* allocator,
    rapidjson::Value* value);

protected:
  std::string version_ = "-1";
  std::function<void(const std::string&,
    const std::string&, const std::string&)> failed_cb_;
  manager::TableInfo* table_info_;
  manager::DataManager* data_manager_;
  int32_t segment_level_;
  size_t thread_queue_size_;
  bool read_on_write_;
  const manager::FieldInfo* key_field_info_;
  int32_t block_size_;
  std::string table_name_;
  std::vector<SnapshotSegment*> segment_vec_;
  jni::JniInfoMap jni_info_map_;
  jni::JniTableInfo jni_table_info_;

private:
  LOG_DECLARE;
};

} // namespace snapshot
} // namespace kernel