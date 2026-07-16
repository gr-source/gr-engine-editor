#pragma once

#include "gr-base/asset/URI.hpp"
#include "view/viewlayout.hpp"

struct FileInfo
{
    std::string name;
    std::string path;

    bool isDirectory;
};

class BrowserFile : public ViewLayout
{
public:
    BrowserFile();

    inline const char* getName() const override
    {
        return "Browser File";
    }

private:
    std::vector<FileInfo> files;

    void onRenderUI() override;

    void render_virtual_directories();

    void render_files();

    void create_hierarchy(const URI& uri);
};


