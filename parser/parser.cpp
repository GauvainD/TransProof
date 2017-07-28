#include <iostream>
#include "mydriver.hpp"

int main(int argc, char const* argv[])
{
    int res = 0;
    mydriver driver;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i] == std::string("-p"))
        {
            driver.trace_parsing = true;
        }
        else if (argv[i] == std::string("-s"))
        {
            driver.trace_scanning = true;
        }
        else if (!driver.parse(argv[i]))
        {
            std::cout << driver.result << "\n";
        }
        else
        {
            res = 1;
        }
    }
    return res;
}
