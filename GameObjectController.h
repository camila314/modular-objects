#pragma once

#if __APPLE__
    #include <Cacao>
    using namespace cocos2d;
#else
    #include "win32cac.h"
#endif

#include "ObjectManager.h"

using object_map = std::map<unsigned int, std::string>;

struct ShineInfo {
    std::string shine_texture;
    float shine_duration;
    cocos2d::_ccColor3B circle_color;
    bool use_circle;
    bool shine;
    bool texture_default;

    ShineInfo(std::string const& texture, float shine_duration, bool circle, cocos2d::_ccColor3B col) :
    shine(true), shine_texture(texture), shine_duration(shine_duration), use_circle(circle), circle_color(col) {}

    ShineInfo(std::string const& texture, float shine_duration) :
    ShineInfo(texture, shine_duration, false, {0,0,0}) {}

    ShineInfo(std::string const& texture) :
    ShineInfo(texture, 70.0) {}

    ShineInfo(bool sh) {
        shine = sh;
        texture_default = true;
        use_circle = false;
    }

    ShineInfo() : ShineInfo(false) {

    }
};

class GameObjectController : public CCNode {
 public:
    inline virtual ~GameObjectController() {
        ObjectManager::object_controllers.erase(ObjectManager::object_controllers.find(GET_UUID(m_object)));
    }


    inline virtual void onSetup() {}
    inline virtual void onTrigger(GJBaseGameLayer* l) {}
    inline virtual void onCollision(PlayerObject* p, GJBaseGameLayer* l) {}
    inline virtual void onReset() {}
    inline virtual object_map onExport() {return {};}
    inline virtual void onImport(object_map m) {}
    inline void init(int object_id, char const* texture, GameObjectType object_type=kGameObjectTypeSolid) {
        m_object = GameObject::createWithFrame(texture);

        #if __APPLE__
        m_object->_id() = object_id;
        m_object->m_type = object_type;
        m_object->m_textureName = gd::string(texture);
        #else
        m_object->m_nObjectID = object_id;
        m_object->m_nObjectType = object_type;
        m_object->m_sTextureName = texture;
        #endif

        m_tmpType = object_type;

        m_glowEnabled = false; // default
        m_shineInfo = ShineInfo(); // default
        m_customTexture = false; // default

        ObjectManager::object_controllers[GET_UUID(m_object)] = this;
    }

    inline static int getObjectID(GameObject* obj) {
        #if __APPLE__
        return obj->_id();
        #else
        return obj->m_nObjectID;
        #endif
    }

    inline bool getGlowEnabled() {return m_glowEnabled;}
    inline void setGlowEnabled(bool v) {m_glowEnabled = v;}
    
    void enableCustomTexture() {
        m_customTexture = true;
        int p = 0;
        if (m_object->getParent()) {
            p = m_object->getParent()->getZOrder();
        }

        m_object->_batchLayer() = -2;
    }
    void setTextureFromImage(void* data, size_t len) {
        enableCustomTexture();
        auto texture = ObjectManager::makeTexture(data, len);
        m_object->setTexture(texture);
        m_object->setTextureRect(CCRectMake(0, 0, texture->getContentSize().width, texture->getContentSize().height));
        printf("%f\n", texture->getContentSize().width);
        m_object->m_objectSize = CCSizeMake(texture->getContentSize().width, texture->getContentSize().height);
        //m_object->_objSize() = 0.2;
    }

    inline GameObject* getObject() {return m_object;}
 protected:
    void setup() {
        #if __APPLE__
        m_object->m_type = m_tmpType;
        #else
        m_object->m_nObjectType = m_tmpType;
        #endif

        onSetup();
    }
    GameObject* m_object;
    ShineInfo m_shineInfo;

    friend void ObjectManager::hooks::playShineEffect(GameObject* self);
 private:
    bool m_glowEnabled;
    bool m_customTexture;
    GameObjectType m_tmpType;
};
