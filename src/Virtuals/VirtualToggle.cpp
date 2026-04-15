#include <VirtualNode.hpp>
#include <Geode/Geode.hpp>
#include <geode.devtools/include/API.hpp>

class VirtualToggle : public VirtualNode, RegisterDOM<VirtualToggle, "Toggle"> {
	std::string m_onSprite = "GJ_checkOn_001.png";
	std::string m_offSprite = "GJ_checkOff_001.png";
	bool m_frameDirty = false;

	CCSprite* m_onSpr = nullptr;
	CCSprite* m_offSpr = nullptr;
 public:
 	CCMenuItemToggler* build() {
 		m_onSpr = createSprite(m_offSprite);
 		m_offSpr = createSprite(m_onSprite);

 		m_onSpr->setScale(getScale());
 		m_offSpr->setScale(getScale());
 		return CCMenuItemExt::createToggler(
 			m_onSpr,
 			m_offSpr,
 			+[](CCMenuItemToggler* item) {
 				log::info("Toggle State: ", item->isToggled() ? "On" : "Off");
 			}
 		);
 	}

 	VirtualToggle() : VirtualNode() {
 		m_tether = build();
 		setAnchorPoint({0.5, 0.5});
 	}

 	void settings() override {
 		VirtualNode::settings();
 		m_frameDirty |= devtools::property("On Sprite", m_onSprite);
 		m_frameDirty |= devtools::property("Off Sprite", m_offSprite);
 
 		if (m_frameDirty) {
 			m_tether = build();
 			m_frameDirty = false;
 		}
 	}

 	matjson::Value exportJSON() override {
 		auto obj = VirtualNode::exportJSON();

 		obj["type"] = "Toggle";
 		obj["onSprite"] = m_onSprite;
 		obj["offSprite"] = m_offSprite;
 		obj.erase("size");

 		return obj;
 	}
 	void importJSON(matjson::Value obj) override {
 		m_onSprite = obj["onSprite"].asString().unwrapOr(m_onSprite);
 		m_offSprite = obj["offSprite"].asString().unwrapOr(m_offSprite);

 		m_tether = build();

 		VirtualNode::importJSON(obj);
 	}

 	std::string emitCode(int indent = 0) override {
 		std::string ind(indent, ' ');
 		std::string out = ind;

 		if (m_onSprite == "GJ_checkOn_001.png" && m_offSprite == "GJ_checkOff_001.png")
 			out += fmt::format("Build(CCMenuItemExt::createTogglerWithStandardSprites({}, +[](auto) {{ }})\n", fmtFloat(getScale()));
 		else {
 			out += fmt::format("Build(CCMenuItemExt::createToggler(\n");
 			out += fmt::format("{}    {}", ind, fmtSprite(m_onSprite));
 			if (getScale() != 1.0)
 				out += fmt::format(".scale({})", fmtFloat(getScale()));
 			out += fmt::format(",\n{}    {}", ind, fmtSprite(m_offSprite));
  			if (getScale() != 1.0)
 				out += fmt::format(".scale({})", fmtFloat(getScale()));
 			out += fmt::format(",\n{}    +[](auto) {{ }}))\n", ind, ind);
		}

		auto json = exportJSON();
		json.erase("scale");

		out += VirtualNode::emitAttributes(exportJSON(), indent + 4);

 		return out;
 	}

 	void updateTether() override {
 		if (m_frameDirty) {
 			m_tether = build();
 			m_frameDirty = false;
 		}

 		m_onSpr->setScale(getScale());
 		m_offSpr->setScale(getScale());

 		setContentSize(m_tether->getContentSize());
 		VirtualNode::updateTether();
 		m_tether->setScale(1.0);
 	}
};
