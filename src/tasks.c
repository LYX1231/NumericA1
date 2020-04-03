/***************************************************************************
 *
 *   File        : tasks.c
 *   Student Id  : <1010124>
 *   Name        : <Yuxuan Liang>
 *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include "tasks.h"
#include <assert.h>

#define INITIAL_SIZE 5000
#define MAX_BUF_LEN 2048
#define DATA_COLUMNS_OF_RHO_U_V_X_Y 5
#define TASK1_AFTER_X 20
#define TASK1_LINES_TO_PRINT 4
#define TASK2_X_LEFT_BOUNDARY -15
#define TASK2_X_RIGHT_BOUNDARY 85
#define TASK2_Y_UP_BOUNDARY 20
#define TASK2_Y_DOWN_BOUNDARY -20
#define TASK4_FIRST_THRESHOLD 5
#define TASK4_LAST_THRESHOLD 25
#define TASK4_THRESHOLD_INTERVAL 5
#define BST_SUCCESS 1
#define BST_FAILURE 0

FILE * safe_fopen(const char* path, const char* mode)
{
	FILE* fp = fopen(path, mode);
	if (fp == NULL) {
		perror("file open error.");
		exit(EXIT_FAILURE);
	}
	return fp;
}

void* safe_malloc(size_t num_bytes)
{
	void* ptr = malloc(num_bytes);
	if (ptr == NULL) {
		printf("ERROR: malloc(%lu)\n", num_bytes);
		exit(EXIT_FAILURE);
	}
	return ptr;
}

void* safe_realloc(void* ptr, size_t num_bytes)
{
	void* new_ptr = realloc(ptr, num_bytes);
	if (new_ptr == NULL) {
		printf("ERROR: realloc(%lu)\n", num_bytes);
		exit(EXIT_FAILURE);
	}
	return new_ptr;
}

typedef struct {
	double rho;
	double u;
	double v;
	double x;
	double y;
} data_t;

typedef struct {
	double avg_rho;
	double avg_u;
	double avg_v;
	double avg_x;
	double avg_y;
	double score;
} avg_t;

// copy and modify the functions of linked-list, bst, binary search and merge sort from LMS
typedef struct list_node list_node_t;

struct list_node
{
	double data;
	list_node_t* next;
};

typedef struct
{
	int num_elements;
	list_node_t* head;
	list_node_t* tail;
	void (*del)(void*);
} list_t;

typedef struct bst_node bst_node_t;

struct bst_node
{
	double data;
	bst_node_t* left;
	bst_node_t* right;
};

typedef struct {
	int num_elements;
	bst_node_t* root;
	void (*del)(double);
	int (*cmp)(const double, const double);
} bst_t;

list_t* list_new(void (*delfunc)(void*))
{
	list_t* list;
	list = (list_t*)safe_malloc(sizeof(list_t));
	list->head = NULL;
	list->tail = NULL;
	list->num_elements = 0;
	list->del = delfunc;
	return list;
}

void list_search(list_t* list, double d)
{
	FILE* fp = safe_fopen("task3.csv", "a");
	list_node_t* tmp = list->head;
	while (tmp && tmp->data != d) {
		fprintf(fp, "%.6f,", tmp->data);
		tmp = tmp->next;
	}
	fprintf(fp, "%.6f\n", tmp->data);
	fclose(fp);
}

double list_pop_front(list_t* list)
{
	assert(list != NULL);
	assert(list->num_elements > 0);
	list_node_t* old;
	assert(list->head != NULL);
	old = list->head;
	list->head = list->head->next;
	double d = old->data;
	free(old);
	list->num_elements--;
	if (list->num_elements == 0) {
		list->head = NULL;
		list->tail = NULL;
	}
	return d;
}

void list_push_back(list_t* list, double d)
{
	assert(list != NULL);
	list_node_t* new = (list_node_t*)safe_malloc(sizeof(list_node_t));
	new->data = d;
	new->next = NULL;
	if (list->tail)
		list->tail->next = new;
	list->tail = new;
	if (list->head == NULL)
		list->head = new;
	list->num_elements++;
}

void list_free(list_t* list)
{
	assert(list != NULL);
	while (list->num_elements) {
		list_pop_front(list);
	}
	free(list);
}

bst_t* bst_new(void (*delfunc)(double), int (*cmpfunc)(const double, const double))
{
	bst_t* bst;
	bst = (bst_t*)safe_malloc(sizeof(bst_t));
	bst->root = NULL;
	bst->num_elements = 0;
	bst->del = delfunc;
	bst->cmp = cmpfunc;
	return bst;
}

/* free all memory assocated with a subtree */
void bst_free_subtree(bst_t* bst, bst_node_t* n)
{
	assert(bst != NULL);
	if (n) {
		bst_free_subtree(bst, n->left);
		bst_free_subtree(bst, n->right);
		bst->del(n->data);
		free(n);
		bst->num_elements--;
	}
}

