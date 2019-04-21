#pragma once
#include <string>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/version.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include "../autolayout/constraint_def.h"

namespace evfl::ast
{
    namespace x3 = boost::spirit::x3;

    using namespace autolayout;

    enum Orientation
    {
        ORIENT_NONE=0, ORIENT_H = 0b01, ORIENT_V = 0b10, ORIENT_BOTH = 0b11
    };

    enum Connector
    {
        CONNECTOR_ARROW, CONNECTOR_HYPHEN, CONNECTOR_TILDE, CONNECTOR_CLOSED
    };

    enum OpSign
    {
        OPSIGN_MUL, OPSIGN_DIV
    };

    struct Multiplier
    {
        OpSign opsign;
        double number;

        double value() const
        {
            switch(opsign)
            {
                case OPSIGN_MUL:return number;
                case OPSIGN_DIV:return 1/number;
            }
        }
    };

    using Priority = unsigned;
    using Number = double;
    using Constant= double;
    using ConstantExpr = double;

    struct Percentage
    {
		double number;
        boost::optional<ConstantExpr> constExpr;

        double value() const { return number / 100; }
    };

    struct ViewPredicate
    {
        std::string viewName;

        boost::optional <Attribute> attribute;
        boost::optional <Multiplier> multiplier;
        boost::optional <ConstantExpr> constantExpr;
    };

    using ObjectOfPredicate = x3::variant<Percentage, Constant, ViewPredicate>;

    struct Predicate
    {
        boost::optional <Relation> relation;
        ObjectOfPredicate elements;
        boost::optional <Priority> priority;
    };

    using PredicateListWithParens = std::vector<Predicate>;

    using PredicateList = x3::variant<Priority, Percentage, Constant, PredicateListWithParens>;

    struct Connection
    {
        boost::optional<PredicateList> predicates;
        Connector connector;
    };

    struct View;
    using ViewGroup = std::vector <x3::forward_ast<View>>;

    struct ConnectionViewGroupPair
    {
        Connection connection;
        ViewGroup views;
    };

    struct CascadedViews
    {
        std::vector <ConnectionViewGroupPair> rest;
        Connection toSuper;

		const Connection& superTo() const { return rest[0].connection; }
		const ViewGroup& first() const { return rest[0].views; }
    };

    struct View
    {
        std::string name;

        PredicateListWithParens predicates;
        boost::optional<CascadedViews> cascadedViews;

        ViewGroup asGroup() const { return { View{ .name = name } };  };
    };

    struct AttributePredicate
    {
        Attribute attribute;
        PredicateListWithParens predicates;
    };

    struct VisualFormat
    {
        Orientation orientation;
        std::vector<ConnectionViewGroupPair> rest;
        boost::optional<char> _superTo;
        Connection __toSuper;
        boost::optional<char> _toSuper;

        static inline const ast::Connection DISCONNECTION = ast::Connection{ .connector = ast::CONNECTOR_ARROW };

        const Connection& superTo() const
        {
			return _superTo ? rest[0].connection : DISCONNECTION;
        }

		const Connection& toSuper() const
		{
			return _toSuper ? __toSuper : DISCONNECTION;
		}
    };

    struct ConstraintFormat
    {
        x3::variant<std::string, std::vector<std::string>> viewName;
        std::vector <AttributePredicate> predicates;
    };

    //using ExtendedVisualFormat = x3::variant<VisualFormat, ConstraintFormat>;

    struct MultiVisualFormatRow
	{
    	Orientation orientation;
		std::vector<VisualFormat> items;
	};

    using MultiConstraintFormatRow = std::vector<ConstraintFormat>;

    using MultiExtendedVisualFormat = std::vector<x3::variant<MultiVisualFormatRow, MultiConstraintFormatRow>>;
}

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(evfl::ast::View, name, predicates, cascadedViews);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::Percentage, number, constExpr);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::CascadedViews, rest, toSuper);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::Multiplier, opsign, number);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::ViewPredicate, viewName, attribute, multiplier, constantExpr);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::Predicate, relation, elements, priority);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::Connection, predicates, connector);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::ConnectionViewGroupPair, connection, views);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::AttributePredicate, attribute, predicates);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::VisualFormat, orientation, _superTo, rest, __toSuper, _toSuper);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::ConstraintFormat, viewName, predicates);
BOOST_FUSION_ADAPT_STRUCT(evfl::ast::MultiVisualFormatRow, orientation, items);
