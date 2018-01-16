
#include <eval.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>

namespace sc {

    namespace {
        std::string generate_source_name(int seed) {
            return "__eval_src_" + std::to_string(seed) + ".cpp";
        }
        std::string generate_log_name(int seed) {
            return "__eval_log_" + std::to_string(seed) + ".txt";
        }
        std::string generate_exe_name(int seed) {
            return "__eval_exe_" + std::to_string(seed) + ".exe";
        }

        int create_process(std::string const& executable) {
            STARTUPINFO si;
            ZeroMemory( &si, sizeof(si) );
            si.cb = sizeof(si);

            PROCESS_INFORMATION pi;
            ZeroMemory( &pi, sizeof(pi) );

            auto command = "./" + executable;
            if(!CreateProcess(NULL,           // No module name (use command line)
                              command.data(), // Command line
                              NULL,           // Process handle not inheritable
                              NULL,           // Thread handle not inheritable
                              FALSE,          // Set handle inheritance to FALSE
                              0,              // No creation flags
                              NULL,           // Use parent's environment block
                              NULL,           // Use parent's starting directory
                              &si,            // Pointer to STARTUPINFO structure
                              &pi)) {         // Pointer to PROCESS_INFORMATION structure
                throw std::runtime_error{
                        "Process creation failed with code "
                        + std::to_string(GetLastError()) };
            }

            WaitForSingleObject(pi.hProcess, INFINITE);

            unsigned long code;
            if (!GetExitCodeProcess(pi.hProcess, &code)) {
                throw std::runtime_error{ "Could not get exit code" };
            }
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return static_cast<int>(code);
        }
    }

    int eval(std::string const& code) {
        int seed = 0;
        std::string srcName;
        {
            std::ofstream srcFile;
            do {
                srcName = generate_source_name(++seed);
                srcFile = std::ofstream{srcName};
            } while (!srcFile.is_open());
            srcFile << code;
        }
        auto logName = generate_log_name(seed);
        auto exeName = generate_exe_name(seed);
        auto command = "g++ " + srcName + " -o " + exeName + " 2> " + logName;

        auto gccRes = std::system(command.c_str());
        std::remove(srcName.c_str());
        if (gccRes) {
            std::ostringstream error;
            {
                std::ifstream logFile{logName};
                logFile >> error.rdbuf();
            }
            std::remove(logName.c_str());
            throw std::runtime_error{ error.str() };
        }

        auto exeRes = create_process(exeName);

        std::remove(logName.c_str());
        std::remove(exeName.c_str());
        return exeRes;
    }

}
