#include "Engine.hpp"

int main()
{
    auto app = Gtk::Application::create("org.vipe");
    vipe::Engine window;

    return app->run(window);
}
