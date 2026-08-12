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

int CheckUpdateTask::doInBackground()
{
    std::string source_dir = gfile::get_current_source_dir() + "/bin";

    std::string source_file = source_dir + "/manifest.txt";

    uint64_t source_hash = GetHashFromFile(source_file);
    if (source_hash == (uint64_t)-1)
        throw std::runtime_error("Manifest source do not found.");

    std::string manifest = gfile::get_data_directory() + "/manifest.txt";
    if (!gfile::exists(manifest))
    {
        if (copydata(source_dir))
            return doInBackground();
        throw std::runtime_error("[CheckUpdate] fail in to copy source dir to dest.");
    }

    std::ifstream is(source_file);
    
    uint64_t src_hash;
    std::string path;
    while ((is >> src_hash >> path))
    {
        std::string src_file = source_dir + "/resources" + path;
        if (!gfile::exists(src_file))
        {
            // throw std::runtime_error(String::format("File '%s' not found in source, ignored.", src_file.c_str()));
            continue;
        }

        std::string dest_file = gfile::get_data_directory() + path;

        if (gfile::exists(dest_file))
        {
            uint64_t dest_hash = GetHashFromFile(dest_file);

            if (src_hash != dest_hash)
            {
                if (gfile::exists(dest_file))
                    gfile::remove(dest_file);

                gfile::copy(src_file.c_str(), dest_file.c_str());
            }
        } else
        {
            try
            {
                gfile::copy(src_file.c_str(), dest_file.c_str());
            } catch (const std::exception& e)
            {
                is.close();

                if (copydata(source_dir))
                    return doInBackground();
            }
        }
    }
    is.close();

    if (gfile::exists(manifest))
        gfile::remove(manifest);

    gfile::copy("./bin/manifest.txt", manifest.c_str());
    return 0;
}

void CheckUpdateTask::onPostExecute(Result<int> result)
{
    m_view->taskFinished();
}

bool CheckUpdateTask::copydata(const std::string& src_dir)
{
    if (!gfile::copy((src_dir + "/resources/assets").c_str(), (gfile::get_data_directory() + "/assets").c_str()))
        return false;

    if (!gfile::copy((src_dir + "/resources/script").c_str(), (gfile::get_data_directory() + "/script").c_str()))
        return false;

    if (!gfile::copy((src_dir + "/resources/example").c_str(), (gfile::get_data_directory() + "/example").c_str()))
        return false;

    if (!gfile::copy((src_dir + "/manifest.txt").c_str(), (gfile::get_data_directory() + "/manifest.txt").c_str()))
        return false;
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

