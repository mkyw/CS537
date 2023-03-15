// do not include other libraries
#include <stdio.h>
#include <stdlib.h>

// ***************************************
// *** struct definitions ****************
// *** do not change these ***************
// ***************************************

typedef struct NODE {int data; struct NODE* next;} NODE; // nodes of the linked list
typedef struct LINKED_LIST {struct NODE *head;} LINKED_LIST; // struct to act as the head of the list

// ***************************************
// *** provided functions  ****************
// ***************************************

// this function returns a LINKED_LIST struct to
// act as the head of the linked list.
// do not change this function
LINKED_LIST Create_List(void) {LINKED_LIST list = {NULL}; return list;}

// call this function to verify malloc worked when you create new nodes
void Verify_Malloc(NODE *node) {if (node == NULL) {printf("Malloc Failed\n"); 
exit(1);} return;}

// do not change this function
// this function prints out all of the nodes in the linked list
void Print_List(FILE *out, LINKED_LIST list) {
    if (list.head == NULL) {printf("\n"); return;} //empty list
    NODE *current = list.head;
    while (current->next != NULL) {
        fprintf(out, "%d -> ",current->data);
        current = current->next;
    }
    fprintf(out, "%d\n",current->data);
    return;
}

// ******************************************************
// *** Complete the following functions  ****************
// ******************************************************
// this function returns the number 
// of elements in the linked list
int Size(LINKED_LIST list){
    if (list.head == NULL) { return 0;} // return 0 if list is empty

    // loop through list starting with head, increment counter s
    NODE *current = list.head;
    int s = 0;
    while (current != NULL) {
        s++;
	current = current-> next;
    }

    // return counter s
    return s;
}

// this function adds an element to
// the front of the list
void Push_Front(LINKED_LIST *list, int data){
    // create new node with given data
    struct NODE* newNode;
    newNode = (struct NODE*) malloc(sizeof(struct NODE));
    newNode->data = data;

    // assign new node to the head of the list
    struct NODE* n = list->head;
    newNode->next = n;
    list->head = newNode;

    return;
}

// this function adds an element 
// to the end of the linked list 
void Push_Back(LINKED_LIST *list, int data) {
    // create new node with given data
    struct NODE* newNode;
    newNode = (struct NODE*) malloc(sizeof(struct NODE));
    newNode->data = data;
    
    // loop to the end of the list
    struct NODE* currNode = list->head;

    if (currNode == NULL) {
        list->head = newNode;
        return;
    }

    else {
        while (currNode->next != NULL) {
            currNode = currNode->next;
        }

        // append new node to the end of the list
        currNode->next = newNode;
    }

    return;
}

// if the list is not empty
// the value of the first element of the list is returned by reference in the parameter data
// the first element of the list is deleted
// returns 0 if the list is empty otherwise returns 1
// remember to free the deleted node
int Pop_Front(LINKED_LIST *list, int *data) {
    // return 0 if list is empty
    if (Size(*list) == 0) return 0;

    // delete head element and reassign to next
    *data = list->head->data;
    struct NODE* n = list->head;
    list->head = list->head->next;
    free(n);

    // return 1
    return 1;

}

// if the list is not empty
// the value of the last element of the list is returned by reference in the parameter data
// the last element of the list is deleted
// returns 0 if the list is empty otherwise returns 1
// remember to free the deleted node
int Pop_Back(LINKED_LIST *list, int *data) {
    // return 0 if list is empty
    if (Size(*list) == 0) return 0;

    // loop to the second last node
    struct NODE* currNode = list->head;
    while (currNode->next->next != NULL) {
	currNode = currNode->next;
    }

    // delete the last node and free the deleted node
    *data = currNode->next->data;
    struct NODE* n = currNode->next;
    currNode->next = NULL;
    free(n);

    // return 1
    return 1;
}

// this function returns the number 
// of times that the value of the parameter 
// data appears in the list
int Count_If(LINKED_LIST list, int data) {
    // start from the first node
    struct NODE* currNode = list.head;
    
    // create a counter
    int s = 0;

    // loop through the linked list, incrementing counter if data matches
    while (currNode != NULL) {
        if (currNode->data == data) {
	    s++;
	}
	currNode = currNode->next;
    }

    // return the value of the counter
    return s;
}

// delete the first node containing the data value
// return 1 if something was deleted otherwise 0
// remember to free the deleted node
int Delete(LINKED_LIST *list, int data) {
    // return 0 if list is empty
    if (Size(*list) == 0) return 0;

    // start from the first node
    struct NODE* currNode = list->head;

    // case if head node matches data
    if (currNode->data == data) {
	struct NODE* n = list->head;
	list->head = list->head->next;
	free(n);
	return 1;
    }

    // loop through rest of linked list, stopping if data matches
    while (currNode != NULL) {
        if (currNode->next != NULL && currNode->next->data == data) {
	    struct NODE* n = currNode->next;
	    currNode->next = currNode->next->next;
	    free(n);
	    return 1;
	}
	currNode = currNode->next;
    }

    // return 0 if data is not found
    return 0;    
}

// return 1 if the list is empty otherwise returns 0
int Is_Empty(LINKED_LIST list) {
    if (Size(list) == 0)
        return 1;
    return 0;
}

// delete all elements of the list
// remember to free the nodes
void Clear(LINKED_LIST *list) {
    struct NODE* currNode = list->head;
    struct NODE* nextNode;

    // loop through linked list, freeing each node one by one
    while (currNode != NULL) {
        nextNode = currNode->next;
	free(currNode);
	currNode = nextNode;
    }

    // set head to null and return
    list->head = NULL;
    return;
}

