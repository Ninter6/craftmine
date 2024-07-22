#include "render/window.hpp"

#include "math/ray.hpp"

#include <iostream>

int main() {
    Window window{{
        .extent = {1600, 1000},
        .title = "craftmine"
    }};

    window.loop();
}
