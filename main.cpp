#define CAC_PROJ_NAME "Gradual Speed"

#include "GameObjectController.h"

class AccelAction : public CCActionInterval {
 public:
    static AccelAction* create(PlayerObject* p, float yAccel, float duration=1.0) {
        auto pRet = new AccelAction();

        pRet->m_fDuration = duration > 0 ? duration : 0.0001;
        pRet->m_elapsed = 0;
        pRet->m_bFirstTick = true;

        pRet->m_destSpeed = yAccel;
        pRet->m_srcSpeed = p->_yAccel();
        pRet->m_player = p;

        return pRet;
    }

    void update(float dur) override {
        m_player->_yAccel() = dur*(m_destSpeed-m_srcSpeed)+m_srcSpeed;
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


class TestObject : public GameObjectController {
 public:
    static TestObject* create(int object_id) {
        auto pRet = new TestObject();
        pRet->init(object_id, "ring_01_001.png", kGameObjectTypeSolid);

        return pRet;
    }

    void onSetup() override {
        #include "/Users/jakrillis/spwn.c"
        setTextureFromImage(&spwn_png, spwn_png_len);
    }

    void onCollision(PlayerObject* p, GJBaseGameLayer* l) override {
        p->runAction(AccelAction::create(p, 16, 0.3));
    }
};


void inject() {
    ObjectManager::enable();
    ObjectManager::addObject(83, &TestObject::create);
}

#if _WIN32
    WIN32CAC_ENTRY(inject)
#endif
