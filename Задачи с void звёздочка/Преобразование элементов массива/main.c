#include <stdio.h>
#include <stdlib.h>

void AddOne(void* x)
{
    int* y = x;
    ++(*y);
}

void Transform(void* array, size_t size, size_t count, void (*f)(void*))
{
    char* newArray = array;

    for(size_t i = 0; i < count; ++i)
    {
        f(&newArray[i * size]);
    }
}

int main(void)
{
    int array[] = {
        0, 1, 2, 3, 4
    };

    Transform(array, sizeof(*array), 5, AddOne);

    for (size_t i = 0; i < sizeof(array) / sizeof(*array); i++)
    {
        printf("%d ", array[i]);
    }
    

    return EXIT_SUCCESS;
}