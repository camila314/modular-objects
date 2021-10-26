#define CAC_PROJ_NAME "Debug Print"

#include "GameObjectController.h"
#include <variant>
enum DebugPrintType {
    kDebugPrintItem,
    kDebugPrintText
};

using debug_value = std::variant<int, std::string>;

class DebugPrintObject : public GameObjectController {
 public:
    static DebugPrintObject* create(int object_id) {
        auto pRet = new DebugPrintObject();
        pRet->init(object_id, "checkpoint_01_001.png");
        return pRet;
    }

    void onSetup() override {
        m_object->_effectSheet() = true;
        m_object->_type() = kGameObjectTypeSpecial;
        m_object->_isTrigger() = true;
    }

    void onTrigger(GJBaseGameLayer* layer) override {
        std::string out = " ";
        for (auto& i : m_printed) {
            switch (i.index()) {
                case kDebugPrintText:
                    out += std::get<kDebugPrintText>(i);
                    break;
                case kDebugPrintItem:
                    out += std::to_string(layer->m_effectManager->m_itemValues[std::get<kDebugPrintItem>(i)]);
                    break;
            }
        }
        if (layer != PL) {
            reinterpret_cast<LevelEditorLayer*>(layer)->onPausePlaytest();
        }
        FLAlertLayer::create(NULL, "Debug", "Ok", NULL, out)->show();
    }

    object_map onExport() override {
        std::string serialized_output;
        for (auto& i : m_printed) {
            switch (i.index()) {
                case kDebugPrintText:
                    serialized_output += "$" +
                                         std::to_string(std::get<kDebugPrintText>(i).size()) +
                                         "$" +
                                         std::get<kDebugPrintText>(i);
                    break;
                case kDebugPrintItem:
                    serialized_output += "%"+std::to_string(std::get<kDebugPrintItem>(i))+"%";
                    break;
            }
        }
        return {
            {1, serialized_output}
        };
    }

    void onImport(object_map m) override {
        std::string serialized_input = m[1];
        std::string tmp = serialized_input;

        while (tmp.size() > 1) {
            switch (tmp.c_str()[0]) {
                case '%':
                    if (tmp.size()>1) {
                        tmp.erase(0, 1);
                        auto item = stoi(tmp.substr(0, tmp.find('%')));
                        tmp.erase(0, tmp.find('%')+1);
                        debug_value v{ std::in_place_index<kDebugPrintItem>, item };
                        m_printed.push_back(v);
                    }
                    break;
                case '$':
                    if (tmp.size()>1) {
                        tmp.erase(0, 1);
                        debug_value v;
                        auto len = stoi(tmp.substr(0, tmp.find('$')+1));
                        tmp.erase(0, tmp.find('$')+1);

                        if (tmp.size()>len) {
                            auto str = tmp.substr(0, len);
                            tmp.erase(0, len);
                            debug_value v{ std::in_place_index<kDebugPrintText>, str };
                            m_printed.push_back(v);
                        }
                    }
                    break;
                default:
                    return;
            }
        }
    }

 protected:
    std::vector<debug_value> m_printed;
};


void inject() {
    ObjectManager::enable();

    ObjectManager::addObject(37, &DebugPrintObject::create);
}

#if _WIN32
    WIN32CAC_ENTRY(inject)
#endif
