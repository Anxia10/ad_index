#pragma once

namespace kernel {
namespace index {


class Index {
    public:
        Index();
        virtual ~Index();
        virtual bool Init() = 0;
        virtual bool Insert();
        virtual bool Delete();
        virtual bool Search();

    protected:

};

}
}