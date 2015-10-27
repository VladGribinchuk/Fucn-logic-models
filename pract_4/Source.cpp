#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <set>
#include <unordered_map>

class Variable
{ // variable, that submits boolean logic
public:
	using Vector = std::vector<Variable>;
	using Stack = std::stack<Variable>;
	using type = char;
	using val = short;

	Variable() = default;
	explicit Variable(Variable::type _name) : name(_name), value(0) {}
	explicit Variable(int _value) : name('#'), value(_value) {}

	// for provide boolean arithmetic 

	const Variable& operator++() {
		if (++value > 1) value = 0;
		return *this;
	}
	const Variable operator++(int) {
		Variable old(*this);
		if (++value > 1) value = 0;
		return old;
	}
	const Variable operator+(const Variable& right) {
		if (this->value == 1 && right.value == 1)
			return Variable{ 1 };
		else
			return Variable{ this->value + right.value };
	}
	const Variable operator*(const Variable& right)	{
		return Variable{ this->value * right.value };
	}
	const Variable operator!()	{
		return Variable{ (this->value ? 0 : 1) };
	}
	bool operator==(const Variable& right)	{
		return this->name == right.name;
	}

	char ValToChar() const	{return (value ? '1' : '0');}

	Variable::type name;
	Variable::val value;
};


class Parser
{ // parse input boolean function
public:
	class IncorrectExpr {};

	Parser(std::string _expr) : expr{ _expr } {}

	std::string ToPostfix()
	{
		// out-string, which will contain expression in postfix notation
		std::string out;	
		// temporary stack of operations
		std::stack<std::string::value_type> stck;

		// for check correctness of expression
		int count_operations{ 0 };
		int pos_var{ -1 };
		int i{ 0 };
		for (const auto& ch : expr)
		{
			++i;
			if (ch >= 'a' && ch <= 'z')
			{
				// between any two vars must be at least an operation
				if(pos_var+1 == i)
					throw Parser::IncorrectExpr();

				// if found character is variable, put it in out-string
				set_vars.insert(ch);
				out.push_back(ch);
				pos_var = i;
			}
			else
			{	// if found character is operation, such as '*', '+', '~', '(', ')'
				++count_operations;
				if (ch == ')')
				{
					while (!stck.empty() && stck.top() != '(')
					{ // while found char is not open parenthesis
						out.push_back(stck.top());
						stck.pop();
					}
					if (stck.empty())
						throw Parser::IncorrectExpr();
					else 
						stck.pop(); // pop '('
				}
				else
					stck.push(ch);
			}
		}
		while (!stck.empty())
		{
			if (stck.top() == '(') throw Parser::IncorrectExpr();
			out.push_back(stck.top());
			stck.pop();
		}

		if(!count_operations) throw Parser::IncorrectExpr();

		return out;
	}

	Variable::Vector GetVariables() noexcept
	{ // retrieve all variables from expression
		Variable::Vector vars;
		for (auto& one : set_vars)
			vars.push_back(Variable{ one });

		return vars;
	}

private:
	std::string expr;			// boolean expression in 'normal'(infix) form
	std::set<char> set_vars;	// storages of variables on expr
};

class Calculator
{ // calculate boolean expression
public:
	class ErrorCommand {}; // exception

	static Variable::type calculate(std::string postfix_expr, Variable::Vector vars)
	{
		Variable::Stack values;
		for (auto ch : postfix_expr)
		{
			if (ch >= 'a' && ch <= 'z')
			{ // if found symbol is variable, put it in stack
				auto it = std::find(vars.begin(), vars.end(), Variable{ch});
				values.push(*it);
			}
			else
			{
				if (ch == '~')
				{
					if (values.empty()) throw Calculator::ErrorCommand();
					auto val = values.top();
					values.pop();
					values.push(!val);
				}
				else
				{
					if (ch == '*' || ch == '+')
					{
						if (values.empty()) throw Calculator::ErrorCommand();
						Variable right = values.top();
						values.pop();
						if (values.empty()) throw Calculator::ErrorCommand();
						Variable left = values.top();
						values.pop();
						Variable res;
						switch (ch)
						{
						case '*':
							res = left * right;
							break;
						case '+':
							res = left + right;
							break;
						}
						values.push(res);
					}
					// error command was found
					else throw Calculator::ErrorCommand(); 
				}
			}
		} // for(...)

		if (values.empty()) throw Calculator::ErrorCommand();
		return values.top().ValToChar();
	}

};

