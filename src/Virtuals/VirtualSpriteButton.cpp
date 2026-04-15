#include <VirtualNode.hpp>
#include <geode.devtools/include/API.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

/*
Circle
Cross
Account
IconSelect
Leaderboard
Editor
Tab
Category
None
*/

template <BaseType T>
struct BaseEnums;

#define BASE_TYPE(name) \
	template <> \
	struct BaseEnums<BaseType::name> { using pair = std::pair<name ## BaseSize, name ## BaseColor>; }

BASE_TYPE(Circle);
BASE_TYPE(Cross);
BASE_TYPE(Account);
BASE_TYPE(IconSelect);
BASE_TYPE(Leaderboard);
BASE_TYPE(Editor);
BASE_TYPE(Tab);
BASE_TYPE(Category);

template <BaseType T>
using BaseSize = typename BaseEnums<T>::pair::first_type;
template <BaseType T>
using BaseColor = typename BaseEnums<T>::pair::second_type;

class VirtualSpriteButton : public VirtualNode, RegisterDOM<VirtualSpriteButton, "Sprite Button"> {
	/*std::tuple<
		std::pair<CircleBaseSize, CircleBaseColor>,
		std::pair<CrossBaseSize, CrossBaseColor>,
		std::pair<AccountBaseSize, AccountBaseColor>,
		std::pair<IconSelectBaseSize, IconSelectBaseColor>,
		std::pair<LeaderboardBaseSize, LeaderboardBaseColor>,
		std::pair<EditorBaseSize, EditorBaseColor>,
		std::pair<TabBaseSize, TabBaseColor>,
		std::pair<CategoryBaseSize, CategoryBaseColor>
	> m_bases;*/

	bool m_flipX = false;
	bool m_flipY = false;
	float m_relativeScale = 1.0f;
	CCPoint m_relativeOffset = CCPointZero;
	bool m_frameDirty = false;
	std::string m_spriteName = "GJ_lock_001.png";

	std::array<std::pair<int, int>, 8> m_bases;
	BaseType m_baseSelection = BaseType::Circle;
	bool m_hasBase = true;

	CCSprite* m_sprite = nullptr;
	CCSprite* m_innerSprite = nullptr;
public:
	VirtualSpriteButton() { 
		rebuildSprite();
		m_tether = CCMenuItemExt::createSpriteExtra(m_sprite, +[](CCMenuItemSpriteExtra* mitem) {
			log::info("Button Pressed: {}", mitem->getID());
		});
		setAnchorPoint({0.5, 0.5});
	}

	void rebuildSprite() {
		auto spr = createSprite(m_spriteName);
		if (!spr)
			return;

		if (m_sprite) {
			m_sprite->removeFromParent();
		}

		m_innerSprite = spr;
		if (m_hasBase) {
			auto based = BasedButtonSprite::create(
				spr,
				m_baseSelection,
				m_bases[static_cast<int>(m_baseSelection)].first,
				m_bases[static_cast<int>(m_baseSelection)].second
			);
			based->setTopRelativeScale(m_relativeScale);
			based->setTopOffset(m_relativeOffset);

			m_sprite = based;
		} else {
			m_sprite = spr;
		}
	}

	template <BaseType T>
	void baseRadio() {
		static std::vector<char const*> sizes;
		static std::vector<char const*> colors;

		if (sizes.empty()) {
			for (int i = 0;; ++i) {
				auto sizeStr = baseEnumToString(static_cast<BaseSize<T>>(i));
				if (std::string_view(sizeStr) == "Unknown")
					break;
				sizes.push_back(sizeStr);
			}
		}
		if (colors.empty()) {
			for (int i = 0;; ++i) {
				auto colorStr = baseEnumToString(static_cast<BaseColor<T>>(i));
				if (std::string_view(colorStr) == "Unknown")
					break;
				colors.push_back(colorStr);
			}
		}

		m_frameDirty |= devtools::radio(
			baseEnumToString(T),
			m_baseSelection,
			T
		);

		if (m_baseSelection == T) {
			m_hasBase = true;

			devtools::sameLine();
			devtools::nextItemWidth(100.0f); 
			m_frameDirty |= devtools::combo("##base_size", 
				m_bases[static_cast<int>(T)].first,
				sizes
			);

			devtools::sameLine();
			devtools::nextItemWidth(100.0f); 
			m_frameDirty |= devtools::combo("##base_color", 
				m_bases[static_cast<int>(T)].second,
				colors
			);
		}
	}

	void settings() override {
		VirtualNode::settings();

		m_frameDirty = devtools::property("Sprite Name", m_spriteName);

		devtools::property("Flip X", m_flipX);
		devtools::sameLine();
		devtools::property("Flip Y", m_flipY);

		devtools::newLine();
		devtools::separator();
		devtools::newLine();

		devtools::label("Button Base");

		m_frameDirty |= devtools::radio("None", m_baseSelection, -1);
		if ((int)m_baseSelection == -1)
			m_hasBase = false;

		baseRadio<BaseType::Circle>();
		baseRadio<BaseType::Cross>();
		baseRadio<BaseType::Account>();
		baseRadio<BaseType::IconSelect>();
		baseRadio<BaseType::Leaderboard>();
		baseRadio<BaseType::Editor>();
		baseRadio<BaseType::Tab>();
		baseRadio<BaseType::Category>();

		if (m_hasBase) {
			m_frameDirty |= devtools::property("Relative Scale", m_relativeScale);
			m_frameDirty |= devtools::property("Offset", m_relativeOffset);
		}
	}

	matjson::Value exportJSON() override {
		auto obj = VirtualNode::exportJSON();

		obj["type"] = "Sprite Button";
		obj["spriteName"] = m_spriteName;
		obj.erase("size");
		if (m_flipX)
			obj["flipX"] = m_flipX;
		if (m_flipY)
			obj["flipY"] = m_flipY;

		obj["hasBase"] = m_hasBase;
		if (m_hasBase) {
			obj["baseType"] = static_cast<int>(m_baseSelection);
			if (m_relativeScale != 1.0f)
				obj["relativeScale"] = m_relativeScale;
			if (m_relativeOffset != CCPointZero)
				obj["relativeOffset"] = std::vector{ m_relativeOffset.x, m_relativeOffset.y };

			obj["baseSizeColor"] = std::vector {
				m_bases[static_cast<int>(m_baseSelection)].first,
				m_bases[static_cast<int>(m_baseSelection)].second
			};
		}

		return obj;
	}

	void importJSON(matjson::Value obj) override {
		m_spriteName = obj["spriteName"].asString().unwrapOr(m_spriteName);
		m_flipX = obj["flipX"].asBool().unwrapOr(false);
		m_flipY = obj["flipY"].asBool().unwrapOr(false);
		m_hasBase = obj["hasBase"].asBool().unwrapOr(true);

		if (m_hasBase) {
			m_baseSelection = static_cast<BaseType>(obj["baseType"].asInt().unwrapOr(static_cast<int>(m_baseSelection)));
			m_relativeScale = obj["relativeScale"].asDouble().unwrapOr(1.0f);

			m_relativeOffset = ccp(
				obj["relativeOffset"][0].asDouble().unwrapOr(0),
				obj["relativeOffset"][1].asDouble().unwrapOr(0)
			);

			m_bases[static_cast<int>(m_baseSelection)].first = obj["baseSizeColor"][0].asInt().unwrapOr(0);
			m_bases[static_cast<int>(m_baseSelection)].second = obj["baseSizeColor"][1].asInt().unwrapOr(0);
		}

		m_frameDirty = true;

		VirtualNode::importJSON(obj);
	}

	std::string emitCode(int indent) override {
		std::string ind(indent, ' ');
		std::string out = ind;

		auto base = baseEnumToString(m_baseSelection);

		if (m_hasBase) {
			out += fmt::format("Build<{}ButtonSprite>::create(", base);
		}

		if (CCSprite::createWithSpriteFrameName(m_spriteName.c_str())) {
			out += fmt::format("Build<CCSprite>::createSpriteName(\"{}\")", m_spriteName);
		} else {
			out += fmt::format("Build<CCSprite>::create(\"{}\")", m_spriteName);
		}

		if (m_flipX)
		    out += fmt::format("{}.flipX(true){}", m_hasBase ? "\n" + ind + "    " : "", m_hasBase ? "\n" : "");
		if (m_flipY)
		    out += fmt::format("{}.flipY(true){}", m_hasBase ? ind + "    " : "", m_hasBase ? "\n" : "");

		auto finale = [&]<BaseType T>() {
			if (m_baseSelection == T) {
				out += fmt::format(", {0}BaseColor::{1}, {0}BaseSize::{2}", base,
					baseEnumToString(static_cast<BaseColor<T>>(m_bases[static_cast<int>(T)].second)),
					baseEnumToString(static_cast<BaseSize<T>>(m_bases[static_cast<int>(T)].first))
				);
			}
		};

		if (m_hasBase) {
			finale.operator()<BaseType::Circle>();
			finale.operator()<BaseType::Cross>();
			finale.operator()<BaseType::Account>();
			finale.operator()<BaseType::IconSelect>();
			finale.operator()<BaseType::Leaderboard>();
			finale.operator()<BaseType::Editor>();
			finale.operator()<BaseType::Tab>();
			finale.operator()<BaseType::Category>();

			out += ")\n";

			if (m_relativeScale != 1.0) {
				out += fmt::format("{}    .topScale({}f)\n", ind, fmtFloat(m_relativeScale));
			}
			if (m_relativeOffset != CCPointZero) {
				out += fmt::format("{}    .topOffset(ccp({}f, {}f))\n", ind,
					fmtFloat(m_relativeOffset.x),
					fmtFloat(m_relativeOffset.y)
				);
			}
		} else if (out.back() != '\n') {
			out += "\n";
		}

		matjson::Value preJson = matjson::Value::object();
		auto json = exportJSON();

		for (auto& key : {"scale", "rotation", "anchor", "ignoreAnchor", "skew"}) {
			if (json.contains(key)) {
				preJson[key] = json[key];
				json.erase(key);
			}
		}

		out += VirtualNode::emitAttributes(preJson, indent + 4);
		out += fmt::format("{}    .intoMenuItem([](auto) {{ log::info(\"Button Pressed\"); }})\n", ind);
		out += VirtualNode::emitAttributes(json, indent + 4);

		return out;
	}

	void updateTether() override {
		if (m_frameDirty) {
			rebuildSprite();

			replaceTether(CCMenuItemExt::createSpriteExtra(m_sprite, +[](CCMenuItemSpriteExtra* mitem) {
				log::info("Button Pressed: {}", mitem->getID());
			}));
		}
		setContentSize(m_sprite->getContentSize());

		VirtualNode::updateTether();

		m_tether->setScale(1.0);
		m_sprite->setScale(getScale());


		m_sprite->setRotation(getRotation());

		m_innerSprite->setFlipX(m_flipX);
		m_innerSprite->setFlipY(m_flipY);
	}
};
