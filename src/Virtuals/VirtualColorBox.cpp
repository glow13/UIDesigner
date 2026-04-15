#include "VirtualRGBA.hpp"
#include <geode.devtools/include/API.hpp>

class VirtualColorBox : public VirtualRGBA, RegisterDOM<VirtualColorBox, "Color Box"> {

public:
    VirtualColorBox() : VirtualRGBA() {
        m_tether = CCLayerColor::create(ccc4(m_color.r, m_color.g, m_color.b, m_opacity));
        setAnchorPoint({0.5, 0.5});
        setContentSize({100., 100.});
    }

    matjson::Value exportJSON() override {
        auto obj = VirtualRGBA::exportJSON();
        obj["type"] = "Color Box";
        return obj;
    }

    std::string emitCode(int indent = 0) override {
        std::string ind(indent, ' ');
        std::string output = fmt::format("{}Build<CCLayerColor>::create()\n", ind);

        output += VirtualRGBA::emitAttributes(exportJSON(), indent + 4);

        return output;
    }
};
