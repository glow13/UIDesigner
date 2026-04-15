#include <VirtualNode.hpp>
#include <geode.devtools/include/API.hpp>
#include <Geode/ui/MDTextArea.hpp>

class VirtualTextArea : public VirtualNode, RegisterDOM<VirtualTextArea, "Text Area"> {
    std::string m_text;
public:
    VirtualTextArea() : VirtualNode(), m_text("Hello <cr>World!</cr>") {
        m_tether = MDTextArea::create(m_text, {200, 100});
        setContentSize({200, 100});
        setAnchorPoint({0.5, 0.5});
    }

    void settings() override {
        VirtualNode::settings();
        devtools::inputMultiline("Text Area", m_text);
    }

    matjson::Value exportJSON() override {
        auto obj = VirtualNode::exportJSON();
    
        obj["type"] = "Text Area";
        obj["text"] = m_text;

        return obj;
    }

    std::string emitCode(int indent = 0) override {
        std::string ind(indent, ' ');
        std::string out = fmt::format(
            "{}Build<MDTextArea>::create({}, CCSize({}, {})\n",
            ind,
            fmtString(m_text),
            fmtFloat(getContentSize().width),
            fmtFloat(getContentSize().height)
        );

        out += VirtualNode::emitAttributes(exportJSON(), indent + 4);
        return out;
    }

    void importJSON(matjson::Value json) override {
        VirtualNode::importJSON(json);
        m_text = json["text"].asString().unwrapOr(m_text);
    }

    void updateTether() override {
        auto tether = static_cast<MDTextArea*>(m_tether.data());

        if (tether->getContentSize() != getContentSize()) {
            replaceTether(MDTextArea::create(m_text, getContentSize()));
        } else if (tether->getString() != m_text) {
            tether->setString(m_text.c_str());
        }

        VirtualNode::updateTether();
    }
};
