#pragma onces

#include <fstream>
#include <sstream>

namespace nihil
{
    enum Platform
    {
        LinuxWayland,
        LinuxX11,
        Windows
    };

    static Platform getPlatform()
    {
        std::ifstream platformFile("./Platform");

        std::stringstream fileStream;
        fileStream << platformFile.rdbuf();

        std::string line;

        while(std::getline(fileStream, line))
        {
            if(line[0] != '#')
            {
                Logger::Log(std::string("Using platform: ") + line);
                if(line == std::string("Linux_Wayland"))
                {
                    return Platform::LinuxWayland;
                }
                else if(line == std::string("Linux_X11"))
                {
                    return Platform::LinuxX11;
                }
                else if(line == std::string("Windows"))
                {
                    return Platform::Windows;
                }
            }
        }

        return (Platform)-1;
    }
}