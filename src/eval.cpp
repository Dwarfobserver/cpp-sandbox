
#include <eval.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>
#include <chrono>
#include <atomic>

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
            STARTUPINFO si {};
            si.cb = sizeof(si);

            PROCESS_INFORMATION pi {};

            auto command = "./" + executable; /// Comments kept (from a Stack Overflow post) :
            if(!CreateProcess(nullptr,        // No module name (use command line)
                              command.data(), // Command line
                              nullptr,        // Process handle not inheritable
                              nullptr,        // Thread handle not inheritable
                              false,          // Set handle inheritance to FALSE
                              0,              // No creation flags
                              nullptr,        // Use parent's environment block
                              nullptr,        // Use parent's starting directory
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
        static std::atomic_int next_id = 0;
        int id = next_id.fetch_add(1, std::memory_order_relaxed);

        std::string srcName;
        {
            std::ofstream srcFile;
            do {
                srcName = generate_source_name(++id);
                srcFile = std::ofstream{ srcName };
            } while (!srcFile.is_open());
            srcFile << code;
        }
        auto logName = generate_log_name(id);
        auto exeName = generate_exe_name(id);
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
