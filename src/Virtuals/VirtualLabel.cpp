#include "VirtualRGBA.hpp"
#include <geode.devtools/include/API.hpp>

constexpr std::array alignments = {"Left", "Center", "Right"};

class VirtualLabel : public VirtualRGBA, RegisterDOM<VirtualLabel, "Label"> {
    std::string m_text;
    std::string m_font;
    float m_kerning = 1.0;
    bool m_fontDirty;
    int m_alignment = kCCTextAlignmentLeft;
    bool m_breakWithoutSpace;

    auto tether() { return typeinfo_cast<CCLabelBMFont*>(m_tether.data()); }
public:
    VirtualLabel() : VirtualRGBA(), m_text("My Label"), m_font("bigFont.fnt"), m_fontDirty(false) {
        m_tether = CCLabelBMFont::create(m_text.c_str(), m_font.c_str());
        setAnchorPoint({0.5, 0.5});
    }

    void settings() override {
        VirtualNode::settings();

        devtools::newLine();
        devtools::separator();
        devtools::newLine();

        devtools::inputMultiline("Label Text", m_text);
        m_fontDirty = devtools::property("Font File", m_font);

        devtools::property("Extra Kerning", m_kerning);
        std::vector alignmentNames(alignments.begin(), alignments.end());
        devtools::combo("Alignment", m_alignment, alignmentNames);
        devtools::property("Break Without Space", m_breakWithoutSpace);
    }

    matjson::Value exportJSON() override {
        auto obj = VirtualRGBA::exportJSON();

        obj["type"] = "Label";
        obj["text"] = m_text;
        obj["font"] = m_font;

        if (m_alignment != kCCTextAlignmentLeft)
            obj["alignment"] = m_alignment;
        if (m_kerning != 0)
            obj["kerning"] = m_kerning;
        if (m_breakWithoutSpace)
            obj["breakWithoutSpace"] = m_breakWithoutSpace;

        obj.erase("size");

        return obj;
    }

    void importJSON(matjson::Value obj) override {
        m_text = obj["text"].asString().unwrapOr("My Label");
        m_font = obj["font"].asString().unwrapOr("bigFont.fnt");
        m_kerning = obj["kerning"].asDouble().unwrapOr(0.f);
        m_alignment = static_cast<CCTextAlignment>(obj["alignment"].asInt().unwrapOr(0));
        m_breakWithoutSpace = obj["breakWithoutSpace"].asBool().unwrapOr(false);

        m_tether = CCLabelBMFont::create(m_text.c_str(), m_font.c_str());

        VirtualRGBA::importJSON(obj);
    }

    std::string emitCode(int indent) override {
        std::string ind(indent, ' ');
        std::string out = fmt::format("{}Build<CCLabelBMFont>::create({}, \"{}\")\n", ind, fmtString(m_text), m_font);

        constexpr static std::array alignEnums = { "kCCTextAlignmentLeft", "kCCTextAlignmentCenter", "kCCTextAlignmentRight" };
        
        if (m_alignment != kCCTextAlignmentLeft)
            out += fmt::format("{}    .alignment({})\n", ind, alignEnums[m_alignment]);
        if (m_kerning != 1)
            out += fmt::format("{}    .kerning({})\n", ind, fmtFloat(m_kerning));
        if (m_breakWithoutSpace)
            out += fmt::format("{}    .breakWithouSpace({})\n", ind, m_breakWithoutSpace ? "true" : "false");

        out += VirtualRGBA::emitAttributes(exportJSON(), indent + 4);
        if (out.back() == '\n')
            out.pop_back();

        return out;
    }

    void updateTether() override {
        tether()->setString(m_text.c_str());
        tether()->setExtraKerning(m_kerning);
        tether()->setAlignment(static_cast<CCTextAlignment>(m_alignment));
        tether()->setLineBreakWithoutSpace(m_breakWithoutSpace);

        if (m_fontDirty && FNTConfigLoadFile(m_font.c_str())) 
            tether()->setFntFile(m_font.c_str());

        setContentSize(m_tether->getContentSize());

        VirtualRGBA::updateTether();
    }
};

VirtualNode* createLabel() {
    return new VirtualLabel();
}
