#define CAC_PROJ_NAME "Template"

#if __APPLE__
	#include <CacKit>
	using namespace cocos2d;
#else
	#include "win32cac.h"
#endif

class $implement(MenuLayer, MyMenuLayer) {
 public:
	static inline bool (__thiscall* _init)(MenuLayer* self);

	void buttonCallback(CCObject* sender) {
		auto alert = FLAlertLayer::create(NULL, "Mod", "Ok", NULL, "<cg>custom button!</c>");
		alert->show();
	}

	bool inithook() {
		if (!_init(this)) return false;

		auto sprite = CCSprite::create("dialogIcon_017.png");
		auto buttonSprite = CCSprite::createWithSpriteFrameName("GJ_stopEditorBtn_001.png");

		sprite->setPosition({100, 100});
		sprite->setScale(0.5f);

		addChild(sprite);

		auto button = CCMenuItemSpriteExtra::create(
		    buttonSprite,
		    this,
		    menu_selector(MyMenuLayer::buttonCallback));

		auto menu = CCMenu::create();
		menu->addChild(button);
		menu->setPosition(ccp(150, 100));

		addChild(menu);
		return true;
	}

	#if __APPLE__
	bool init() {return inithook();}
	#endif
};

void inject() {
	#if _WIN32
	auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
	
	MH_CreateHook(
	    reinterpret_cast<void*>(base + 0x1907b0),
		reinterpret_cast<void*>(extract(&MyMenuLayer::inithook)),
	    reinterpret_cast<void**>(&MyMenuLayer::_init));

	MH_EnableHook(MH_ALL_HOOKS);
	#endif
}

#if _WIN32
	WIN32CAC_ENTRY(inject)
#endif
