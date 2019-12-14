# Multithreaded-Internet-calculator
Multithreaded Calculator Server that listens for client TCP connections, reads a sequence of expressions, and evaluates each expression.

The calculator implementation do operations exclusively using the int data type.

The following kinds of expressions are be supported:

	* operand
	* operand op operand
	* var = operand
	* var = operand op operand
	
An operand is either a literal integer or a variable name. A variable name (var) is a sequence of one or more alphabetic characters (A-Z or a-z.)

An op is one of the following operators: + (addition), - (subtraction), * (multiplication), / (division).

Space characters should be ignored. The calculator assume that all tokens (operands, operators, and =) are separated by at least one space character. So, for example,

a + 4

is a valid expression, but

a+4

is not a valid expression.

Instuctions:

	* Clone this repo
	* Open a terminal and type "make" to compile the project
	* Run the server using ./calcServer <port number>
	* Run clients using telnet <localhost> <port number>
