#include "app/app.hpp"

int main (int argc, char *argv[])
{
    App app{"127.0.0.1", 6969, 10};
    app.run();

    return 0;
}
