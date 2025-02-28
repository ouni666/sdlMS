#include "AnimatedSprite.h"
#include "Core/Map.h"
#include <vector>
#include "wz/Property.hpp"

static std::unordered_map<wz::Node *, AnimatedSpriteWarp *> cache;

AnimatedSpriteWarp *AnimatedSpriteWarp::load(wz::Node *node, int alpha, bool caches)
{
    if (caches == false)
    {
        return new AnimatedSpriteWarp(node, alpha, false);
    }
    if (cache.contains(node))
    {
        return cache[node];
    }
    else
    {
        AnimatedSpriteWarp *asprw = new AnimatedSpriteWarp(node, alpha, true);
        cache[node] = asprw;
        return asprw;
    }
}

AnimatedSpriteWarp::AnimatedSpriteWarp(wz::Node *node, int alpha, bool caches)
{
    if (node->type == wz::Type::UOL)
    {
        node = dynamic_cast<wz::Property<wz::WzUOL> *>(node)->get_uol();
    }
    if (node->type == wz::Type::Canvas)
    {
        // 单帧动画图
        auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(node);
        sprites.push_back(SpriteWarp::load(canvas, alpha, caches));
    }
    else
    {
        // 从第0帧顺序读
        for (int i = 0; i < node->children_count(); i++)
        {
            auto it = node->get_child(std::to_string(i));
            if (it == nullptr)
            {
                // 如果发现没读取到,说明已经读完,则退出读取
                break;
            }
            wz::Property<wz::WzCanvas> *canvas;
            if (it->type == wz::Type::UOL)
            {
                auto uol = dynamic_cast<wz::Property<wz::WzUOL> *>(it);
                canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(uol->get_uol());
            }
            else if (it->type == wz::Type::Canvas)
            {
                canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(it);
            }
            else
            {
                continue;
            }
            sprites.push_back(SpriteWarp::load(canvas, alpha, caches));
        }
    }
    if (node->get_child(u"zigzag") != nullptr)
    {
        // 如果存在zigzag属性,则认为属于zigzag动画
        z = true;
    }
}

AnimatedSprite::AnimatedSprite(wz::Node *node, int alpha)
{
    asprw = AnimatedSpriteWarp::load(node, alpha);
    anim_size = asprw->sprites.size();
}

AnimatedSprite::AnimatedSprite(AnimatedSpriteWarp *asprw) : asprw(asprw)
{
    anim_size = asprw->sprites.size();
}

void AnimatedSpriteWarp::clean_up()
{
    for (auto &[key, val] : cache)
    {
        delete val;
    }
    cache.clear();
}