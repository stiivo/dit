
#include "Prototypes.h"

/*{

#ifndef DEFAULT_SIZE
#define DEFAULT_SIZE -1
#endif

typedef void(*Vector_procedure)(void*);
typedef int(*Vector_booleanFunction)(const Object*,const Object*);

struct Vector_ {
   ObjectClass* type;
   Object **array;
   int items;
   int arraySize;
   int growthRate;
   Vector_booleanFunction compareFunction;
   bool owner;
};

}*/

Vector* Vector_new(ObjectClass* type, bool owner, int size) {
   if (size == DEFAULT_SIZE)
      size = 10;
   Vector* this = (Vector*) malloc(sizeof(Vector));

   this->growthRate = size;
   this->array = (Object**) calloc(size, sizeof(Object*));
   this->arraySize = size;
   this->compareFunction = Vector_compareFunction;
   this->items = 0;
   this->type = type;
   this->owner = owner;
   return this;
}

void Vector_delete(Vector* this) {
   Vector_prune(this);
   free(this->array);
   free(this);
}

static inline bool Vector_isConsistent(Vector* this) {
   if (this->owner) {
      for (int i = 0; i < this->items; i++)
         if ( ! Call(Object, instanceOf, this->array[i], this->type) )
            return false;
      return true;
   } else {
      return true;
   }
}

void Vector_prune(Vector* this) {
   assert(Vector_isConsistent(this));
   int i;

   for (i = 0; i < this->items; i++)
      if (this->array[i]) {
         if (this->owner)
            Msg0(Object, delete, this->array[i]);
         this->array[i] = NULL;
      }
   this->items = 0;
}

int Vector_compareFunction(const Object* v1, const Object* v2) {
   return !(Msg(Object, equals, v1, v2));
}

void Vector_setCompareFunction(Vector* this, Vector_booleanFunction f) {
   this->compareFunction = f;
}

void Vector_sort(Vector* this) {
   assert(Vector_isConsistent(this));
   int i, j;
   /* Insertion sort -- behaves best when array is mostly sorted */
   for (i = 1; i < this->items; i++) {
      void* t = this->array[i];
      for (j = i-1; j >= 0 && this->compareFunction(this->array[j], t) < 0; j--)
         this->array[j+1] = this->array[j];
      this->array[j+1] = t;
   }
   assert(Vector_isConsistent(this));

   /* Bubble sort -- "reference" implementation */
   /*
   for (int i = 0; i < this->items; i++) {
      for (int j = i+1; j < this->items; j++) {
         if (this->compareFunction(this->array[i], this->array[j]) < 0) {
            void* tmp = this->array[i];
            this->array[i] = this->array[j];
            this->array[j] = tmp;
         }
      }
   }
   */
}

static inline void Vector_checkArraySize(Vector* this) {
   assert(Vector_isConsistent(this));
   if (this->items >= this->arraySize) {
      int i;
      i = this->arraySize;
      this->arraySize = this->items + this->growthRate;
      this->array = (Object**) realloc(this->array, sizeof(Object*) * this->arraySize);
      for (; i < this->arraySize; i++)
         this->array[i] = NULL;
   }
   assert(Vector_isConsistent(this));
}

void Vector_insert(Vector* this, int index, void* data_) {
   assert(index >= 0);
   assert( Call(Object, instanceOf, data_, this->type) );
   Object* data = data_;
   assert(Vector_isConsistent(this));
   
   Vector_checkArraySize(this);
   assert(this->array[this->items] == NULL);
   for (int i = this->items; i >= index; i--) {
      this->array[i+1] = this->array[i];
   }
   this->array[index] = data;
   this->items++;
   assert(Vector_isConsistent(this));
}

Object* Vector_take(Vector* this, int index) {
   assert(index >= 0 && index < this->items);
   assert(Vector_isConsistent(this));
   Object* removed = this->array[index];
   assert (removed != NULL);
   this->items--;
   for (int i = index; i < this->items; i++)
      this->array[i] = this->array[i+1];
   this->array[this->items] = NULL;
   assert(Vector_isConsistent(this));
   return removed;
}

Object* Vector_remove(Vector* this, int index) {
   Object* removed = Vector_take(this, index);
   if (this->owner) {
      Msg0(Object, delete, removed);
      return NULL;
   } else
      return removed;
}

void Vector_moveUp(Vector* this, int index) {
   assert(index >= 0 && index < this->items);
   assert(Vector_isConsistent(this));
   if (index == 0)
      return;
   Object* temp = this->array[index];
   this->array[index] = this->array[index - 1];
   this->array[index - 1] = temp;
}

void Vector_moveDown(Vector* this, int index) {
   assert(index >= 0 && index < this->items);
   assert(Vector_isConsistent(this));
   if (index == this->items - 1)
      return;
   Object* temp = this->array[index];
   this->array[index] = this->array[index + 1];
   this->array[index + 1] = temp;
}

void Vector_set(Vector* this, int index, void* data_) {
   assert(index >= 0);
   assert( Call(Object, instanceOf, data_, this->type) );
   Object* data = data_;
   assert(Vector_isConsistent(this));

   Vector_checkArraySize(this);
   if (index >= this->items) {
      this->items = index+1;
   } else {
      if (this->owner) {
         Object* removed = this->array[index];
         assert (removed != NULL);
         if (this->owner)
            Msg0(Object, delete, removed);
      }
   }
   this->array[index] = data;
   assert(Vector_isConsistent(this));
}

inline Object* Vector_get(Vector* this, int index) {
   assert(index < this->items);
   assert(Vector_isConsistent(this));
   return this->array[index];
}

inline int Vector_size(Vector* this) {
   assert(Vector_isConsistent(this));
   return this->items;
}

void Vector_merge(Vector* this, Vector* v2) {
   int i;
   assert(Vector_isConsistent(this));
   
   for (i = 0; i < v2->items; i++)
      Vector_add(this, v2->array[i]);
   v2->items = 0;
   Vector_delete(v2);
   assert(Vector_isConsistent(this));
}

void Vector_add(Vector* this, void* data_) {
   assert( Call(Object, instanceOf, data_, this->type) );
   Object* data = data_;
   assert(Vector_isConsistent(this));

   Vector_set(this, this->items, data);
   assert(Vector_isConsistent(this));
}

int Vector_indexOf(Vector* this, void* search_) {
   assert( Call(Object, instanceOf, search_, this->type) );
   Object* search = search_;
   assert(Vector_isConsistent(this));

   int i;

   for (i = 0; i < this->items; i++) {
      Object* o = (Object*)this->array[i];
      if (o && Msg(Object, equals, o, search))
         return i;
   }
   return -1;
}

void Vector_foreach(Vector* this, Vector_procedure f) {
   int i;
   assert(Vector_isConsistent(this));

   for (i = 0; i < this->items; i++)
      f(this->array[i]);
   assert(Vector_isConsistent(this));
}
