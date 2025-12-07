#include <VirtualNode.hpp>
#include <Geode/modify/Traits.hpp>
#include <Geode/utils/function.hpp>
#include <geode.devtools/include/API.hpp>



bool VirtualNode::init() {
	CCNode::init();
	autorelease();

	scheduleUpdate();
	m_tether = CCNode::create();
	m_extraData = std::make_unique<Impl>();

	return true;
}

void VirtualNode::addTetherChild(VirtualNode* virt) {
	virt->m_tether->removeFromParentAndCleanup(false);

	auto parent = virt->getParent();
	if (parent && parent->getChildrenCount() > 0 && m_tether->getChildrenCount() > 0) {
		size_t idx = parent->getChildren()->indexOfObject(virt);

		if (idx > 0) {
			auto objBefore = (CCNode*)m_tether->getChildren()->objectAtIndex(idx - 1);
			m_tether->insertAfter(virt->m_tether, objBefore);
		} else {
			auto objAfter = (CCNode*)m_tether->getChildren()->objectAtIndex(0);
			m_tether->insertBefore(virt->m_tether, objAfter);
		}
	} else {
		m_tether->addChild(virt->m_tether);
	}
}

void VirtualNode::addChild(CCNode* child, int a, int b) {
	CCNode::addChild(child, a, b);
	if (auto virt = typeinfo_cast<VirtualNode*>(child)) {
		addTetherChild(virt);
	}
}

void VirtualNode::removeFromParent() {
	CCNode::removeFromParent();
	m_tether->removeFromParent();
}

void VirtualNode::replaceTether(CCNode* tether) {
	if (m_tether && m_tether != tether)
		m_tether->removeFromParentAndCleanup(true);
	m_tether = tether;

	auto parent = getParent();
	if (auto virtParent = typeinfo_cast<VirtualNode*>(parent)) {
		virtParent->addTetherChild(this);
	} else {
		parent->addChild(tether);
	}

	for (auto child : CCArrayExt<VirtualNode>(getChildren())) {
		addTetherChild(child);
	}
}

void VirtualNode::update(float dt) {
	updateLayout();
	updateTether();
}

void VirtualNode::updateTether() {
	m_tether->setID(getID());
	m_tether->setPosition(getPosition());

	m_tether->setScaleX(getScaleX());
	m_tether->setScaleY(getScaleY());

	m_tether->setContentSize(getContentSize());
	m_tether->setSkewX(getSkewX());
	m_tether->setSkewY(getSkewY());

	m_tether->setRotationX(getRotationX());
	m_tether->setRotationY(getRotationY());

	if (m_tether->getLayout() != getLayout())
		m_tether->setLayout(getLayout());
	m_tether->setLayoutOptions(getLayoutOptions());

	// this messes with node order too much
	if (getZOrder() != m_tether->getZOrder())
		m_tether->setZOrder(getZOrder());

	m_tether->setVisible(isVisible());
	m_tether->setTag(getTag());
	m_tether->ignoreAnchorPointForPosition(isIgnoreAnchorPointForPosition());

	m_tether->setAnchorPoint(getAnchorPoint());
}

std::string VirtualNode::emitCode(int indent) {
	std::string ind(indent, ' ');
	std::string out = ind + "Build<CCNode>::create()\n";

	out += emitAttributes(exportJSON(), indent + 4);

	return out;
}

