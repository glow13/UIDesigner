#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Dispatch.hpp>

#define MY_MOD_ID "camila314.uidesigner"
using namespace geode::prelude;


namespace uidesigner {
    class VirtualNode;
    using VirtualCreator = VirtualNode*(*)();

    class VirtualDOMManager final {
        std::unordered_map<std::string, VirtualCreator> m_creators;
        std::vector<char const*> m_creatorNames;

        VirtualDOMManager();
    public:
        static VirtualDOMManager* get() GEODE_EVENT_EXPORT_NORES(&VirtualDOMManager::get, ());

        virtual VirtualNode* initialize(CCLayer*);
        virtual void registerType(std::string_view name, VirtualCreator ctor);
        virtual VirtualNode* createFromJSON(matjson::Value);

        template <std::derived_from<VirtualNode> T>
        static int registerCreate(char const name[]) {
            VirtualDOMManager::get()->registerType(name, +[]() -> VirtualNode* { return new T(); });
            return 0;
        }
    };

    template <typename T, string::ConstexprString Name>
    class RegisterDOM {
        static inline int registration = VirtualDOMManager::registerCreate<T>(Name.data());
        static inline auto registerRef = &registration;
    };

    inline std::string fmtFloat(matjson::Value thing) {
        auto val = thing.asDouble().unwrapOr(0.f);
        auto formatted = fmt::format("{:.4f}", val);

        if (auto last = formatted.find_last_not_of('0'); last != std::string::npos) {
            formatted = formatted.substr(0, last + 1);
        }

        return formatted;
    };

    inline CCSprite* createSprite(std::string const& name) {
        auto spr = CCSprite::createWithSpriteFrameName(name.c_str());
        if (!spr || spr->getUserObject("geode.texture-loader/fallback"))
            spr = CCSprite::create(name.c_str());

        if (!spr || spr->getUserObject("geode.texture-loader/fallback"))
            return nullptr;

        return spr;
    }

    inline std::string fmtSprite(std::string const& spriteName) {
        if (CCSprite::createWithSpriteFrameName(spriteName.c_str()))
            return fmt::format("Build<CCSprite>::createSpriteFrame(\"{}\")", spriteName);
        else
            return fmt::format("CCSprite::create(\"{}\")", spriteName);
    }

    inline std::string fmtString(std::string const& str) {
        // replace all backslash with double backslash, all newline with \n, all double quotes with \"
        std::string formatted;
        for (auto c : str) {
            switch (c) {
                case '\\':
                    formatted += "\\\\";
                    break;
                case '\n':
                    formatted += "\\n";
                    break;
                case '\"':
                    formatted += "\\\"";
                    break;
                default:
                    formatted += c;
            }
        }
        return fmt::format("\"{}\"", formatted);
    }
}
