#include "VirtualRGBA.hpp"
#include <geode.devtools/include/API.hpp>

class VirtualScrollable : public VirtualRGBA, RegisterDOM<VirtualScrollable, "Scrollable"> {
	bool m_scrollEnabled = false;
	bool m_vertical = true;
	bool m_dirty = false;
	float m_restriction = 100;
public:
	VirtualScrollable() : VirtualRGBA() {
		setContentSize({100, 100});
		m_tether = build();
	}

	CCRGBAProtocol* rgba_tether() override {
		return static_cast<ScrollLayer*>(m_tether.data())->m_contentLayer;
	}

	ScrollLayer* build() {
		return ScrollLayer::create({ccp(0, 0), getContentSize()}, m_scrollEnabled, m_vertical);
	}

	void settings() override {
		VirtualRGBA::settings();
		devtools::property("Scroll Wheel", m_scrollEnabled);
		m_dirty |= devtools::property("Vertical", m_vertical);

		devtools::property((std::string("Viewport ") + (m_vertical ? "Y" : "X")).c_str(), m_restriction);
	}

	matjson::Value exportJSON() override {
		auto obj = VirtualRGBA::exportJSON();

		obj["type"] = "Scrollable";
		obj["scrollEnabled"] = m_scrollEnabled;
		obj["vertical"] = m_vertical;
		obj["restriction"] = m_restriction;

		return obj;
	}
	void importJSON(matjson::Value obj) override {
		m_scrollEnabled = obj["scrollEnabled"].asBool().unwrapOr(false);
		m_vertical = obj["vertical"].asBool().unwrapOr(true);
		m_restriction = obj["restriction"].asDouble().unwrapOr(100.f);

		m_tether = build();

		VirtualRGBA::importJSON(obj);
	}

	void updateTether() override {
		if (m_dirty) {
			auto scroll = build();
			replaceTether(scroll);
			scroll->m_contentLayer->setPosition(ccp(0, 0));
			m_dirty = false;

			m_restriction = m_vertical ? scroll->getContentHeight() : scroll->getContentWidth();
		}

		auto scroll = static_cast<ScrollLayer*>(m_tether.data());
		scroll->enableScrollWheel(m_scrollEnabled);

		Ref<Layout> layout = getLayout();
		setLayout(nullptr);
		VirtualRGBA::updateTether();
		setLayout(layout);

		scroll->m_contentLayer->setContentSize(getContentSize());
		scroll->m_contentLayer->setLayout(getLayout());

		if (m_vertical) {
			m_restriction = std::min(m_restriction, getContentHeight());
			m_tether->setContentHeight(m_restriction);
		} else {
			m_restriction = std::min(m_restriction, getContentWidth());
			m_tether->setContentWidth(m_restriction);
		}
	}

	void addTetherChild(VirtualNode* virt) override {
		auto content = static_cast<ScrollLayer*>(m_tether.data())->m_contentLayer;
		auto virtTether = static_cast<VirtualScrollable*>(virt)->m_tether;

		virtTether->removeFromParentAndCleanup(false);

		auto parent = virt->getParent();
		if (parent && parent->getChildrenCount() > 0 && content->getChildrenCount() > 0) {
			size_t idx = parent->getChildren()->indexOfObject(virt);

			if (idx > 0) {
				auto objBefore = (CCNode*)content->getChildren()->objectAtIndex(idx - 1);
				content->insertAfter(virtTether, objBefore);
			} else {
				auto objAfter = (CCNode*)content->getChildren()->objectAtIndex(0);
				content->insertBefore(virtTether, objAfter);
			}
		} else {
			content->addChild(virtTether);
		}
	}
};
