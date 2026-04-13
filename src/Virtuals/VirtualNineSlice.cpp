#include "VirtualRGBA.hpp"
#include <geode.devtools/include/API.hpp>

class VirtualNineSlice : public VirtualRGBA {
    std::string m_spriteName = "GJ_square01.png";
    bool m_frameDirty;
public:
    VirtualNineSlice() : VirtualRGBA() {
        m_tether = NineSlice::create(m_spriteName.c_str());
        m_tether->setContentSize({100, 100});
        setContentSize({100, 100});
        setAnchorPoint({0.5, 0.5});
    }

    void settings() override {
        VirtualNode::settings();
        m_frameDirty = devtools::property("Sprite Name", m_spriteName);
    }

    matjson::Value exportJSON() override {
        auto obj = VirtualRGBA::exportJSON();

        obj["type"] = "Nine Slice";
        obj["spriteName"] = m_spriteName;

        return obj;
    }

    void importJSON(matjson::Value obj) override {
        m_spriteName = obj["spriteName"].asString().unwrapOr("GJ_square01.png");

        auto spr = CCSprite::create(m_spriteName.c_str());
        if (!spr || spr->getUserObject("geode.texture-loader/fallback")) 
            m_spriteName = "GJ_square01.png";

        
        m_tether = NineSlice::create(m_spriteName.c_str());
        VirtualRGBA::importJSON(obj);
    }

    std::string emitCode(int indent = 0) override {
        std::string ind(indent, ' ');
        std::string out = fmt::format("{}Build<NineSlice>::create(\"{}\")\n", ind, m_spriteName);

        out += VirtualRGBA::emitAttributes(exportJSON(), indent + 4);

        return out;
    }

    void updateTether() override {
        if (m_frameDirty) {
            auto spr = CCSprite::create(m_spriteName.c_str());
            if (spr && !spr->getUserObject("geode.texture-loader/fallback")) {
                replaceTether(NineSlice::create(m_spriteName.c_str()));
            }
        }

        VirtualRGBA::updateTether();
    }
};

static RegisterDOM<VirtualNineSlice, "Nine Slice"> reg;