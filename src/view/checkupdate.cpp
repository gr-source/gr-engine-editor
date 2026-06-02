#include "view/checkupdate.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/thread/threadpool.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/ecs/default.hpp"
#include "imgui.h"

#include <exception>
#include <fstream>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>

#include <sys/stat.h>

void CheckUpdate::init()
{
    Threadpool::AddTask(&CheckUpdate::doInBackground, this);
}

// private
void CheckUpdate::onRenderUI()
{
    auto viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowPos({});
    ImGui::Begin("Check Update", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    const char* text = "Checking resources...";

    ImGui::SetCursorPos(viewport->GetWorkCenter() - (ImGui::CalcTextSize(text) / 2.0f));
    ImGui::Text("%s", text);

    ImGui::End();
}

#include <iostream>
void CheckUpdate::doInBackground()
{
    std::string source_dir = "./bin";

    std::string source_file = source_dir + "/manifest.txt";

    uint64_t source_hash = GetHashFromFile(source_file);
    if (source_hash == (uint64_t)-1)
        return logger::critical("[CheckUpdate] Manifest source do not found.");

    std::string manifest = gfile::get_data_directory() + "/manifest.txt";
    if (!gfile::exists(manifest))
    {
        if (copydata(source_dir))
            return doInBackground();
        return logger::critical("[CheckUpdate] fail in to copy source dir to dest.");
    }

    std::ifstream is(source_file);
    
    uint64_t src_hash;
    std::string path;
    while ((is >> src_hash >> path))
    {
        std::string src_file = source_dir + "/resources" + path;
        if (!gfile::exists(src_file))
        {
            logger::warn("[CheckUpdate] File '%s' not found in source, ignored.", src_file.c_str());
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

    MainThreadDispatcher::Add(Priority::NORMAL, m_function);
}

bool CheckUpdate::copydata(const std::string& src_dir)
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

uint64_t CheckUpdate::GetHashFromFile(const std::string& file)
{
    size_t length = 0;
    uint8_t* buffer = gfile::read(file.c_str(), &length);
    if (buffer == NULL || length == 0)
        return (uint64_t)-1;

    Hash64 hash = hash::fnv1_buff(buffer, length);

    free(buffer);

    return hash;
}


