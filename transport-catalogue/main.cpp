#include "request_handler.h"
#include "json_reader.h"

#include <iostream>
#include <fstream>
#include <string_view>
//#include <Windows.h>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    const char* command = argv[1];
    if (argc != 2) {
//#ifdef _DEBUG
//        //command = "make_base";
//        command = "process_requests";
//#else
//        PrintUsage();
//        return 1;
//#endif
        PrintUsage();
        return 1;
    }

    //SetConsoleOutputCP(1251);
    //SetConsoleCP(1251);
    transport_catalogue::TransportCatalogue db;
    renderer::MapRenderer renderer;
    router::TransportRouter route;
    std::string query;

    const std::string_view mode(argv[1]);
    //const std::string_view mode(command);

    if (mode == "make_base"sv) {

        // make base here
        json_reader::Read(db, renderer, route, std::cin, query);

    }
    else if (mode == "process_requests"sv) {

        // process requests here
        json_reader::Out(db, renderer, route, std::cin, std::cout, query);

    }
    else {
        PrintUsage();
        return 1;
    }
}