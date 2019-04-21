#pragma once
#include <boost/optional/optional.hpp>
#include <array>
#include "./kiwi_fwd.h"
#include "constraint_def.h"

namespace autolayout
{
    class SubView
    {
        std::string _name;
        std::string _type;
        kiwi::Solver* _solver;
        std::array<boost::optional<kiwi::Variable>, ATTR__COUNT> _attr = {};
        boost::optional<double> _intrinsicWidth = {};
        boost::optional<double> _intrinsicHeight = {};
        friend class View;

    public:
        explicit SubView(kiwi::Solver* solver, std::string name="", std::string type="") : _name(std::move(name)), _type(std::move(type)), _solver(solver)
        {
            if(_name.empty())
            {
                _attr[ATTR_LEFT].emplace();
                _attr[ATTR_TOP].emplace();

                _solver->addConstraint(kiwi::Constraint{ *_attr[ATTR_LEFT] == 0 });
                _solver->addConstraint(kiwi::Constraint{ *_attr[ATTR_TOP] == 0 });
            }
        }

        const std::string& name() const { return _name; }
        const std::string& type() const { return _type; }
        double top() { return _getAttr(ATTR_TOP).value(); }
        double bottom() { return _getAttr(ATTR_BOTTOM).value(); }
        double centerX() { return _getAttr(ATTR_CENTERX).value(); }
        double centerY() { return _getAttr(ATTR_CENTERY).value(); }
        double left() { return _getAttr(ATTR_LEFT).value(); }
        double right() { return _getAttr(ATTR_RIGHT).value(); }
        double width() { return _getAttr(ATTR_WIDTH).value(); }
        double height() { return _getAttr(ATTR_HEIGHT).value(); }

        boost::optional<double> getValue(Attribute attr)
        {
            if(_attr[attr])
                return _attr[attr]->value();
            return {};
        }

        boost::optional<double> intrinsicWidth() { return _intrinsicWidth; }

        void setIntrinsicWidth(boost::optional<double> value)
        {
            if(value)
            {
                auto& attr = _getAttr(ATTR_WIDTH);
                if(!_intrinsicWidth)
                    _solver->addEditVariable(attr, kiwi::strength::create(_name.empty() ? 999 : 998, 1000, 1000 ));
                _intrinsicWidth = value;
                _solver->suggestValue(attr, *value);
            }
            else if(_intrinsicWidth)
            {
                _intrinsicWidth.reset();
                _solver->removeEditVariable(_getAttr(ATTR_WIDTH));
            }
        }

        boost::optional<double> intrinsicHeight() { return _intrinsicHeight; }

        void setIntrinsicHeight(boost::optional<double> value)
        {
            if(value)
            {
                auto& attr = _getAttr(ATTR_HEIGHT);
                if(!_intrinsicHeight)
                    _solver->addEditVariable(attr, kiwi::strength::create(_name.empty() ? 999 : 998, 1000, 1000 ));
                _intrinsicHeight = value;
                _solver->suggestValue(attr, *value);
            }
            else if(_intrinsicHeight)
            {
                _intrinsicHeight.reset();
                _solver->removeEditVariable(_getAttr(ATTR_HEIGHT));
            }
        }

    private:
        const kiwi::Variable& _getAttr(Attribute attr)
        {
            if(_attr[attr])
                return *_attr[attr];
            _attr[attr].emplace();

            switch(attr)
            {
                case ATTR_RIGHT:
                    _solver->addConstraint(kiwi::Constraint( *_attr[attr] == (_getAttr(ATTR_LEFT) + _getAttr(ATTR_WIDTH))));
                    break;
                case ATTR_BOTTOM:
                    _solver->addConstraint(kiwi::Constraint( *_attr[attr] == (_getAttr(ATTR_TOP) + _getAttr(ATTR_HEIGHT)) ));
                    break;
                case ATTR_CENTERX:
                    _solver->addConstraint(kiwi::Constraint( *_attr[attr] == (_getAttr(ATTR_LEFT) + (_getAttr(ATTR_WIDTH) / 2)) ));
                    break;
                case ATTR_CENTERY:
                    _solver->addConstraint(kiwi::Constraint( *_attr[attr] == (_getAttr(ATTR_TOP) + (_getAttr(ATTR_HEIGHT) / 2)) ));
                    break;
                default:break;
            }
            return *_attr[attr];
        }

    };

}
