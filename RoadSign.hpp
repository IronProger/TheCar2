//
// Created by rege on 27.03.19.
//

#ifndef THECAR_ROADSIGN_HPP
#define THECAR_ROADSIGN_HPP

class RoadSign
{
    int roadSignNumber;

    static bool selfTest () // if true then all right
    {
        forEach([] (int sign)
                {
                    assert(getName(sign) != "");
                    assert(getRoadSignNumber(getName(sign)));
                });
        assert(getRoadSignNumbers().size() == getNames().size());
    }

    bool success = selfTest();

public:
    enum Color
    {
        RED, BLUE, UNKNOWN
    };

    RoadSign (int roadSign) : roadSignNumber(roadSign)
    {}

    RoadSign (const RoadSign & roadSign) : roadSignNumber(roadSign.roadSignNumber)
    {}

    RoadSign (const std::string & name) : roadSignNumber(getRoadSignNumber(name))
    {}

    RoadSign & operator= (const RoadSign & roadSign)
    {
        if (&roadSign != this)
        {
            roadSignNumber = roadSign.roadSignNumber;
        }
        return *this;
    }

    static bool isRoadSign (int roadSignNumber)
    {
        for (const int & s : getRoadSignNumbers())
        {
            if (s == roadSignNumber) return true;
        }
        return false;
    }

    RoadSign & operator= (const int & roadSignNumber)
    {
        if (!isRoadSign(roadSignNumber))
        {
            LOGE << "You gave me invalid road sign number. I'll just set none.";
            this->roadSignNumber = NONE;
        } else
        {
            this->roadSignNumber = roadSignNumber;
        }
        return *this;
    }

    inline bool operator== (const RoadSign & roadSign)
    {
        return roadSignNumber == roadSign.roadSignNumber;
    }

    inline bool operator== (const int & roadSignNumber)
    {
        return roadSignNumber == this->roadSignNumber;
    }

    static void forEach (std::function<void (int)> f)
    {
        for (int s = __FIRST; s != __LAST; ++s)
        {
            f(s);
        }
    }

    static std::vector<int> getRoadSignNumbers ()
    {
        std::vector<int> signs;
        forEach([&signs] (int s)
                {
                    signs.emplace_back(s);
                });
        return signs;
    }

    enum
    {
        FORWARD_ONLY, RIGHT_TURN_ONLY, LEFT_TURN_ONLY, RIGHT_TURN_OR_FORWARD_ONLY, STOP, DO_NOT_ENTER, NONE,
        __FIRST = FORWARD_ONLY, __LAST = NONE
    };


    static Color getColor (int roadSign)
    {
        switch (roadSign)
        {
            case FORWARD_ONLY:
            case RIGHT_TURN_ONLY:
            case LEFT_TURN_ONLY:
            case RIGHT_TURN_OR_FORWARD_ONLY:
                return BLUE;
            case STOP:
            case DO_NOT_ENTER:
                return RED;
            default:
                return UNKNOWN;
        }
    }

    static std::string getName (int roadSignNumber)
    {
        switch (roadSignNumber)
        {
            case FORWARD_ONLY:
                return "forward_only";
            case RIGHT_TURN_ONLY:
                return "right_turn_only";
            case LEFT_TURN_ONLY:
                return "left_turn_only";
            case RIGHT_TURN_OR_FORWARD_ONLY:
                return "right_turn_or_forward_only";
            case STOP:
                return "stop";
            case DO_NOT_ENTER:
                return "do_not_enter";
            case NONE:
                return "none";
            default:
                return "";
        }
    }

    static int getRoadSignNumber (std::string name)
    {
        if (name == "forward_only")
            return FORWARD_ONLY;
        if (name == "right_turn_only")
            return RIGHT_TURN_ONLY;
        if (name == "left_turn_only")
            return LEFT_TURN_ONLY;
        if (name == "right_turn_or_forward_only")
            return RIGHT_TURN_OR_FORWARD_ONLY;
        if (name == "stop")
            return STOP;
        if (name == "do_not_enter")
            return DO_NOT_ENTER;
        return NONE;
    }

    inline std::string getName ()
    {
        return getName(roadSignNumber);
    }

    static std::vector<std::string> getNames ()
    {
        std::vector<std::string> names;
        forEach([&names] (int s)
                {
                    names.emplace_back(getName(s));
                });
        return names;
    }
};

#endif //THECAR_ROADSIGN_HPP
