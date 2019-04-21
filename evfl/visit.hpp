#pragma once
#include <string>
#include <boost/spirit/home/x3.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include "ast.hpp"
#include "syntax.hpp"
#include "../autolayout/constraint_def.h"

namespace evfl::visit
{
	using namespace std::string_literals;
	using boost::optional;
	using boost::variant;
	using boost::get;

	auto static const ROOTVIEW = ast::View{ .name = "^" };
	auto static const ROOTGROUP = ast::ViewGroup{ ast::View{ .name = "^"}};
	auto static const CNSUPER = "C:"s;

	enum class ConnectionType { SIBLING, FROMSUPER, TOSUPER };

	struct Cascade
	{
		const ast::Connection& superTo;
		const std::vector <ast::ConnectionViewGroupPair>& rest;
		const ast::Connection& toSuper;

		const ast::ViewGroup& first() const { return rest[0].views; }
	};

	inline void visitPredicate(
			const ast::Predicate& pred,
			const std::string& view1,
			ast::Attribute attr1,
			const std::string& super,
			std::vector<ast::ConstraintDef>& output)
	{
		auto& [relation, elements, priority] = pred;

		auto const rel = relation.value_or(ast::REL_EQU);
		auto isCnExpr = super == "C:";

		if(auto* simpleConstant = get<ast::Constant>(&elements))
		{
			output.emplace_back(view1, attr1, rel, isCnExpr ? "^" : super, isCnExpr ? attr1 : ast::ATTR_CONST, 1, *simpleConstant, priority);
			return;
		}

		if(auto* percent = get<ast::Percentage>(&elements))
		{
			auto constant = percent->constExpr.value_or(0);
			output.emplace_back(view1, attr1, rel, isCnExpr ? "^" : super, attr1, percent->value(), constant, priority);
			return;
		}

		{
			auto& vp = get<ast::ViewPredicate>(elements);
			auto const& view2 = isCnExpr ? "^" : (vp.viewName == "^"  ? super : vp.viewName);
			auto attr2 = vp.attribute.value_or(attr1);

			auto multiplier = optional<double>{1};
			auto constant = vp.constantExpr.value_or(0);

			if(auto* mul = vp.multiplier.get_ptr())
				multiplier = mul->value();

			output.emplace_back(view1, attr1, rel, view2 , attr2, multiplier, constant, priority);
		}
	}

	inline void visitView(
			const ast::View& view,
			ast::Orientation orient,
			const std::string& super,
			std::vector<ast::ConstraintDef>& output)
	{
		auto const attr1 = orient == ast::ORIENT_H ? ast::ATTR_WIDTH : ast::ATTR_HEIGHT;

		for(auto const& pred : view.predicates)
			visitPredicate(pred, view.name, attr1, super, output);
	}

	inline void _connectSpacer(
			const std::string& spacerName,
			ast::Orientation orient,
			const ast::ViewGroup &prevGroup,
			const ast::ViewGroup &nextGroup,
			ConnectionType type,
			std::vector<ast::ConstraintDef> &output)
	{
		auto const isHorizontal = orient == ast::ORIENT_H;
		ast::Attribute attr1, attr2;

		//connect spacer to neighbors
		attr1 = isHorizontal ? ast::ATTR_LEFT : ast::ATTR_TOP;
		attr2 = type == ConnectionType::FROMSUPER ? attr1 : (isHorizontal ? ast::ATTR_RIGHT : ast::ATTR_BOTTOM);
		for(const ast::View& view : prevGroup)
			output.emplace_back(spacerName, attr1, ast::REL_EQU, view.name, attr2);

		attr1 = isHorizontal ? ast::ATTR_RIGHT : ast::ATTR_BOTTOM;
		attr2 = type == ConnectionType::TOSUPER ? attr1 : (isHorizontal ? ast::ATTR_LEFT : ast::ATTR_TOP);

		for(const ast::View& view : nextGroup)
			output.emplace_back(spacerName, attr1, ast::REL_EQU, view.name, attr2, 1, 0);
	}

	inline void _connectGroups(
			ast::Orientation orient,
			const ast::ViewGroup& prevGroup,
			const ast::ViewGroup& nextGroup,
			ConnectionType type,
			ast::Relation rel,
			boost::optional<double> constant,
			boost::optional<unsigned> prio,
			std::vector<ast::ConstraintDef>& output)
	{
		auto attr1 = orient == ast::ORIENT_H ? ast::ATTR_RIGHT : ast::ATTR_BOTTOM;
		auto attr2 = orient == ast::ORIENT_H ? ast::ATTR_LEFT : ast::ATTR_TOP;
		if(type == ConnectionType::FROMSUPER)
			attr1 = attr2;
		else if(type == ConnectionType::TOSUPER)
			attr2 = attr1;

		auto constVal = constant.map([](auto v){ return -v; });
		for(const ast::View& view1 : prevGroup)
		for(const ast::View& view2 : nextGroup)
			output.emplace_back( view1.name, attr1, rel, view2.name, attr2, 1, constVal, prio);
	}

