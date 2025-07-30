
#include "test_helper.h"

std::string test_helper::get_test_serial()
{
    char* value = getenv("TEST_SERIAL_IC4SRC");

    if (value)
    {
        return std::string(value);
    }

    return {};

}
