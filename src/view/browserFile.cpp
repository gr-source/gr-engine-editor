#include "view/browserFile.hpp"

#include "gr-base/asset/assetDB.hpp"
#include "gr-base/asset/assetResolver.hpp"
#include "gr-base/core/file/file.hpp"
#include "imgui.h"

#include <filesystem>

#include <iostream>

BrowserFile::BrowserFile()
{
    visible = true;

    for (auto & id : AssetDB::GetAssets(Scheme::Project))
    {
        auto info = AssetDB::GetAsset(id);

        std::cout << "info: " << info->path << std::endl;

        create_hierarchy(URI::parse(info->path));
    }

    std::cout << "ss" << std::endl;

    // readDir(AssetResolver::Resolve(URI::parse("project://")));
}

void BrowserFile::onRenderUI()
{
    auto avail = ImGui::GetContentRegionAvail();

    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    if (ImGui::BeginChild("aaa", ImVec2(avail.x / 3.0f, 0), ImGuiChildFlags_AlwaysUseWindowPadding))
        render_virtual_directories();

    ImGui::EndChild();

    ImGui::SameLine();
    if (ImGui::BeginChild("bbb", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding))
        render_files();

    ImGui::EndChild();
}

void BrowserFile::render_virtual_directories()
{
    for (auto& info : files)
    {
        if (!info.isDirectory)
            continue;

        auto& name = info.name;

        auto isOpen = ImGui::TreeNodeEx(name.c_str());

        if (isOpen)
        {
            ImGui::TreePop();
        }
    }
}

void BrowserFile::render_files()
{}

#include <algorithm>

void BrowserFile::create_hierarchy(const URI& uri)
{
    std::string path = uri.path;

    std::cout << "path: " << path << std::endl;

    std::vector<std::string> parents;

    std::string current = path;

    while (!(current = gfile::parent(current)).empty())
    {
        parents.push_back(current);

    }

    std::reverse(parents.begin(), parents.end());

    for (const auto& dir : parents)
    {
        std::cout << "dir: " dir << '\n';
    }
}

