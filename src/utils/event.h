//
// Created by Ninter6 on 2024/7/16.
//

#pragma once

#include <queue>
#include <memory>
#include <optional>
#include <functional>
#include <unordered_map>

struct Event {
    // 0:none, 1:l_shift, 2:l_ctrl, 3:l_atl, 4:l_super, 5:r_shift, 6:r_ctrl, 7:r_atl, 8:r_super (KeyCode+339 when testing)
    std::optional<int> MainKey;
    std::optional<int> MinorKey;
    // 0:none, 32:space, 48~57:'0'~'9', 65~90:A~Z, 256:esc, 257:entry, 258:tap, 259:backspace, 260:insert, 261:delete, →←↓↑:262~265
    std::optional<int> NormalKey; // it will conduct separate testing
    // 0:none, 1:left, 2:right, 3:mid (KeyCode-1 when testing)
    std::optional<int> MouseButton;
    std::optional<bool> MouseMove;

    [[nodiscard]] size_t hash() const {
        size_t r = 0;
        if (MainKey) r ^= *MainKey; else r ^= 0xf;
        r <<= 4;
        if (MinorKey) r ^= *MinorKey; else r ^= 0xf;
        r <<= 4;
        if (MouseButton) r ^= *MouseButton; else r ^= 0xf;
        r <<= 2;
        if (MouseMove) r ^= *MouseMove; else r ^= 0b11;
        return r;
    }
    bool operator<(const Event& o) const {
        return hash() < o.hash();
    }
    bool operator==(const Event& o) const {
        return MainKey == o.MainKey     &&
            MinorKey == o.MinorKey      &&
            MouseMove == o.MouseMove    &&
            MouseButton == o.MouseButton;
    }
};

template <>
struct std::hash<Event> {
    size_t operator()(const Event& e) const {
        return e.hash();
    }
};

struct Listener {
    Listener(std::function<bool(int)> get_key,
             std::function<bool(int)> get_mouse_bottom,
             std::function<void(double&, double&)> get_mouse_pos) :
        GetKey(std::move(get_key)),
        GetMouseBottom(std::move(get_mouse_bottom)),
        GetMousePos(std::move(get_mouse_pos)) {
        oldNormalKeyList = normalKeyList;
    }

    void Update() {
        oldNormalKeyList.swap(normalKeyList);
        for (auto& i : normalKeyList)
            i.second = GetKey(i.first);

        status.MainKey = TestSpecialKey();
        status.MinorKey = TestSpecialKey(*status.MainKey);

        lastMousePos = currMousePos;
        GetMousePos(currMousePos.x, currMousePos.y);
        deltaMousePos.x = currMousePos.x - lastMousePos.x;
        deltaMousePos.y = currMousePos.y - lastMousePos.y;
        status.MouseMove = abs(deltaMousePos.x) > 1e-7 || abs(deltaMousePos.y) > 1e-7;

        if (status.MouseButton)
            lastMouseButtonState = *status.MouseButton;
        status.MouseButton = 0;
        for (int i = 0; i < 3; i++)
            if (GetMouseBottom(i)) {
                status.MouseButton = i + 1;
                break;
            }
    }

    [[nodiscard]] bool IsKeyDown(int key) const {
        return normalKeyList.at(key);
    }

    [[nodiscard]] bool IsKeyPressed(int key) const {
        return !oldNormalKeyList.at(key) && normalKeyList.at(key);
    }

    [[nodiscard]] bool IsKeyReleased(int key) const {
        return oldNormalKeyList.at(key) && !normalKeyList.at(key);
    }

    [[nodiscard]] bool IsMouseButtonDown(int key) const {
        return status.MouseButton == key;
    }

    [[nodiscard]] bool IsMouseButtonPressed(int key) const {
        return status.MouseButton == key && lastMouseButtonState != key;
    }

    [[nodiscard]] bool IsMouseButtonReleased(int key) const {
        return status.MouseButton != key && lastMouseButtonState == key;
    }

    [[nodiscard]] int TestSpecialKey(int ignore = 0) const {
        for (int i = 0; i < 8; i++)
            if (i + 1 != ignore)
                if (GetKey(i + 340))
                    return i + 1;
        return 0; // none pressed
    }

    Event status;

    struct Vec2 {double x, y;};

    std::function<bool(int)> GetKey;
    std::function<bool(int)> GetMouseBottom;
    std::function<void(double&, double&)> GetMousePos;

    int lastMouseButtonState{};
    Vec2 currMousePos{}, lastMousePos{}, deltaMousePos{};

