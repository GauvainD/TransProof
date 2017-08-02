#include <iostream>
#include <string>
#include "transfodriver.hpp"

int main(int argc, char const* argv[])
{
    int res = 0;
    transfodriver driver;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i] == std::string("-p"))
        {
            std::cout << "parsing" << "\n";
            driver.trace_parsing = true;
        }
        else if (argv[i] == std::string("-s"))
        {
            std::cout << "scanning" << "\n";
            driver.trace_scanning = true;
        }
        else
        {
            driver.parse(argv[i]);
            std::cout << driver.result << "\n";
        }
    }
    return res;
}
