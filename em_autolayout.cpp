#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <array>
#include <string>

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/emscripten.h>

#include "autolayout/view.h"

using namespace emscripten;
using namespace autolayout;

//Attribute parseAttribute(const std::string& name)
//{
//	if(name == "const") return ATTR_CONST;
//	if(name == "left") return ATTR_LEFT;
//    if(name == "right") return ATTR_RIGHT;
//    if(name == "top") return ATTR_TOP;
//    if(name == "bottom") return ATTR_BOTTOM;
//    if(name == "width") return ATTR_WIDTH;
//    if(name == "height") return ATTR_HEIGHT;
//    if(name == "centerX") return ATTR_CENTERX;
//    if(name == "centerY") return ATTR_CENTERY;
//    emscripten_log(EM_LOG_ERROR, "%s: invalid attribute name: %s", __func__, name.data());
//    return ATTR_CONST;
//}
//
//Relation parseRelation(const std::string& name)
//{
//    if(name == "equ") return REL_EQU;
//    if(name == "leq") return REL_LEQ;
//    if(name == "geq") return REL_GEQ;
//    emscripten_log(EM_LOG_ERROR, "%s: invalid relation name: %s", __func__, name.data());
//    return REL_EQU;
//}
//
//namespace dto
//{
//    enum ConsDefHasFields
//    {
//        view1 = 1,
//        view2 = 1 << 1,
//        constant = 1 << 2,
//        priority = 1 << 3,
//        multiplier = 1 << 4,
//
//        constantDefault = 1 << 5
//    };
//
//    struct ConstraintDefDto
//    {
//        int hasFields;
//
//        std::string view1;
//        std::string view2;
//        uint32_t attr1_attr2_relation_priority; // attr1 & (attr2 << 4) & (relation << 8) & (priority << 12)
//        double constant;
//        double multiplier;
//
//        bool hasField(ConsDefHasFields f)
//        {
//            return (hasFields & f) != 0;
//        }
//
//        std::string getView1()
//        {
//            if(hasField(ConsDefHasFields::view1))
//                return view1;
//            return {};
//        }
//
//        std::string getView2()
//        {
//            if(hasField(ConsDefHasFields::view2))
//                return view2;
//            return {};
//        }
//
//        Attribute getAttr1()
//        {
//            return (Attribute)(attr1_attr2_relation_priority & 0xF);
//        }
//
//        Attribute getAttr2()
//        {
//            return (Attribute)((attr1_attr2_relation_priority & 0xF0) >> 4);
//        }
//
//        Relation getRelation()
//        {
//            return (Relation) ((attr1_attr2_relation_priority & 0xF00) >> 8);
//        }
//
//        boost::optional<unsigned> getPriority()
//        {
//            if(hasField(ConsDefHasFields::priority))
//                return (attr1_attr2_relation_priority & 0xFFFFF000) >> 12;
//            return {};
//        }
//
//        boost::optional<double> getConstant()
//        {
//            if(hasField(ConsDefHasFields::constantDefault))
//                return {};
//            if(hasField(ConsDefHasFields::constant))
//                return {constant};
//            return {0.0};
//        }
//
//        boost::optional<double> getMultiplier()
//        {
//            if(hasField(ConsDefHasFields::multiplier))
//                return multiplier;
//            return {};
//        }
//    };
//}


//ConstraintDef constraintDefCtor(dto::ConstraintDefDto dto)
//{
//    return {
//            dto.getView1(),
//            dto.getAttr1(),
//            dto.getRelation(),
//            dto.getView2(),
//            dto.getAttr2(),
//            dto.getMultiplier(),
//            dto.getConstant(),
//            dto.getPriority()
//    };
//}

//size_t raw_createConstraintDef(dto::ConstraintDefDto dto)
//{
//    return (size_t) new ConstraintDef(constraintDefCtor(dto));
//}

namespace view
{
    static val* class_ConstraintDef;  //= val::module_property("ConstraintDef");
    static val* class_ViewConstraint; // = val::module_property("ViewConstraint");

