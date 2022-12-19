///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//QUEUE
#define MAX_SIZE 3
typedef struct NODE{
    float data;
    struct NODE* Next;
    struct NODE* Fore;
}node;

typedef struct QUEUE{
    node* front;
    node* rear;
    int count;
}queue;

int isEmpty(queue *myqueue)
{
    return myqueue->count == 0;   
}

void init_Queue(queue* myqueue){
    int temp = 0;
    while(temp < MAX_SIZE){
        en_de_Queue(myqueue, valid_range);
        temp++;
    }
}

void en_de_Queue(queue* myqueue, float info){
    node* newnode = (node*)malloc(sizeof(node));
    newnode->data = info;
    newnode->Next = NULL;
    newnode->Fore = NULL;

    if(isEmpty(myqueue) == 0){
        myqueue->front = newnode;
        myqueue->rear = newnode;
    }else{
        myqueue->rear->Next = newnode;
    }
    myqueue->rear = newnode;
    myqueue->count++;

    if(myqueue->count > MAX_SIZE){
        node* temp;
        temp = myqueue->front;
        myqueue->front = temp->Next;
        free(temp);
    }
}

bool check_Queue(queue* myqueue){           // 큐의 모든 측정값이 거리 내에 들어올경우에 참 반환
    node* ptr;
    ptr = myqueue->front;
    while (ptr != myqueue->rear->Next) //NULL 이 아닐 떄까지 반복
    {
        int count = 0;
        if(count < MAX_SIZE){
            return true;
        }else{
            if(ptr->data > valid_range){
                return false;
            }else{
                count++;
            }
        }
        ptr = ptr->Next;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