class BinaryTree
{
public:

	BinaryTree(Variable::Vector _vars, std::string _bool_func) : 
		vars{ _vars }, 
		bool_func{ _bool_func } 
	{}

	using Row = std::vector<Variable::type>;
	using Table = std::vector<Row>;

	void Create()
	{
		const auto& it = vars.begin();
		newNode(&head, it, 0); // create tree and fill nodes with variables
		try {
			compute(head); // compute all paths
		}
		catch(Calculator::ErrorCommand& e) {
			throw;
		}
	}

	void PrintTree()
	{
		// Output tree to standart out as binary tree.
		// This block has some tricky and subtle areas, 
		// so there is good advice for you: 
		// do not try introspect too much.
		
		using std::cout;
		using std::endl;

		cout << "Binary tree of solutions" << endl;

		high_level =  (int)std::pow(2, max_level) ;
		if (head != nullptr)
		{
			for (int i = 0; i < max_level - 1; ++i) cout << ' ';
			cout << head->getName() << endl;
			for (int i = 0; i < max_level - 1; ++i) cout << ' ';
			cout << "/\\" << endl;
		}
		for (int i = 1; i < max_level; ++i)
		{
			for (int l = 0; l < max_level - i ; ++l) cout << ' ';
			printNode(head, i);
			cout << endl;
			for (int l = 0; l < max_level - i - 1; ++l) cout << ' ';
			for (int k = 0; k < (int)std::pow(2,i); ++k)
			{
				cout << "/\\";
				for (int c = 0; c < high_level / (int)std::pow(2, i) - 2; ++c)
					cout << ' ';
			}
			cout << endl;
		}
		printNode(head, max_level);
		cout << endl;
	}

	void PrintTable()
	{
		using std::cout;
		using std::endl;
		
		Table table = treeAsTable();

		cout << "Truth table:" << endl;
		outTable(table);

		// I do not want to explain
		// next few dozens lines of code,
		// but all of this is performing upgrade
		// simple truth table to primitives cubes representation
		int count_var = vars.size();
		std::set<std::pair<int,int>> mp;
		std::vector<int> to_erase;
		int ind{ 0 };
		while (ind < table.size() - 1)
		{ // main idea: compare all rows
			for (int j = ind + 1; j < table.size(); ++j)
			{
				auto got = mp.find({ ind,j });
				if (got != mp.end()) continue;
				if (table[ind].back() == table[j].back())
				{
					int count{ 0 };
					int pos{ 0 };
					for (int k = 0; k < table[ind].size() - 1; ++k)
					{
						// maybe this block is a little bit ugly,
						// but it is neccessary check
						if (table[ind][k] == 'x' || table[j][k] == 'x') --count;
						if (table[ind][k] == 'x' && table[j][k] == 'x') ++count;
						if (table[ind][k] == table[j][k])
							++count;
						else
							pos = k;
					}
					if (count >= count_var - 1)
					{
						if (count < count_var)
						{
							to_erase.push_back(ind);
							to_erase.push_back(j);
							mp.insert({ ind, j });
						
							Row tmp;
							for (int k = 0; k < table[ind].size(); ++k)
							{
								if (k == pos)
									tmp.push_back('x');
								else
									tmp.push_back(table[ind][k]);
							}
							table.push_back(tmp);
							ind = -1;
							break;
						}
						else
							to_erase.push_back(ind);
					}
				}
			}
			++ind;
		}

		Table prim;
		for (int i = 0; i < table.size(); ++i)
		{
			auto got = std::find(to_erase.begin(), to_erase.end(), i);
			if (got == to_erase.end())
				prim.push_back(table[i]);
		}
		cout << "Primitive cubes:" << endl;
		outTable(prim);
	}

	~BinaryTree()
	{
		if (head != nullptr) deleteNode(&head);
	}

private:

	class Node
	{ // one node in tree, that has two pointers to other nodes - left and right child
	public:
		Node(Variable _data, int _level) : data{ _data }, level{ _level } {}

		Node* left{ nullptr };
		Node* right{ nullptr };
		int level; // depth from head

		Variable::type getName() noexcept 
		{
			return data.name;
		}
	private:
		Variable data;
	};

	void newNode(Node** vertex, Variable::Vector::iterator var, int level)
	{
		if (var != vars.end())
		{ 
			*vertex = new Node(*var, level);
			++var;
			newNode(&(*vertex)->left, var, level + 1);
			newNode(&(*vertex)->right, var, level + 1);
		}
	}