    void getSubViews(View& self, val outObj)
    {
        for(const auto& kv : self.getSubViews())
        {
            auto& [name, subView] = kv;
            if(name.empty() || name[0] == '-' || name[0] == '~' )
                continue;

			outObj.set(name, subView);
        }
    }

    void setSpacing(View& self, const val& sp)
    {
        if(sp.isArray())
        {
            std::array<double, SPACE__COUNT> parsed {};
            double sp0, sp1;

            switch(sp["length"].as<int>())
            {
                // convert spacing into array: [top, right, bottom, left, horz, vert]
                case 1:  parsed = { sp0 = sp["0"].as<double>(), sp0, sp0, sp0, sp0, sp0};break;
                case 2:  parsed = { sp1 = sp["1"].as<double>(), sp0 = sp["0"].as<double>(), sp1, sp0, sp0, sp1}; break;
                case 6:  parsed = {sp["0"].as<double>(), sp["1"].as<double>(), sp["2"].as<double>(), sp["3"].as<double>(), sp["4"].as<double>(), sp["5"].as<double>()}; break;
                default:
                {
                    emscripten_log(EM_LOG_ERROR, "%s: Invalid spacing syntax", __func__);
                    return;
                }
            }
            self.setSpacing(parsed);
        }
        else if(sp.isNumber())
            self.setSpacing(sp.as<double>());
        else
            emscripten_log(EM_LOG_ERROR, "%s: Either array or number expeected", __func__);
    }

//    val addConstraint(View& self, const val& con_or_def_or_json)
//    {
//        if(con_or_def_or_json.instanceof(*class_ConstraintDef))
//            return val(self.addConstraint(con_or_def_or_json.as<ConstraintDef>()));
//        else
//            return val(self.addConstraint(constraintDefCtor(con_or_def_or_json)));
//    }

    void addViewConstraintBack(View& self, const val& viewCon)
    {
        self.addConstraint(viewCon.as<ViewConstraint>());
    }

    void removeViewConstraint(View& self, const val& viewCon)
    {
        self.removeConstraint(viewCon.as<ViewConstraint>());
    }

    void removeConstraint(View& self, const val& viewCon)
    {
        if(! viewCon.instanceof(*class_ViewConstraint))
            emscripten_log(EM_LOG_ERROR, "%s: argument must be a ViewConstraint instance", __func__);
        else
            self.removeConstraint(viewCon.as<ViewConstraint>());
    }


    ViewConstraint raw_addConstraint(View& self, size_t def)
    {
        return self.addConstraint(*(ConstraintDef*)def);
    }

    size_t raw_addConstraints(View& self, size_t vecOfDef, bool collect)
	{
		auto* vec = (std::vector<ConstraintDef>*)vecOfDef;

		if(collect)
		{
			auto* out = new std::vector<ViewConstraint>();
			out->reserve(vec->size());

			for(auto const& def : *vec)
				out->emplace_back(self.addConstraint(def));
			return (size_t)(void*)out;
		}
		else
		{
			for(auto const& def : *vec)
				self.addConstraint(def);
			return 0;
		}

	}

    //todo:
    //addConstraint-S

    void raw_addViewConstraintBack(View& self, const ViewConstraint &viewCon)
    {
        self.addConstraint(viewCon);
    }

    void raw_removeViewConstraint(View& self, const ViewConstraint &viewCon)
    {
        self.removeConstraint(viewCon);
    }

    void raw_addViewConstraintsBack(View& self, size_t vecOfViewCons)
	{
    	auto* vec = (std::vector<ViewConstraint>*) vecOfViewCons;
    	for(auto const& vc : *vec)
    		self.addConstraint(vc);
	}

	void raw_removeViewConstraints(View& self, size_t vecOfViewCons)
	{
		auto* vec = (std::vector<ViewConstraint>*) vecOfViewCons;
		for(auto const& vc : *vec)
			self.removeConstraint(vc);
	}
}