/* free all memory associated with a bst */
void bst_free(bst_t* bst)
{
	assert(bst != NULL);
	bst_free_subtree(bst, bst->root);
	free(bst);
}

/* insert a new element into the bst */
int bst_insert(bst_t* bst, double d)
{
	assert(bst != NULL);
	bst_node_t* parent = NULL;
	bst_node_t* tmp = bst->root;
	while (tmp) {
		parent = tmp;
		if (bst->cmp(tmp->data, d) > 0) { // element is smaller
			tmp = tmp->left;
		}
		else if (bst->cmp(tmp->data, d) < 0) { // element is bigger
			tmp = tmp->right;
		}
		else {
			/* ALREADY EXISTS! -> do nothing and return ERROR */
			return BST_FAILURE;
		}
	}

	/* insert as child of parent */
	bst_node_t* new_node = (bst_node_t*)safe_malloc(sizeof(bst_node_t));
	new_node->data = d;
	new_node->left = NULL;
	new_node->right = NULL;
	if (parent != NULL) {
		if (bst->cmp(parent->data, d) > 0) { // element is smaller
			assert(parent->left == NULL);
			parent->left = new_node;
		}
		else {
			assert(parent->right == NULL);
			parent->right = new_node;
		}
	}
	else {
		assert(bst->root == NULL);
		bst->root = new_node;
	}
	bst->num_elements++;

	return BST_SUCCESS;
}

bst_node_t* bst_find(bst_t* bst, double d) {
	assert(bst != NULL);
	bst_node_t* tmp = bst->root;
	FILE* fp = safe_fopen("task3.csv", "a");
	while (tmp) {
		if (bst->cmp(tmp->data, d) > 0) { // element is smaller
			fprintf(fp, "%.6f,", tmp->data);
			tmp = tmp->left;
		}
		else if (bst->cmp(tmp->data, d) < 0) { // element is bigger
			fprintf(fp, "%.6f,", tmp->data);
			tmp = tmp->right;
		}
		else {
			fprintf(fp, "%.6f\n", tmp->data);
			break;
		}
	}
	fclose(fp);
	return tmp;
}

void perfect_insert(bst_t* bst, double* array, int low, int high)
{
	if (low <= high) {
		int mid = low + (high - low) / 2;
		double* ptr = array + mid;
		bst_insert(bst, *ptr);
		perfect_insert(bst, array, low, mid - 1);
		perfect_insert(bst, array, mid + 1, high);
	}
}

void no_free(double v)
{
}

int make_unique(double* array, int n)
{
	int dest = 0;
	int itr = 1;
	while (itr != n) {
		if (array[dest] != array[itr]) {
			array[++dest] = array[itr];
		}
		itr++;
	}
	return dest + 1;
}

int binary_search(double* A, int lo, int hi, double x)
{
	FILE* fp = safe_fopen("task3.csv", "a");
	while (lo <= hi)
	{
		int m = (lo + hi) / 2;
		if (x < A[m]) {
			fprintf(fp, "%.6f,", A[m]);
			hi = m - 1;
		}
		else if (x > A[m]) {
			fprintf(fp, "%.6f,", A[m]);
			lo = m + 1;
		}
		else {
			fprintf(fp, "%.6f\n", A[m]);
			fclose(fp);
			return m;
		}
	}
	return -1;
}

