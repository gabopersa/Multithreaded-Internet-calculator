#include "calc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

#define MAX_OPERANDS 300000

struct tuple {
	char operand[16];
	int value;
};

struct Calc {
	struct tuple dictionary[MAX_OPERANDS];
	int size;
	pthread_mutex_t mutex;
};

/* Function to create and allocate memory for a new Calc structure */
struct Calc *calc_create(void) {
	struct Calc *new = (struct Calc *) malloc(sizeof(struct Calc));
	new->size = 0;
    pthread_mutex_init(&new->mutex, NULL);
	return new;
}

/* Function to deallocate memory reserved for structure calc passed as parameter */
void calc_destroy(struct Calc *calc) {
	pthread_mutex_destroy(&calc->mutex);
	free(calc);
}

/* Function to check if the string passed as parameter is an operator */
int isOperator(char *token) {
	if (!strcmp(token, "+")) {
		return TRUE;
	}
	if (!strcmp(token, "-")) {
		return TRUE;
	}
	if (!strcmp(token, "*")) {
		return TRUE;
	}
	if (!strcmp(token, "/")) {
		return TRUE;
	}
	return FALSE;
}

/* Function to check if the string passed as parameter is an assing symbol */
int isAssign(char *token) {
	if (!strcmp(token, "=")) {
		return TRUE;
	}
	return FALSE;
}

/* Function to check if an operand is already in the dictonary
 * Return the index in the dictonary of the operand if is found
 */
int findOperand(struct Calc *calc, char *op) {
	for (int i = 0; i < calc->size; ++i) {
		if (!strcmp(calc->dictionary[i].operand, op)) {
			return i;
		}
	}
	return -1;
}

/* Function that tokenizes the expression passed as parameter and store each token in second parameter */
int tokenize_input_line(char *expr, char *tokens[]) {
	int i = 0;
	char *sp;
	char *tok = strtok_r(expr, " \n\r", &sp);
	while (tok != NULL) { 			
		tokens[i] = tok;
		tok = strtok_r(NULL, " \n\r", &sp);
		i++;
	}
	return i;
}

