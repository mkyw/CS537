/* Michael Wong
 * mwong49
*/

/* This function takes a string as input and removes 
 * leading and trailing whitespace including spaces
 * tabs and newlines. It also removes multiple internal
 * spaces in a row. Arrays are passed by reference.
 */

#include <stdio.h>
#include <stdlib.h>
#include "student_functions.h"

void Clean_Whitespace(char str[]) {
    int i = 0, j = 0, size = 0;
    for (int k = 0; str[k] != '\0'; k++) {
	    size++;
    }
    char newStr[size];
    while (str[i] != '\0') {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
            newStr[j] = str[i];
	    j++;
        }
	if (j != 0 && str[i] == ' ' && str[i-1] != ' ') {
		newStr[j] = str[i];
		j++;
	}
	i++;
    }
    newStr[j] = '\0';
    str = newStr;
    return;
}

/* This function takes a string and makes the first
 * letter of every word upper case and the rest of the
 * letters lower case
 */ 
void Fix_Case(char str[]) {
    // do your work here
    // newStr strtok, change first letters to uppercase and everything else to lower, add spaces back in
       	int i = 0;
    while (str[i] != '\0') {
        if ((i == 0 || str[i-1] == ' ') && str[i] != ' ' && str[i] >= 97)
		str[i] = str[i] - 32;
	else if (str[i] <= 90)
		str[i] = str[i] + 32;
    }
    return;
}

/* this function takes a string and returns the 
 * integer equivalent
 */
int String_To_Year(char str[]) {
    // do your work here
    return atoi(str);
}


/* this function takes the name of a 
 * director as a string and removes all but
 * the last name.  Example:
 * "Bucky Badger" -> "Badger"
 */
void Director_Last_Name(char str[]) {
    // do your work here
    int i = 0, j = 0, size = 0;
    for (int a = 0; str[a] != '\0'; a++)
	    size++;
    char newStr[size];
    for (int b = 0; str[b] != ' '; b++)
	    i++;
    for (; str[i] != '\0'; i++) {
	    newStr[j] = str[i];
	    j++;
    }
    str = newStr;
    return;
}


/* this function takes the a string and returns
 * the floating point equivalent
 */
float String_To_Rating(char str[]) {
    // do your work here
    return atof(str);
}


/* this function takes a string representing
 * the revenue of a movie and returns the decimal
 * equivlaent. The suffix M or m represents millions,
 * K or k represents thousands.
* example: "123M" -> 123000000 
*/
long long String_To_Dollars(char str[])  {
    // do your work here
    // if str == M or m, add 000000
    // if str == K or k, add 000
    int len = 0;
    for (int a = 0; str[a] != '\0'; a++) {
	    len++;
    }
    char newStr[len];
    int i = 0;
    for (int b = 0; str[b] <= 57; b++) {
	    newStr[i] = str[b];
	    i++;
    }
    if (newStr[i] == 'K' || newStr[i] == 'k') {
	    for (int c = 0; c < 3; c++) {
		    newStr[i] = 0;
		    i++;
	    }
    }
    if (newStr[i] == 'M' || newStr[i] == 'm') {
	    for (int d = 0; d < 6; d++) {
		    newStr[i] = 0;
		    i++;
	    }
    }
    return atol(newStr);
}


/* This function takes the array of strings representing 
 * the csv movie data and divides it into the individual
 * components for each movie.
 * Use the above helper functions.
 */
void Split(char csv[10][1024], int num_movies, char titles[10][1024], int years[10], char directors[10][1024], float ratings[10], long long dollars[10]) {
    // do your work here
    int numMovies;
    for (int i = 0; i < 10; i++) {
	if (csv[i][1] != '\0') {
	    numMovies++;
            Clean_Whitespace(csv[i]);
            Fix_Case(csv[i]);
	    
	    int a = 0;
	    char t [1024];
	    while (csv[i][a] != ',') {
                t[a] = csv[i][a];
		a++;
	    }
	    if (csv[i][a] == ',') {
		t[a] = '\0';
	    }
	    Clean_Whitespace(t);
	    for (int x = 0; t[x] != '\0'; x++) {
	        titles[i][x] = t[x];
	    }
	    a++;

            int b = 0;
	    char y[1024];
	    while (csv[i][a] != ',') {
                y[b] = csv[i][a];
		a++;
		b++;
	    }
	    Clean_Whitespace(y);
	    years[i] = String_To_Year(y);
	    a++;
	    b = 0;

	    char d[1024];
	    while (csv[i][a] != ',') {
                d[b] = csv[i][a];
		a++;
		b++;
	    }
	    if (csv[i][a] == ',') {
		d[a] = '\0';
	    }
	    Clean_Whitespace(d);
	    Director_Last_Name(d);
	    for (int y = 0; d[y] != '\0'; y++) {
                directors[i][y] = d[y];
	    }
	    a++;
	    b = 0;

	    char r[1024];
	    while (csv[i][a] != ',') {
                r[b] = csv[i][a];
		a++;
		b++;
	    }
	    Clean_Whitespace(r);
	    ratings[i] = String_To_Rating(r);
            a++;
	    b = 0;

	    char m[1024];
	    while (csv[i][a] != '\0') {
		    m[b] = csv[i][a];
		    a++;
		    b++;
	    }
	    Clean_Whitespace(m);
	    dollars[i] = String_To_Dollars(m);
	}
    }
    num_movies = numMovies;
    return;
}



/* This function prints a well formatted table of
 * the movie data 
 * Row 1: Header - use name and field width as below
 * Column 1: Id, field width = 3, left justified
 * Column 2: Title, field width = length of longest movie + 2 or 7 which ever is larger, left justified, first letter of each word upper case, remaining letters lower case, one space between words
 * Column 3: Year, field with = 6, left justified
 * Column 4: Director, field width = length of longest director last name + 2 or 10 (which ever is longer), left justified, only last name, first letter upper case, remaining letters lower case
 * column 5: Rating, field width = 6, precision 1 decimal place (e.g. 8.9, or 10.0), right justified
 * column 6: Revenue, field width = 11, right justified
 */
void Print_Table(int num_movies, char titles[10][1024], int years[10], char directors[10][1024], float ratings[10], long long dollars[10]) {
    // do your work here
    int tfw = 7, dfw = 10;
    int j = 0;
    for (int i = 0; i < num_movies; i++) {
	while (titles[i][j] != '\0') {
		j++;
	}
	if (j > tfw) {
		tfw = j;
	}
	j = 0;
	while (directors[i][j] != '\0') {
		j++;
	}
	if (j > dfw) {
		dfw = j;
	}
	j = 0;
    }

    char* Id = "Id";
    char* Title = "Title";
    char* Year = "Year";
    char* Director = "Director";
    char* Rating = "Rating";
    char* Revenue = "Revenue";

    printf("%3s%*s%6s%*s%6s%11s\n", Id, tfw, Title, Year, dfw, Director, Rating, Revenue);

    for (int a = 0; a < num_movies; a++) {
            printf("%-3d%-*s%-6d%-*s%6.1f%11lld\n", a, tfw, titles[a], years[a], dfw, directors[a], ratings[a], dollars[a]);
    }
    return;
}


