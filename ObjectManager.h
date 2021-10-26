#pragma once

#if __APPLE__
	#include <Cacao>
	using namespace cocos2d;
#else
	#include "win32cac.h"

	namespace gd {
		using std::string;
	}
#endif

#if __APPLE__
#define GET_ID(x) (x->_id())
#define GET_UUID(x) (x->m_uuid)
#else
#define GET_ID(x) (x->m_nObjectID)
#define GET_UUID(x) (x->m_nUniqueID)
#endif

//#include "GameObjectController.h"
class GameObjectController;

typedef GameObjectController* (*createfunc_t)(int);
namespace ObjectManager {
	inline std::map<int, createfunc_t> object_methods;
	inline std::map<int, GameObjectController*> object_controllers;

	template <typename T>
	void addObject(int obj_id, T func) {
		createfunc_t casted = reinterpret_cast<createfunc_t>(func);
		object_methods[obj_id] = casted;
	}

	namespace hooks {
		GameObject* createWithKey(int object_id);
		void setDisplayFrame(CCSprite* self, CCSpriteFrame* frame);
		void addGlow(GameObject* self);
		void customSetup(GameObject* self);
		void triggerObject(GameObject* self, PlayLayer* layer);
		void collidedWithObject(PlayerObject* self, GameObject* object);
		void resetObject(GameObject* self);
		gd::string getSaveString(GameObject* self);
		GameObject* objectFromString(gd::string str, bool idk);
		void playShineEffect(GameObject* self);
		CCNode* parentForZLayer(GJBaseGameLayer* self, int zLayer, bool detailChannel, int batchLayer);
	}
	namespace origs {
		inline void (*__thiscall addGlow)(GameObject*);
		inline void (*__thiscall customSetup)(GameObject*);
		inline void (*__thiscall collidedWithObject)(PlayerObject*, GameObject*);
		inline void (*__thiscall resetObject)(GameObject*);
		inline gd::string (*__thiscall getSaveString)(GameObject*);
		inline GameObject* (*__thiscall objectFromString)(gd::string, bool);
		inline CCNode* (*__thiscall parentForZLayer)(GJBaseGameLayer*, int, bool, int);
	}

	inline CCTexture2D* makeTexture(void* data, size_t dsize, CCImage::EImageFormat fmt = CCImage::EImageFormat::kFmtPng) {
		auto img = new CCImage();
		img->initWithImageData(data, dsize, fmt);

		auto texture = new CCTexture2D();
		texture->initWithImage(img);
		return texture;
	}

	inline void enable() {
		void* stub;
		#if _WIN32
		auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
		
		MH_CreateHook(
		    reinterpret_cast<void*>(base + 0xcf4f0),
			reinterpret_cast<void*>(&ObjectManager::hooks::createWithKey),
		    &stub);

		/*MH_CreateHook(
		    reinterpret_cast<void*>(base + 0xcf4f0),
			reinterpret_cast<void*>(&ObjectManager::hooks::addGlow),
		    reinterpret_cast<void**>(&ObjectManager::origs::addGlow));*/
		MH_CreateHook(
		    reinterpret_cast<void*>(base + 0xd1c10),
			reinterpret_cast<void*>(&ObjectManager::hooks::customSetup),
		    reinterpret_cast<void**>(&ObjectManager::origs::customSetup));

		MH_CreateHook(
		    reinterpret_cast<void*>(base + 0xd1790),
			reinterpret_cast<void*>(&ObjectManager::hooks::triggerObject),
		    &stub);

		MH_CreateHook(
		    reinterpret_cast<void*>(base + 0xd1470),
			reinterpret_cast<void*>(&ObjectManager::hooks::resetObject),
		    reinterpret_cast<void**>(&ObjectManager::origs::resetObject));

		MH_CreateHook(
		    reinterpret_cast<void*>(base + 0xed0c0),
			reinterpret_cast<void*>(&ObjectManager::hooks::getSaveString),
		    reinterpret_cast<void**>(&ObjectManager::origs::getSaveString));

		MH_CreateHook(
		    reinterpret_cast<void*>(base + 0xebe50),
			reinterpret_cast<void*>(&ObjectManager::hooks::objectFromString),
		    reinterpret_cast<void**>(&ObjectManager::origs::objectFromString));

		#pragma error idk how to do cocos hook stuff
		#pragma error you fuckers dont have addGlow? shame 
		#pragma error you dont have collidedWithObject either?!?!?!?!!?!?!?!?!!?!?!?!?!?

		MH_EnableHook(MH_ALL_HOOKS);
		#else
		m->registerHook(getBase()+0x2f4ce0, ObjectManager::hooks::createWithKey);
		m->registerHook(getBase()+0x135610, ObjectManager::hooks::setDisplayFrame);
		m->registerHook(getBase()+0x2fa8f0, ObjectManager::hooks::triggerObject);
		m->registerHook(getBase()+0x2fa9d0, ObjectManager::hooks::playShineEffect);

		ObjectManager::origs::addGlow = m->registerHook(getBase()+0x2f5c10, ObjectManager::hooks::addGlow);
		ObjectManager::origs::customSetup = m->registerHook(getBase()+0x2fbba0, ObjectManager::hooks::customSetup);
		ObjectManager::origs::collidedWithObject = m->registerHook(getBase()+0x21d880, ObjectManager::hooks::collidedWithObject);
		ObjectManager::origs::resetObject = m->registerHook(getBase()+0x2fa620, ObjectManager::hooks::resetObject);
		ObjectManager::origs::getSaveString = m->registerHook(getBase()+0x33d3d0, ObjectManager::hooks::getSaveString);
		ObjectManager::origs::objectFromString = m->registerHook(getBase()+0x33b720, ObjectManager::hooks::objectFromString);
		ObjectManager::origs::parentForZLayer = m->registerHook(getBase()+0xb55d0, ObjectManager::hooks::parentForZLayer);
		m->enable();
		#endif
	}
}