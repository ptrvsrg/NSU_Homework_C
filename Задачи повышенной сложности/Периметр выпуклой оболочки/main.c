#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)(x)

static jmp_buf position;

enum
{
    SUCCESS_JUMP = 1,
    MAX_POINT_COUNT = 100000
};

typedef struct
{
    int X;
    int Y;
} TPoint;

float VectorLength(TPoint a, TPoint b)
{
    return (float)sqrt((a.X - b.X)*(a.X - b.X) + (a.Y - b.Y)*(a.Y - b.Y));
}

///////////////////////////////////  ADDITIONAL TYPE  ///////////////////////////////////

long double VectorMultiplication(TPoint a, TPoint b, TPoint c)
{
    long double dx1 = (long double)b.X - a.X;
    long double dx2 = (long double)c.X - a.X;
    long double dy1 = (long double)b.Y - a.Y;
    long double dy2 = (long double)c.Y - a.Y;

    return (dx1 * dy2) - (dy1 * dx2);
}

///////////////////////////////////  VECTOR TYPE  ///////////////////////////////////

typedef struct
{
    int Count;
    TPoint* Array;
} TVector;

TVector CreateVector(int count, TPoint* array)
{
    TVector vector = { count, array };
    return vector;
}

void DestroyVector(TVector* vector)
{
    free(vector->Array);
}

///////////////////////////////////  ERRORS  ///////////////////////////////////

void BadNumberOfPointsError(int pointCount)
{
    if(pointCount < 0 || pointCount > MAX_POINT_COUNT)
    {
        printf("bad number of points");
        longjmp(position, SUCCESS_JUMP);
    }
}

void BadNumberOfLinesError(void)
{
    printf("bad number of lines");
    longjmp(position, SUCCESS_JUMP);
}

///////////////////////////////////  INPUT  ///////////////////////////////////

int InputPointCount(void)
{
    int pointCount = 0;
    if (scanf("%d", &pointCount) != 1)
    {
        BadNumberOfLinesError();
    }
    BadNumberOfPointsError(pointCount);

    return pointCount;
}

TPoint* InputPointArray(int pointCount)
{
    TPoint* pointArray = calloc(pointCount, sizeof(*pointArray));
    assert(pointArray != NULL);

    for (int i = 0; i < pointCount; ++i)
    {
        if (scanf("%d%d", &pointArray[i].X, &pointArray[i].Y) != 2)
        {
            free(pointArray);
            BadNumberOfLinesError();
        }
    }

    return pointArray;
}

TVector InputVector(void)
{
    int count = InputPointCount();
    TPoint* array = InputPointArray(count);
    return CreateVector(count, array);
} 

///////////////////////////////////  STACK TYPE  ///////////////////////////////////

typedef struct
{
    int Count;
    int Max;
    TPoint* Array;
} TStack;

TStack CreateStack(int max)
{
    TStack stack = { 0, max, NULL };

    stack.Array = calloc(max, sizeof(*stack.Array));
    assert(stack.Array != NULL);

    return stack;
}

bool IsEmptyStack(TStack stack)
{
    return stack.Count == 0;
}

void PushStack(TPoint value, TStack* stack)
{
    assert(stack->Count != stack->Max);
    stack->Array[stack->Count] = value;
    ++stack->Count;
}

TPoint TopStack(TStack stack)
{
    assert(stack.Count > 0);
    return stack.Array[stack.Count - 1];
}

TPoint NextToTopSTack(TStack stack)
{
    assert(stack.Count > 1);
    return stack.Array[stack.Count - 2];
}

TPoint PopStack(TStack* stack)
{
    assert(stack->Count > 0);
    TPoint point = TopStack(*stack);
    --stack->Count;
    return point;
}

void DestroyStack(TStack* stack) 
{
    free(stack->Array);
    stack->Count = 0;
    stack->Max = 0;
}

void PrintStack(TStack stack)
{
    for (int i = 0; i < stack.Count; ++i)
    {
        printf("%d %d\n", stack.Array[i].X, stack.Array[i].Y);
    }
}

///////////////////////////////////  CREATE UPPER AND LOWER SET  ///////////////////////////////////

void FindMinMax(TVector vector, TPoint* min, TPoint* max)
{
    TPoint minimum = { INT_MAX, INT_MAX };
    TPoint maximum = { INT_MIN, INT_MIN };

    for (int i = 0; i < vector.Count; ++i)
    {
        if (vector.Array[i].X < minimum.X || (vector.Array[i].X == minimum.X && vector.Array[i].Y < minimum.Y))
        {
            minimum = vector.Array[i];
        }

        if (maximum.X < vector.Array[i].X || (vector.Array[i].X == maximum.X && maximum.Y < vector.Array[i].Y))
        {
            maximum = vector.Array[i];
        }  
    }

    *min = minimum;
    *max = maximum;
}

