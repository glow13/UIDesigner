#include <VirtualNode.hpp>
#include <geode.devtools/include/API.hpp>
#include <Geode/ui/TextInput.hpp>

constexpr std::array filters = {
	"Uint",
	"Int",
	"Float",
	"ID",
	"Name",
	"Any",
	"Hex",
	"Base64Normal",
	"Base64URL",
	"Alphanumeric",
	"Alphabetic"
};

class VirtualTextInput : public VirtualNode {
	std::string m_placeholder = "Enter text...";

	std::string m_font = "bigFont.fnt";
	bool m_fontDirty;

	std::string m_label = "";

	int m_commonFilter = static_cast<int>(CommonFilter::Any);
	std::string m_filter;
	bool m_customFilter = false;

	unsigned int m_maxLength = 0;
	bool m_isPassword = false;
	TextInputAlign m_align;

	std::string m_value = "";
	bool m_valueDirty = false;
 public:
 	VirtualTextInput() : VirtualNode() {
 		setContentWidth(200);
		m_tether = TextInput::create(200, m_placeholder.c_str(), m_font.c_str());
		setAnchorPoint({0.5, 0.5});
	}

	void settings() override {
		VirtualNode::settings();

		devtools::newLine();
		devtools::separator();
		devtools::newLine();

		m_valueDirty = devtools::property("Default Value", m_value);
		devtools::property("Placeholder", m_placeholder);
		m_fontDirty = devtools::property("Font", m_font);
		devtools::property("Label", m_label);

		devtools::property("Custom Filter", m_customFilter);
		devtools::nextItemWidth(220);
		if (m_customFilter) {
			devtools::sameLine();
			devtools::property("", m_filter);
		} else {
			devtools::sameLine();
			std::vector filterNames(filters.begin(), filters.end());
			devtools::combo("", m_commonFilter, filterNames);
		}

		devtools::nextItemWidth(100);
		devtools::property("Max Length (0 = infinite)", m_maxLength);
		devtools::property("Password", m_isPassword);

		devtools::combo("Alignment", m_align, {"Center", "Left"});
	}

	matjson::Value exportJSON() override {
		auto obj = VirtualNode::exportJSON();

		obj["type"] = "Text Input";

		auto setValIf = [&obj](char const* key, auto value, auto def) {
			if (value != def)
				obj[key] = value;
		};

		setValIf("placeholder", m_placeholder, "Enter text...");
		setValIf("font", m_font, "bigFont.fnt");
		setValIf("label", m_label, "");
		if (m_customFilter) {
			setValIf("filter", m_filter, "");
		} else {
			setValIf("filter", static_cast<int>(m_commonFilter), static_cast<int>(CommonFilter::Any));
		}
		setValIf("maxLength", m_maxLength, 0);
		setValIf("isPassword", m_isPassword, false);
		setValIf("align", static_cast<int>(m_align), static_cast<int>(TextInputAlign::Center));
		setValIf("value", m_value, "");

		return obj;
	}

	void importJSON(matjson::Value value) override {
		VirtualNode::importJSON(value);

		m_placeholder = value["placeholder"].asString().unwrapOr("Enter text...");
		m_font = value["font"].asString().unwrapOr("bigFont.fnt");
		m_fontDirty = true;
		m_label = value["label"].asString().unwrapOr("");

		if (value["filter"].isString()) {
			m_filter = value["filter"].asString().unwrapOr("");
			m_customFilter = true;
		} else {
			m_commonFilter = value["filter"].asInt().unwrapOr(static_cast<int>(CommonFilter::Any));
			m_customFilter = false;
		}

		m_maxLength = static_cast<unsigned int>(value["maxLength"].asInt().unwrapOr(0));
		m_isPassword = value["isPassword"].asBool().unwrapOr(false);
		m_align = static_cast<TextInputAlign>(value["align"].asInt().unwrapOr(static_cast<int>(TextInputAlign::Center)));
		m_value = value["value"].asString().unwrapOr("");
	}

	std::string emitCode(int indent = 0) override {
		std::string ind(indent, ' ');
		std::string out = fmt::format("{}Build<TextInput>::create({}f, {}, \"{}\")\n", ind, fmtFloat(getContentWidth()), fmtString(m_placeholder), m_font);
	
		auto emitIf = [&](auto field, auto def, std::string_view code) {
			if (field != def) {
				out += fmt::format("{}    .$build_wrap({})\n", ind, code);
			}
		};

		emitIf(m_label, "", fmt::format("setLabel({})", fmtString(m_label)));
		if (m_customFilter) {
			emitIf(m_filter, "", fmt::format("setFilter({})", fmtString(m_filter)));
		} else {
			emitIf(static_cast<CommonFilter>(m_commonFilter), CommonFilter::Any, fmt::format("setCommonFilter(CommonFilter::{})", filters[m_commonFilter]));
		}
		emitIf(m_maxLength, 0, fmt::format("setMaxCharCount({})", m_maxLength));
		emitIf(m_isPassword, false, "setPasswordMode(true)");
		emitIf(m_align, TextInputAlign::Center, fmt::format("setTextAlign(TextInputAlign::{})", m_align == TextInputAlign::Center ? "Center" : "Left"));
		emitIf(m_value, "", fmt::format("setString({})", fmtString(m_value)));
		emitIf(getContentWidth(), 200.f, fmt::format("setWidth({}f)", fmtFloat(getContentWidth())));

		auto json = exportJSON();
		json.erase("size");
		out += VirtualNode::emitAttributes(exportJSON(), indent + 4);

		return out;
	}

	void updateTether() override {
        if (m_fontDirty && FNTConfigLoadFile(m_font.c_str()))
            replaceTether(TextInput::create(getContentWidth(), m_placeholder.c_str(), m_font.c_str()));

		auto tether = reinterpret_cast<TextInput*>(m_tether.data());

		if (m_valueDirty || m_fontDirty)
			tether->setString(m_value.c_str());

		tether->setPlaceholder(m_placeholder.c_str());
		tether->setLabel(m_label.c_str());
		tether->setMaxCharCount(m_maxLength);
		tether->setPasswordMode(m_isPassword);
		tether->setTextAlign(m_align);

		if (m_commonFilter == -1) {
			tether->setFilter(m_filter.c_str());
		} else {
			tether->setCommonFilter(static_cast<CommonFilter>(m_commonFilter));
		}

		tether->setWidth(getContentWidth());
		setContentSize(tether->getContentSize());

		VirtualNode::updateTether();
	}
};

static RegisterDOM<VirtualTextInput, "Text Input"> reg;