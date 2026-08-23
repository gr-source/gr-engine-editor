#include "task/filmboxLoadTask.hpp"

#include <gr-base/core/scene.hpp>

#include <gr-base/externals/hash.hpp>

#include "event/importAssetViewPopupEvent.hpp"

#include "filmbox/filmbox.hpp"

#include "gr-base/managers/eventManager.hpp"

#include "view/loadingTaskPopup.hpp"

// #include <gr-base/core/file/memoryStream.hpp>
#include <gr-base/file/file.hpp>

#include <gr-base/core/gameObject.hpp>

#include <gr-base/core/console.hpp>

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/transform.hpp>

#include <gr-base/managers/assetsManager.hpp>
#include <stdexcept>

FilmboxLoadTask::FilmboxLoadTask(const std::string& file) :
    m_file(file),
    m_shader("engine://shader/simple_model"),
    m_dialog(HANDLE_INVALID)
{}

void FilmboxLoadTask::onPreExecute()
{
    auto filename = gfile::get_file_name(m_file);

    m_dialog =
        LoadingTaskPopup::Create(
            "Filmbox Task",
            String::format("Loading file '%s'", filename.c_str())
        );
}

FbxScene FilmboxLoadTask::doInBackground()
{
    Filmbox filmbox;

    FbxScene fbxScene;
    
    std::string error;
    if (!filmbox.load(m_file, error, &fbxScene))
        throw std::runtime_error(error);

    if (fbxScene.name.empty())
        fbxScene.name = gfile::get_file_name(m_file);

    fbxScene.root.name = gfile::get_file_name(m_file);

    return fbxScene;
}

void FilmboxLoadTask::onPostExecute(Result<FbxScene> result)
{
    LoadingTaskPopup::Destroy(m_dialog);

    if (!result.success)
        return Console::LogError("[FilmboxLoadTask] fail in to load '%s' error= '%s'.", m_file.c_str(), result.error.c_str());

    EventManager::Emit<ImportAssetViewPopup_onAddFbxScene>(ImportAssetViewPopup_onAddFbxScene{
            .fbxScene = result.value
        });
}