namespace subview
{
    val intrinsicWidth(SubView& self)
    {
        auto v = self.intrinsicWidth();
        return v ? val(v.value()) : val::undefined();
    }

    void setIntrinsicWidth(SubView& self, double v)
    {
    	self.setIntrinsicWidth(v);
    }

    void setIntrinsicHeight(SubView& self, double v)
    {
    	self.setIntrinsicHeight(v);
    }

    val intrinsicHeight(SubView& self)
    {
        auto v = self.intrinsicHeight();
        return v ? val(v.value()) : val::undefined();
    }

    void clearIntrinsicWidth(SubView& self)
	{
    	self.setIntrinsicWidth(boost::none);
	}

	void clearIntrinsicHeight(SubView& self)
	{
    	self.setIntrinsicHeight(boost::none);
	}
}

EMSCRIPTEN_BINDINGS(View)
{
    //function("raw_createConstraintDef", &raw_createConstraintDef, allow_raw_pointers());

//    class_<ConstraintDef>("ConstraintDef")
//            .constructor(&constraintDefCtor)
//            ;

    class_<View>("View")
            .constructor()
            .function("setSpacing", &view::setSpacing)
            //.function("addConstraint", &view::addConstraint)
            .function("addViewConstraintBack", &view::addViewConstraintBack)

            .function("raw_addConstraint", &view::raw_addConstraint, allow_raw_pointers())
            .function("raw_addConstraints", &view::raw_addConstraints, allow_raw_pointers())
            .function("raw_addViewConstraintBack", &view::raw_addViewConstraintBack, allow_raw_pointers())
            .function("raw_removeViewConstraint", &view::raw_removeViewConstraint, allow_raw_pointers())

            .function("raw_addViewConstraintsBack", &view::raw_addViewConstraintsBack, allow_raw_pointers())
            .function("raw_removeViewConstraints", &view::raw_removeViewConstraints, allow_raw_pointers())

            .function("removeViewConstraint", &view::removeViewConstraint)
            .function("removeConstraint", &view::removeConstraint)
            .function("setSize", &View::setSize)
            .function("getSubViews", &view::getSubViews)
            .function("update", &View::update)
            ;

    class_<SubView>("SubView")
            .function("top", &SubView::top)
            .function("bottom", &SubView::bottom)
            .function("centerX", &SubView::centerX)
            .function("centerY", &SubView::centerY)
            .function("left", &SubView::left)
            .function("right", &SubView::right)
            .function("width", &SubView::width)
            .function("height", &SubView::height)
            .function("name", &SubView::name)
            .function("type", &SubView::type)
            .function("intrinsicWidth", &subview::intrinsicWidth)
            .function("intrinsicHeight", &subview::intrinsicHeight)
            .function("setIntrinsicWidth", &subview::setIntrinsicWidth)
            .function("setIntrinsicHeight", &subview::setIntrinsicHeight)
            .function("clearIntrinsicWidth", &subview::clearIntrinsicWidth)
            .function("clearIntrinsicHeight", &subview::clearIntrinsicHeight)
            ;

//    value_object<dto::ConstraintDefDto>("ConstraintDefDto")
//            .field("hasFields", &dto::ConstraintDefDto::hasFields)
//            .field("view1", &dto::ConstraintDefDto::view1)
//            .field("view2", &dto::ConstraintDefDto::view2)
//            .field("attr1_attr2_relation_priority", &dto::ConstraintDefDto::attr1_attr2_relation_priority)
//            .field("constant", &dto::ConstraintDefDto::constant)
//            .field("multiplier", &dto::ConstraintDefDto::multiplier)
//            ;

    class_<ViewConstraint>("ViewConstraint")
            ;

    //static val class_ConstraintDef = val::module_property("ConstraintDef");
    static val class_ViewConstraint = val::module_property("ViewConstraint");

    //view::class_ConstraintDef = &class_ConstraintDef;
    view::class_ViewConstraint = &class_ViewConstraint;
}