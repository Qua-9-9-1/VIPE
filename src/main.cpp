#include "Engine.hpp"

int main()
{
    auto app = Gtk::Application::create("org.vipe");
    vipe::MyWindow window;

    return app->run(window);
}
