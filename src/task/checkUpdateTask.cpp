#include "task/checkUpdateTask.hpp"

#include <gr-base/externals/hash.hpp>

#include <gr-base/file/file.hpp>

#include <stdexcept>
#include <fstream>

CheckUpdateTask::CheckUpdateTask(CheckUpdate* view) : m_view(view)
{}

void CheckUpdateTask::onPreExecute()
{
    m_view->addFile(gfile::get_current_source_dir());
}

#include <filesystem>
#include <iostream>

int CheckUpdateTask::doInBackground()
{
    std::string src_dir = gfile::get_current_source_dir() + "/bin/resources";
    std::string dst_dir = gfile::get_data_directory();

    return CopyUpdateFile(src_dir, dst_dir);
}

void CheckUpdateTask::onPostExecute(Result<int> result)
{
    m_view->taskFinished();
}

bool CheckUpdateTask::CopyUpdateFile(const std::string& src_dir, const std::string& dst_dir)
{
    if (!gfile::exists(src_dir))
        return false;

    if (gfile::is_directory(src_dir))
    {
        if (!gfile::exists(dst_dir))
            gfile::create_directory(dst_dir);

        for (auto& entry : std::filesystem::directory_iterator(src_dir))
        {
            std::string current = dst_dir + "/" + entry.path().filename().string();

            CopyUpdateFile(entry.path(), current);
        }
    } else
    {
        if (gfile::exists(dst_dir))
        {
            uint64_t src_hash =
                GetHashFromFile(src_dir);

            uint64_t dst_hash =
                GetHashFromFile(dst_dir);

            if (src_hash != dst_hash)
            {
                gfile::remove(dst_dir);

                std::filesystem::copy_file(src_dir, dst_dir);
            }
        } else
        {
            std::filesystem::copy_file(src_dir, dst_dir);
        }
    }
    return true;
}

uint64_t CheckUpdateTask::GetHashFromFile(const std::string& file)
{
    size_t length = 0;
    uint8_t* buffer = gfile::read(file.c_str(), &length);
    if (buffer == NULL || length == 0)
        return (uint64_t)-1;

    Hash64 hash = engine::hash::fnv1_buff(buffer, length);

    free(buffer);

    return hash;
}

