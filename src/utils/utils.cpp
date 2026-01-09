#pragma once

#include <filesystem>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <algorithm>

#include "utils/utils.h"
#include "managers/windowManager.h"

#define COLOR_TEXTURE_UNIT              GL_TEXTURE0
#define COLOR_TEXTURE_UNIT_INDEX        0
#define SHADOW_TEXTURE_UNIT             GL_TEXTURE1
#define SHADOW_TEXTURE_UNIT_INDEX       1
#define NORMAL_TEXTURE_UNIT             GL_TEXTURE2
#define NORMAL_TEXTURE_UNIT_INDEX       2
#define RANDOM_TEXTURE_UNIT             GL_TEXTURE3
#define RANDOM_TEXTURE_UNIT_INDEX       3
#define DISPLACEMENT_TEXTURE_UNIT       GL_TEXTURE4
#define DISPLACEMENT_TEXTURE_UNIT_INDEX 4
#define ALBEDO_TEXTURE_UNIT             GL_TEXTURE5
#define ALBEDO_TEXTURE_UNIT_INDEX       5          
#define ROUGHNESS_TEXTURE_UNIT          GL_TEXTURE6
#define ROUGHNESS_TEXTURE_UNIT_INDEX    6
#define MOTION_TEXTURE_UNIT             GL_TEXTURE7
#define MOTION_TEXTURE_UNIT_INDEX       7
#define SPECULAR_EXPONENT_UNIT             GL_TEXTURE8
#define SPECULAR_EXPONENT_UNIT_INDEX       8
#define CASCACDE_SHADOW_TEXTURE_UNIT0               SHADOW_TEXTURE_UNIT
#define CASCACDE_SHADOW_TEXTURE_UNIT0_INDEX         SHADOW_TEXTURE_UNIT_INDEX
#define CASCACDE_SHADOW_TEXTURE_UNIT1               GL_TEXTURE9
#define CASCACDE_SHADOW_TEXTURE_UNIT1_INDEX         9
#define CASCACDE_SHADOW_TEXTURE_UNIT2               GL_TEXTURE10
#define CASCACDE_SHADOW_TEXTURE_UNIT2_INDEX         10
#define SHADOW_CUBE_MAP_TEXTURE_UNIT                GL_TEXTURE11
#define SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX          11
#define SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT       GL_TEXTURE12
#define SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT_INDEX 12
#define METALLIC_TEXTURE_UNIT                       GL_TEXTURE14
#define METALLIC_TEXTURE_UNIT_INDEX                 14
#define HEIGHT_TEXTURE_UNIT                         GL_TEXTURE15
#define HEIGHT_TEXTURE_UNIT_INDEX                   15

#define GET_GL_TEXTURE_UNIT(N) GL_TEXTURE##N


namespace SpaceEngine
{
    void Utils::applyRatioScreenRes(Vector2 anchor, Vector2 pos, float& outScale, Vector2& outOffset, Vector2& outPos) 
    {
            float scaleX = WindowManager::width / static_cast<float>(REF_WIDTH);
            float scaleY = WindowManager::height / static_cast<float>(REF_HEIGHT);
            outScale = std::min(scaleX, scaleY);

            //offset when change the resolution
            outOffset.x = (WindowManager::width - REF_WIDTH * outScale) * 0.5f;
            outOffset.y = (WindowManager::height - REF_HEIGHT * outScale) * 0.5f;
            //anchor valuation
            float anchorX = anchor.x * REF_WIDTH;
            float anchorY = anchor.y * REF_HEIGHT;
            //space postion
            outPos.x = (anchorX + pos.x) * outScale + outOffset.x;
            outPos.y = (anchorY + pos.y) * outScale + outOffset.y;
    }

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

    bool Utils::directoryExists(const std::string& pathDir)
    {
        #ifdef _WIN64
        DWORD attrib = GetFileAttributesA(pathDir.c_str());
        return (attrib != INVALID_FILE_ATTRIBUTES &&
           (attrib & FILE_ATTRIBUTE_DIRECTORY));
        #else
            return false
        #endif
    }

    std::string Utils::getFullPath(const std::string& dir, const aiString& path)
    {
        std::string p(path.data);
        p = getFileNameFormPath(p);

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

    std::string Utils::getFileNameNoExt(const std::string& filePath)
    {
        std::filesystem::path p(filePath);
        return p.stem().string();   // stem() = filename without extension
    }
    
    std::string Utils::joinPaths(const std::string& a, const std::string& b)
    {
        std::filesystem::path p = std::filesystem::path(a) / std::filesystem::path(b);  // Correct handling of slashes and OS differences
        return p.string();
    }
}