	void deleteNode(Node** vertex)
	{
		// recursive descent on all nodes
		if ((*vertex)->left != nullptr)
			deleteNode(&(*vertex)->left);
		else
		{
			delete *vertex;
			*vertex = nullptr;
			return;
		}
		if ((*vertex)->right != nullptr)
		{
			deleteNode(&(*vertex)->right);
			delete *vertex;
			*vertex = nullptr;
		}
		else
		{
			delete *vertex;
			*vertex = nullptr;
		}
	}

	void printNode(Node* vertex, int level)
	{
		// level - all nodes that placed on desired depth
		// would be printed
		if (vertex->level == level)
		{
			std::cout << vertex->getName();
			if (level != max_level)
			{
				// print offset for node from left edge of console window
				for (int i = 0; i < high_level / std::pow(2,vertex->level) - 1; ++i)
					std::cout << ' ';
			}
			return;
		}
		printNode(vertex->left, level);
		
		if (vertex->level == level)
		{
			std::cout << vertex->getName();
			if (level != max_level)
			{
				for (int i = 0; i < high_level / std::pow(2, vertex->level) - 1; ++i)
					std::cout << ' ';
			}
			return;
		}
		printNode(vertex->right, level);
		
	}

	void compute(Node* vertex)
	{
		if (vertex->left != nullptr)
			compute(vertex->left);
		else
		{
			// recall that vars is vector of all variables
			Variable::type res = Calculator::calculate(bool_func, vars);
			vertex->left = new Node(Variable(res), vertex->level + 1);
			max_level = vertex->left->level;
			vars[vertex->level]++;
		}
		
		if (vertex->right != nullptr)
		{
			vars[vertex->level]++;
			compute(vertex->right);
			vars[vertex->level]++;
		}
		else
		{
			Variable::type res = Calculator::calculate(bool_func, vars);
			vertex->right = new Node(Variable(res), vertex->level + 1);
			max_level = vertex->right->level;
			vars[vertex->level]++;
		}
		
	}

	Table treeAsTable() noexcept
	{
		if (head != nullptr)
		{
			Table table;
			toTable(head, table);
			return table;
		}
		else
			// return empty table
			return Table{};
	}

	void toTable(Node* vertex, Table& table)
	{
		static Row row;

		if (vertex->left != nullptr)
		{
			row.push_back('0');
			toTable(vertex->left, table);
			row.pop_back();
		}
		else
		{
			row.push_back(vertex->getName());
			table.push_back(row);
			row.pop_back();
			return;
		}

		if (vertex->right != nullptr)
		{
			row.push_back('1');
			toTable(vertex->right, table);
			row.pop_back();
		}
		else
		{
			row.push_back(vertex->getName());
			table.push_back(row);
			row.pop_back();
		}
	}

	void outTable(Table table)
	{
		using std::cout;
		using std::endl;

		for (auto each : vars)
			cout << ' ' << each.name << '\t';
		cout << " F" << endl;
		for (auto row : table)
		{
			for (auto cell : row)
				cout << '|' << cell << '\t';
			cout << endl;
		}
		cout << endl;
	}


	Node* head{ nullptr };
	Variable::Vector vars;
	std::string bool_func;
	
	int max_level{ 0 };		// max depth(Note: level for head is 0)
	int high_level{ 0 };	// actually, count of leaves of tree
};

class Receiver
{
public:
	void Go()
	{
		read();
		print();
	}
	
private:

	void read()
	{
		std::cout << "Type boolean function: " << std::endl;
		std::getline(std::cin, bool_func);
	}
	void print()
	{
		
		try {

			// parse input string, reduce it to the postfix form
			Parser parser{ bool_func };
			bool_func = parser.ToPostfix(); // may throw IncorrectExpr
			// and retrieve variables from expression
			vars = parser.GetVariables();

			// create binary tree of boolean expression
			BinaryTree bitree{ vars, bool_func };
			bitree.Create(); //may throw ErrorCommand
			bitree.PrintTable();
			bitree.PrintTree();

		}
		catch (Parser::IncorrectExpr& e){
			std::cout << "incorrect expression" << std::endl;
			return;
		}
		catch (Calculator::ErrorCommand& e) {
			std::cout << "wrong operand" << std::endl;
			return;
		}
	}

	std::string bool_func;
	Variable::Vector vars;
};

int main()
{
	Receiver r;
	r.Go();

	system("pause");
	return EXIT_SUCCESS;
}

