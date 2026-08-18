#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "view/checkupdate.hpp"

class CheckUpdateTask : public IAsyncTask<int>
{
public:
    CheckUpdateTask(CheckUpdate* view);

private:
    CheckUpdate* m_view;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(Result<int> result) override;

    bool CopyUpdateFile(const std::string& src_dir, const std::string& dst_dir);

    uint64_t GetHashFromFile(const std::string& file);
};

