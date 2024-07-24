#include "AST.hpp"
#include "SequentialDictionary.hpp"
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;

/**
 * Constructors and Destructors
 */

FunctionDefinition::FunctionDefinition(std::string fName, const std::vector<std::string> &args)
{
    name = fName;
    variable_names = args;
    function_body = nullptr;
}

FunctionDefinition::~FunctionDefinition()
{
    delete function_body;
}

BooleanNode::~BooleanNode()
{
    delete left;
    delete right;
}

FunctionCallNode::~FunctionCallNode()
{
    for (unsigned i = 0; i < args.size(); i++)
    {
        delete args[i];
    }
}

IfThenElseNode::~IfThenElseNode()
{
    delete condition;
    delete then_expression;
    delete else_expression;
}

MathNode::~MathNode()
{
    delete left;
    delete right;
}

NumberNode::~NumberNode()
{
    // no dynamic memory to deallocate
}

VariableNode::~VariableNode()
{
    // no dynamic memory to deallocate
}

/**
 * Recursive methods to evaluate a cs20c Tree (Part 1)
 */

bool BooleanNode::evaluate(const Dictionary<string, double> &variable_values,
                           const Dictionary<string, FunctionDefinition *> &function_map) const
{
    // TODO Part 1
    // Evaluate the left operand of the bool expression
    double leftValue = left->evaluate(variable_values, function_map);
    // Evaluate the right operand of the bool expression
    double rightValue = right->evaluate(variable_values, function_map);
    switch (op)
    {
    // Check if the left operand is less than the right operand
    case Comparison::LESS_THAN:
        return leftValue < rightValue;
    // Check if the left operand is greater than the right operand
    case Comparison::GREATER_THAN:
        return leftValue > rightValue;
    // Throw an exception for invalid comparison operations.
    case Comparison::EQUAL:
        return leftValue == rightValue;
    default:
        throw std::string("Invalid comparison operation.");
    }
    return false;
}

double FunctionCallNode::evaluate(
    const Dictionary<string, double> &variable_values,
    const Dictionary<string, FunctionDefinition *> &function_map) const
{
    // TODO Part 1
    // Seed the random number generator with the current time, this is used for the extra credit (logic.cs20c file)
    srand(time(0));
    // Check if the function name is "rand"
    if (name == "rand")
    {
        if (args.size() != 2)
        {
            throw std::string("Rand takes 2 args.");
        }
        // Evaluate the first argument to get the minimum value of the range
        double min_val = args[0]->evaluate(variable_values, function_map);

        // Evaluate the second argument to get the maximum value of the range
        double max_val = args[1]->evaluate(variable_values, function_map);
        
        // Calculate the range
        int range = static_cast<int>(max_val - min_val + 1);

        // Generate a random number within the range
        int randomNumber = rand() % range + static_cast<int>(min_val);

        // Return the generated random number as a double
        return static_cast<double>(randomNumber);
    }
    // Find the function definition in the function map
    FunctionDefinition *f = function_map.find(name);
    // If can't find definition, throw an exception
    if (f == nullptr)
    {
        throw std::string("Function definition not found.");
    }

    // Create a dictionary to store argument values for the function call
    SequentialDictionary<string, double> function_variable_values;
    // Find the definition of the function being called

    // Evaluate the arguments of the function call and store their values.
    for (int i = 0; i < args.size(); i++)
    {
        // Evaluate each argument expression
        double arg_value = args[i]->evaluate(variable_values, function_map);
        // Insert the argument value into the argument dictionary
        function_variable_values.insert(f->variable_names[i], arg_value);
    }

    // Evaluate the function body using the provided argument values and return the result.
    // This involves substituting the argument values into the function body
    // and evaluating the resulting expression.
    return f->function_body->evaluate(function_variable_values, function_map);
}

double IfThenElseNode::evaluate(
    const Dictionary<string, double> &variable_values,
    const Dictionary<string, FunctionDefinition *> &function_map) const
{
    // TODO Part 1
    // Evaluate the condition expression to determine whether it's true or not
    bool conditionResult = condition->evaluate(variable_values, function_map);
    // Check if the condition evaluates to true
    if (conditionResult)
    {
        // If true, evaluate the `then-expression` and return the result
        return then_expression->evaluate(variable_values, function_map);
    }
    else
    {
        // Else, evaluate the `else-expression` and return the result.
        return else_expression->evaluate(variable_values, function_map);
    }
}

