#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "view/popup/modelCache.hpp"

class BuildImpotedTask : public IAsyncTask<int>
{
public:
    BuildImpotedTask(const ModelCache& cache);
};


