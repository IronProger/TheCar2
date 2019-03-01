//
// Created by rege on 09.03.17.
//

#ifndef THECAR_CONFIG_HPP
#define THECAR_CONFIG_HPP

#include <string>

const std::string DEFAULT_CONFIG_PATH = "/home/rege/Workspace/CLionProjects/TheCar2/TheCarConfig.lua";

// singleton
class Config
{
private:
    static std::string filePath;

    Config ();

    Config (const Config &);

    Config & operator= (Config &);

public:
    static Config & getInstance ()
    {
        static Config instance;
        return instance;
    }

    void init (const char * pathToConfig);

    std::string getString (std::string path);

    int getInt (std::string path);

    unsigned int getUint (std::string path);

    long long int getLLong (std::string path);

    unsigned long long int getUllong (std::string path);

    double getDouble (std::string path);

    float getFloat (std::string path);

    bool getBool (std::string path);

    std::vector<std::string> getStringVector (std::string path);
};

inline std::string getString (std::string path)
{
    return Config::getInstance().getString(path);
}

inline int getInt (std::string path)
{
    return Config::getInstance().getInt(path);
}

inline unsigned int getUint (std::string path)
{
    return Config::getInstance().getUint(path);
}

inline long long int getLLong (std::string path)
{
    return Config::getInstance().getLLong(path);
}

inline unsigned long long int getUllong (std::string path)
{
    return Config::getInstance().getUllong(path);
}

inline double getDouble (std::string path)
{
    return Config::getInstance().getDouble(path);
}

inline float getFloat (std::string path)
{
    return Config::getInstance().getFloat(path);
}

inline bool getBool (std::string path)
{
    return Config::getInstance().getBool(path);
}

// separator is ';'
inline std::vector<std::string> getStringVector (std::string path)
{
    return Config::getInstance().getStringVector(path);
}

#endif //THECAR_CONFIG_HPP
