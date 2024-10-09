module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>
#include <variant>

module systems;

import components;
import commons;
import core;
import entities;
import :move;

void player_statemachine_run()
{
    if (auto ent = Player::ent; World::registry->valid(ent))
    {
        player_cooldown(Window::delta_time);
        if (auto hit = World::registry->try_get<Hit>(ent))
        {
            player_hit(hit, &ent);
            World::registry->remove<Hit>(ent);
        }
        player_statemachine(&ent, (float)Window::delta_time / 1000);
    }
}

void player_statemachine(entt::entity *ent, float delta_time)
{
    auto mv = World::registry->try_get<Move>(*ent);
    auto tr = World::registry->try_get<Transform>(*ent);
    auto cha = World::registry->try_get<Character>(*ent);
    auto state = cha->state;

    switch (cha->state)
    {
    case Character::State::ALERT:
    case Character::State::STAND:
    {
        player_flip(tr);
        if (player_climb(mv, tr, cha->state))
        {
            cha->state = Character::State::CLIMB;
            break;
        }
        if (player_prone(mv, tr, cha->state))
        {
            cha->state = Character::State::PRONE;
            break;
        }
        cha->state = player_walk(mv, tr, delta_time);
        if (player_jump(mv, cha, tr, cha->state))
        {
            cha->state = Character::State::JUMP;
        }
        if (player_skill(mv, cha, tr, cha->state, ent))
        {
            cha->state = Character::State::SKILL;
            break;
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::WALK:
    {
        // normal status,can walk or jump
        player_flip(tr);
        if (player_climb(mv, tr, cha->state))
        {
            cha->state = Character::State::CLIMB;
            break;
        }
        if (player_prone(mv, tr, cha->state))
        {
            cha->state = Character::State::PRONE;
            break;
        }
        cha->state = player_walk(mv, tr, delta_time);
        if (player_jump(mv, cha, tr, cha->state))
        {
            cha->state = Character::State::JUMP;
        }
        if (player_skill(mv, cha, tr, cha->state, ent))
        {
            cha->state = Character::State::SKILL;
            break;
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::JUMP:
    {
        player_flip(tr);
        if (!player_fall(mv, tr, delta_time))
        {
            cha->state = Character::State::STAND;
            player_statemachine(ent, 0);
        }
        if (player_climb(mv, tr, cha->state))
        {
            cha->state = Character::State::CLIMB;
            break;
        }
        if (player_skill(mv, cha, tr, cha->state, ent))
        {
            cha->state = Character::State::SKILL;
            break;
        }
        if (player_double_jump(mv, tr, ent))
        {
            break;
        }
        cha->state = player_attack(mv, cha, tr, cha->state, ent);
    }
    break;
    case Character::State::ATTACK:
    {
        if (!player_attacking(mv, cha, tr, ent, delta_time))
        {
            player_statemachine(ent, 0);
        }
    }
    break;
    case Character::State::SKILL:
    {
        if (!player_skilling(mv, cha, tr, ent, delta_time))
        {
            player_statemachine(ent, 0);
        }
    }
    break;
    case Character::State::CLIMB:
    {
        player_flip(tr);
        if (player_jump(mv, cha, tr, cha->state))
        {
            cha->state = Character::State::JUMP;
            break;
        }
        if (player_skill(mv, cha, tr, cha->state, ent))
        {
            cha->state = Character::State::SKILL;
            break;
        }
        cha->state = player_climbing(cha, mv, tr, cha->state, ent, delta_time);
    }
    break;
    case Character::State::PRONE:
    {
        player_flip(tr);
        if (player_jump(mv, cha, tr, cha->state))
        {
            cha->state = Character::State::JUMP;
            break;
        }
        if (player_skill(mv, cha, tr, cha->state, ent))
        {
            cha->state = Character::State::SKILL;
            break;
        }
        if (!player_proning())
        {
            cha->state = Character::State::STAND;
            player_statemachine(ent, 0);
        }
    }
    break;
    case Character::State::DIE:
    {
        player_invincible_cooldown = 2000;
        return;
    }
    break;
    default:
        break;
    }
    player_portal(mv, ent);
    player_action(cha, state, cha->state, mv);
}

void player_flip(Transform *tr)
{
    if (Input::state[SDL_SCANCODE_RIGHT])
    {
        tr->flip = 1;
    }
    else if (Input::state[SDL_SCANCODE_LEFT])
    {
        tr->flip = 0;
    }
}

int player_walk(Move *mv, Transform *tr, float delta_time)
{
    auto state = Character::State::WALK;

    if (Input::state[SDL_SCANCODE_RIGHT])
    {
        mv->hforce = 1400;
    }
    else if (Input::state[SDL_SCANCODE_LEFT])
    {
        mv->hforce = -1400;
    }
    else
    {
        if (player_alert())
        {
            state = Character::State::ALERT;
        }
        else
        {
            state = Character::State::STAND;
        }
        mv->hforce = 0;
        // 如果没有左右的输入并且速度为0,则可以直接return提高性能
        if (mv->hspeed == 0)
        {
            return state;
        }
    }
    if (move_move(mv, tr, 800, delta_time))
    {
        return state;
    }
    else if (mv->foo)
    {
        return Character::State::WALK;
    }
    else
    {
        return Character::State::JUMP;
    }
}

bool player_fall(Move *mv, Transform *tr, float delta_time)
{
    if (Input::state[SDL_SCANCODE_RIGHT])
    {
        mv->hspeed += 0.3f;
    }
    else if (Input::state[SDL_SCANCODE_LEFT])
    {
        mv->hspeed -= 0.3f;
    }

    // 默认重力为2000
    mv->vspeed += delta_time * 2000;

    return move_fall(mv, tr, delta_time, CHARACTER_Z, player_foothold_cooldown <= 0);
}

bool player_jump(Move *mv, Character *cha, Transform *tr, int state)
{
    if (Input::state[SDL_SCANCODE_LALT])
    {
        if (mv->foo)
        {
            if (state == Character::State::PRONE)
            {
                if (player_down_jump(mv, tr))
                {
                    mv->vspeed = -100;
                    mv->page = mv->foo->page;
                    mv->zmass = mv->foo->zmass;
                    mv->foo = nullptr;
                    mv->lr = nullptr;
                    player_foothold_cooldown = 120;
                    SkillWarp::cooldowns[u"4111006"] = 550;

                    Sound::sound_list.push_back(Sound(u"Game.img/Jump"));

                    return true;
                }
            }
            else
            {
                mv->vspeed = -555;
                mv->page = mv->foo->page;
                mv->zmass = mv->foo->zmass;
                mv->foo = nullptr;
                mv->lr = nullptr;
                SkillWarp::cooldowns[u"4111006"] = 250;

                Sound::sound_list.push_back(Sound(u"Game.img/Jump"));

                return true;
            }
        }
        else if (mv->lr)
        {
            if (!Input::state[SDL_SCANCODE_UP] && (Input::state[SDL_SCANCODE_RIGHT] || Input::state[SDL_SCANCODE_LEFT]))
            {
                cha->animate = true;

                mv->vspeed = -310;
                mv->lr = nullptr;
                if (Input::state[SDL_SCANCODE_RIGHT])
                {
                    mv->hspeed = 140;
                }
                else if (Input::state[SDL_SCANCODE_LEFT])
                {
                    mv->hspeed = -140;
                }
                player_ladderrope_cooldown = 80;

                SkillWarp::cooldowns[u"4111006"] = 300;

                Sound::sound_list.push_back(Sound(u"Game.img/Jump"));

                return true;
            }
        }
    }
    return false;
}

bool player_down_jump(Move *mv, Transform *tr)
{
    auto view = World::registry->view<FootHold>();
    for (auto &e : view)
    {
        auto fh = &view.get<FootHold>(e);
        if (fh->get_y(tr->position.x).has_value() &&
            fh->get_y(tr->position.x).value() < tr->position.y + 600 &&
            fh->get_y(tr->position.x).value() > tr->position.y &&
            fh != mv->foo)
        {
            return true;
        }
    }
    return false;
}

int player_attack(Move *mv, Character *cha, Transform *tr, int state, entt::entity *ent)
{
    if (Input::state[SDL_SCANCODE_LCTRL])
    {
        if (state != Character::State::JUMP)
        {
            mv->hspeed = 0;
        }
        // add afterimg
        World::registry->emplace_or_replace<AfterImage>(*ent);

        state = Character::State::ATTACK;
        player_alert_cooldown = 4000;
        cha->animated = false;
    }
    return state;
}

bool player_attacking(Move *mv, Character *cha, Transform *tr, entt::entity *ent, float delta_time)
{
    if (mv->foo == nullptr)
    {
        if (player_fall(mv, tr, delta_time))
        {
            // 空中
            if (cha->animated)
            {
                // 攻击动画完成
                cha->state = Character::State::JUMP;
                return false;
            }
        }
        else
        {
            // 落地
            if (cha->animated)
            {
                // 攻击动画完成
                cha->state = Character::State::STAND;
                return false;
            }
            mv->hspeed = 0;
        }
    }
    else
    {
        // 地面
        if (cha->animated)
        {
            // 攻击动画完成
            cha->state = Character::State::STAND;
            return false;
        }
    }
    return true;
}

bool player_climb(Move *mv, Transform *tr, int state)
{
    if (player_ladderrope_cooldown <= 0)
    {
        if (Input::state[SDL_SCANCODE_UP] || Input::state[SDL_SCANCODE_DOWN])
        {
            auto view = World::registry->view<LadderRope>();
            for (auto &e : view)
            {
                auto lr = &view.get<LadderRope>(e);

                // 判断x坐标是否在梯子范围内
                if (tr->position.x >= lr->x - 15 && tr->position.x <= lr->x + 15)
                {
                    int t = 0;
                    int b = lr->b + 5;
                    if (mv->foo)
                    {
                        if (Input::state[SDL_SCANCODE_DOWN])
                        {
                            t = lr->t - 10;
                            b = lr->t;
                        }
                        else
                        {
                            t = lr->b;
                        }
                    }
                    else
                    {
                        t = lr->t;
                    }
                    if (!(mv->foo == nullptr && Input::state[SDL_SCANCODE_DOWN]))
                    {
                        if (tr->position.y >= t && tr->position.y <= b)
                        {
                            // 爬到梯子
                            mv->lr = lr;
                            mv->zmass = 0;
                            mv->hspeed = 0;
                            mv->vspeed = 0;
                            mv->foo = nullptr;

                            tr->position.x = lr->x;
                            tr->position.y = std::clamp(tr->position.y, (float)lr->t, (float)lr->b);

                            tr->z = lr->page * LAYER_Z + CHARACTER_Z;
                            World::zindex = true;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

int player_climbing(Character *cha, Move *mv, Transform *tr, int state, entt::entity *ent, float delta_time)
{
    if (state == Character::State::JUMP)
    {
        return state;
    }
    state = Character::State::CLIMB;
    if (Input::state[SDL_SCANCODE_DOWN])
    {
        mv->vspeed = 100;
        cha->animate = true;
    }
    else if (Input::state[SDL_SCANCODE_UP])
    {
        mv->vspeed = -100;
        cha->animate = true;
    }
    else
    {
        mv->vspeed = 0;
        cha->animate = false;
    }

    auto d_y = mv->vspeed * delta_time;
    auto y = d_y + tr->position.y;

    if (y >= mv->lr->t && y <= mv->lr->b)
    {
        tr->position.y = y;
    }
    else if (y < mv->lr->t)
    {
        if (mv->lr->uf == 1)
        {
            tr->position.y = mv->lr->t - 5;
            mv->vspeed = 0;
            state = Character::State::JUMP;
            player_ladderrope_cooldown = 80;
            cha->animate = true;
        }
        else
        {
            cha->animate = false;
        }
    }
    else if (y > mv->lr->b)
    {
        tr->position.y = mv->lr->b;
        mv->vspeed = 0;
        state = Character::State::JUMP;
        player_ladderrope_cooldown = 80;
        cha->animate = true;
    }
    return state;
}

bool player_prone(Move *mv, Transform *tr, int state)
{
    if (Input::state[SDL_SCANCODE_DOWN])
    {
        if (mv->foo)
        {
            mv->hspeed = 0;
            return true;
        }
    }
    return false;
}

bool player_proning()
{
    if (Input::state[SDL_SCANCODE_DOWN])
    {
        return true;
    }
    return false;
}

void player_action(Character *cha, int state, int new_state, Move *mv)
{
    if (state != new_state)
    {
        int action = new_state;
        switch (action)
        {
        case Character::State::STAND:
        {
            auto weaponinfo = World::registry->try_get<WeaponInfo>(Player::ent);
            if (weaponinfo != nullptr && weaponinfo->stand1 == false)
            {
                action = Character::ACTION::STAND2;
            }
            else
            {
                action = Character::ACTION::STAND1;
            }
        }
        break;
        case Character::State::WALK:
        {
            auto weaponinfo = World::registry->try_get<WeaponInfo>(Player::ent);
            if (weaponinfo != nullptr && weaponinfo->walk1 == false)
            {
                action = Character::ACTION::WALK2;
            }
            else
            {
                action = Character::ACTION::WALK1;
            }
        }
        break;
        case Character::State::JUMP:
            action = Character::ACTION::JUMP;
            break;
        case Character::State::ATTACK:
        {
            action = player_attack_action(World::registry->try_get<WeaponInfo>(Player::ent));
            if (auto aft = World::registry->try_get<AfterImage>(Player::ent))
            {
                auto weaponinfo = World::registry->try_get<WeaponInfo>(Player::ent);
                aft->aspr = AnimatedSprite(AfterImage::afterimages[weaponinfo->afterImage][u"0"][action].asprw);
                aft->info = AfterImage::afterimages[weaponinfo->afterImage][u"0"][action];
            }
        }
        break;
        case Character::State::CLIMB:
        {
            switch (mv->lr->l)
            {
            case 1:
                action = Character::ACTION::LADDER;
                break;
            default:
                action = Character::ACTION::ROPE;
                break;
            }
        }
        break;
        case Character::State::PRONE:
            action = Character::ACTION::PRONE;
            break;
        case Character::State::ALERT:
            action = Character::ACTION::ALERT;
            break;
        case Character::State::SKILL:
            if (cha->action_str == u"")
            {
                // 从攻击动作随机选择一个
                action = player_attack_action(World::registry->try_get<WeaponInfo>(Player::ent));
            }
            else if (Character::type_map.contains(cha->action_str))
            {
                action = Character::type_map.at(cha->action_str);
            }
        }
        cha->action_frame = 0;
        cha->action_index = 0;
        cha->action_time = 0;
        cha->action = action;
        cha->action_str = Character::type_map2.at(action);
    }
}

void player_cooldown(int delta_time)
{
    if (player_foothold_cooldown > 0)
    {
        player_foothold_cooldown -= delta_time;
    }
    if (player_portal_cooldown > 0)
    {
        player_portal_cooldown -= delta_time;
    }
    if (player_alert_cooldown > 0)
    {
        player_alert_cooldown -= delta_time;
    }
    if (player_invincible_cooldown > 0)
    {
        player_invincible_cooldown -= delta_time;
    }
    if (player_ladderrope_cooldown > 0)
    {
        player_ladderrope_cooldown -= delta_time;
    }
}

bool player_alert()
{
    return player_alert_cooldown > 0;
}

bool player_hit(Hit *hit, entt::entity *ent)
{
    player_invincible_cooldown = 2000;

    auto mv = World::registry->try_get<Move>(*ent);
    auto tr = World::registry->try_get<Transform>(*ent);
    auto cha = World::registry->try_get<Character>(*ent);

    cha->hp -= hit->damage;
    if (cha->hp > 0)
    {
        if (mv->foo)
        {
            mv->vspeed = -320;

            auto hit_x = hit->x;
            auto cha_x = tr->position.x;
            if (cha_x < hit_x)
            {
                mv->hspeed = -110;
            }
            else
            {
                mv->hspeed = 110;
            }
        }
        mv->foo = nullptr;
        player_alert_cooldown = 5000;

        if (cha->state == Character::State::STAND || cha->state == Character::State::WALK || cha->state == Character::State::ALERT || cha->state == Character::State::PRONE)
        {
            cha->state = Character::State::JUMP;
            cha->action_index = 0;
            cha->action_time = 0;
            cha->action_frame = 0;
            cha->action = Character::ACTION::JUMP;
            cha->action_str = u"jump";
        }
    }
    else
    {
        cha->state = Character::State::DIE;
        cha->action_index = 0;
        cha->action_time = 0;
        cha->action_frame = 0;
        cha->action = Character::ACTION::DEAD;
        cha->action_str = u"dead";

        if (!mv->foo)
        {
            auto view = World::registry->view<FootHold>();
            std::optional<float> y = std::nullopt;
            for (auto &e : view)
            {
                auto fh = &view.get<FootHold>(e);
                if (fh->get_y(tr->position.x).has_value() && fh->get_y(tr->position.x).value() > tr->position.y)
                {
                    if (!y.has_value())
                    {
                        y = (float)fh->get_y(tr->position.x).value();
                    }
                    else
                    {
                        y = std::min(y.value(), (float)fh->get_y(tr->position.x).value());
                    }
                }
            }
            if (y.has_value())
            {
                tr->position.y = y.value();
            }
        }

        auto &tomb = World::registry->emplace_or_replace<Tomb>(*ent);
        tomb.f.position = tr->position;
        tomb.f.position.y -= 200;
        tomb.l.position = tr->position;
    }
    return false;
}

bool player_skill(Move *mv, Character *cha, Transform *tr, int state, entt::entity *ent)
{
    auto play_skill_sound = [](SkillWarp *souw) -> void
    {
        // 技能音效
        Sound sou;
        sou.souw = souw->sounds[u"Use"];
        Sound::sound_list.push_back(sou);
    };

    std::u16string id = u"";

    if (Input::state[SDL_SCANCODE_A] && state != Character::State::CLIMB)
    {
        id = u"2201005";
        if (state != Character::State::JUMP)
        {
            mv->hspeed = 0;
        }
    }
    else if (Input::state[SDL_SCANCODE_SPACE] && SkillWarp::cooldowns[u"2201002"] <= 0)
    {
        // 快速移动
        id = u"2201002";

        // 添加effect
        auto eff = World::registry->try_get<Effect>(*ent);
        eff->effects.push_back({new Transform(tr->position.x, tr->position.y), AnimatedSprite(Effect::load(u"BasicEff.img/Teleport"))});
        eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"BasicEff.img/Teleport"))});

        auto x = tr->position.x;
        auto y = tr->position.y;

        if (Input::state[SDL_SCANCODE_RIGHT])
        {
            x += 300;
        }
        else if (Input::state[SDL_SCANCODE_LEFT])
        {
            x -= 300;
        }
        else if (Input::state[SDL_SCANCODE_DOWN])
        {
            y += 300;
        }
        else if (Input::state[SDL_SCANCODE_UP])
        {
            y -= 300;
        }
        if (x != tr->position.x)
        {
            if (!(state == Character::State::JUMP || state == Character::State::CLIMB))
            {
                // 地面快速移动
                // 朝右移动
                while (x > mv->foo->r)
                {
                    if (mv->foo->next != nullptr && mv->foo->next->k.has_value())
                    {
                        mv->foo = mv->foo->next;
                    }
                    else
                    {
                        x = mv->foo->r - 1;
                        break;
                    }
                }
                while (x < mv->foo->l)
                {
                    if (mv->foo->prev != nullptr && mv->foo->prev->k.has_value())
                    {
                        mv->foo = mv->foo->prev;
                    }
                    else
                    {
                        x = mv->foo->l + 1;
                        break;
                    }
                }
                y = mv->foo->get_y(x).value();
            }
            else
            {
                // 空中快速移动,只需要根据fh链表找
                x = tr->position.x;
                y = tr->position.y;
            }
        }
        else if (y != tr->position.y)
        {
            bool find = false;
            auto view = World::registry->view<FootHold>();
            for (auto &e : view)
            {
                auto fh = &view.get<FootHold>(e);
                if (fh->k.has_value() && fh != mv->foo)
                {
                    if (x >= fh->l && x <= fh->r)
                    {
                        auto pos_y = fh->get_y(x).value();
                        if (pos_y == std::clamp(pos_y, std::min(y, tr->position.y), std::max(y, tr->position.y)))
                        {
                            mv->foo = fh;
                            find = true;
                            break;
                        }
                    }
                }
            }
            if (find)
            {
                y = mv->foo->get_y(x).value() - 5;
                cha->state = Character::State::JUMP;
                cha->action_index = 0;
                cha->action_time = 0;
                cha->action = Character::ACTION::JUMP;
                cha->action_str = u"jump";
            }
            else
            {
                y = tr->position.y;
            }
        }
        mv->hspeed = 0;
        mv->vspeed = 0;

        tr->position.x = x;
        tr->position.y = y;
        // 技能音效
        play_skill_sound(SkillWarp::load(id));

        SkillWarp::cooldowns[u"2201002"] = 500;
        player_portal_cooldown = 600;

        return false;
    }
    if (id != u"")
    {
        auto eff = World::registry->try_get<Effect>(*ent);
        auto &ski = World::registry->emplace_or_replace<Skill>(*ent, id);

        for (auto &it : ski.ski->effects)
        {
            eff->effects.push_back({nullptr, AnimatedSprite(it)});
        }

        state = Character::State::SKILL;
        player_alert_cooldown = 4000;
        if (ski.ski->action_str.has_value())
        {
            cha->action_str = ski.ski->action_str.value();
        }
        else
        {
            // 随机动作
            cha->action_str = u"";
        }
        cha->animated = false;

        // 技能音效
        play_skill_sound(ski.ski);
        return true;
    }
    return false;
}

bool player_skilling(Move *mv, Character *cha, Transform *tr, entt::entity *ent, float delta_time)
{
    return player_attacking(mv, cha, tr, ent, delta_time);
}

void player_portal(Move *mv, entt::entity *ent)
{
    if (player_portal_cooldown <= 0)
    {
        auto tr = World::registry->try_get<Transform>(*ent);
        auto view = World::registry->view<Portal>();
        for (auto &e : view)
        {
            auto por = &view.get<Portal>(e);
            if (por->tm != 999999999)
            {
                if (por->pt == 3 || Input::state[SDL_SCANCODE_UP])
                {
                    auto player_pos = tr->position;
                    auto por_tr = World::registry->try_get<Transform>(e);
                    auto por_x = por_tr->position.x;
                    auto por_y = por_tr->position.y;
                    if (player_pos.x == std::clamp(player_pos.x, por_x - 40, por_x + 40) &&
                        player_pos.y == std::clamp(player_pos.y, por_y - 50, por_y + 50))
                    {
                        if (por->tm != Map::id)
                        {
                            // need to change map
                            World::TransPort::id = por->tm;
                            World::TransPort::tn = std::get<std::u16string>(por->tn);
                        }
                        else
                        {
                            auto eff = World::registry->try_get<Effect>(*ent);
                            eff->effects.push_back({new Transform(tr->position.x, tr->position.y), AnimatedSprite(Effect::load(u"BasicEff.img/Summoned"))});
                            eff->effects.push_back({nullptr, AnimatedSprite(Effect::load(u"BasicEff.img/Summoned"))});

                            auto position = std::get<SDL_FPoint>(por->tn);
                            tr->position.x = position.x;
                            tr->position.y = position.y - 5;
                            auto cha = World::registry->try_get<Character>(Player::ent);
                            cha->state = Character::State::JUMP;
                        }
                        mv->hspeed = 0;
                        mv->vspeed = 0;
                        player_portal_cooldown = 800;
                        break;
                    }
                }
            }
        }
    }
}

bool player_double_jump(Move *mv, Transform *tr, entt::entity *ent)
{
    // 二段跳
    if (Input::state[SDL_SCANCODE_LALT] && SkillWarp::cooldowns[u"4111006"] <= 0)
    {
        mv->vspeed -= 420;
        if (tr->flip == 1)
        {
            // 朝右
            mv->hspeed += 480;
        }
        else
        {
            mv->hspeed -= 480;
        }
        // 添加effect
        auto eff = World::registry->try_get<Effect>(*ent);
        eff->effects.push_back({new Transform(tr->position.x, tr->position.y), AnimatedSprite(Effect::load(u"BasicEff.img/Flying"))});

        // 技能音效
        auto ski = SkillWarp::load(u"4111006");
        auto souw = ski->sounds[u"Use"];

        Sound sou;
        sou.souw = souw;
        Sound::sound_list.push_back(sou);

        SkillWarp::cooldowns[u"4111006"] = 10000;
        return true;
    }
    return false;
}

uint8_t player_attack_action(WeaponInfo *wea)
{
    auto &v = wea->attack_stances[wea->attack];
    int random = std::rand() % v.size();
    return v[random];
}

bool player_pick_drop(Character *cha, Transform *tr)
{
    if (Input::state[SDL_SCANCODE_Z])
    {
        // 捡起物品
        for (auto &ent : World::registry->view<Drop>())
        {
            auto dro = World::registry->try_get<Drop>(ent);
            if (dro->pick == false)
            {
                auto player_pos = tr->position;
                auto dro_tr = World::registry->try_get<Transform>(ent);
                auto dro_x = dro_tr->position.x;
                auto dro_y = dro_tr->position.y;
                if (player_pos.x == std::clamp(player_pos.x, dro_x - 10, dro_x + 10) &&
                    player_pos.y == std::clamp(player_pos.y, dro_y - 10, dro_y + 10))
                {
                    // 捡起物品
                    dro->pick = true;

                    auto mv = World::registry->try_get<Move>(ent);
                    mv->vspeed = -555;

                    // 播放声音
                    Sound::sound_list.push_back(Sound(u"Game.img/PickUpItem"));
                }
            }
        }
    }
    return false;
}
