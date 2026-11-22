#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/ui/Notification.hpp>
#include <array>
#include <cstdlib>

using namespace geode::prelude;

static constexpr std::array<const char*, 10> TIPS = {
	"Use <cy>3 or more colors</c> per area: a base, a highlight and a shadow.",
	"Put decoration on <cy>different Z layers</c> so it frames the gameplay instead of covering it.",
	"Keep the area around the player <cy>clean</c>. Players need to read the gameplay.",
	"Pick a <cy>light source</c> direction and keep highlights and shadows consistent.",
	"Use <cy>pulses and color triggers</c> to make the level feel alive.",
	"Copy and paste good details, then <cy>rotate, flip or recolor</c> them for variety.",
	"Add <cy>depth</c> with a darker background, mid-ground decor and a bright foreground.",
	"Use <cy>transparency</c> (blending) for glows, fog and light effects.",
	"Vary object <cy>sizes</c>. Mixed scales look more natural than uniform ones.",
	"Save often and keep a <cy>backup copy</c> of the level before big decoration passes."
};

class $modify(DecorEditorUI, EditorUI) {
	bool init(LevelEditorLayer* editorLayer) {
		if (!EditorUI::init(editorLayer)) return false;

		auto menu = this->getChildByID("undo-menu");
		if (!menu) return true; // layout changed: skip the button rather than crash

		auto sprite = ButtonSprite::create("Decor");
		sprite->setScale(0.6f);
		auto button = CCMenuItemExt::createSpriteExtra(sprite, [this](CCMenuItemSpriteExtra*) {
			this->openDecorPopup();
		});
		button->setID("decor-button"_spr);
		menu->addChild(button);
		menu->updateLayout();
		return true;
	}

	void openDecorPopup() {
		std::string text = "Select objects, then press <cg>Auto Shadow</c> to add a shadow copy behind them.";
		if (Mod::get()->getSettingValue<bool>("show-tip")) {
			text += "\n\n<cj>Tip:</c> ";
			text += TIPS[std::rand() % TIPS.size()];
		}
		Ref<DecorEditorUI> self = this;
		createQuickPopup(
			"Decor Helper", text, "Close", "Auto Shadow",
			[self](auto, bool autoShadow) {
				if (autoShadow) self->autoShadow();
			}
		);
	}

	void autoShadow() {
		auto selected = this->getSelectedObjects();
		if (!m_editorLayer || !selected || selected->count() == 0) {
			Notification::create("Select some objects first!", NotificationIcon::Warning)->show();
			return;
		}

		auto offset = ccp(
			static_cast<float>(Mod::get()->getSettingValue<double>("offset-x")),
			static_cast<float>(Mod::get()->getSettingValue<double>("offset-y"))
		);
		int color = static_cast<int>(Mod::get()->getSettingValue<int64_t>("shadow-color"));

		// Copy the list first so we never modify the selection while looping over it.
		std::vector<GameObject*> sources;
		for (auto obj : CCArrayExt<GameObject*>(selected)) {
			if (obj) sources.push_back(obj);
		}

		int made = 0;
		for (auto src : sources) {
			auto copy = m_editorLayer->createObject(src->m_objectID, src->getPosition() + offset, true);
			if (!copy) continue;

			copy->setRotation(src->getRotation());
			copy->setFlipX(src->isFlipX());
			copy->setFlipY(src->isFlipY());
			copy->updateCustomScaleX(src->m_scaleX);
			copy->updateCustomScaleY(src->m_scaleY);
			if (copy->m_baseColor) copy->m_baseColor->m_colorID = color;
			copy->m_zLayer = ZLayer::B2; // behind the original
			made++;
		}

		Notification::create(
			fmt::format("Added {} shadow object(s)", made),
			NotificationIcon::Success
		)->show();
	}
};
