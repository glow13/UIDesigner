#include "VirtualRGBA.hpp"
#include <geode.devtools/include/API.hpp>

class VirtualSprite : public VirtualRGBA, RegisterDOM<VirtualSprite, "Sprite"> {
    std::string m_spriteName = "GJ_lock_001.png";
    bool m_frameDirty;

    bool m_flipX;
    bool m_flipY;

    auto tether() { return typeinfo_cast<CCSprite*>(m_tether.data()); }
public:
    VirtualSprite() : VirtualRGBA() {
        m_tether = CCSprite::createWithSpriteFrameName(m_spriteName.c_str());
        setAnchorPoint({0.5, 0.5});
    }

    void settings() override {
        VirtualNode::settings();
        m_frameDirty = devtools::property("Sprite Name", m_spriteName);

        devtools::property("Flip X", m_flipX);
        devtools::sameLine();
        devtools::property("Flip Y", m_flipY);
    }

    matjson::Value exportJSON() override {
        auto obj = VirtualRGBA::exportJSON();

        obj["type"] = "Sprite";
        obj["spriteName"] = m_spriteName;
        if (m_flipX)
            obj["flipX"] = m_flipX;
        if (m_flipY)
            obj["flipY"] = m_flipY;
        obj.erase("size");

        return obj;
    }

    void importJSON(matjson::Value obj) override {
        m_spriteName = obj["spriteName"].asString().unwrapOr(m_spriteName);
        m_flipX = obj["flipX"].asBool().unwrapOr(false);
        m_flipY = obj["flipY"].asBool().unwrapOr(false);

        m_tether = createSprite(m_spriteName);

        VirtualRGBA::importJSON(obj);
    }

    std::string emitCode(int indent = 0) override {
        std::string ind(indent, ' ');
        std::string out = ind + "Build<CCSprite>::create";

        if (auto spr = CCSprite::createWithSpriteFrameName(m_spriteName.c_str()))
            out += "SpriteFrame";
        out += fmt::format("(\"{}\")\n", m_spriteName);

        if (m_flipX)
            out += fmt::format("{}    .flipX(true)\n", ind);
        if (m_flipY)
            out += fmt::format("{}    .flipY(true)\n", ind);

        out += VirtualRGBA::emitAttributes(exportJSON(), indent + 4);

        return out;
    }

    void updateTether() override {
        if (m_frameDirty) {
            if (auto spr = createSprite(m_spriteName)) {
                replaceTether(spr);
            }
        }

        setContentSize(m_tether->getContentSize());

        VirtualRGBA::updateTether();

        tether()->setFlipX(m_flipX);
        tether()->setFlipY(m_flipY);
    }
};
