#include "framework/framework.hpp"

int main (int argc, char *argv[])
{
    App app{"0.0.0.0", 6969, 10};
    app.run();

    return 0;
}
