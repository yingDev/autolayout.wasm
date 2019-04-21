#pragma once
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>

#include <array>
#include <string>


namespace autolayout
{
    enum Attribute
    {
        ATTR_CONST,
        ATTR_LEFT,
        ATTR_RIGHT,
        ATTR_TOP,
        ATTR_BOTTOM,
        ATTR_WIDTH,
        ATTR_HEIGHT,
        ATTR_CENTERX,
        ATTR_CENTERY,
        ATTR__COUNT
    };

    static const char* attr_str(Attribute attr)
    {
        switch (attr)
        {
            case ATTR_CONST: return "const";
            case ATTR_LEFT: return "left";
            case ATTR_RIGHT: return "right";
            case ATTR_TOP: return "top";
            case ATTR_BOTTOM: return "bottom";
            case ATTR_WIDTH: return "width";
            case ATTR_HEIGHT: return "height";
            case ATTR_CENTERX: return "centerX";
            case ATTR_CENTERY: return "centerY";
            default: throw "invalid attr value;";
        }
    }

    enum Priority
    {
        PRIO_REQUIRED = 1000,
        PRIO_DEFAULTHIGH = 750,
        PRIO_DEFAULTLOW = 250
    };

    enum Relation
    {
        REL_LEQ,REL_EQU,REL_GEQ,
    };

    static const char* rel_str(Relation rel)
    {
        switch(rel)
        {
            case REL_LEQ:return "<=";
            case REL_EQU:return "==";
            case REL_GEQ:return ">=";
        }
    }

    //todo: all var
    struct ConstraintDef
    {
        std::string view1 {};
        Attribute attr1 = ATTR_WIDTH;
        std::string view2 {};
        Attribute attr2 = ATTR_CONST;
        Relation relation = REL_EQU;
		boost::optional<double> multiplier {};
		boost::optional<double> constant {};
		boost::optional<unsigned> priority {};

        ConstraintDef(
                std::string view1,
                Attribute attr1,
                Relation relation,
                std::string view2,
                Attribute attr2,
				boost::optional<double> multiplier = 1,
                boost::optional<double> constant = 0,
				boost::optional<unsigned> priority = boost::none
        ) : view1{std::move(view1)}, attr1{attr1}, view2{std::move(view2)}, attr2{attr2}, relation{relation}, multiplier(multiplier), constant{std::move(constant)}, priority{priority}
        {
        }

#ifndef EMSCRIPTEN
        friend std::ostream& operator<<(std::ostream&, const ConstraintDef&);

        std::ostream& print(std::ostream& os) const
        {
            os << view1 <<  "." <<  attr_str(attr1) << ' ' << rel_str(relation) << ' ' <<  view2 <<  "." <<  attr_str(attr2);

            if(!multiplier)
            	os << " * 1";
            else
            	os << " * " << *multiplier;

            if(!constant)
            	os << " + default";
            else
            	os << " + " << *constant;

            if(!priority)
            	os << " @ default";
			else
            	os << " @ " << *priority;

            return os;
        }
#endif //! EMSCRIPTEN

    };

#ifndef EMSCRIPTEN
    std::ostream& operator<<(std::ostream& os, const ConstraintDef& def)
    {
        return def.print(os);
    }
#endif // !EMSCRIPTEN
}