template <typename T>
T getInt(matjson::Value const& obj) {
	return static_cast<T>(obj.asInt().unwrapOr(0));
};
std::string VirtualNode::emitAttributes(matjson::Value json, int indent) {
	std::string out;
	std::string ind(indent, ' ');

	if (getParent() && typeinfo_cast<AxisLayout*>(getParent()->getLayout())) {
		json.erase("anchor");
	}

	if (auto id = json["id"].asString())
		out += fmt::format("{}.id(\"{}\")\n", ind, *id);
	if (auto store = json["store"].asString())
		out += fmt::format("{}.store({})\n", ind, store.unwrap());
	if (auto pos = json["pos"].asArray())
		out += fmt::format("{}.pos({}f, {}f)\n", ind, fmtFloat(pos.unwrap()[0]), fmtFloat(pos.unwrap()[1]));
	if (auto size = json["size"].asArray())
		out += fmt::format("{}.contentSize({}f, {}f)\n", ind, fmtFloat(size.unwrap()[0]), fmtFloat(size.unwrap()[1]));
	if (auto anchor = json["anchor"].asArray())
		out += fmt::format("{}.anchorPoint({}f, {}f)\n", ind, fmtFloat(anchor.unwrap()[0]), fmtFloat(anchor.unwrap()[1]));
	if (auto rotation = json["rotation"].asDouble())
		out += fmt::format("{}.rotation({}f)\n", ind, *rotation);
	if (auto skew = json["skew"].asArray())
		out += fmt::format("{}.skewX({}f).skewY({}f)\n", ind, fmtFloat(skew.unwrap()[0]), fmtFloat(skew.unwrap()[1]));
	if (auto visible = json["visible"].asBool())
		out += fmt::format("{}.visible({})\n", ind, *visible ? "true" : "false");
	if (auto ignoreAnchor = json["ignoreAnchor"].asBool())
		out += fmt::format("{}.ignoreAnchorPointForPos({})\n", ind, *ignoreAnchor ? "true" : "false");
	if (auto zOrder = json["zOrder"].asInt())
		out += fmt::format("{}.zOrder({})\n", ind, *zOrder);
	if (auto scale = json["scale"].asArray()) {
		std::pair<float, float> scales = {
			(*scale)[0].asDouble().unwrapOr(1.f),
			(*scale)[1].asDouble().unwrapOr(1.f)
		};
		if (scales.first == scales.second)
			out += fmt::format("{}.scale({}f)\n", ind, scales.first);
		else
			out += fmt::format("{}.scaleX({}f).scaleY({}f)\n", ind, scales.first, scales.second);
	}
	if (auto tag = json["tag"].asInt())
		out += fmt::format("{}.tag({})\n", ind, *tag);

	bool hasLayout = false;
	if (auto layout = json["layout"]; layout.get("type")) {
		hasLayout = true;
		std::string type = layout["type"].asString().unwrapOr("AxisLayout");

		out += fmt::format("{}.layout(Build<{}>::create()\n", ind, type);

		if (type == "AxisLayout") {
			constexpr char const* axes[] = {
				"Row",
				"Column"
			};
			constexpr char const* aligns[] = {
				"Start",
				"Center",
				"End",
				"Even",
				"Between"
			};

			out += fmt::format("{}    .axis(Axis::{})\n", ind, axes[layout["axis"].asInt().unwrapOr(0)]);


			if (auto align = layout["alignment"].asInt().unwrapOr(0); align != 1)
				out += fmt::format("{}    .align(AxisAlignment::{})\n", ind, aligns[align]);
			if (auto crossAlign = layout["alignment"].asInt().unwrapOr(0); crossAlign != 1)
				out += fmt::format("{}    .crossAlign(AxisAlignment::{})\n", ind, aligns[crossAlign]);
			if (auto lineAlign = layout["alignment"].asInt().unwrapOr(0); lineAlign != 1)
				out += fmt::format("{}    .lineAlign(AxisAlignment::{})\n", ind, aligns[lineAlign]);
			if (auto gap = layout["gap"]; gap.asDouble().unwrapOr(5) != 5)
				out += fmt::format("{}    .gap({}f)\n", ind, fmtFloat(gap));
			if (layout["reverse"].asBool().unwrapOr(false))
				out += fmt::format("{}    .reverse(true)\n", ind);
			if (layout["crossReverse"].asBool().unwrapOr(false))
				out += fmt::format("{}    .crossReverse(true)\n", ind);
			if (!layout["autoScale"].asBool().unwrapOr(true))
				out += fmt::format("{}    .autoScale(false)\n", ind);
			if (layout["crossGrow"].asBool().unwrapOr(false))
				out += fmt::format("{}    .growCross(true)\n", ind);
			if (!layout["crossOverflow"].asBool().unwrapOr(true))
				out += fmt::format("{}    .crossOverflow(false)\n", ind);
			if (auto autoGrow = layout["autoGrow"]; autoGrow.isNumber())
				out += fmt::format("{}    .autoGrow({}f)\n", ind, fmtFloat(autoGrow));
		}

		if (layout["ignoreInvisible"].asBool().unwrapOr(false))
			out += fmt::format("{}    .ignoreInvisibleChildren(true)\n", ind);

		if (out.back() == '\n')
			out.pop_back();
		out += ")\n";
	}

	if (auto layoutOpts = json["layoutOpts"]; layoutOpts.get("type")) {
		auto type = layoutOpts["type"].asString().unwrapOr("AxisLayoutOptions");

		out += fmt::format("{}.layoutOpts(Build<{}>::create()\n", ind, type);

		if (type == "AxisLayoutOptions") {
			if (auto relScale = layoutOpts["relativeScale"].asDouble().unwrapOr(1.f); relScale != 1.f)
				out += fmt::format("{}    .relativeScale({}f)\n", ind, fmtFloat(relScale));
			if (layoutOpts["breakLine"].asBool().unwrapOr(false))
				out += fmt::format("{}    .breakLine(true)\n", ind);
			if (layoutOpts["sameLine"].asBool().unwrapOr(false))
				out += fmt::format("{}    .sameLine(true)\n", ind);
			if (auto prio = layoutOpts["scalePrio"].asInt().unwrapOr(0); prio != 0)
				out += fmt::format("{}    .scalePrio({})\n", ind, prio);
			if (auto crossAlign = layoutOpts["crossAlign"]; crossAlign.isNumber())
				out += fmt::format("{}    .crossAlign(AxisAlignment::{})\n", ind, crossAlign.asInt().unwrap());
			if (auto autoScale = layoutOpts["autoScale"]; autoScale.isBool())
				out += fmt::format("{}    .autoScale({})\n", ind, autoScale.asBool().unwrap() ? "true" : "false");
			if (auto length = layoutOpts["length"]; length.isNumber())
				out += fmt::format("{}    .length({}f)\n", ind, fmtFloat(length));
			if (auto prevGap = layoutOpts["prevGap"]; prevGap.isNumber())
				out += fmt::format("{}    .prevGap({}f)\n", ind, fmtFloat(prevGap));
			if (auto nextGap = layoutOpts["nextGap"]; nextGap.isNumber())
				out += fmt::format("{}    .nextGap({}f)\n", ind, fmtFloat(nextGap));
		} else if (type == "AnchorLayoutOptions") {
			constexpr char const* anchors[] = {
				"Center",
				"TopLeft",
				"Top",
				"TopRight",
				"Right",
				"BottomRight",
				"Bottom",
				"BottomLeft",
				"Left",
			};

			auto anchor = layoutOpts["anchor"].asInt().unwrapOr(0);
			out += fmt::format("{}    .anchor(Anchor::{})\n", ind, anchors[anchor]);
			if (auto offset = layoutOpts["offset"].asArray())
				out += fmt::format("{}    .offset(ccp({}f, {}f))\n", ind, fmtFloat((*offset)[0]), fmtFloat((*offset)[1]));
		}

		if (out.back() == '\n')
			out.pop_back();
		out += ")\n";
	}

	if (getChildrenCount() > 0 && json.get("children")) {
		out += ind + ".children(";
		for (auto& child : CCArrayExt<VirtualNode>(getChildren())) {
			if (!typeinfo_cast<VirtualNode*>(child))
				continue;

			out += "\n" + child->emitCode(indent + 4);
			if (out.back() == '\n')
			    out.pop_back();
			out += ",";
		}
		if (out.back() == ',')
			out.pop_back();
		out += ")\n";

		if (hasLayout)
			out += ind + ".updateLayout()\n";
	}

	return out;
}

