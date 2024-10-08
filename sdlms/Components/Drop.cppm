module;

#include <array>
#include <string>

export module components:drop;

import :animatedsprite;

export struct Drop
{
    std::u16string id;
    uint64_t nums;
    int rotate;
    bool land = false;
    AnimatedSprite aspr;

    enum MesoIcon
    {
        BRONZE,
        GOLD,
        BUNDLE,
        BAG,
        NUM_ICONS
    };

    Drop() = default;

    static inline std::array<AnimatedSpriteWarp *, NUM_ICONS> mesoicons;
    static void init();
};