// the merge sort on LMS, I use it to sort structure arrays
void struct_merge(void** array, int n, int mid, int cmp(const void*, const void*))
{
	// need extra space for merging
	void** tmp = safe_malloc(n * sizeof(void*));
	void** left = array;
	void** right = array + mid;
	int i = 0;
	int j = 0;
	int left_size = mid;
	int right_size = n - mid;
	// perform the merge
	for (int k = 0; k < n; k++) {
		if (j == right_size)
			tmp[k] = left[i++];
		else if (i == left_size)
			tmp[k] = right[j++];
		else if (cmp(left[i], right[j]) < 1)
			tmp[k] = left[i++];
		else
			tmp[k] = right[j++];
	}
	// copy the merged array
	for (int i = 0; i < n; i++) {
		array[i] = tmp[i];
	}
	// clean up
	free(tmp);
}

void struct_merge_sort(void** array, int n, int cmp(const void*, const void*))
{
	if (n > 1) {
		int mid = n / 2;
		struct_merge_sort(array, mid, cmp);
		struct_merge_sort(array + mid, n - mid, cmp);
		struct_merge(array, n, mid, cmp);
	}
}

// I modified the merge sort function to another one whose first argument is an array instead of a pointer to the array, so that I don't have to allocate extra memory for a pointer to every simple array
void merge(double* array, int n, int mid, int cmp(const void*, const void*))
{
	// need extra space for merging
	double* tmp = safe_malloc(n * sizeof(double));
	double* left = array;
	double* right = array + mid;
	int i = 0;
	int j = 0;
	int left_size = mid;
	int right_size = n - mid;
	// perform the merge
	for (int k = 0; k < n; k++) {
		if (j == right_size)
			tmp[k] = left[i++];
		else if (i == left_size)
			tmp[k] = right[j++];
		else if (cmp(&left[i], &right[j]) < 1)
			tmp[k] = left[i++];
		else
			tmp[k] = right[j++];
	}
	// copy the merged array
	for (int i = 0; i < n; i++) {
		array[i] = tmp[i];
	}
	// clean up
	free(tmp);
}

void merge_sort(double* array, int n, int cmp(const void*, const void*))
{
	if (n > 1) {
		int mid = n / 2;
		merge_sort(array, mid, cmp);
		merge_sort(array + mid, n - mid, cmp);
		merge(array, n, mid, cmp);
	}
}

int x_cmp(const void* a, const void* b)
{
	data_t* aa = (data_t*)a;
	data_t* bb = (data_t*)b;
	return aa->x > bb->x ? 1 : -1;
}

int score_cmp(const void* a, const void* b)
{
	avg_t* aa = (avg_t*)a;
	avg_t* bb = (avg_t*)b;
	return aa->score < bb->score ? 1 : -1;
}

int double_array_cmp(const void* a, const void* b)
{
	double aa = *(double*)a;
	double bb = *(double*)b;
	return aa > bb ? 1 : -1;
}

int double_cmp(const double a, const double b)
{
	if (a == b)
	{
		return 0;
	}
	return a > b ? 1 : -1;
}

