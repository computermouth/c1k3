// libdsa
// https://github.com/joao-conde/libdsa

// MIT License

// Copyright (c) 2022 João Dias Conde Azevedo

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct vector vector;

vector* vector_init(size_t type_size);

vector* vector_with_capacity(size_t type_size, size_t capacity);

void vector_free(vector *v);

size_t vector_size(const vector *v);

size_t vector_capacity(const vector *v);

bool vector_empty(const vector *v);

void vector_clear(vector *v);

void* vector_at(const vector *v, size_t index);

void* vector_set(const vector *v, size_t index, const void *value);

void* vector_begin(const vector *v);

void* vector_back(const vector *v);

void* vector_end(const vector *v);

void* vector_push(vector *v, const void *value);

void* vector_pop(vector *v);

void* vector_insert(vector *v, size_t index, const void *value);

void* vector_erase(vector *v, size_t index);

void* vector_resize(vector *v, size_t capacity);

#endif
