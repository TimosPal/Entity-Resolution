typedef struct Tuple{
    void* value1;
    void* value2;
}Tuple;

void Tuple_Init(Tuple* tuple, void* value1, unsigned int size1, void* value2, unsigned int size2);
void Tuple_FreeValues(Tuple tuple, void (*free_func)(void*));
void Tuple_Free(void* tuple);