void maxfluxdiff(const char* flow_file)
{
	int size = INITIAL_SIZE, data_read = 0;
	data_t* data = (data_t*)safe_malloc(size * sizeof(data_t));
	FILE* fp = safe_fopen(flow_file, "r");
	char* buf = (char*)safe_malloc(MAX_BUF_LEN * sizeof(char));
	fgets(buf, MAX_BUF_LEN, fp); // skip the first line of the file
	free(buf);
	while (fscanf(fp, "%lf,%lf,%lf,%lf,%lf\n", &data[data_read].rho, &data[data_read].u, &data[data_read].v, &data[data_read].x, &data[data_read].y) == DATA_COLUMNS_OF_RHO_U_V_X_Y)
	{
		data_read++;
		// when the lines reach the maximum size of the matrix, double the size
		if (size == data_read)
		{
			size = 2 * size;
			data = (data_t*)safe_realloc(data, size * sizeof(data_t));
		}
	}
	fclose(fp);
	size = data_read; // record number of rows of the data (except the title)
	data = (data_t*)safe_realloc(data, size * sizeof(data_t)); // realloc the memory to the final size (free the extra memory)

	double rhoumax, rhoumin, rhovmax, rhovmin;
	int index_of_rhoumax, index_of_rhoumin, index_of_rhovmax, index_of_rhovmin;

	for (int i = 0; i < size; i++)
	{
		if (data[i].x > TASK1_AFTER_X)
		{
			rhoumax = data[i].rho * data[i].u;
			rhoumin = data[i].rho * data[i].u;
			rhovmax = data[i].rho * data[i].v;
			rhovmin = data[i].rho * data[i].v;
			index_of_rhoumax = i;
			index_of_rhoumin = i;
			index_of_rhovmax = i;
			index_of_rhovmin = i;
			break;
		}
	}

	for (int i = 0; i < size; i++)
	{
		if (data[i].x > TASK1_AFTER_X)
		{
			if (data[i].rho * data[i].u > rhoumax) { rhoumax = data[i].rho * data[i].u; index_of_rhoumax = i; }
			if (data[i].rho * data[i].u < rhoumin) { rhoumin = data[i].rho * data[i].u; index_of_rhoumin = i; }
			if (data[i].rho * data[i].v > rhovmax) { rhovmax = data[i].rho * data[i].v; index_of_rhovmax = i; }
			if (data[i].rho * data[i].v < rhovmin) { rhovmin = data[i].rho * data[i].v; index_of_rhovmin = i; }
		}
	}
	fp = safe_fopen("task1.csv", "a");
	fprintf(fp, "rho,u,v,x,y\n");
	fprintf(fp, "%.6f,%.6f,%.6f,%.6f,%.6f\n", data[index_of_rhoumax].rho, data[index_of_rhoumax].u, data[index_of_rhoumax].v, data[index_of_rhoumax].x, data[index_of_rhoumax].y);
	fprintf(fp, "%.6f,%.6f,%.6f,%.6f,%.6f\n", data[index_of_rhoumin].rho, data[index_of_rhoumin].u, data[index_of_rhoumin].v, data[index_of_rhoumin].x, data[index_of_rhoumin].y);
	fprintf(fp, "%.6f,%.6f,%.6f,%.6f,%.6f\n", data[index_of_rhovmax].rho, data[index_of_rhovmax].u, data[index_of_rhovmax].v, data[index_of_rhovmax].x, data[index_of_rhovmax].y);
	fprintf(fp, "%.6f,%.6f,%.6f,%.6f,%.6f\n", data[index_of_rhovmin].rho, data[index_of_rhovmin].u, data[index_of_rhovmin].v, data[index_of_rhovmin].x, data[index_of_rhovmin].y);
	fclose(fp);

	free(data);
}

