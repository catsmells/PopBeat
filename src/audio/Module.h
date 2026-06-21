#pragma once
#include <vector>
#include "Param.h"
class Module {
public:
    virtual ~Module() = default;
    virtual void reset() = 0;
    virtual void collectParams(std::vector<Param>& /*out*/) {}
};