	inline void _appendGroupName(const ast::ViewGroup& group, std::string& out)
	{
		if(group.size() == 1 && group[0].get().name.empty())
			out.push_back('|');
		else
			for(const ast::View& v : group)
				out.append(v.name);
	}

	inline std::string _getSpacerName(ast::Orientation orient, const ast::ViewGroup& prev, const ast::ViewGroup& next, ast::Connector connector)
	{
		char conn;
		if(connector == ast::CONNECTOR_HYPHEN)
			conn = '-';
		else if(connector ==  ast::CONNECTOR_TILDE)
			conn = '~';
		else assert("Invalid connector type"==0);

		std::string out;
		out.push_back(conn);
		out.push_back(orient == ast::ORIENT_H ? 'H' : 'V');//
		_appendGroupName(prev, out);
		out.push_back(conn);
		_appendGroupName(next, out);

		return out;
	}

	template<typename F, typename T = std::invoke_result_t<F>>
	struct Lazy
	{
		Lazy(F&& f) : _f(std::forward<F>(f)){}

		T& operator*()  { return get(); }
		T& operator->() { return get(); }
		operator bool() const { return _value.has_value(); }

		T& get()
		{
			T* val = _value.get_ptr();
			if(val)
				return *val;

			_value.emplace(_f());
			return _value.value();
		}
	private:
		F _f;
		boost::optional<T> _value;
	};

	inline void visitConnection(
			const ast::Connection& connection,
			ast::Orientation orient,
			const ast::ViewGroup& prevGroup,
			const ast::ViewGroup& nextGroup,
			ConnectionType type,
			const std::string& super,
			std::string& firstTildeName,
			std::vector<ast::ConstraintDef>& output)
	{

		auto const& predicates = connection.predicates;
		auto const connector = connection.connector;

		if(connector == ast::CONNECTOR_ARROW)
			return;

		auto const attr_w_or_h = orient == ast::ORIENT_H ? ast::ATTR_WIDTH : ast::ATTR_HEIGHT;
		auto const* preds = predicates.get_ptr();
		auto const isTilde = connector == ast::CONNECTOR_TILDE;

		auto spacerName = Lazy([&](){ return _getSpacerName(orient, prevGroup, nextGroup, connector);});

		if(isTilde)
		{
			if(firstTildeName.empty())
				firstTildeName = *spacerName;
			else
				output.emplace_back(*spacerName, attr_w_or_h, ast::REL_EQU, firstTildeName, attr_w_or_h);
			_connectSpacer(*spacerName, orient, prevGroup, nextGroup, type, output);
		}

		if(!preds)
		{
			auto constant = connector == ast::CONNECTOR_HYPHEN ? boost::none : boost::optional<double>{0};
			if(!isTilde)
				_connectGroups(orient, prevGroup, nextGroup, type, ast::REL_EQU, constant, boost::none, output);
			return;
		}

		if(auto* constant = get<ast::Constant>(preds))
		{
			if(isTilde)
				output.emplace_back(*spacerName, attr_w_or_h, ast::REL_EQU, super, attr_w_or_h, 1, -*constant);
			else
				_connectGroups(orient, prevGroup, nextGroup, type, ast::REL_EQU, *constant, boost::none, output);
			return;
		}

		if(auto* percent = get<ast::Percentage>(preds))
		{
			output.emplace_back(*spacerName, attr_w_or_h, ast::REL_EQU, super, attr_w_or_h, percent->value());

			if(!isTilde)
				_connectSpacer(*spacerName, orient, prevGroup, nextGroup, type, output);
			return;
		}

		if(auto* prio = get<ast::Priority>(preds))
		{
			if(isTilde)
				output.emplace_back(*spacerName, attr_w_or_h, ast::REL_EQU, super, attr_w_or_h, 1, 0, *prio);
			else
				_connectGroups(orient, prevGroup, nextGroup, type, ast::REL_EQU, boost::none, *prio, output);
			return;
		}

		{ //predicate list
			auto* predList = get<ast::PredicateListWithParens>(preds);
			auto const isSingleConst = (predList->size() == 1) && get<ast::Constant>(&(*predList)[0].elements);

			if(isSingleConst)
			{
				auto const& pred = (*predList)[0];
				auto const& value = get<ast::Constant>((*predList)[0].elements);

				if(isTilde)
					output.emplace_back(*spacerName, attr_w_or_h, pred.relation.value_or(ast::REL_EQU), super, attr_w_or_h, 1, value, pred.priority);
				else
					_connectGroups(orient, prevGroup, nextGroup, type, pred.relation.value_or(ast::REL_EQU),  value, pred.priority, output);
			}
			else
			{
				if(! isTilde)
					_connectSpacer(*spacerName, orient, prevGroup, nextGroup, type, output);
				for(auto const& pred : *predList)
					visitPredicate(pred, *spacerName,  attr_w_or_h, super, output);
			}
		}
	}

