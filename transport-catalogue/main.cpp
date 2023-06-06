#include "request_handler.h"
#include "json_reader.h"

#include <iostream>
#include <fstream>
//#include <Windows.h>

int main()
{
    //SetConsoleOutputCP(1251);
    //SetConsoleCP(1251);
    transport_catalogue::TransportCatalogue db;
    renderer::MapRenderer renderer;
    json_reader::Read(db, renderer, std::cin, std::cout, true);
}