
#include "SP_LAB_3.h"
#include "ast.h"

#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <filesystem>

extern int yyparse();
extern FILE* yyin;
extern std::unique_ptr<Program> root;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: SP_LAB_3 <source_file>\n";
        return 1;
    }

    FILE* file = nullptr;

    if (fopen_s(&file, argv[1], "r") != 0 || !file)
    {
        std::cerr << "Cannot open file: " << argv[1] << '\n';
        return 1;
    }

    yyin = file;

    int result = yyparse();
    fclose(file);

    if (result != 0 || !root)
    {
        std::cerr << "Syntax analysis: FAILED\n";
        return 1;
    }

    std::cout << "Syntax analysis: SUCCESS\n";

    try
    {
        std::filesystem::path sourcePath(argv[1]);

        std::filesystem::path astPath =
            sourcePath.parent_path() / "ast.dot";

        root->exportToDot(astPath.string());

        std::cout << "AST saved to: "
            << std::filesystem::absolute(astPath).string()
            << '\n';

        std::cout << "\nProgram output:\n";
        int returnValue = root->execute();
        std::cout << "\nProgram return value: "
            << returnValue << '\n';
    }
    catch (const std::exception& error)
    {
        std::cerr << "Runtime error: "
            << error.what() << '\n';
        return 1;
    }


    return 0;
}