double MathNode::evaluate(const Dictionary<string, double> &variable_values,
                          const Dictionary<string, FunctionDefinition *> &function_map) const
{
    // TODO Part 1
    // Evaluate the left operand expression
    double leftValue = left->evaluate(variable_values, function_map);
    // Evaluate the right operand expression
    double rightValue = right->evaluate(variable_values, function_map);

    switch (op)
    {
    case MathOP::PLUS:
        // Add values of left and right operands
        return leftValue + rightValue;
    case MathOP::MINUS:
        // Subtract the value of right from left operand
        return leftValue - rightValue;
    case MathOP::TIMES:
        // Multiply the left and right operands
        return leftValue * rightValue;
    case MathOP::DIVIDED_BY:
        // Check for division by 0, then divide if appropriate
        if (rightValue == 0.0)
        {
            throw std::string("Division by zero error.");
        }
        // Divide left by right operand
        return leftValue / rightValue;
    default:
        throw std::string("Invalid mathematical operation.");
    }
}

double NumberNode::evaluate(const Dictionary<string, double> &variable_values,
                            const Dictionary<string, FunctionDefinition *> &function_map) const
{
    // TODO Part 1
    // just return the value
    return value;
}

double VariableNode::evaluate(const Dictionary<string, double> &variable_values,
                              const Dictionary<string, FunctionDefinition *> &function_map) const
{
    // TODO Part 1
    try
    {
        // Attempt to find the value of the variable in the dictionary
        double value = variable_values.find(variable_name);
        // If found, return the value
        return value;
    }
    catch (std::string &e)
    {
        // If not, return default value of 0.0
        return 0.0;
    }
}

/**
 * Recursive methods to build a call graph from a cs20c Tree (Part 2)
 */

void BooleanNode::buildCallGraph(const std::string &caller, CallGraph &cg) const
{
    // TODO Part 2
    // Check if the current node is a function call node
    if (const FunctionCallNode *functionCallNode = dynamic_cast<const FunctionCallNode *>(this))
    {
        // If it is a function call node, call setCall on caller and functionCallNode->name
        cg.setCall(caller, functionCallNode->name);
        return; // Exit the function call node case to prevent from further traversal
    }
    // If not, recursively traverse the left and right subtrees with the updated caller
    if (left)
    {
        left->buildCallGraph(caller, cg);
    }
    if (right)
    {
        right->buildCallGraph(caller, cg);
    }
}

void FunctionCallNode::buildCallGraph(const std::string &caller, CallGraph &cg) const
{
    // TODO Part 2
    // Set the call information for the current function call node
    cg.setCall(caller, name);
    // Traverse each argument expression
    for (ExpressionNode *arg : args)
    {
        // Recursively traverse the argument expression with the updated caller
        arg->buildCallGraph(caller, cg);
    }
}

void IfThenElseNode::buildCallGraph(const std::string &caller, CallGraph &cg) const
{
    // TODO Part 2
    // Traverse the condition expression
    if (condition)
    {
        condition->buildCallGraph(caller, cg);
    }
    // Traverse the then expression
    if (then_expression)
    {
        then_expression->buildCallGraph(caller, cg);
    }
    // Traverse the else expression
    if (else_expression)
    {
        else_expression->buildCallGraph(caller, cg);
    }
}

void MathNode::buildCallGraph(const std::string &caller, CallGraph &cg) const
{
    // TODO Part 2
    // Recursively build the call graph for mathematical operations
    if (left)
    {
        // If so, build a call graph on caller and cg
        left->buildCallGraph(caller, cg);
    }
    if (right)
    {
        // build a call graph on caller and cg
        right->buildCallGraph(caller, cg);
    }
}

void NumberNode::buildCallGraph(const std::string &caller, CallGraph &cg) const
{
    // This is always a leaf node, and it doesn't call a function,
    // so it has nothing to do.
    return;
}

void VariableNode::buildCallGraph(const std::string &caller, CallGraph &cg) const
{
    // This is always a leaf node, and it doesn't call a function,
    // so it has nothing to do.
    return;
}