	inline void visitCascade(
			const Cascade& cascade,
			ast::Orientation orient,
			const ast::View& super,
			std::vector<ast::ConstraintDef>& output);

	inline void visitGroup(
			const ast::ViewGroup& group,
			ast::Orientation orient,
			const std::string& super,
			std::vector<ast::ConstraintDef>& output)
	{
		for(const ast::View& view : group)
		{
			visitView(view, orient, super, output);

			if(const ast::CascadedViews* cascade = view.cascadedViews.get_ptr())
				visitCascade(Cascade {cascade->superTo(),cascade->rest, cascade->toSuper }, orient, view, output);
		}
	}

	inline void visitCascade(
			const Cascade& cascade,
			ast::Orientation orient,
			const ast::View& super,
			std::vector<ast::ConstraintDef>& output)
	{
		const ast::ViewGroup& superGroup = super.name.empty() ? ROOTGROUP : super.asGroup();
		auto firstTildeName = std::string{};

		auto const& [superTo, rest, toSuper] = cascade;
		auto* prevGroup = &cascade.first();

		visitConnection(superTo, orient, superGroup, *prevGroup, ConnectionType::FROMSUPER, super.name, firstTildeName, output);
		visitGroup(*prevGroup, orient, super.name, output);

		for(auto it=++cascade.rest.begin(); it!= cascade.rest.end(); ++it)
		{
			auto const& [connection, views] = *it;
			visitGroup(views, orient, super.name, output);
			visitConnection(connection, orient, *prevGroup, views, ConnectionType::SIBLING, super.name, firstTildeName,  output);
			prevGroup = &views;
		}

		visitConnection(toSuper, orient, *prevGroup, superGroup, ConnectionType::TOSUPER, super.name, firstTildeName, output);
	}

//	inline void visitEvfl(
//			const ast::ExtendedVisualFormat& ast,
//			std::vector<ast::ConstraintDef>& output)
//	{
//		if(auto* visualFormat = get<ast::VisualFormat>(&ast))
//		{
//			auto const orient = visualFormat->orientation;
//			auto const cascade = Cascade {visualFormat->superTo(), visualFormat->rest, visualFormat->toSuper() };
//
//			if(orient & ast::ORIENT_H)
//				visitCascade(cascade, ast::ORIENT_H, ROOTVIEW,  output);
//
//			if(orient & ast::ORIENT_V)
//				visitCascade(cascade, ast::ORIENT_V, ROOTVIEW,  output);
//
//			return;
//		}
//
//		{
//			auto* constraintFormat = get<ast::ConstraintFormat>(&ast);
//			if(auto* viewName = get<std::string>(&constraintFormat->viewName))
//			{
//				for (auto const&[attr, predicates] : constraintFormat->predicates)
//					for (auto const &pred : predicates)
//						visitPredicate(pred, *viewName, attr, CNSUPER, output);
//				return;
//			}
//
//			for(auto const& viewName : get<std::vector<std::string>>(constraintFormat->viewName))
//			{
//				for(auto const& [attr, predicates] : constraintFormat->predicates)
//					for(auto const& pred : predicates)
//						visitPredicate(pred, viewName, attr, CNSUPER, output);
//			}
//		}
//	}

	inline void visitMultiEvfl(
			const ast::MultiExtendedVisualFormat& ast,
			std::vector<ast::ConstraintDef>& output)
	{
		for(auto const& line : ast)
		{
			if(auto const* visualFormatRow = boost::get<ast::MultiVisualFormatRow>(&line))
			{
				auto const orient = visualFormatRow->orientation;
				for(auto const& visualFormat : visualFormatRow->items)
				{
					auto const cascade = Cascade {visualFormat.superTo(),  visualFormat.rest, visualFormat.toSuper() };

					if(orient & ast::ORIENT_H)
						visitCascade(cascade, ast::ORIENT_H, ROOTVIEW, output);

					if(orient & ast::ORIENT_V)
						visitCascade(cascade, ast::ORIENT_V, ROOTVIEW, output);
				}
				continue;
			}

			{
				auto const* constraintFormatVec = boost::get<ast::MultiConstraintFormatRow>(&line);
				for(auto const& constraintFormat : *constraintFormatVec)
				{
					if(auto* viewName = get<std::string>(&constraintFormat.viewName))
					{
						for(auto const& [attr, predicates] : constraintFormat.predicates)
							for(auto const& pred : predicates)
								visitPredicate(pred, *viewName, attr, CNSUPER, output);

						continue;
					}

					for(auto const& viewName : get<std::vector<std::string>>(constraintFormat.viewName))
					{
						for(auto const& [attr, predicates] : constraintFormat.predicates)
							for(auto const& pred : predicates)
								visitPredicate(pred, viewName, attr, CNSUPER, output);
					}
				}
			}
		}
	}
}