int calc_eval(struct Calc *calc, const char *expr, int *result) {
	char *tokens[128];
	char line[128] = {0};
	strcpy(line, expr);
	/* Tokenizes the expression */
	int count = tokenize_input_line(line, tokens);
	int index, left, right;

	if (count == 2) {	// Invalid number of tokens
		return FALSE;
	}
	else if (count == 1) {	// only one token
		if(!isalpha(tokens[0][0])) {
			if(isdigit(tokens[0][0]) || tokens[0][0] == '-') { // Case only a number. i.e 42
				*result = atoi(tokens[0]);
				return TRUE;
			}
			return FALSE;
		}
		else {	// case onnly operand. i.e a
			index = findOperand(calc, tokens[0]);	// check if the operand already exist in the dictionary
			if(index >= 0) {
				*result = calc->dictionary[index].value;
				return TRUE;
			}
			else{
				return FALSE;
			}
		}
	}
	else if(count == 3) {	// case 3 tokens. i.e a=2 or only computation like number op number or operand op number
		if (isOperator(tokens[1])) { // Only computation
			if(!isalpha(tokens[0][0]) && !isalpha(tokens[2][0])) { // case number op number
				if (!strcmp(tokens[1], "+")) {
					*result = atoi(tokens[0]) + atoi(tokens[2]);
				}
				if (!strcmp(tokens[1], "-")) {
					*result = atoi(tokens[0]) - atoi(tokens[2]);
				}
				if (!strcmp(tokens[1], "*")) {
					*result = atoi(tokens[0]) * atoi(tokens[2]);
				}
				if (!strcmp(tokens[1], "/")) {
					if(!strcmp(tokens[2], "0"))
						return FALSE;
					*result = atoi(tokens[0]) / atoi(tokens[2]);
				}
				return TRUE;
			}
			else if(isalpha(tokens[0][0]) && !isalpha(tokens[2][0])) { // case operand op number
				index = findOperand(calc, tokens[0]); // check if the left operand exist in the dictonary
				if(index >= 0) {
					left = calc->dictionary[index].value; // get the left operand value
				}	
				else {
					return FALSE;
				}				
				if (!strcmp(tokens[1], "+")) {
					*result = left + atoi(tokens[2]);
				}
				if (!strcmp(tokens[1], "-")) {
					*result = left - atoi(tokens[2]);
				}
				if (!strcmp(tokens[1], "*")) {
					*result = left * atoi(tokens[2]);
				}
				if (!strcmp(tokens[1], "/")) {
					if(!strcmp(tokens[2], "0"))
						return FALSE;
					*result = left / atoi(tokens[2]);
				}
				return TRUE;
			}
			else if(!isalpha(tokens[0][0]) && isalpha(tokens[2][0])) { // case number op operand
				index = findOperand(calc, tokens[2]); // check if the left operand exist in the dictonary
				if(index >= 0) {
					right = calc->dictionary[index].value; // get the left operand value
				}	
				else {
					return FALSE;
				}				
				if (!strcmp(tokens[1], "+")) {
					*result = atoi(tokens[0]) + right;
				}
				if (!strcmp(tokens[1], "-")) {
					*result = atoi(tokens[0]) - right;
				}
				if (!strcmp(tokens[1], "*")) {
					*result = atoi(tokens[0]) * right;
				}
				if (!strcmp(tokens[1], "/")) {
					if(!strcmp(tokens[2], "0"))
						return FALSE;
					*result = atoi(tokens[0]) / right;
				}
				return TRUE;
			}
			else if(isalpha(tokens[0][0]) && isalpha(tokens[2][0])) { // case operand op operand
				pthread_mutex_lock(&calc->mutex);
				index = findOperand(calc, tokens[0]); // check if the left operand exist in the dictonary
				if(index >= 0) {
					left = calc->dictionary[index].value; // get the left operand value
				}	
				else {
					pthread_mutex_unlock(&calc->mutex);
					return FALSE;
				}	
				
				index = findOperand(calc, tokens[2]); // check if the right operand exist in the dictonary
				if(index >= 0) {
					right = calc->dictionary[index].value; // get the right operand value
				}	
				else {
					pthread_mutex_unlock(&calc->mutex);
					return FALSE;
				}				
				if (!strcmp(tokens[1], "+")) {
					*result = left + right;
				}
				if (!strcmp(tokens[1], "-")) {
					*result = left - right;
				}
				if (!strcmp(tokens[1], "*")) {
					*result = left * right;
				}
				if (!strcmp(tokens[1], "/")) {
					if(!strcmp(tokens[2], "0"))
						return FALSE;
					*result = left / right;
				}
				pthread_mutex_unlock(&calc->mutex);
				return TRUE;
			}
		}
		else if(isAssign(tokens[1])) {	
			if(isalpha(tokens[2][0])) {	// case update a operand is already in the dictonary
				pthread_mutex_lock(&calc->mutex);
				index = findOperand(calc, tokens[2]);
				if(index >= 0) {
					int index2 = findOperand(calc, tokens[0]);
					if(index2 >= 0) {
						calc->dictionary[index2].value = calc->dictionary[index].value;
						*result = calc->dictionary[index].value;
				  		pthread_mutex_unlock(&calc->mutex); 
						return TRUE;    
					}
					else
					{
						strcpy(calc->dictionary[calc->size].operand, tokens[0]);
						calc->dictionary[calc->size].value = calc->dictionary[index].value;
						*result = calc->dictionary[index].value;
						calc->size++;
				  		pthread_mutex_unlock(&calc->mutex); 
						return TRUE;					
					}
				}	
				else {
			  		pthread_mutex_unlock(&calc->mutex);
					return FALSE;
				}
			}
			else { // case create a new operand and store in the dictonary
				pthread_mutex_lock(&calc->mutex);
				index = findOperand(calc, tokens[0]);
				if(index >= 0) {
					calc->dictionary[index].value = atoi(tokens[2]);
					*result = calc->dictionary[index].value;
			  		pthread_mutex_unlock(&calc->mutex); 
					return TRUE;
				}
				else
				{
					strcpy(calc->dictionary[calc->size].operand, tokens[0]);
					calc->dictionary[calc->size].value = atoi(tokens[2]);
					*result = calc->dictionary[calc->size].value;
					calc->size++;
			  		pthread_mutex_unlock(&calc->mutex); 
					return TRUE;
				}
			}
		}
	}
	else if(count == 5) { // Case 5 tokens. i.e: a = 2 + 2; a = b + c; a = 2 + c; a = d + 2;
		if(isalpha(tokens[0][0]) && isAssign(tokens[1])) { // always should be first to tokens operand = 
			if(!isalpha(tokens[2][0]) && !isalpha(tokens[4][0]) && isOperator(tokens[3])) { // case operand = number op number
				if (!strcmp(tokens[3], "+")) {
					*result = atoi(tokens[2]) + atoi(tokens[4]);
				}
				if (!strcmp(tokens[3], "-")) {
					*result = atoi(tokens[2]) - atoi(tokens[4]);
				}
				if (!strcmp(tokens[3], "*")) {
					*result = atoi(tokens[2]) * atoi(tokens[4]);
				}
				if (!strcmp(tokens[3], "/")) {
					if(!strcmp(tokens[4], "0")){
						return FALSE;
					}
					*result = atoi(tokens[2]) / atoi(tokens[4]);
				}
			  	pthread_mutex_lock(&calc->mutex);
				index = findOperand(calc, tokens[0]);
				if(index >= 0) {	// if the operand already exist in the dictonary only update the value
					calc->dictionary[index].value = *result;
			  		pthread_mutex_unlock(&calc->mutex);
				}	
				else { // if the operand does not exist in the dictonary create a new operand in the dictonary
					strcpy(calc->dictionary[calc->size].operand, tokens[0]);
					calc->dictionary[calc->size].value = *result;
					calc->size++;
			  		pthread_mutex_unlock(&calc->mutex); 
				}
				return TRUE;				
			}
			else if(isalpha(tokens[2][0]) && !isalpha(tokens[4][0]) && isOperator(tokens[3])) { // case operand = operand op number
			  	pthread_mutex_lock(&calc->mutex);
				index = findOperand(calc, tokens[2]); // check if the left operand exist in the dictonary
				if(index >= 0) {
					left = calc->dictionary[index].value; // get the left operand value
				}	
				else {
			  		pthread_mutex_unlock(&calc->mutex);
					return FALSE;
				}

				if (!strcmp(tokens[3], "+")) {
					*result = left + atoi(tokens[4]);
				}
				if (!strcmp(tokens[3], "-")) {
					*result = left - atoi(tokens[4]);
				}
				if (!strcmp(tokens[3], "*")) {
					*result = left * atoi(tokens[4]);
				}
				if (!strcmp(tokens[3], "/")) {
					if(!strcmp(tokens[4], "0")){
						return FALSE;
					}
					*result = left / atoi(tokens[4]);
				}

				index = findOperand(calc, tokens[0]); // check if the first operand already exist in the dictonary
				if(index >= 0) {	// if the operand exist only update the valye
					calc->dictionary[index].value = *result;
			  		pthread_mutex_unlock(&calc->mutex);
				}	
				else {	// if the operand does not exist create it and store the value
					strcpy(calc->dictionary[calc->size].operand, tokens[0]);
					calc->dictionary[calc->size].value = *result;
					calc->size++;
					pthread_mutex_unlock(&calc->mutex);
				}
				return TRUE;				
			}
			else if(!isalpha(tokens[2][0]) && isalpha(tokens[4][0]) && isOperator(tokens[3])) { // case operand = number op operand
			  	pthread_mutex_lock(&calc->mutex);
				index = findOperand(calc, tokens[4]); // check if the right operand exist in the dictonary
				if(index >= 0) { // if the right operand exist get the value
					right = calc->dictionary[index].value;
				}	
				else {
				  	pthread_mutex_unlock(&calc->mutex);
					return FALSE;
				}

				if (!strcmp(tokens[3], "+")) {
					*result = atoi(tokens[2]) + right;
				}
				if (!strcmp(tokens[3], "-")) {
					*result = atoi(tokens[2]) - right;
				}
				if (!strcmp(tokens[3], "*")) {
					*result = atoi(tokens[2]) * right;
				}
				if (!strcmp(tokens[3], "/")) {
					if(!strcmp(tokens[4], "0")){
						return FALSE;
					}
					*result = atoi(tokens[2]) / right;
				}

				index = findOperand(calc, tokens[0]); // check if the first operand exist in the dictonary
				if(index >= 0) { // Case only update
					calc->dictionary[index].value = *result;
			  		pthread_mutex_unlock(&calc->mutex);
				}	
				else { // case create the new operand
					strcpy(calc->dictionary[calc->size].operand, tokens[0]);
					calc->dictionary[calc->size].value = *result;
					calc->size++;
					pthread_mutex_unlock(&calc->mutex);
				}
				return TRUE;				
			}
			else if(isalpha(tokens[2][0]) && isalpha(tokens[4][0]) && isOperator(tokens[3])) { // case operand = operand op operand
			  	pthread_mutex_lock(&calc->mutex);
				index = findOperand(calc, tokens[2]); // check if left operand exist
				if(index >= 0) { // get the left operand value
					left = calc->dictionary[index].value;
				}	
				else {
			  		pthread_mutex_unlock(&calc->mutex);
					return FALSE;
				}

				index = findOperand(calc, tokens[4]); // check if right operand exist
				if(index >= 0) { // get the right operand value
					right = calc->dictionary[index].value;
				}	
				else {
			  		pthread_mutex_unlock(&calc->mutex);
					return FALSE;
				}

				if (!strcmp(tokens[3], "+")) {
					*result = left + right;
				}
				if (!strcmp(tokens[3], "-")) {
					*result = left - right;
				}
				if (!strcmp(tokens[3], "*")) {
					*result = left * right;
				}
				if (!strcmp(tokens[3], "/")) {
					if(!strcmp(tokens[4], "0")){
						return FALSE;
					}
					*result = left / right;
				}

				index = findOperand(calc, tokens[0]); // check if the first operand exist
				if(index >= 0) { // if exists, update the value
					calc->dictionary[index].value = *result;
			  		pthread_mutex_unlock(&calc->mutex);
				}	
				else { // if does not exist create the new operand and store the result
					strcpy(calc->dictionary[calc->size].operand, tokens[0]);
					calc->dictionary[calc->size].value = *result;
					calc->size++;
					pthread_mutex_unlock(&calc->mutex);
				}
				return TRUE;				
			}
		}
		else {
			return FALSE;
		}
	}
	return FALSE;
}
