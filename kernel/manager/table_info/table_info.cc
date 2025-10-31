/*
Copyright [2021-1-21] <xuyao>
*/
#include <vector>
#include <utility>
#include <set>
#include <algorithm>
#include "kernel/manager/table_info/table_info.h"
#include "kernel/manager/data_config.h"
#include "kernel/manager/table_info/kv_table_info.h"
#include "kernel/manager/table_info/kkv_table_info.h"
#include "kernel/manager/table_info/structure_table_info.h"
#include "hangu/kernel_option.pb.h"
#include "kernel/common/utils.h"
#include "kernel/common/string_util.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.pb.h>

namespace kernel {
namespace manager {

using google::protobuf::FieldDescriptor;
LOG_SETUP("kernel", TableInfo);

#define RETURN_FALSE_IF_NOT(X, MSG) \
    do { \
        if (!(X)) { \
            LOG_ERROR(MSG); \
            return false; \
        } \
    } while (false)

// v2
const char kWorkerDepsPath[] = "/opt/meituan/hangu_workspace/deps/";
const char kUnitTestDepsPath[] = "sophon/manager/test/testdata/";
const char HanguFullDataPath[] = "/opt/meituan/fulldata/hangu/v2/";
const char kTables[] = "tables";
const char kKey[] = "key";
const char kTableNameJsonStr[] = "table_name";
const char kClazz[] = "clazz";
const char kDataPath[] = "data_path";
const char kProtoFile[] = "proto_file";
const char kPartitionNum[] = "partition_num";
const char kRelated[] = "related";
const char kInc[] = "inc";
const char kReloadType[] = "reload_type";
const char kBuildOption[] = "build_option";
const char kAliasFileName[] = "alias_name";
const char kLogConfig[] = "log_config";
const char kIndexesJsonStr[] = "indexs";
const char kIndexTypeJsonStr[] = "index_type";
const char kBplusTreeJsonStr[] = "b+tree";
const char kRoaringBitmapStr[] = "roaring_bitmap";
// table
const char kTableNameXmlStr[] = "name";
const char kTableClazzNameXmlStr[] = "clazz";
const char kTableTypeXmlStr[] = "type";
const char kFieldDescriptorXmlStr[] = "descriptor";
const char kKernelConfigXmlStr[] = "kernel";
// kkv table
const char kKkvTableTypeStr[] = "kkv";
const char kKkvTablePkeyXmlStr[] = "pkey";
const char kKkvTableSkeyXmlStr[] = "skey";
const char kKkvTableValueXmlStr[] = "value";
// kv table
const char kKvTableTypeStr[] = "kv";
const char kKvTableKeyXmlStr[] = "key";
const char kKvTableValueXmlStr[] = "value";
// structure table
const char kStructureTableTypeStr[] = "structure";
const char kStructureTableKeyXmlStr[] = "key";
const char kStructureTableRelatedXmlStr[] = "related";
const char kStructureTableFieldNameXmlStr[] = "field_name";
// view table
const char kViewTableTypeStr[] = "view";
// field
const char kFieldXmlStr[] = "field";
const char kTupleXmlStr[] = "tuple";
const char kFieldNameXmlStr[] = "name";
const char kFieldTypeXmlStr[] = "type";
const char kFieldCountXmlStr[] = "count";
const char kFieldClazzNameXmlStr[] = "clazz";
// forward
const char kForwardXmlStr[] = "forward";
// inverted
const char kInvertedXmlStr[] = "inverted";
const char kIndexXmlStr[] = "index";
const char kIndexNameXmlStr[] = "name";
const char kIndexTypeXmlStr[] = "type";
const char kIndexSegmentLevelXmlStr[] = "segment_level";
// inverted belong faiss
const char kMetric[] = "metric";
const char kIndexSecondaryType[] = "secondary_type";
const char kNcentroids[] = "ncentroids";
const char kBucket[] = "bucket";
const char kDimension[] = "dimension";
const char kTypeIndexThreshold[] = "threshold"; //构建索引阈值，一般配合kTypeIndexSplit一起使用
const char kTypePQNbitsPerCode[] = "pq_nbits_per_code"; //表示每个子段聚为256个类，所以8bit即可表达
const char kTypePQCodeNum[] = "pq_code_num"; //code_num表达128维的向量，拆分为<code_num>个子段。
const char kTypeIndexIVFFlat[] = "IndexIVFFlat"; //ivf_flat索引
const char kTypeIndexPQ[] = "IndexPQ"; //pq索引
const char kTypeIndexFaissHnsw[] = "IndexFaissHnsw"; //hnsw索引
const char kTypeIndexHnswLib[] = "IndexHnswLib"; //hnswlib索引
const char kTypeIndexFaiss[] = "IndexBaseFaiss";
const char kEfConstruction[] = "efConstruction";
const char kEfSearch[] = "efSearch";
const char kNlinks[] = "nlinks";
// field type
const char kForeign[] = "foreign";
const char kReference[] = "reference";

TableInfo::TableInfo(DataConfig* data_config) :
    type_(TableType::Unknown),
    data_config_(data_config) {
    faiss_index_set_.insert(std::string(kTypeIndexIVFFlat));
    faiss_index_set_.insert(std::string(kTypeIndexPQ));
    faiss_index_set_.insert(std::string(kTypeIndexFaissHnsw));
    faiss_index_set_.insert(std::string(kTypeIndexHnswLib));
}

TableInfo::~TableInfo() {
}

bool TableInfo::ParseTableConfig(const TiXmlElement* table_xml) {
    return true;
}

class MyErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector {
    virtual void AddError(
        const std::string & filename,
        int line,
        int column,
        const std::string & message) {
        filename_ = filename;
        line_ = line;
        column_ = column;
        message_ = message;
    }
public:
    std::string filename_;
    int line_;
    int column_;
    std::string message_;
};

bool TableInfo::ParseTableConfig(const rapidjson::Value& table) {
    // "key" 主键
    if (!table.HasMember(kKey)) {
        // 这里先不直接return false, 允许在proto里定义key
        LOG_ERROR("Not found key for structure table, but maybe define in proto");
        // return false;
    }
    const std::string& key = table[kKey].GetString();
    table_config_[kStructureTableKeyXmlStr] = key;
    if (!table.HasMember(kProtoFile)) {
        LOG_ERROR("Not found proto_path for structure table.");
        return false;
    }
    const std::string& proto_file = table[kProtoFile].GetString();
    std::string name = table[kTableNameJsonStr].GetString();

    std::size_t last_index =  proto_file.find_last_of("/");
    const std::string& proto_file_path = proto_file.substr(0,last_index+1);
    const std::string& proto_file_name = proto_file.substr(last_index+1);

    // desc
    google::protobuf::compiler::DiskSourceTree source_tree;
    MyErrorCollector error_collector;
    auto importer_ = new google::protobuf::compiler::Importer(&source_tree, &error_collector);

    // bs use
    source_tree.MapPath("", kWorkerDepsPath);
    // for unit test
    source_tree.MapPath("", kUnitTestDepsPath);
    // online use
    source_tree.MapPath("", HanguFullDataPath + name);
    source_tree.MapPath("", proto_file_path);
    source_tree.MapPath("", "/usr/include");
    //importer_->Import("google/protobuf/descriptor.proto");
    //importer_->Import("kernel_option.proto");

    importer_->Import(proto_file_name);
    const google::protobuf::FileDescriptor * file_desc =
    importer_->pool()->FindFileByName(proto_file_name);
    if (file_desc == nullptr) {
        LOG_ERROR("get file_desc failed, reason:%s, %d, %d, %s\n",
            error_collector.filename_.c_str(), error_collector.line_,
            error_collector.column_, error_collector.message_.c_str());
        return false;
    }
    for (int i = 0; i < file_desc->message_type_count(); ++i) {
        const google::protobuf::Descriptor* desc = file_desc->message_type(i);
        std::string name = desc->options().GetExtension(table_name);
        if (name == "") {
            continue;
        }
        descriptor_.push_back(const_cast<google::protobuf::Descriptor*>(desc));
    }

    for (auto descriptor : descriptor_) {
        int32_t total_size = 0;
        for (int i = 0; i < descriptor->field_count(); ++i) {
            const google::protobuf::FieldDescriptor* field = descriptor->field(i);
            if (field->number() > 10000) { // >10000 是内置的kernel字段
                continue;
            }

            std::string fd_related_table = field->options().GetExtension(related_table);
            std::string fd_related_key = field->options().GetExtension(related_key);
            if (fd_related_table != "" && fd_related_key != "") {
                // hangu v1 没显式声明关联表，因此这里也直接跳过
                // 要求related_key 声明在前
                FieldInfo* field_info =
                FieldInfo::GetFieldInfoByUnionName(fd_related_key, &field_desc_);
                if (field_info == nullptr) {
                    LOG_ERROR("related_key:%s not found before related_table",
                        fd_related_key.c_str());
                    continue;
                } else {
                    field_info->related_table_name = fd_related_table;
                    field_info->count = 1;
                }
                related_field_map_[fd_related_table] = field_info;
                LOG_DEBUG("%s related :%s\n",
                    fd_related_key.c_str(), fd_related_table.c_str());
                continue;
            }
            std::string fd_key = field->options().GetExtension(field_type);
            if (fd_key == "key") {
            }
            const std::string& orig_name = field->name();
            FieldInfo field_info;
            field_info.clazz_name = descriptor->full_name();
            field_info.offset = total_size;
            int32_t size = ParseFieldDesc(field, &field_info, total_size);
            if (size == -1) {
                LOG_ERROR("parse field %s descriptor failed", orig_name.c_str());
                return false;
            }
            total_size += size;
            field_desc_.insert(
                std::pair<std::string, FieldInfo>(orig_name, field_info));
            LOG_DEBUG("add field:%s, count:%d, total_count:%d, offset:%d\n",
                orig_name.c_str(), field_info.count, field_desc_.size(), field_info.offset);
        }
    }
    return true;
}

int32_t TableInfo::ParseFieldDesc(
    const google::protobuf::FieldDescriptor* field,
    FieldInfo* field_info,
    int32_t& total_size) {
    std::string name = field->name();
    field_info->name = name;
    // count 定长 变长count 0
    field_info->count = 1;

    int32_t field_count = field->options().GetExtension(count);
    if (field_count > 0) {
        field_info->count = field_count;
    } else if (field->label() == google::protobuf::FieldDescriptor::Label::LABEL_REPEATED) {
        field_info->count = 0;
    }
    switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
        {
            const google::protobuf::Descriptor* descriptor = field->message_type();
            field_info->type = FieldInfo::GetFieldType("tuple");
            int32_t tmp_total_size = 0;
            for (int i = 0; i < descriptor->field_count(); ++i) {
                const google::protobuf::FieldDescriptor* sub_field = descriptor->field(i);
                if (sub_field->number() > 10000) { // >10000 是内置的kernel字段
                    continue;
                }
                FieldInfo sub_field_info;
                ParseFieldDesc(sub_field, &sub_field_info, tmp_total_size);
                sub_field_info.offset = tmp_total_size;
                tmp_total_size += sub_field_info.size;
                LOG_DEBUG("sub_field name:%s,offset:%ld\n",
                    sub_field_info.name.c_str(), sub_field_info.offset);
                field_info->tuple_field_map[sub_field_info.name] = sub_field_info;
            }
            //total_size += tmp_total_size;
        }
        break;
        case FieldDescriptor::CppType::CPPTYPE_INT32:
        {
            field_info->type = FieldInfo::GetFieldType("int32");
            std::string filed_extend_type = field->options().GetExtension(extend_type);
            if (filed_extend_type != "") {
                field_info->type = FieldInfo::GetFieldType(filed_extend_type);
            }
        }
        break;
        case FieldDescriptor::CppType::CPPTYPE_INT64:
        {
            field_info->type = FieldInfo::GetFieldType("int64");
            //LOG_DEBUG("%s type:%d", name.c_str(), field_info->type);
            std::string filed_extend_type = field->options().GetExtension(extend_type);
            if (filed_extend_type != "") {
                field_info->type = FieldInfo::GetFieldType(filed_extend_type);
            }
        }
        break;
        case FieldDescriptor::CppType::CPPTYPE_DOUBLE:
            field_info->type = FieldInfo::GetFieldType("double");
            break;
        case FieldDescriptor::CppType::CPPTYPE_FLOAT:
            field_info->type = FieldInfo::GetFieldType("float");
            break;
        case FieldDescriptor::CppType::CPPTYPE_STRING:
        {
            field_info->type = FieldInfo::GetFieldType("string");
            std::string filed_extend_flag = field->options().GetExtension(extend_flag);
            if (filed_extend_flag == "uniq_string") {
                field_info->type = FieldInfo::GetFieldType("uniq_string");
            }
        }
        break;
        default:
        {
            field_info->type = FieldType::UnknownField;
        }
        break;
    }
    int32_t size = FieldInfo::GetFieldSize(*field_info);
    field_info->size = size;
    field_info->element_size = FieldInfo::GetFieldElementSize(*field_info);
    LOG_DEBUG("field :%s, size:%ld\n", field_info->name.c_str(), field_info->size);
    return field_info->size;
}