matjson::Value VirtualNode::exportJSON() {
	matjson::Value obj;

	obj["type"] = "Node";
	obj["anchor"] = std::vector {getAnchorPoint().x, getAnchorPoint().y};

	if (getPosition() != CCPointZero)
		obj["pos"] = std::vector { getPositionX(), getPositionY() };
	obj["size"] = std::vector { getContentSize().width, getContentSize().height };

	if (!getID().empty())
		obj["id"] = getID();
	if (m_extraData->m_store != "")
		obj["store"] = m_extraData->m_store;
	if (getRotation() != 0)
		obj["rotation"] = getRotation();
	if (getSkewX() != 0 || getSkewY() != 0)
		obj["skew"] = std::vector { getSkewX(), getSkewY() };
	if (!isVisible())
		obj["visible"] = isVisible();
	if (isIgnoreAnchorPointForPosition())
		obj["ignoreAnchor"] = isIgnoreAnchorPointForPosition();
	if (getZOrder() != 0)
		obj["zOrder"] = getZOrder();
	if (getScaleX() != 1 || getScaleY() != 1)
		obj["scale"] = std::vector { getScaleX(), getScaleY() };
	if (getTag() != -1)
		obj["tag"] = getTag();

	if (auto layout = getLayout()) {
		matjson::Value layoutObj;

		if (layout->isIgnoreInvisibleChildren())
			layoutObj["ignoreInvisible"] = layout->isIgnoreInvisibleChildren();

		if (auto axis = typeinfo_cast<AxisLayout*>(layout)) {
			layoutObj["type"] = "AxisLayout";
			layoutObj["axis"] = static_cast<int>(axis->getAxis());
			layoutObj["alignment"] = static_cast<int>(axis->getAxisAlignment());
			layoutObj["crossAlignment"] = static_cast<int>(axis->getCrossAxisAlignment());
			layoutObj["lineAlignment"] = static_cast<int>(axis->getCrossAxisLineAlignment());

			if (axis->getGap() != 5)
				layoutObj["gap"] = axis->getGap();

			if (axis->getAxisReverse())
				layoutObj["reverse"] = axis->getAxisReverse();
			if (axis->getCrossAxisReverse())
				layoutObj["crossReverse"] = axis->getCrossAxisReverse();
			if (!axis->getAutoScale())
				layoutObj["autoScale"] = axis->getAutoScale();
			if (axis->getGrowCrossAxis())
				layoutObj["crossGrow"] = axis->getGrowCrossAxis();
			if (!axis->getCrossAxisOverflow())
				layoutObj["crossOverflow"] = axis->getCrossAxisOverflow();

			if (axis->getAutoGrowAxis().has_value())
				layoutObj["autoGrow"] = axis->getAutoGrowAxis().value();
		} else if (auto anchor = typeinfo_cast<AnchorLayout*>(layout)) {
			layoutObj["type"] = "AnchorLayout";
		}

		obj["layout"] = layoutObj;
	}

	if (auto layoutOpts = getLayoutOptions()) {
		matjson::Value layoutOptsObj;

		if (auto axisOpts = typeinfo_cast<AxisLayoutOptions*>(layoutOpts)) {
			layoutOptsObj["type"] = "AxisLayoutOptions";

			if (axisOpts->getRelativeScale() != 1)
				layoutOptsObj["relativeScale"] = axisOpts->getRelativeScale();
			if (axisOpts->getBreakLine())
				layoutOptsObj["breakLine"] = axisOpts->getBreakLine();
			if (axisOpts->getSameLine())
				layoutOptsObj["sameLine"] = axisOpts->getSameLine();
			if (axisOpts->getScalePriority() != 0)
				layoutOptsObj["scalePrio"] = axisOpts->getScalePriority();

			if (axisOpts->getCrossAxisAlignment().has_value())
				layoutOptsObj["crossAlign"] = static_cast<int>(axisOpts->getCrossAxisAlignment().value());
			if (axisOpts->getAutoScale().has_value())
				layoutOptsObj["autoScale"] = axisOpts->getAutoScale().value();
			if (axisOpts->getLength().has_value())
				layoutOptsObj["length"] = axisOpts->getLength().value();
			if (axisOpts->getPrevGap().has_value())
				layoutOptsObj["prevGap"] = axisOpts->getPrevGap().value();
			if (axisOpts->getNextGap().has_value())
				layoutOptsObj["nextGap"] = axisOpts->getNextGap().value();
		} else if (auto anchorOpts = typeinfo_cast<AnchorLayoutOptions*>(layoutOpts)) {
			layoutOptsObj["type"] = "AnchorLayoutOptions";
			layoutOptsObj["anchor"] = static_cast<int>(anchorOpts->getAnchor());

			if (anchorOpts->getOffset() != CCPointZero)
				layoutOptsObj["offset"] = std::vector { anchorOpts->getOffset().x, anchorOpts->getOffset().y };
		}

		obj["layoutOpts"] = layoutOptsObj;
	}

	// now let's be serious here
	if (auto parent = getParent()) {
		if (parent->getLayout()) {
			if (auto axis = typeinfo_cast<AxisLayout*>(parent->getLayout())) {
				if (axis->getAutoScale())
					obj.erase("scale");
				obj.erase("pos");
			} else if (auto anchor = typeinfo_cast<AnchorLayout*>(parent->getLayout()); anchor && getLayoutOptions()) {
				obj.erase("pos");
			}
		}
	}

	std::vector<matjson::Value> vec;
	for (auto& child : CCArrayExt<CCNode>(getChildren())) {
		if (auto vchild = typeinfo_cast<VirtualNode*>(child)) {
			vec.push_back(vchild->exportJSON());
		}
	}

	if (!vec.empty())
		obj["children"] = std::move(vec);

	if (typeid(*this) == typeid(VirtualNode)) {
		if (getAnchorPoint() == CCPointZero)
			obj.erase("anchor");		
		if (getContentSize() == CCSizeZero)
			obj.erase("size");

		if (typeinfo_cast<CCMenu*>(m_tether.data())) {
			obj["menu"] = true;
		}
	}

	return obj;
}

