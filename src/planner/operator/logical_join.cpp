#include "planner/operator/logical_join.hpp"

#include "parser/expression/list.hpp"
#include "planner/operator/list.hpp"

using namespace duckdb;
using namespace std;

LogicalJoin::LogicalJoin(JoinType type, LogicalOperatorType logical_type) : LogicalOperator(logical_type), type(type) {
}

vector<string> LogicalJoin::GetNames() {
	auto names = children[0]->GetNames();
	if (type == JoinType::SEMI || type == JoinType::ANTI) {
		// for SEMI and ANTI join we only project the left hand side
		return names;
	}
	if (type == JoinType::MARK) {
		// MARK join has an additional MARK attribute
		names.push_back("MARK");
		return names;
	}
	// for other joins we project both sides
	auto right_names = children[1]->GetNames();
	names.insert(names.end(), right_names.begin(), right_names.end());
	return names;
}

void LogicalJoin::ResolveTypes() {
	types.insert(types.end(), children[0]->types.begin(), children[0]->types.end());
	if (type == JoinType::SEMI || type == JoinType::ANTI) {
		// for SEMI and ANTI join we only project the left hand side
		return;
	}
	if (type == JoinType::MARK) {
		// for MARK join we project the left hand side, plus a BOOLEAN column indicating the MARK
		types.push_back(TypeId::BOOLEAN);
		return;
	}
	// for any other join we project both sides
	types.insert(types.end(), children[1]->types.begin(), children[1]->types.end());
}

void LogicalJoin::GetTableReferences(LogicalOperator &op, unordered_set<size_t> &bindings) {
	TableBindingResolver resolver;
	resolver.VisitOperator(op);
	for(auto &table : resolver.bound_tables) {
		bindings.insert(table.table_index);
	}
}