TVector CreateLowerOrUpperVector(TVector vector, TPoint min, TPoint max, bool isLower)
{
    TPoint* set = calloc(vector.Count, sizeof(*vector.Array));
    assert(set != NULL);

    int index = 0;
    for (int i = 0; i < vector.Count; ++i)
    {
        long double mult = VectorMultiplication(min, max, vector.Array[i]);
        if ((mult < 0 && isLower) || (mult > 0 && !isLower))
        {
            set[index] = vector.Array[i];
            ++index;
        }
    }

    set[index] = min;
    set[index + 1] = max;
    index += 2;
    
    return CreateVector(index, set);
}

///////////////////////////////////  COMPARISON FUNCTION FOR QUICK SORT  ///////////////////////////////////

int CompareLower(const void* a, const void* b)
{
    const TPoint* A = a;
    const TPoint* B = b;

    long long difference = (long long)A->X - B->X;
    if (difference == 0)
    {
        difference = (long long)A->Y - B->Y;
    }

    return (difference > 0) ? 1 : (difference == 0) ? 0 : -1;
}

int CompareUpper(const void* a, const void* b)
{
    const TPoint* A = a;
    const TPoint* B = b;

    long long difference = (long long)B->X - A->X;
    if (difference == 0)
    {
        difference = (long long)B->Y - A->Y;
    }

    return (difference > 0) ? 1 : (difference == 0) ? 0 : -1;
}

///////////////////////////////////  CHANGED GRAHAM ALGORITHM  ///////////////////////////////////

bool RotationCheck(TPoint a, TPoint b, TPoint c)
{
    return VectorMultiplication(a, b, c) > 0;
}

TStack GrahamAlgorithm(TVector vector, int (*compare)(const void*, const void*))
{
    TStack stack = CreateStack(vector.Count);

    if (vector.Count == 0)
    {
        return stack;
    }
    else if (vector.Count == 1)
    {
        PushStack(vector.Array[0], &stack);
        return stack;
    }
    
    qsort(vector.Array, vector.Count, sizeof(*vector.Array), compare);

    PushStack(vector.Array[0], &stack);
    PushStack(vector.Array[1], &stack);

    for (int i = 2; i < vector.Count; ++i)
    {
        while (stack.Count > 1 && !RotationCheck(TopStack(stack), vector.Array[i], NextToTopSTack(stack)))
        {
            PopStack(&stack);
        }
        
        PushStack(vector.Array[i], &stack);
    }
    
    return stack;
}

///////////////////////////////////  SEARCH CONVEX HULL AND CALC PERIMETER ///////////////////////////////////

float HullPerimeter(TStack stack)
{
    float perimeter = 0;
    TPoint point1 = PopStack(&stack);
    TPoint begin = point1;
    PrintStack(stack);

    while (!IsEmptyStack(stack))
    {
        TPoint point2 = PopStack(&stack);
        perimeter += VectorLength(point1, point2);
        point1 = point2;
    }

    perimeter += VectorLength(point1, begin);
    
    return perimeter;
}

float LowerOrUpperHullPerimeter(TVector vector, TPoint min, TPoint max, bool isLower)
{
    TVector newVector = CreateLowerOrUpperVector(vector, min, max, isLower);
    TStack newStack = GrahamAlgorithm(newVector, (isLower) ? CompareLower : CompareUpper);
    DestroyVector(&newVector);

    float perimeter = HullPerimeter(newStack);
    DestroyStack(&newStack);

    return perimeter;
}

float ConvexHullPerimeter(TVector vector)
{
    if (vector.Count == 0 || vector.Count == 1)
    {
        return 0;
    }

    TPoint min = { INT_MAX, INT_MAX };
    TPoint max = { INT_MIN, INT_MIN };

    FindMinMax(vector, &min, &max);
    
    float perimeter = LowerOrUpperHullPerimeter(vector, min, max, true);
    perimeter += LowerOrUpperHullPerimeter(vector, min, max, false);

    perimeter -= 2 * VectorLength(min, max);

    return perimeter;
}

///////////////////////////////////  MAIN  ///////////////////////////////////

int main(void)
{
    FILE* in = freopen("in.txt", "r", stdin);
    UNUSED(in);
    assert(in != NULL);
    FILE* out = freopen("out.txt", "w", stdout);
    UNUSED(out);
    assert(out != NULL);

    if (setjmp(position) == 0)
    {
        TVector vector = InputVector();
        printf("Convex hull perimeter is %f\n", ConvexHullPerimeter(vector));
        DestroyVector(&vector);
    }

    fclose(stdin);
    fclose(stdout);

    return EXIT_SUCCESS;
}