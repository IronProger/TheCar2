//
// Created by rege on 09.03.17.
//

#include <kaguya.hpp>
#include <pugixml.hpp>
#include <plog/Log.h>

#include "Config.hpp"

using namespace std;
using namespace pugi;

xml_document config;
xml_parse_result result;

Config::Config ()
{}

string Config::filePath = DEFAULT_CONFIG_PATH;

void Config::init (const char * pathToConfig)
{
    result = config.load_file(pathToConfig);
    if (result)
    {
        LOGD << "XML [" << pathToConfig << "] parsed without errors, attr value: ["
             << config.child("node").attribute("attr").value() << "]\n\n";
    } else
    {
        LOGE << "XML [" << pathToConfig << "] parsed with errors, attr value: ["
             << config.child("node").attribute("attr").value() << "]\n";
        LOGE << "Error description: " << result.description() << "\n";
        LOGE << "Error offset: " << result.offset << " (error at [..." << (pathToConfig + result.offset) << "]\n\n";
    }
}


vector<string> splitString (string s, const vector<char> & separators)
{
    vector<string> v;

    string temperate;

    for (char c : s)
    {
        for (const char sep : separators)
        {
            if (c == sep)
            {
                if (!temperate.empty())
                {
                    v.emplace_back(string(temperate));
                    temperate.clear();
                }
                goto EXTRA_CONTINUE;
            }
        }
        temperate += c;
        EXTRA_CONTINUE:
        continue;
    }
    if (!temperate.empty())
        v.emplace_back(temperate);

    return v;
}

xml_text getXmlText (std::string path)
{
    assert(!path.empty());
    path = "configuration/" + path;
    xml_node targetNode;
    for (string & s : splitString(path, vector<char>{'/'}))
    {
        if (targetNode.empty()) targetNode = config.child(s.c_str());
        else targetNode = targetNode.child(s.c_str());

        if (targetNode.empty())
        {
            LOGF << string("Invalid configuration request! Node is empty! Path is ") + path;
            assert(!targetNode.empty());
        }
    }
    if (!targetNode.text())
    {
        LOGF << string("Invalid configuration request! Node has not a text body! Path is ") + path;
        assert(targetNode.text());
    }
    return targetNode.text();
}

string Config::getString (std::string path)
{
    return getXmlText(path).as_string();
}

int Config::getInt (std::string path)
{
    return getXmlText(path).as_int();
}

unsigned int Config::getUint (std::string path)
{
    return getXmlText(path).as_uint();
}

long long Config::getLLong (std::string path)
{
    return getXmlText(path).as_llong();
}

unsigned long long Config::getUllong (std::string path)
{
    return getXmlText(path).as_ullong();
}

double Config::getDouble (std::string path)
{
    return getXmlText(path).as_double();
}

float Config::getFloat (std::string path)
{
    return getXmlText(path).as_float();
}

bool Config::getBool (std::string path)
{
    return getXmlText(path).as_bool();
}

vector<string> Config::getStringVector (std::string path)
{
    return splitString(getXmlText(path).as_string(), vector<char>{' ', '\n', '\t'});
}
