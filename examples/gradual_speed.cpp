#define CAC_PROJ_NAME "Gradual Speed"

#include "GameObjectController.h"

class SpeedAction : public CCActionInterval {
 public:
    static SpeedAction* create(PlayerObject* p, float speed, float duration=1.0) {
        auto pRet = new SpeedAction();

        pRet->m_fDuration = duration > 0 ? duration : 0.0001;
        pRet->m_elapsed = 0;
        pRet->m_bFirstTick = true;

        pRet->m_destSpeed = speed * 5.77;
        pRet->m_srcSpeed = p->_speed();
        pRet->m_player = p;

        return pRet;
    }

    void update(float dur) override {
        m_player->_speed() = dur*(m_destSpeed-m_srcSpeed)+m_srcSpeed;
    }
    virtual void step(float dt) override {
        if (m_bFirstTick) {
            m_bFirstTick = false;
            m_elapsed = 0;
        } else {
            m_elapsed += dt;
        }
        
        this->update(m_elapsed / m_fDuration);
    }

    virtual bool isDone() override {
        return m_elapsed > m_fDuration;
    }

 protected:
    float m_destSpeed;
    float m_srcSpeed;
    PlayerObject* m_player;
};

class SmoothSpeedObject : public GameObjectController {
 public:
    static SmoothSpeedObject* create(int object_id) {
        auto pRet = new SmoothSpeedObject();
        pRet->init(object_id, "boost_01_001.png", kGameObjectTypeSolid);
        return pRet;
    }

    void onSetup() override {
        m_object->_effectSheet() = true;

        if (PL)
            m_object->createAndAddParticle(getObjectID(m_object), "boost_01_effect.plist", -2, kCCPositionTypeGrouped);
        m_shineInfo = ShineInfo("boost_01_shine_001.png", 130, true, ccc3(255, 255, 0));
    }

    void onCollision(PlayerObject* p, GJBaseGameLayer* l) override {
        if (!m_object->m_activated) {
            m_object->m_activated = true;

            p->runAction(SpeedAction::create(p, 0.3));
            if (l == PL) {
                PL->playSpeedParticle(0.7);
                m_object->playShineEffect();
            }
        }
    }
};


void inject() {
    ObjectManager::enable();
    ObjectManager::addObject(83, &SmoothSpeedObject::create);
}

#if _WIN32
    WIN32CAC_ENTRY(inject)
#endif
