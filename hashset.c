#include "hashset.h"  /* the .h file does NOT reside in /usr/local/include/ADTs */
#include <stdlib.h>
#include <string.h>

/* any other includes needed by your code */
#define UNUSED __attribute__((unused))

//hashset is used to store unique values with no duplicates, in no particular order
#define MAX_CAPACITY 134217728L
#define TRIGGER 100

typedef struct node{
	struct node *next;
	void *value;
} Node;

typedef struct s_data {
    /* definitions of the data members of self */
	Node **buckets; //for hash table
	long capacity;
	double loadFactor;
	long size;
	long changes;
	double increment;
	double load;
	int (*cmp)(void *, void*);
	long (*hash)(void *, long N);
	void (*freeV)(void *v);
} SData;


/*
 * important - remove UNUSED attributed in signatures when you flesh out the
 * methods
 */

static void purge(SData *sd, void (*freeV)(void *v)){
	long i;
	//traverse set, calling freeV on each entry
	for(i = 0L; i < sd->capacity; i++){
		Node *p, *q;
		p = sd->buckets[i]; //p is now a node
		while(p != NULL){
			if(freeV != NULL){
				(*freeV)(p->value);
			}
			q = p->next;
			free(p);
			p = q;
			}
		sd->buckets[i] = NULL;
		}	
	}


static void s_destroy(const Set *s) {
    /* implement the destroy() method */
	SData *sd = (SData *)s->self;
	purge(sd, sd->freeV);
	free(sd->buckets); //free bucket array here 
	free(sd);
	free((void *)s);
}

static void s_clear(const Set *s) {
    /* implement the clear() method */
    SData *sd = (SData *)s->self;
    purge(sd, sd->freeV);
    sd->size = 0; //set everything to zero
    sd->load = 0.0;
    sd->changes = 0;
}

static Node *findValue(SData *sd, void *value, long *bucket){
	//finding value in bucket array
	long i = sd->hash(value, sd->capacity);
	Node *p;

	*bucket = i;
	for(p = sd->buckets[i]; p != NULL; p = p->next){
		if(sd->cmp(p->value, value) == 0){
			//returns an integer
			break;
		}
	}
	return p;
}


static void resize(SData *sd){
	int N;
	Node *p, *q, **array;
	long i, j;

	N = 2*sd->capacity; //making set bigger
	if(N > MAX_CAPACITY)
		N = MAX_CAPACITY;
	if(N == sd->capacity)
		return;
	array = (Node **)malloc(N * sizeof(Node *));
	if(array == NULL)
		return;
	for(j = 0; j < N; j++)
		array[j] = NULL;

	//redistribute entries into new set of buckets
	for(i = 0; i < sd->capacity; i++){
		for(p = sd->buckets[i]; p != NULL; p = q){
			q = p->next;
			j = sd->hash(p->value,N);
			p->next = array[j];
			array[j] = p;
		}
	}
	free(sd->buckets); //free old bucket
	sd->buckets = array; //now set equal to new array
	sd->capacity = N;
	sd->load /= 2.0;
	sd->changes = 0;
	sd->increment = 1.0 / (double)N;
}




static int insertEntry(SData *sd, void *value, long i){
	//insert entry into index i in bucket array
	Node *p = (Node *)malloc(sizeof(Node));
	int ans = 0;

	if(p != NULL){
		p->value = value;
		p->next = sd->buckets[i];
		sd->buckets[i] = p;
		sd->size++;
		sd->load += sd->increment;
		sd->changes++;
		ans = 1;
	}
	return ans;
}


static int s_add(const Set *s, void *member) {
    /* implement the add() method */
	
	SData *sd = (SData *)s->self;
    Node *p;
    long i; //bucket 
    int ans = 0;
    if(sd->changes > TRIGGER){
	sd->changes = 0;
	if(sd->load > sd->loadFactor)
		resize(sd);
    }
    p = findValue(sd, member, &i);
    if(p == NULL){
	ans = insertEntry(sd, member, i);
    }
	return ans;
}

static int s_contains(const Set *s, void *member) {
    /* implement the contains() method */
	SData *sd = (SData *)s->self;
	long bucket;

	//findValue function returns Node *
	return(findValue(sd, member, &bucket) != NULL);
    }

