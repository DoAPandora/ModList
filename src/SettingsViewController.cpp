#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/UI/HorizontalLayoutGroup.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/TextAnchor.hpp"
using namespace UnityEngine::UI;

#include "TMPro/TextAlignmentOptions.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
using namespace QuestUI::BeatSaberUI;

#include "modloader/shared/modloader.hpp"

#include "main.hpp"
#include "SettingsViewController.hpp"
using namespace ModList;

DEFINE_CLASS(SettingsViewController);

struct ListItem {
    std::string content;
    std::string hoverHint;
};

void createList(UnityEngine::Transform* parent, std::string title, std::vector<ListItem> content) {
    VerticalLayoutGroup* layout = CreateVerticalLayoutGroup(parent);
    layout->set_spacing(1.0);

    // Create a layout for displaying the title.
    VerticalLayoutGroup* titleLayout = CreateVerticalLayoutGroup(layout->get_rectTransform());
    CreateText(titleLayout->get_rectTransform(), title)->set_alignment(TMPro::TextAlignmentOptions::Center);
    titleLayout->GetComponent<LayoutElement*>()->set_minWidth(50.0); // Make sure the list has a set width.

    // Create a layout for the list itself
    VerticalLayoutGroup* listLayout = CreateVerticalLayoutGroup(layout->get_rectTransform());
    listLayout->GetComponent<LayoutElement*>()->set_minWidth(50.0); // Make sure the list has a set width.
    listLayout->GetComponent<LayoutElement*>()->set_minHeight(70.0); // Make sure the list takes up most of the space

    // Make both the sections have a nice panel background
    titleLayout->get_gameObject()->AddComponent<QuestUI::Backgroundable*>()->ApplyBackground(il2cpp_utils::createcsstr("round-rect-panel"));
    listLayout->get_gameObject()->AddComponent<QuestUI::Backgroundable*>()->ApplyBackground(il2cpp_utils::createcsstr("round-rect-panel"));
    
    // Add some padding so that the messages aren't totally squished
    titleLayout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));
    listLayout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));

    // Make sure the list items are in the top left
    listLayout->set_childAlignment(UnityEngine::TextAnchor::UpperLeft);
    listLayout->set_childForceExpandHeight(false);
    listLayout->set_childControlHeight(true);

    // Create a line of text for each in the list
    for(ListItem element : content) {
        TMPro::TextMeshProUGUI* text = CreateText(listLayout->get_rectTransform(), element.content);
        // Add a hover hint if there is one
        if(element.hoverHint != "") {
            AddHoverHint(text->get_gameObject(), element.hoverHint);
        }
        text->set_fontSize(2.25);
    }
}

void SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if(!(firstActivation && addedToHierarchy))  {return;}

    HorizontalLayoutGroup* mainLayout = CreateHorizontalLayoutGroup(get_rectTransform());
    mainLayout->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter); // The lists should be centred

    // Check to see which libraries loaded/failed to load
    getLogger().info("Checking library load info.");
    std::vector<LibraryLoadInfo> libsLoadInfo = checkLibraryLoadStatus("sdcard/Android/data/com.beatgames.beatsaber/files/libs");
    std::vector<ListItem> libsList;

    for(LibraryLoadInfo loadInfo : libsLoadInfo) {
        if(loadInfo.errorMessage.has_value()) {
            // If there was an error loading the library, display it in red
            getLogger().debug("Adding failed library " + loadInfo.libraryName);
            ListItem item;
            item.content = "<color=red>" + loadInfo.libraryName + " (failed)";
            item.hoverHint = *loadInfo.errorMessage; // Allow you to hover over the mod to see the fail reason
            libsList.push_back(item);
        }   else    {
            // Otherwise, make the library name green
            getLogger().debug("Adding successful library " + loadInfo.libraryName);
            ListItem item;
            item.content = "<color=green>" + loadInfo.libraryName;
            libsList.push_back(item);
        }
        
    }

    getLogger().info("Adding loaded mods . . .");
    // Find the list of all loaded mods
    std::vector<ListItem> loadedMods;
    for(const std::pair<std::string, const Mod>& modEntry : Modloader::getMods()) {
        const Mod& mod = modEntry.second;
        getLogger().info("Adding mod " + mod.info.id);
        ListItem item;
        item.content = "<color=green>" + mod.info.id + "</color><color=white> v" + mod.info.version;
        loadedMods.push_back(item);
    }

    // Find the info about why the libraries in the mods directory loaded/didn't load
    std::vector<LibraryLoadInfo> modsLoadInfo = checkLibraryLoadStatus("sdcard/Android/data/com.beatgames.beatsaber/files/mods");
    std::vector<ListItem> failedMods;

    getLogger().info("Checking for failed mods . . .");
    for(LibraryLoadInfo loadInfo : modsLoadInfo) {
        // If there was an error loading the library, add it to the list in red
        if(loadInfo.errorMessage.has_value()) {
            getLogger().debug("Adding failed mod " + loadInfo.libraryName);
            ListItem item;
            item.content = "<color=red>" + loadInfo.libraryName + " (failed)";
            item.hoverHint = *loadInfo.errorMessage; // Allow you to hover over the mod to see the fail reason
            failedMods.push_back(item);
        }
    }

    // Create lists for each group
    createList(mainLayout->get_rectTransform(), "Loaded Mods", loadedMods);
    createList(mainLayout->get_rectTransform(), "Failed Mods", failedMods);
    createList(mainLayout->get_rectTransform(), "Libraries", libsList);
}