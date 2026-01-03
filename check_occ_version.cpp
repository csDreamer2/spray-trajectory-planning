#include <Standard_Version.hxx>
#include <iostream>

int main() {
    std::cout << "OpenCASCADE Version: " << OCC_VERSION_COMPLETE << std::endl;
    std::cout << "Major: " << OCC_VERSION_MAJOR << std::endl;
    std::cout << "Minor: " << OCC_VERSION_MINOR << std::endl;
    std::cout << "Maintenance: " << OCC_VERSION_MAINTENANCE << std::endl;
    return 0;
}