static int s_isEmpty(const Set *s) {
    /* implement the isEmpty() method */
	SData *sd = (SData *)s->self;
	return (sd->size == 0L);
}

static int s_remove(const Set *s, void *member) {
    /* implement the remove() method */
    SData *sd = (SData *)s->self;
    long i;
    Node *entry;
    int status = 0;

    entry = findValue(sd, member, &i);
    if(entry != NULL){
	Node *p, *c;
	for(p = NULL, c = sd->buckets[i]; c != entry; p = c, c = c->next)
		;
	if(p == NULL)
		sd->buckets[i] = entry->next;
	else
		p->next = entry->next;
	sd->size--;
	sd->load -= sd->increment;
	sd->changes++;
	if(sd->freeV != NULL)
		sd->freeV((entry->value));
	free(entry);
	status = 1;
    }
    return status;
}

static long s_size(const Set *s) {
    /* implement the size() method */
    SData *sd = (SData *)s->self;
    return sd->size;
}

static void **values(SData *sd){
	void **temp = NULL;
	if(sd->size > 0L){
		size_t nbytes = sd->size * sizeof(void *);
		temp = (void **)malloc(nbytes);
		if(temp != NULL){
			long i, n = 0L;
			for(i = 0L; i < sd->capacity; i++){
				Node *p = sd->buckets[i];
				while(p != NULL){
					temp[n++] = p->value;
					p = p->next;
				}
			}
		}
	}
	return temp;
}

static void **s_toArray(const Set *s, long *len) {
    /* implement the toArray() method */
	SData *sd = (SData *)s->self;
	void **temp = values(sd);

	if(temp != NULL)
		*len = sd->size;
	return temp;
  
}

static Node **nodes(SData *sd){
	Node **temp = NULL;
	if(sd->size > 0L){
		size_t nbytes = sd->size * sizeof(Node *);
		temp = (Node **)malloc(nbytes);
		if(temp != NULL){
			long i, n = 0L;
			for(i = 0L; i < sd->capacity; i++){
				Node *p = sd->buckets[i];
				while(p != NULL){
					temp[n++] = (p->value); //add node values to temp array
					p = p->next;
				}
			}
		}
	}
	return temp; //return new array of node *
}

static const Iterator *s_itCreate(const Set *s) {
    /* implement the itCreate() method */
    SData *sd = (SData *)s->self;
    const Iterator *iterator = NULL;
    void **temp = (void **)nodes(sd);

    if(temp != NULL){
	    iterator = Iterator_create(sd->size, temp);
	    if(iterator == NULL)
		    free(temp);
    }
	return iterator;	

}

static UNUSED Set template = {
    NULL, s_destroy, s_clear, s_add, s_contains, s_isEmpty, s_remove,
    s_size, s_toArray, s_itCreate
};

const Set *HashSet(void (*freeValue)(void*), int (*cmpFxn)(void*, void*),
                   long capacity, double loadFactor,
                   long (*hashFxn)(void *m, long N)
                  ) {
    /* construct a Set instance and return to the caller */
	long N;
	double lf;
	Node **array;
	long i;

	Set *s = (Set *)malloc(sizeof(Set));
	if(s != NULL){
		SData *sd = (SData *)malloc(sizeof(SData));
		if(sd != NULL){
			N = ((capacity > 0) ? capacity: DEFAULT_SET_CAPACITY);
			if(N > MAX_CAPACITY)
				N = MAX_CAPACITY;
			
			lf = ((loadFactor > 0.000001) ? loadFactor : DEFAULT_LOAD_FACTOR);
			array = (Node **)malloc(N * sizeof(Node *));
			if(array != NULL){
				sd->capacity = N;
				sd->loadFactor = lf;
				sd->size = 0L;
				sd->load = 0.0;
				sd->changes = 0L;
				sd->increment = 1.0/ (double)N;

				sd->hash = hashFxn;
				sd->cmp = cmpFxn;
				sd->freeV = freeValue;
				sd->buckets = array;
				for(i = 0; i < N; i++)
					array[i] = NULL;
				*s = template;
				s->self = sd;
				} else {
					free(sd);
					free(s);
					s = NULL;
				} 
		} else {
				free(s);
				s = NULL;
				}
		}

    return s;
}

