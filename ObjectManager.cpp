#include "GameObjectController.h"

//#include "ObjectManager_checkCollision.hpp"

CCNode* ObjectManager::hooks::parentForZLayer(GJBaseGameLayer* self, int zLayer, bool detailChannel, int batchLayer) {
    if (batchLayer == -2) {
        if (!self->m_unusedDict)
            self->m_unusedDict = CCDictionary::create();

        CCNode* ret;

        Cacao::ccAsMap<int, CCNode*>(self->m_unusedDict, [&](auto mymap) {
            if (mymap.size() == 0) {
                mymap[-3] = CCNode::create();
                mymap[-1] = CCNode::create();
                mymap[1] = CCNode::create();
                mymap[3] = CCNode::create();
                mymap[5] = CCNode::create();
                mymap[7] = CCNode::create();
                mymap[9] = CCNode::create();
                mymap[11] = CCNode::create(); //unsure if this is even correct

                for (auto [k, v] : mymap)
                    self->m_objectLayer->addChild(v, k);
            }
            ret = mymap[zLayer];
        });

        return ret;
    } else {
        return ObjectManager::origs::parentForZLayer(self, zLayer, detailChannel, batchLayer);
    }
}
void ObjectManager::hooks::playShineEffect(GameObject* self) {
    if (self->m_activated || self->m_activatedByP2 || self->getOpacity() == 0 || PL==NULL)
        return;

    std::string shineFrame;
    bool playCircle;

    auto obj_uuid = GET_UUID(self);
    GameObjectController* custom = nullptr;
    if (ObjectManager::object_controllers.count(obj_uuid) > 0) {
        custom = ObjectManager::object_controllers[obj_uuid];
        if (!custom->m_shineInfo.shine)
            return;
    }

    if (custom && !custom->m_shineInfo.texture_default) {
        shineFrame = custom->m_shineInfo.shine_texture;
        playCircle = custom->m_shineInfo.use_circle;
    } else if (GET_ID(self) == 1331) {
        shineFrame = "portal_16_shine_001.png";
        playCircle = false;
    } else if (GET_ID(self) < 204 || GET_ID(self) == 1334) {
        shineFrame = StaticTools::replaceAll(self->m_textureName, "_001.png", "_shine_001.png");
        playCircle = true;
    } else {
        shineFrame = StaticTools::replaceAll(self->m_textureName, "_front_001.png", "_shine_001.png");
        playCircle = false;
    }

    auto circleColor = ccc3(255, 255, 0);

    if (custom) {
        circleColor = custom->m_shineInfo.circle_color;
    } else switch (GET_ID(self)) {
        case 201:
            circleColor = ccc3(0, 150, 255);//0xff9600;
            break;
        case 202:
            circleColor = ccc3(0, 255, 150);//0x96ff00;
            break;
        case 203:
            circleColor = ccc3(255, 0, 255);//0xff00ff;
            break;
        case 1334:
            circleColor = ccc3(255, 50, 50);//0x3232ff;
            break;
        default:
            if (GET_ID(self) > 203) {
                circleColor = ccc3(0, 150, 255);//0xff9600;
            } else {
                circleColor = ccc3(255, 255, 255);
            }
    }

    auto shineSprite = CCSpritePlus::createWithSpriteFrameName(shineFrame.c_str());
    if (shineSprite == 0)
        return;
    shineSprite->setPosition(self->getRealPosition());
    shineSprite->setRotation(self->getRotation());
    shineSprite->setFlipX(self->isFlipX());
    shineSprite->setFlipY(self->isFlipY());
    shineSprite->setScaleX(self->getScaleX());
    shineSprite->setScaleY(self->getScaleY());
    shineSprite->setOpacity(self->getOpacity());
    shineSprite->followSprite(self);
    shineSprite->setBlendFunc({700, 1});
    if (self->getParent()) {
        PL->m_objectLayer->addChild(shineSprite, self->getParent()->getZOrder() + 1);
    }

    auto sequence = CCSequence::create(
        CCFadeIn::create(0.05),
        CCFadeOut::create(0.3),
        CCCallFunc::create(shineSprite, (SEL_CallFunc)&CCSpritePlus::stopFollow),
        CCCallFunc::create(shineSprite, (SEL_CallFunc)&CCNode::removeMeAndCleanup),
        nullptr
    );
    shineSprite->runAction(sequence);

    if (playCircle) {
        float duration = 70.0;
        if (custom) {
            duration = custom->m_shineInfo.shine_duration;
        } else switch (self->_id()) {
            case 200:
                duration = 60.0;
                break;
            case 201:
                duration = 65.0;
                break;
            case 203:
                duration = 80.0;
                break;
            case 1334:
                duration = 90.0;
                break;
            default:
                duration = 70.0;
                break;
        }
        auto circleWave = CCCircleWave::create(5.0, duration, 0.3, false);
        circleWave->_color() = circleColor;

        circleWave->setPosition(self->getPosition());
        if (self->getParent()) {
            PL->m_objectLayer->addChild(circleWave, self->getParent()->getZOrder() - 1);
        }
        
        //*(circleWave + 0x140) = 0x600000001; ill sort self out later, its just visual who cares
        circleWave->followObject(self, true);
        circleWave->_delegate() = PL;
        PL->addCircle(circleWave);
    }
}