void coarsegrid(const char* flow_file, int resolution)
{
	int size = INITIAL_SIZE, data_read = 0;
	data_t** data = (data_t**)safe_malloc(size * sizeof(data_t*));
	for (int i = 0; i < size; i++)
	{
		data[i] = (data_t*)safe_malloc(sizeof(data_t));
		memset(data[i], 0, sizeof(data_t));
	}

	FILE* fp = safe_fopen(flow_file, "r");
	char* buf = (char*)safe_malloc(MAX_BUF_LEN * sizeof(char));
	fgets(buf, MAX_BUF_LEN, fp); // skip the first line of the file
	free(buf);

	while (fscanf(fp, "%lf,%lf,%lf,%lf,%lf\n", &data[data_read]->rho, &data[data_read]->u, &data[data_read]->v, &data[data_read]->x, &data[data_read]->y) == DATA_COLUMNS_OF_RHO_U_V_X_Y)
	{
		data_read++;
		// when the lines reach the maximum size of the matrix, double the size
		if (size == data_read)
		{
			data = (data_t**)safe_realloc(data, 2 * size * sizeof(data_t*));
			for (int i = size; i < 2 * size; i++)
			{
				data[i] = (data_t*)safe_malloc(sizeof(data_t));
				memset(data[i], 0, sizeof(data_t));
			}
			size = 2 * size;
		}
	}
	fclose(fp);

	for (int i = data_read; i < size; i++)
	{
		free(data[i]);
	}
	size = data_read; // record number of rows of the data (except the title)
	data = (data_t**)safe_realloc(data, size * sizeof(data_t*)); //realloc the memory to the final size (free the extra memory)

	struct_merge_sort((void**)data, size, x_cmp); // use merge sort as a stable sorting method to sort the data in the ascending order of x

	int num_of_grid = pow(resolution, 2);

	avg_t** avg = (avg_t**)safe_malloc(num_of_grid * sizeof(avg_t*));
	for (int i = 0; i < num_of_grid; i++)
	{
		avg[i] = (avg_t*)safe_malloc(sizeof(avg_t));
		memset(avg[i], 0, sizeof(avg_t));
	}
	int x_interval = (TASK2_X_RIGHT_BOUNDARY - TASK2_X_LEFT_BOUNDARY) / resolution;
	int	y_interval = (TASK2_Y_UP_BOUNDARY - TASK2_Y_DOWN_BOUNDARY) / resolution;
	int points_in_grid = 0;
	int avg_index = 0;
	int i = 0, x_left_boundary_index = 0;

	for (int x_left_boundary = TASK2_X_LEFT_BOUNDARY, x_right_boundary = TASK2_X_LEFT_BOUNDARY + x_interval; x_right_boundary <= TASK2_X_RIGHT_BOUNDARY; x_left_boundary += x_interval, x_right_boundary += x_interval)
	{
		for (int y_down_boundary = TASK2_Y_DOWN_BOUNDARY, y_up_boundary = TASK2_Y_DOWN_BOUNDARY + y_interval; y_up_boundary <= TASK2_Y_UP_BOUNDARY; y_down_boundary += y_interval, y_up_boundary += y_interval)
		{
			points_in_grid = 0;
			for (i = x_left_boundary_index; i < size && data[i]->x < x_right_boundary; i++)
			{
				if (data[i]->x > x_left_boundary && data[i]->y > y_down_boundary && data[i]->y < y_up_boundary)
				{
					points_in_grid++;
					// sum up the data of points in a certain grid
					avg[avg_index]->avg_rho += data[i]->rho;
					avg[avg_index]->avg_u += data[i]->u;
					avg[avg_index]->avg_v += data[i]->v;
					avg[avg_index]->avg_x += data[i]->x;
					avg[avg_index]->avg_y += data[i]->y;
				}
			}
			// divide the sum by number of points to get the average
			avg[avg_index]->avg_rho /= points_in_grid;
			avg[avg_index]->avg_u /= points_in_grid;
			avg[avg_index]->avg_v /= points_in_grid;
			avg[avg_index]->avg_x /= points_in_grid;
			avg[avg_index]->avg_y /= points_in_grid;
			avg[avg_index]->score = 100 * sqrt(pow(avg[avg_index]->avg_u, 2) + pow(avg[avg_index]->avg_v, 2)) / sqrt(pow(avg[avg_index]->avg_x, 2) + pow(avg[avg_index]->avg_y, 2));
			avg_index++;
		}
		x_left_boundary_index = i;
	}
	struct_merge_sort((void**)avg, num_of_grid, score_cmp); // use merge sort as a stable sorting method to sort the avg in the descending order of score

	fp = safe_fopen("task2.csv", "a");
	fprintf(fp, "rho,u,v,x,y,S\n");
	for (i = 0; i < num_of_grid; i++)
	{
		fprintf(fp, "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n", avg[i]->avg_rho, avg[i]->avg_u, avg[i]->avg_v, avg[i]->avg_x, avg[i]->avg_y, avg[i]->score);
	}
	fclose(fp);
	for (i = 0; i < num_of_grid; i++)
	{
		free(avg[i]);
	}

	free(avg);

	for (int i = 0; i < size; i++)
	{
		free(data[i]);
	}

	free(data);
}

