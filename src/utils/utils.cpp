#pragma once
#include "utils/utils.h"
#include <filesystem>
#include <Windows.h>

namespace SpaceEngine
{
    std::wstring toWide(const std::string& str)
    {
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
        std::wstring result(size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
        result.pop_back(); // remove null terminator
        return result;
    }

    bool Utils::fileExists(const std::string &path) 
    {
        #ifdef _WIN64
            std::wstring wpath = toWide(path);
    
            WIN32_FIND_DATAW data;
            HANDLE h = FindFirstFileW(wpath.c_str(), &data);
            if (h == INVALID_HANDLE_VALUE)
                return false;
    
            bool match = (std::filesystem::path(path).filename().wstring() == data.cFileName);
            FindClose(h);
            return match;
        #else
            struct stat info;
            int ret = -1;

            ret = stat(path.c_str(), &info);


            return 0 == ret;
        #endif
    }

    std::string Utils::getFullPath(const std::string& dir, const aiString& path)
    {
        std::string p(path.data);

        if (p == "C:\\\\") {
            p = "";
        }
        else if (p.substr(0, 2) == ".\\") {
            p = p.substr(2, p.size() - 2);
        }

        std::string fullPath = dir + "/" + p;

        return fullPath;
    }

    std::string Utils::getDirFromFilename(const std::string& fileName)
    {
        // Extract the directory part from the file name
        std::string::size_type slashIndex;

    #ifdef _WIN64
        slashIndex = fileName.find_last_of("\\");

        if (slashIndex == -1) {
            slashIndex = fileName.find_last_of("/");
        }
    #else
        SlashIndex = Filename.find_last_of("/");
    #endif

        std::string dir;

        if (slashIndex == std::string::npos) {
            dir = ".";
        }
        else if (slashIndex == 0) {
            dir = "/";
        }
        else {
            dir = fileName.substr(0, slashIndex);
        }

        return dir;
    }

    std::string Utils::getExtension(const char *name) 
    {
        std::string nameStr(name);
        size_t dotLoc = nameStr.find_last_of('.');
        
        if (dotLoc != std::string::npos)
        {
            std::string ext = nameStr.substr(dotLoc);
            return ext;
        }

        return "";
    }

    std::string Utils::getFileNameFormPath(const std::string& path)
    {
        std::filesystem::path p(path);
        return p.filename().string();
    }

}