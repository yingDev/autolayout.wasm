#pragma once
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <array>
#include "./kiwi_fwd.h"
#include <unordered_map>
#include "constraint_def.h"
#include "subview.h"

namespace autolayout
{
    enum SpacingType { SPACE_TOP, SPACE_RIGHT, SPACE_BOTTOM, SPACE_LEFT, SPACE_HORIZ, SPACE_VERT, SPACE__COUNT };
    using Spacing = std::array<double, SPACE__COUNT>;

    class ViewConstraint
    {
        kiwi::Constraint _con;
        explicit ViewConstraint(kiwi::Constraint con) : _con(con){}
        friend class View;
    };

    class View
    {
        kiwi::Solver* _solver;
        std::unordered_map<std::string, SubView*> _subViews = {};
        SubView* _parentSubView;
        Spacing _spacing = {};
        mutable std::array<boost::optional<kiwi::Variable>, SPACE__COUNT> _spacingVars = {};
        mutable std::array<boost::optional<kiwi::Expression>, SPACE__COUNT> _spacingExpr = {};

    public:
        View() : _solver(new kiwi::Solver()), _parentSubView{new SubView(_solver)}
        {
            _subViews.reserve(16);
            setSpacing(8);
        }

        void setSize(double width, double height)
        {
            _parentSubView->setIntrinsicWidth(width);
            _parentSubView->setIntrinsicHeight(height);
        }

        std::unordered_map<std::string, SubView*>& getSubViews() { return _subViews; }

        void setSpacing(Spacing spacing)
        {
            if(_spacing != spacing)
            {
                _spacing = spacing;
                for(int i=0; i<SPACE__COUNT; i++)
                {
                    if(_spacingVars[i])
                        _solver->suggestValue(*_spacingVars[i], spacing[i]);
                }
            }
        }

        void setSpacing(double value){ setSpacing({value, value, value, value, value, value}); }

        ViewConstraint addConstraint(const ConstraintDef& con)
        {
			auto left = _getSubView(con.view1)->_getAttr(con.attr1);
			auto right = boost::optional<kiwi::Expression>{};
			auto strength = kiwi::strength::create(0, con.priority.value_or(500), 1000);

			if(con.view2 == "-")
				right =  -_getSpacing(con);
			else
				right = kiwi::Term{ _getSubView(con.view2)->_getAttr(con.attr2) };

			if(auto* m = con.multiplier.get_ptr(); m && *m != 1)
				right = *right * *m;

            if (auto* constant = con.constant.get_ptr())
				right = *right + *constant;
            else
				right = *right + _getSpacing(con);

			auto cn = boost::optional<kiwi::Constraint>{};
            switch(con.relation)
			{
				case REL_EQU:cn = kiwi::Constraint(left == *right, strength);break;
				case REL_GEQ:cn = kiwi::Constraint(left >= *right, strength);break;
				case REL_LEQ:cn = kiwi::Constraint(left <= *right, strength);break;
			}

            _solver->addConstraint(*cn);
            return ViewConstraint(*cn);
        }

        ViewConstraint addConstraint(const ViewConstraint& con)
        {
            _solver->addConstraint(con._con);
            return con;
        }

        void removeConstraint(const ViewConstraint& con)
        {
            _solver->removeConstraint(con._con);
        }

        void update() { _solver->updateVariables(); }

        void reset()
        {
            _parentSubView->_intrinsicHeight = _parentSubView->_intrinsicWidth = {};

            _solver->reset();
            for(auto kv : _subViews)
                delete kv.second;
            _subViews.clear();

            _spacingVars.fill({});
            _spacingExpr.fill({});
        }

        ~View()
        {
            for(const auto &kv : _subViews)
                delete kv.second;
            _subViews.clear();
            delete _solver;
            delete _parentSubView;
        }

    private:
        SubView* _getSubView(const std::string& name)
        {
            if(name.empty() || name == "^")
                return _parentSubView;
            else
            {
                auto it = _subViews.find(name);
                if(it != _subViews.end())
                    return it->second;

                auto* newItem = new SubView(_solver, name);
                _subViews[name] = newItem;
                return newItem;
            }
        }

        const kiwi::Expression& _getSpacing(const ConstraintDef& con) const
        {
        	auto sp = SPACE_HORIZ;
			if(con.view2 == "-")
			{
				switch (con.attr2)
				{
					case ATTR_LEFT:sp = SPACE_LEFT;break;
					case ATTR_RIGHT:sp = SPACE_RIGHT;break;
					case ATTR_WIDTH:sp = SPACE_HORIZ;break;
					case ATTR_HEIGHT:sp = SPACE_VERT;break;
					case ATTR_TOP:sp = SPACE_TOP;break;
					case ATTR_BOTTOM:sp = SPACE_BOTTOM;break;
					default:
						throw "unexpected value for -.attr2";
				}
			}
			else
			{
				if((con.view1.empty() || con.view1 == "^") && (con.attr1 == ATTR_LEFT))
					sp = SPACE_LEFT;
				else if((con.view1.empty() || con.view1 == "^") && (con.attr1 == ATTR_TOP))
					sp = SPACE_TOP;
				else if((con.view2.empty() || con.view2 == "^") && (con.attr2 == ATTR_RIGHT))
					sp = SPACE_RIGHT;
				else if((con.view2.empty() || con.view2 == "^") && (con.attr2 == ATTR_BOTTOM))
					sp = SPACE_BOTTOM;
				else switch(con.attr1)
					{
						case ATTR_LEFT:
						case ATTR_RIGHT:
						case ATTR_CENTERY:
							sp = SPACE_HORIZ;
							break;
						default:
							sp = SPACE_VERT;
					}
			}

            return _getSpacing(sp);
        }

        const kiwi::Expression& _getSpacing(SpacingType sp) const
		{
			if(!_spacingVars[sp])
			{
				auto& var = *(_spacingVars[sp] = kiwi::Variable());
				_solver->addEditVariable(var, kiwi::strength::create(999, 1000, 1000));
				_spacingExpr[sp] = -var;
				_solver->suggestValue(var, _spacing[sp]);
			}

			return *_spacingExpr[sp];
		}
    };
}