void searching(const char* flow_file)
{
	int size = INITIAL_SIZE, data_read = 0;
	data_t* data = (data_t*)safe_malloc(size * sizeof(data_t));
	FILE* fp = safe_fopen(flow_file, "r");
	char* buf = (char*)safe_malloc(MAX_BUF_LEN * sizeof(char));
	fgets(buf, MAX_BUF_LEN, fp); // skip the first line of the file
	free(buf);
	while (fscanf(fp, "%lf,%lf,%lf,%lf,%lf\n", &data[data_read].rho, &data[data_read].u, &data[data_read].v, &data[data_read].x, &data[data_read].y) == DATA_COLUMNS_OF_RHO_U_V_X_Y)
	{
		data_read++;
		// when the lines reach the maximum size of the matrix, double the size
		if (size == data_read)
		{
			size = 2 * size;
			data = (data_t*)safe_realloc(data, size * sizeof(data_t));
		}
	}
	fclose(fp);
	size = data_read; // record number of rows of the data (except the title)
	data = (data_t*)safe_realloc(data, size * sizeof(data_t)); //realloc the memory to the final size (free the extra memory)

	// allocate an array to store the streamwise flux of points at centreline
	double* array = (double*)safe_malloc(size * sizeof(double));
	memset(array, 0, size * sizeof(double));

	int points_at_centreline = 0;
	for (int i = 0; i < size; i++)
	{
		if (data[i].y == 0)
		{
			array[points_at_centreline] = data[i].rho * data[i].u;
			points_at_centreline++;
		}
	}
	// realloc the array to the number of points at centreline
	array = (double*)safe_realloc(array, points_at_centreline * sizeof(double));

	// sort the array by using merge sort
	merge_sort(array, points_at_centreline, double_array_cmp);
	
	int aim_value_index = 0;
	double aim_value = 0;

	// find the index of two value that are closest to 40% of maximum rho*u, i.e. aim_value_index and (aim_value_index + 1)
	for (aim_value_index = 0; aim_value_index < points_at_centreline && array[aim_value_index] < 0.4 * array[points_at_centreline - 1]; aim_value_index++);

	// find the one which is closer to 40% of maximum rho*u
	aim_value = array[aim_value_index - 1] - 0.4 * array[points_at_centreline - 1] < array[aim_value_index] - 0.4 * array[points_at_centreline - 1] ? array[aim_value_index - 1] : array[aim_value_index];

	// create a new linked list and insert the values at the back of it
	list_t* list = list_new(free);
	assert(list->num_elements == 0);
	for (int i = 0; i < points_at_centreline; i++)
	{
		list_push_back(list, array[i]);
	}

	// create a balanced BST and insert the values into it
	bst_t* bst = bst_new(no_free, double_cmp);
	int n = make_unique(array, points_at_centreline);
	perfect_insert(bst, array, 0, n - 1);
	assert(bst->num_elements == n);

	struct timeval start;
	struct timeval stop;

	// linear search on the array to find the value
	gettimeofday(&start, NULL);
	fp = safe_fopen("task3.csv", "a");
	for (int i = 0; i < points_at_centreline; i++)
	{
		if (array[i] == aim_value)
		{
			fprintf(fp, "%.6f\n", array[i]);
			break;
		}
		else
		{
			fprintf(fp, "%.6f,", array[i]);
		}
	}
	fclose(fp);

	gettimeofday(&stop, NULL);
	double elapsed_ms = (stop.tv_sec - start.tv_sec) * 1000.0;
	elapsed_ms += (stop.tv_usec - start.tv_usec) / 1000.0;
	printf("TASK 3 Array Linear Search:  %.2f milliseconds\n", elapsed_ms);

	// binary search on the array to find the value
	gettimeofday(&start, NULL);
	binary_search(array, 0, points_at_centreline - 1, aim_value);
	gettimeofday(&stop, NULL);
	elapsed_ms = (stop.tv_sec - start.tv_sec) * 1000.0;
	elapsed_ms += (stop.tv_usec - start.tv_usec) / 1000.0;
	printf("TASK 3 Array Binary Search:  %.2f milliseconds\n", elapsed_ms);

	// linear search on the linked list to find the value
	gettimeofday(&start, NULL);
	list_search(list, aim_value);
	gettimeofday(&stop, NULL);
	elapsed_ms = (stop.tv_sec - start.tv_sec) * 1000.0;
	elapsed_ms += (stop.tv_usec - start.tv_usec) / 1000.0;
	printf("TASK 3 List Linear Search:  %.2f milliseconds\n", elapsed_ms);

	// search on the balanced BST for the value
	gettimeofday(&start, NULL);
	bst_find(bst, aim_value);
	gettimeofday(&stop, NULL);
	elapsed_ms = (stop.tv_sec - start.tv_sec) * 1000.0;
	elapsed_ms += (stop.tv_usec - start.tv_usec) / 1000.0;
	printf("TASK 3 BST Search:  %.2f milliseconds\n", elapsed_ms);

	// free all the memory
	bst_free(bst);
	list_free(list);
	free(array);

	free(data);
}