GameObject* ObjectManager::hooks::objectFromString(gd::string str, bool idk) {
    auto object = ObjectManager::origs::objectFromString(str, idk);
    if (!object) return object;
    std::string rstr = std::string(str);

    auto obj_id = GET_UUID(object);
    if (ObjectManager::object_controllers.count(obj_id)) {
        object_map m;
        std::string tmp;
        std::string tmp2;
        bool even = false;

        std::stringstream pain;
        pain << rstr;
        while (std::getline(pain, tmp, ',')) {
            if (!even)
                tmp2 = std::move(tmp);
            else {
                int key = stoi(tmp2);
                if (key > 1000) {
                    m[key-1000] = tmp;
                }
            }
            even = !even;
        }

        ObjectManager::object_controllers[obj_id]->onImport(m);
    }

    return object;
}

gd::string ObjectManager::hooks::getSaveString(GameObject* self) {
    std::string dat(ObjectManager::origs::getSaveString(self));
    auto obj_id = GET_UUID(self);
    if (ObjectManager::object_controllers.count(obj_id)) {
        auto out = ObjectManager::object_controllers[obj_id]->onExport();
        for (auto [k, v] : out) {
            dat += "," + std::to_string(k+1000) + "," + v;
        }
    }
    return gd::string(dat);
}

void ObjectManager::hooks::resetObject(GameObject* self) {
    auto obj_id = GET_UUID(self);
    if (ObjectManager::object_controllers.count(obj_id)) {
        ObjectManager::object_controllers[obj_id]->onReset();
    }
    ObjectManager::origs::resetObject(self);
}

void ObjectManager::hooks::collidedWithObject(PlayerObject* self, GameObject* object) {
    auto obj_id = GET_UUID(object);
    if (ObjectManager::object_controllers.count(obj_id)) {
        ObjectManager::object_controllers[obj_id]->onCollision(self, GJBL);
    } else {
        ObjectManager::origs::collidedWithObject(self, object);
    }
}

void ObjectManager::hooks::triggerObject(GameObject* self, PlayLayer* layer) {
    auto obj_id = GET_UUID(self);
    if (ObjectManager::object_controllers.count(obj_id)) {
        ObjectManager::object_controllers[obj_id]->onTrigger(layer);
    }

    float speed;
    switch (GET_ID(self)) {
        case 200:
            speed = 0.70;
            break;
        case 201:
            speed = 0.9;
            break;
        case 202:
            speed = 1.10;
            break;
        case 203:
            speed = 1.30;
            break;
        case 1334:
            speed = 1.60;
        default:
            return;
    }
    layer->updateTimeMod(speed, false);
    self->playShineEffect();
    layer->addToSpeedObjects(self);
}