void VirtualNode::importJSON(matjson::Value obj) {

	if (obj["type"] == "Node" && obj["menu"].asBool().unwrapOr(false)) {
		m_tether = CCMenu::create();
	}

	setTag(obj["tag"].asInt().unwrapOr(-1));
	setID(obj["id"].asString().unwrapOr(""));
	m_extraData->m_store = obj["store"].asString().unwrapOr("");

	setPosition(ccp(
		obj["pos"][0].asDouble().unwrapOr(0.f),
		obj["pos"][1].asDouble().unwrapOr(0.f)
	));

	setContentSize(CCSize(
		obj["size"][0].asDouble().unwrapOr(0.f),
		obj["size"][1].asDouble().unwrapOr(0.f)
	));

	setAnchorPoint(ccp(
		obj["anchor"][0].asDouble().unwrapOr(0.f),
		obj["anchor"][1].asDouble().unwrapOr(0.f)
	));

	setRotation(obj["rotation"].asDouble().unwrapOr(0.f));

	setSkewX(obj["skew"][0].asDouble().unwrapOr(0.f));
	setSkewY(obj["skew"][1].asDouble().unwrapOr(0.f));

	setVisible(obj["visible"].asBool().unwrapOr(true));
	ignoreAnchorPointForPosition(obj["ignoreAnchor"].asBool().unwrapOr(false));
	setZOrder(obj["zOrder"].asInt().unwrapOr(0));

	setScaleX(obj["scale"][0].asDouble().unwrapOr(1.f));
	setScaleY(obj["scale"][1].asDouble().unwrapOr(1.f));

	// Layout
	if (obj.contains("layout")) {
		auto layoutObj = obj["layout"];
		std::string type = layoutObj["type"].asString().unwrapOr("");

		Ref<Layout> layout;

		if (type == "AxisLayout") {
			auto axis = AxisLayout::create();

			axis->ignoreInvisibleChildren(layoutObj["ignoreInvisible"].asBool().unwrapOr(false));
			axis->setAxis(static_cast<Axis>(layoutObj["axis"].asInt().unwrapOr(0)));
			axis->setAxisAlignment(static_cast<AxisAlignment>(layoutObj["alignment"].asInt().unwrapOr(0)));
			axis->setCrossAxisAlignment(static_cast<AxisAlignment>(layoutObj["crossAlignment"].asInt().unwrapOr(0)));
			axis->setCrossAxisLineAlignment(static_cast<AxisAlignment>(layoutObj["lineAlignment"].asInt().unwrapOr(0)));

			axis->setGap(layoutObj["gap"].asDouble().unwrapOr(5.f));
			axis->setAxisReverse(layoutObj["reverse"].asBool().unwrapOr(false));
			axis->setCrossAxisReverse(layoutObj["crossReverse"].asBool().unwrapOr(false));
			axis->setAutoScale(layoutObj["autoScale"].asBool().unwrapOr(true));
			axis->setGrowCrossAxis(layoutObj["crossGrow"].asBool().unwrapOr(false));
			axis->setCrossAxisOverflow(layoutObj["crossOverflow"].asBool().unwrapOr(true));

			if (layoutObj.contains("autoGrow"))
				axis->setAutoGrowAxis(layoutObj["autoGrow"].asInt().unwrap());

			layout = axis;
		} else if (type == "AnchorLayout") {
			auto anchor = AnchorLayout::create();
			anchor->ignoreInvisibleChildren(layoutObj["ignoreInvisible"].asBool().unwrapOr(false));

			layout = anchor;
		}

		setLayout(layout);
	}

	// Layout Options
	if (obj.contains("layoutOpts")) {
		auto layoutOptsObj = obj["layoutOpts"];
		std::string type = layoutOptsObj["type"].asString().unwrapOr("");

		Ref<LayoutOptions> layoutOpts;

		if (type == "AxisLayoutOptions") {
			auto axisOpts = AxisLayoutOptions::create();

			if (layoutOptsObj.contains("crossAlign"))
				axisOpts->setCrossAxisAlignment(static_cast<AxisAlignment>(layoutOptsObj["crossAlign"].asInt().unwrap()));
			if (layoutOptsObj.contains("autoScale"))
				axisOpts->setAutoScale(layoutOptsObj["autoScale"].asBool().unwrap());
			if (layoutOptsObj.contains("length"))
				axisOpts->setLength(layoutOptsObj["length"].asDouble().unwrap());
			if (layoutOptsObj.contains("prevGap"))
				axisOpts->setPrevGap(layoutOptsObj["prevGap"].asDouble().unwrap());
			if (layoutOptsObj.contains("nextGap"))
				axisOpts->setNextGap(layoutOptsObj["nextGap"].asDouble().unwrap());

			axisOpts->setRelativeScale(layoutOptsObj["relativeScale"].asDouble().unwrapOr(1.f));
			axisOpts->setBreakLine(layoutOptsObj["breakLine"].asBool().unwrapOr(false));
			axisOpts->setSameLine(layoutOptsObj["sameLine"].asBool().unwrapOr(false));
			axisOpts->setScalePriority(layoutOptsObj["scalePrio"].asInt().unwrapOr(0));

			layoutOpts = axisOpts;
		} else if (type == "AnchorLayoutOptions") {
			auto anchorOpts = AnchorLayoutOptions::create();

			anchorOpts->setAnchor(static_cast<Anchor>(layoutOptsObj["anchor"].asInt().unwrapOr(0)));

			anchorOpts->setOffset(ccp(
				layoutOptsObj["offset"][0].asDouble().unwrapOr(0.f),
				layoutOptsObj["offset"][1].asDouble().unwrapOr(0.f)
			));

			layoutOpts = anchorOpts;
		}

		setLayoutOptions(layoutOpts);
	}

	for (auto child : obj["children"]) {
		CCNode::addChild(VirtualDOMManager::get()->createFromJSON(child));
	}
}