void vortcalc(const char* flow_file)
{
	int size = INITIAL_SIZE, data_read = 0;
	data_t* data = (data_t*)safe_malloc(size * sizeof(data_t));
	FILE* fp = safe_fopen(flow_file, "r");
	char* buf = (char*)safe_malloc(MAX_BUF_LEN * sizeof(char));
	fgets(buf, MAX_BUF_LEN, fp); // skip the first line of the file
	free(buf);
	while (fscanf(fp, "%lf,%lf,%lf,%lf,%lf\n", &data[data_read].rho, &data[data_read].u, &data[data_read].v, &data[data_read].x, &data[data_read].y) == DATA_COLUMNS_OF_RHO_U_V_X_Y)
	{
		data_read++;
		// when the lines reach the maximum size of the matrix, double the size
		if (size == data_read)
		{
			size = 2 * size;
			data = (data_t*)safe_realloc(data, size * sizeof(data_t));
		}
	}
	fclose(fp);
	size = data_read; //record number of rows of the data (except the title)
	data = (data_t*)safe_realloc(data, size * sizeof(data_t)); //realloc the memory to the final size (free the extra memory)

	// get n and m
	int n = 1;
	while (data[n - 1].y == data[n].y)
	{
		n++;
	}
	int m = size / n;

	// create a n * m 2D array
	int** array = (int**)safe_malloc(n * sizeof(int*));
	for (int i = 0; i < n; i++)
	{
		array[i] = (int*)safe_malloc(m * sizeof(int));
	}

	// store the index of data in 2D array
	int index = 0;
	for (int j = 0; j < m; j++)
	{
		for (int i = 0; i < n; i++)
		{
			array[i][j] = index;
			index++;
		}
	}

	// create an 1D array to store omega, ignoring the sequence
	double* omega = (double*)safe_malloc(size * sizeof(double));
	int omega_index = 0;

	// calculate omega for i go from 0:n-2 and for j go from 0:m-2
	for (int i = 0; i <= n - 2; i++)
	{
		for (int j = 0; j <= m - 2; j++)
		{
			omega[omega_index] = (data[array[i + 1][j]].v - data[array[i][j]].v) / (data[array[i + 1][j]].x - data[array[i][j]].x) - (data[array[i][j + 1]].u - data[array[i][j]].u) / (data[array[i][j + 1]].y - data[array[i][j]].y);
			omega_index++;
		}
	}
	// calculate omega for i = n - 1
	for (int j = 0; j < m - 1; j++)
	{
		omega[omega_index] = (data[array[n - 1][j]].v - data[array[n - 2][j]].v) / (data[array[n - 1][j]].x - data[array[n - 2][j]].x) - (data[array[n - 1][j + 1]].u - data[array[n - 1][j]].u) / (data[array[n - 1][j + 1]].y - data[array[n - 1][j]].y);
		omega_index++;
	}
	// calculate omega for j = m - 1
	for (int i = 0; i < n - 1; i++)
	{
		omega[omega_index] = (data[array[i + 1][m - 1]].v - data[array[i][m - 1]].v) / (data[array[i + 1][m - 1]].x - data[array[i][m - 1]].x) - (data[array[i][m - 1]].u - data[array[i][m - 2]].u) / (data[array[i][m - 1]].y - data[array[i][m - 2]].y);
		omega_index++;
	}
	// calculate omega for i = n - 1 and j = m - 1
	omega[omega_index] = (data[array[n - 1][m - 1]].v - data[array[n - 2][m - 1]].v) / (data[array[n - 1][m - 1]].x - data[array[n - 2][m - 1]].x) - (data[array[n - 1][m - 1]].u - data[array[n - 1][m - 2]].u) / (data[array[n - 1][m - 1]].y - data[array[n - 1][m - 2]].y);

	for (int i = 0; i < size; i++)
	{
		omega[i] = fabs(omega[i]); // get the absolute omega
	}

	// sort the absolute value of omega
	merge_sort(omega, size, double_array_cmp);
	omega_index = 0;

	//report the number of datapoints
	fp = safe_fopen("task4.csv", "a");
	fprintf(fp, "threshold,points\n");
	for (int threshold = TASK4_FIRST_THRESHOLD, points = 0; threshold <= TASK4_LAST_THRESHOLD; threshold += TASK4_THRESHOLD_INTERVAL)
	{
		points = 0;
		while (omega_index < size && omega[omega_index] >= threshold - TASK4_THRESHOLD_INTERVAL && omega[omega_index] < threshold)
		{
			points++;
			omega_index++;
		}
		fprintf(fp, "%d,%d\n", threshold, points);
	}
	fclose(fp);

	//free all the memory
	free(omega);

	for (int i = 0; i < n; i++)
	{
		free(array[i]);
	}
	free(array);

	free(data);

}