void ObjectManager::hooks::customSetup(GameObject* self) {
    ObjectManager::origs::customSetup(self);

    auto obj_id = GET_UUID(self);
    if (ObjectManager::object_controllers.count(obj_id)) {
        ObjectManager::object_controllers[obj_id]->onSetup();
    }
}

__fastcall void ObjectManager::hooks::addGlow(GameObject* self) {
    auto obj_id = GET_UUID(self);
    if (ObjectManager::object_controllers.count(obj_id)) {
        auto custom = ObjectManager::object_controllers[obj_id];
        if (!custom->getGlowEnabled())
            return;
    }

    ObjectManager::origs::addGlow(self);
}


#if _WIN32
__fastcall void ObjectManager::hooks::setDisplayFrame(CCSprite* self, void* dummy, CCSpriteFrame* frame)
#else
void ObjectManager::hooks::setDisplayFrame(CCSprite* self, CCSpriteFrame* frame)
#endif
{
    if (frame == nullptr) {
        //FLAlertLayer::create(NULL, "Problem", "Ok", NULL, "Sprite used for this object is invalid.")->show();
        return;
    }

    self->m_obUnflippedOffsetPositionFromCenter = frame->getOffset();

    CCTexture2D *pNewTexture = frame->getTexture();
    // update texture before updating texture rect
    if (pNewTexture != self->m_pobTexture)
    {
        self->setTexture(pNewTexture);
    }

    // update rect
    self->m_bRectRotated = frame->isRotated();
    self->setTextureRect(frame->getRect(), self->m_bRectRotated, frame->getOriginalSize());
}

GameObject* ObjectManager::hooks::createWithKey(int object_id) {
    GameObject* ret = nullptr;

    if (ObjectManager::object_methods.count(object_id)) {
        auto controller = ObjectManager::object_methods[object_id](object_id);
        ret = controller->getObject();
    } else {
        auto texture = ObjectToolbox::sharedState()->intKeyToFrame(object_id);

        if (!texture || strlen(texture) < 3) {
            return nullptr;
        }

        if (object_id < 1008 && object_id > 748) {
            ret = LabelGameObject::create(texture);
        } else if (object_id <= 1819 && object_id > 1704) {
            ret = LabelGameObject::create(texture);
        } else switch (object_id) {
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 32:
            case 33:
            case 55:
            case 56:
            case 57:
            case 58:
            case 59:
            case 105:
            case 1268:
            case 1342:
            case 1343:
            case 1344:
            case 1346:
            case 1347:
            case 1585:
            case 1587:
            case 1589:
            case 1595:
            case 1598:
            case 1611:
            case 1612:
            case 1613:
            case 1614:
            case 1616:
            case 899:
            case 900:
            case 901:
            case 913:
            case 744:
            case 1049:
            case 1520:
                ret = LabelGameObject::create(texture);
                break;
            case 36:
            case 84:
            case 141:
            case 1022:
            case 1330:
            case 1333:
            case 1594:
            case 1704:
            case 1751:
                ret = RingObject::create(texture);
                break;
            case 1327:
            case 1328:
            case 1584:
                ret = AnimatedGameObject::create(object_id);
                break;
            case 1615:
                ret = RingObject::create();
                break;
            case 31:
                ret = StartPosObject::create();
                break;
            case 747: 
                ret = TeleportPortalObject::create(texture);
                break;
            case 914: {
            	#if __APPLE__
            	int efont = GM->_editorFont();
            	#else
            	#pragma error implement this pls
            	#endif

                auto font = cocos2d::CCTextureCache::sharedTextureCache()->addImage(GM->getFontTexture(efont), false);
                ret = new GameObject;
                if (!ret->initWithTexture(font)) {
                    delete ret;
                    return nullptr;
                }
                ret->autorelease();
                break;
            }
            default:
                ret = GameObject::createWithFrame(texture);
                break;
        }
    }

    if (ret) {
    	#if __APPLE__
        ret->_id() = object_id;
        #else
        ret->m_nObjectID = object_id;
        #endif
    }
    return ret;
}