    std::unordered_map<int, bool> oldNormalKeyList, normalKeyList = {
        {32, false}, {39, false}, {44, false}, {45, false}, {46, false}, {47, false}, {48, false}, {49, false}, {50, false}, {51, false}, {52, false}, {53, false}, {54, false}, {55, false}, {56, false}, {57, false}, {65, false}, {66, false}, {67, false}, {68, false}, {69, false}, {70, false}, {71, false}, {72, false}, {73, false}, {74, false}, {75, false}, {76, false}, {77, false}, {78, false}, {79, false}, {80, false}, {81, false}, {82, false}, {83, false}, {84, false}, {85, false}, {86, false}, {87, false}, {88, false}, {89, false}, {90, false}, {91, false}, {92, false}, {93, false}, {96, false}, {256, false}, {257, false}, {258, false}, {259, false}, {260, false}, {261, false}, {262, false}, {263, false}, {264, false}, {265, false}
    };
};

using EventCallback0 = std::function<void()>;
using EventCallback1 = std::function<void(double, double)>; // mouse delta
using EventCallback2 = std::function<void(const Listener&)>;

class Eventor {
public:
    explicit Eventor(std::shared_ptr<Listener> listener) : listener(std::move(listener)) {}

    void Update() const {
        listener->Update();
        const auto& status = listener->status;

        auto& normal_key_list = listener->normalKeyList;

        {  // event callback 0
            auto [b, e] = callbackmap0.equal_range(status);
            while (b != e) {
                if (!b->first.NormalKey || normal_key_list[*b->first.NormalKey])
                    callbacks0[b->second]();
                ++b;
            }
        }{ // event callback 1
            auto [b, e] = callbackmap1.equal_range(status);
            while (b != e) {
                if (!b->first.NormalKey || normal_key_list[*b->first.NormalKey])
                    callbacks1[b->second](listener->deltaMousePos.x, listener->deltaMousePos.y);
                ++b;
            }
        }{ // event callback 2
            auto [b, e] = callbackmap2.equal_range(status);
            while (b != e) {
                if (!b->first.NormalKey || normal_key_list[*b->first.NormalKey])
                    callbacks2[b->second](*listener);
                ++b;
            }
        }
    }

    void add_event(const Event& evt, const EventCallback0& callback) {
        int id = (int)callbacks0.size();
        callbacks0.push_back(callback);
        process_event(evt, id, callbackmap0);
    }

    void add_event(const Event& evt, const EventCallback1& callback) {
        int id = (int)callbacks1.size();
        callbacks1.push_back(callback);
        process_event(evt, id, callbackmap1);
    }

    void add_event(const Event& evt, const EventCallback2& callback) {
        int id = (int)callbacks2.size();
        callbacks2.push_back(callback);
        process_event(evt, id, callbackmap2);
    }

    static void process_event(const Event& evt, int id, std::unordered_multimap<Event, int>& map) {
        std::queue<Event> que;

        // MainKey
        if (!evt.MainKey) {
            auto nevt = evt;
            for (int i = 0; i < 9; i++) {
                nevt.MainKey = i;
                que.push(nevt);
            }
        } else que.push(evt);

        // MinorKey
        if (!evt.MinorKey && (!evt.MainKey || *evt.MainKey != 0)) {
            while (!que.front().MinorKey) {
                auto nevt = que.front();
                que.pop();
                for (int i = 0; i < 9; i++) {
                    nevt.MinorKey = i;
                    que.push(nevt);
                }
            }
        }

        // MouseButton
        if (!evt.MouseButton) {
            while (!que.front().MouseButton) {
                auto nevt = que.front();
                que.pop();
                for (int i = 0; i < 4; i++) {
                    nevt.MouseButton = i;
                    que.push(nevt);
                }
            }
        }

        // MouseMove
        if (!evt.MouseMove) {
            while (!que.front().MouseMove) {
                auto nevt = que.front();
                que.pop();
                nevt.MouseMove = false;
                que.push(nevt);
                nevt.MouseMove = true;
                que.push(nevt);
            }
        }

        while (!que.empty()) {
            auto e = que.front();
            que.pop();
            map.emplace(e, id);
        }
    }

private:
    std::shared_ptr<Listener> listener;

    std::vector<EventCallback0> callbacks0;
    std::vector<EventCallback1> callbacks1;
    std::vector<EventCallback2> callbacks2;

    std::unordered_multimap<Event, int> callbackmap0;
    std::unordered_multimap<Event, int> callbackmap1;
    std::unordered_multimap<Event, int> callbackmap2;

};