void VirtualNode::settings() {
	std::string id = getID();
	devtools::nextItemWidth(200.f);

	auto prevStore = m_extraData->m_store;
	devtools::property("Store as Var", m_extraData->m_store);

	if (std::all_of(m_extraData->m_store.begin(), m_extraData->m_store.end(), [](char c) {
		return std::isalnum(c) || c == '_';
	}) == false || (!m_extraData->m_store.empty() && std::isdigit(m_extraData->m_store[0]))) {
		m_extraData->m_store = prevStore;
	}

	devtools::property("Node ID", id);
	setID(id);

	if (typeid(*this) == typeid(VirtualNode)) {
		bool isMenu = typeinfo_cast<CCMenu*>(m_tether.data());
		if (devtools::property("Treat as CCMenu", isMenu)) {
			if (isMenu) {
				replaceTether(CCMenu::create());
			} else {
				replaceTether(CCNode::create());
			}
		}
	}
}

$execute {
	VirtualDOMManager::registerCreate<VirtualNode>("Node");

    auto vdom = VirtualDOMManager::get();
    vdom->registerType("Row Node", +[]() {
        auto node = new VirtualNode;
        node->setID("row");
        node->setAnchorPoint({0.5, 0.5});
        node->setLayout(RowLayout::create()->setAutoGrowAxis(0)->setAutoScale(false));
        return node;
    });
    vdom->registerType("Column Node", +[]() {
        auto node = new VirtualNode;
        node->setID("column");
        node->setAnchorPoint({0.5, 0.5});
        node->setLayout(ColumnLayout::create()->setAutoGrowAxis(0)->setAutoScale(false));
        return node;
    });
}