// if an element appears in the list 
// retain the first occurance but
// remove all other nodes with the same 
// data value
void Remove_Duplicates(LINKED_LIST *list) {
    struct NODE* currNode = list->head;
    struct NODE* newList = NULL;
    struct NODE* tail = NULL;

    LINKED_LIST nlist;

    // copy original list to new list if the data has not already been copied over
    while (currNode != NULL) {
        if (newList == NULL) {
	    newList = (struct NODE*) malloc(sizeof(struct NODE));
	    newList->data = currNode->data;
	    newList->next = NULL;
	    tail = newList;
            nlist.head = newList;
	}
	else {
	    if (Count_If(nlist, currNode->data) == 0) {
	        tail->next = (struct NODE*) malloc(sizeof(struct NODE));
	        tail = tail->next;
	        tail->data = currNode->data;
	        tail->next = NULL;
	    }
	}
	currNode = currNode->next;
    }

    // set head of original list to new list
    list->head = newList;

    return;
}

// main modified to completely test implemented functions 
int main() {
    // create a linked list
    printf("creating linked list\n");
    LINKED_LIST list = Create_List();
   
    // add some data (hardcoded for testing)
    printf("hardcoding some data\n");
    NODE *first  = malloc(sizeof(NODE)); Verify_Malloc(first);
    NODE *second = malloc(sizeof(NODE)); Verify_Malloc(second);
    first->data  = 1;
    second->data = 2;
    list.head = first;
    first->next = second;
    second->next = NULL;
    // print the list
    printf("Testing Print_List\n");
    Print_List(stdout, list);
    printf("\n");
    
    // test for Size
    printf("Testing Size\n");
    printf("size = %d\n",Size(list));
    
    // test Size after adding an element
    NODE *third = malloc(sizeof(NODE)); Verify_Malloc(third);
    third->data = 3;
    second->next = third;
    third->next = NULL;

    printf("Size after adding an element: %d\n\n", Size(list));

    // test for Push_Front
    printf("Testing Push_Front\n");
    printf("Original list: ");
    Print_List(stdout, list);
    printf("List after pushing one element: ");
    Push_Front(&list, 0);
    Print_List(stdout, list);
    printf("List after pushing a second element: ");
    Push_Front(&list, 2);
    Print_List(stdout, list);
    printf("\n");
    
    // test for Push_Back
    printf("Testing Push_Back\n");
    printf("Original list: ");
    Print_List(stdout, list);
    printf("List after appending one element: ");
    Push_Back(&list, 3);
    Print_List(stdout, list);
    printf("List after appending a second element: ");
    Push_Back(&list, 4);
    Print_List(stdout, list);
    printf("\n");
    
    // test for Pop_Front
    printf("Testing Pop_Front\n");
    printf("Original list: ");
    Print_List(stdout, list);
    {
        int a = Size(list) + 1;
        for (int i = 0; i < a; i++) {
            int x; 
            int not_empty = Pop_Front(&list, &x);
            if (not_empty) {
                printf("Element popped was %d\nNew list: ",x);
                Print_List(stdout,list);
                printf("New list size = %d\n",Size(list));
            }
            else 
                printf("List was empty\n");
        }
    }
    printf("\n");

    // test for Pop_Back
    printf("Testing Pop_Back\n");
    {
        Push_Back(&list, 0);
        Push_Back(&list, 0);
        Push_Back(&list, 1);
        Push_Back(&list, 2);
        Push_Back(&list, 3);

        int x;
        int not_empty = Pop_Back(&list, &x);
        if (not_empty) {
            printf("Element popped was %d\nNew list: ",x);
            Print_List(stdout,list);
            printf("New list size = %d\n",Size(list));
        }
        else 
            printf("List was empty\n");
    }
    printf("\n");

    // test for Count_If
    printf("Testing Count_If\n");
    printf("Original list: ");
    Print_List(stdout, list);
    printf("0 count = %d\n",Count_If(list, 0));
    printf("New list: ");
    Print_List(stdout, list);
    printf("Count of element not in the list (5) = %d\n\n",Count_If(list, 5));
    
    // test for Delete 
    printf("Testing Delete\n");
    printf("Original list: ");
    Print_List(stdout, list);
    printf("List after deleting 1: ");
    Delete(&list, 1);
    Print_List(stdout, list);
    printf("List after deleting an element not in the list (3): ");
    Delete(&list, 3);
    Print_List(stdout, list);
    printf("\n");
    
    // test for Is_Empty
    printf("Testing Is_Empty\n");
    printf("List: ");
    Print_List(stdout, list);
    if (Is_Empty(list)) printf("List is Empty\n"); else printf("List is not empty\n");
    printf("\n");

    // test for Clear
    printf("Testing Clear\n");
    printf("List: ");
    Print_List(stdout, list);
    Clear(&list);
    printf("List after running Clear: ");
    Print_List(stdout, list);
    if (Is_Empty(list)) printf("List is Empty\n"); else printf("List is not empty\n");
    printf("\n");
 
    // test for Remove_Duplicates
    Push_Back(&list, 1);
    Push_Back(&list, 1);
    Push_Back(&list, 1);
    Push_Back(&list, 2);
    Push_Back(&list, 2);
    Push_Back(&list, 3);
    Push_Back(&list, 3);
    Push_Back(&list, 3);
    printf("List: ");
    Print_List(stdout, list);
    Remove_Duplicates(&list);
    printf("List after removing duplicates: ");
    Print_List(stdout, list);
    printf("Running Remove_Duplicates on unique list: ");
    Remove_Duplicates(&list);
    Print_List(stdout, list);
    return 0;
}
