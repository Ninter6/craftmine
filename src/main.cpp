#include "application.hpp"

int main() {
    app = new Application;
    app->init();
    app->run();
    app->destroy();
}
