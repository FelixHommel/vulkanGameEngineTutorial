#include "core/Application.hpp"

#include <exception>
#include <iostream>
#include <ostream>

int main()
{
    Application app{};

    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    return 0;
}
