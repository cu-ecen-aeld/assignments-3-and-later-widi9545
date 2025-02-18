//https://www.geeksforgeeks.org/stack-using-linked-list-in-c/



struct t_data{
    int connection;
    int completion;
    pthread_t thread_id; 
 };



typedef struct Node {
    pthread_t data;
    struct Node* next;
} node;


node* createNode(pthread_t data)
{
    node* newNode = (node*)malloc(sizeof(node));


    if (newNode == NULL)
        return NULL;

    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

int insertBeforeHead(node** head, pthread_t data)
{
    // creating new node
    node* newNode = createNode(data);

    if (!newNode)
        return -1;

    if (*head == NULL) {
        *head = newNode;
        return 0;
    }

    newNode->next = *head;
    *head = newNode;
    return 0;
}

int deleteHead(node** head)
{

    node* temp = *head;
    *head = (*head)->next;
    free(temp);
    return 0;
}


int isEmpty(node** stack) { return *stack == NULL; }

void push(node** stack, pthread_t data)
{

    if (insertBeforeHead(stack, data)) {
        printf("Stack Overflow!\n");
    }
}


int pop(node** stack)
{
    if (isEmpty(stack)) {
        printf("Stack Underflow\n");
        return -1;
    }


    deleteHead(stack);
}

int peek(node** stack)
{
    if (!isEmpty(stack))
        return (*stack)->data;
    else
        return -1;
}

void printStack(node** stack)
{
    node* temp = *stack;
    while (temp != NULL) {
        printf("%lu-> ", temp->data);
        temp = temp->next;
    }
    printf("\n");
}
