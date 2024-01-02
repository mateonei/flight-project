/*
* ArrayList implementation written by myself 
* Used throughout FlightSimulator.c for simplicity.
* Hopefully no unsafe operations were done from here 
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "array_list.h"

ArrayList* alist_initialize(int maxSize, int itemSize, char* type) {

	ArrayList* arrList = malloc(sizeof(ArrayList));

	arrList->size = 0;
	arrList->maxSize = maxSize;
	arrList->itemSize = itemSize;
	arrList->type = malloc(strlen(type));
	strcpy(arrList->type, type);

	arrList->arr = calloc(maxSize, sizeof(itemSize));
	for (int i = 0; i < maxSize; i++) {
		arrList->arr[i] = NULL;
	}

	return arrList;
}

bool alist_add(ArrayList* arrList, void* item) {
	if (arrList == NULL || item == NULL) {
		return false;
	}

	if (arrList->size == arrList->maxSize) {
		_alist_resize(arrList);
	}

	arrList->arr[arrList->size] = malloc(arrList->itemSize);
	memcpy(arrList->arr[arrList->size], item, arrList->itemSize);
	arrList->size++;
	return true;
}

bool alist_add_at(ArrayList* arrList, int index, void* item) {
	if (arrList == NULL || item == NULL || index > arrList->size) {
		return false;
	}

	if (index == arrList->size) {
		alist_add(arrList, item);
		return true;
	}

	if (arrList->size == arrList->maxSize) {
		_alist_resize(arrList);
	}

	arrList->arr[arrList->size] = malloc(arrList->itemSize);
	for (int i = arrList->size; i > index; i--) {
		memcpy(arrList->arr[i], arrList->arr[i - 1], arrList->itemSize);
	}

	memcpy(arrList->arr[index], item, arrList->itemSize);
	arrList->size++;
	return true;
}

void alist_clear(ArrayList* arrList) {
	if (arrList == NULL) {
		return;
	}

	for (int i = 0; i < arrList->size; i++) {
		free(arrList->arr[i]);
		arrList->arr[i] = NULL;
	}
	arrList->size = 0;
}

void* alist_get(ArrayList* arrList, int index) {
	if (arrList == NULL || index >= arrList->size) {
		return NULL;
	}

	return arrList->arr[index];
}

int alist_index_of(ArrayList* arrList, void* item) {
	if (arrList == NULL || item == NULL) {
		return -1;
	}

	for (int i = 0; i < arrList->size; i++) {
		if (memcmp(arrList->arr[i], item, arrList->itemSize) == 0) {
			return i;
		}
	}
	return -1;
}

void* alist_remove(ArrayList* arrList, int index) {
	if (arrList == NULL || index >= arrList->size) {
		return NULL;
	}

	void* remItem = malloc(arrList->itemSize);
	memcpy(remItem, arrList->arr[index], arrList->itemSize);

	for (int i = index + 1; i < arrList->size; i++) {
		memcpy(arrList->arr[i - 1], arrList->arr[i], arrList->itemSize);
	}
	arrList->size--;
	free(arrList->arr[arrList->size]);
	arrList->arr[arrList->size] = NULL;

	return remItem;
}

bool alist_destroy(ArrayList* arrList) {
	if (arrList == NULL) {
		return false;
	}

	alist_clear(arrList);
	free(arrList->arr);
	free(arrList->type);
	free(arrList);
	return true;
}

bool _alist_resize(ArrayList* arrList) {
	if (arrList == NULL) {
		return false;
	}

	arrList->maxSize = arrList->maxSize * 2;
	void** temp = realloc(arrList->arr, arrList->maxSize * sizeof(arrList->itemSize));
	arrList->arr = temp;
	for (int i = arrList->maxSize / 2; i < arrList->maxSize; i++) {
		arrList->arr[i] = NULL;
	}

	return true;
}