/* Lambda matrix and vector interface.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   Contributed by Daniel Berlin <dberlin@dberlin.org>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef LAMBDA_H
#define LAMBDA_H

#include "vec.h"
/* APPLE LOCAL lno */
#include "ggc.h"

/* An integer vector.  A vector formally consists of an element of a vector
   space. A vector space is a set that is closed under vector addition
   and scalar multiplication.  In this vector space, an element is a list of
   integers.  */
typedef int *lambda_vector;
/* An integer matrix.  A matrix consists of m vectors of length n (IE
   all vectors are the same length).  */
typedef lambda_vector *lambda_matrix;

/* A transformation matrix.  */
typedef struct
{
  lambda_matrix matrix;
  int rowsize;
  int colsize;
  int denominator;
} *lambda_trans_matrix;
#define LTM_MATRIX(T) ((T)->matrix)
#define LTM_ROWSIZE(T) ((T)->rowsize)
#define LTM_COLSIZE(T) ((T)->colsize)
#define LTM_DENOMINATOR(T) ((T)->denominator)

/* A vector representing a statement in the body of a loop.  */
typedef struct
{
  lambda_vector coefficients;
  int size;
  int denominator;
} *lambda_body_vector;
#define LBV_COEFFICIENTS(T) ((T)->coefficients)
#define LBV_SIZE(T) ((T)->size)
#define LBV_DENOMINATOR(T) ((T)->denominator)

/* Piecewise linear expression.  */
typedef struct lambda_linear_expression_s
{
  lambda_vector coefficients;
  int constant;
  lambda_vector invariant_coefficients;
  int denominator;
  struct lambda_linear_expression_s *next;
} *lambda_linear_expression;

#define LLE_COEFFICIENTS(T) ((T)->coefficients)
#define LLE_CONSTANT(T) ((T)->constant)
#define LLE_INVARIANT_COEFFICIENTS(T) ((T)->invariant_coefficients)
#define LLE_DENOMINATOR(T) ((T)->denominator)
#define LLE_NEXT(T) ((T)->next)

lambda_linear_expression lambda_linear_expression_new (int, int);
void print_lambda_linear_expression (FILE *, lambda_linear_expression, int,
				     int, char);

/* Loop structure.  */
typedef struct lambda_loop_s
{
  lambda_linear_expression lower_bound;
  lambda_linear_expression upper_bound;
  lambda_linear_expression linear_offset;
  int step;
} *lambda_loop;

#define LL_LOWER_BOUND(T) ((T)->lower_bound)
#define LL_UPPER_BOUND(T) ((T)->upper_bound)
#define LL_LINEAR_OFFSET(T) ((T)->linear_offset)
#define LL_STEP(T)   ((T)->step)

/* Loop nest structure.  */
typedef struct
{
  lambda_loop *loops;
  int depth;
  int invariants;
} *lambda_loopnest;

#define LN_LOOPS(T) ((T)->loops)
#define LN_DEPTH(T) ((T)->depth)
#define LN_INVARIANTS(T) ((T)->invariants)

lambda_loopnest lambda_loopnest_new (int, int);
lambda_loopnest lambda_loopnest_transform (lambda_loopnest, lambda_trans_matrix);
struct loop;
struct loops;
bool perfect_nest_p (struct loop *);
bool lambda_transform_legal_p (lambda_trans_matrix, int, varray_type);
void print_lambda_loopnest (FILE *, lambda_loopnest, char);

#define lambda_loop_new() (lambda_loop) ggc_alloc_cleared (sizeof (struct lambda_loop_s))

void print_lambda_loop (FILE *, lambda_loop, int, int, char);

lambda_matrix lambda_matrix_new (int, int);

void lambda_matrix_id (lambda_matrix, int);
bool lambda_matrix_id_p (lambda_matrix, int);
void lambda_matrix_copy (lambda_matrix, lambda_matrix, int, int);
void lambda_matrix_negate (lambda_matrix, lambda_matrix, int, int);
void lambda_matrix_transpose (lambda_matrix, lambda_matrix, int, int);
void lambda_matrix_add (lambda_matrix, lambda_matrix, lambda_matrix, int,
			int);
void lambda_matrix_add_mc (lambda_matrix, int, lambda_matrix, int,
			   lambda_matrix, int, int);
void lambda_matrix_mult (lambda_matrix, lambda_matrix, lambda_matrix,
			 int, int, int);
void lambda_matrix_delete_rows (lambda_matrix, int, int, int);
void lambda_matrix_row_exchange (lambda_matrix, int, int);
void lambda_matrix_row_add (lambda_matrix, int, int, int, int);
void lambda_matrix_row_negate (lambda_matrix mat, int, int);
void lambda_matrix_row_mc (lambda_matrix, int, int, int);
void lambda_matrix_col_exchange (lambda_matrix, int, int, int);
void lambda_matrix_col_add (lambda_matrix, int, int, int, int);
void lambda_matrix_col_negate (lambda_matrix, int, int);
void lambda_matrix_col_mc (lambda_matrix, int, int, int);
int lambda_matrix_inverse (lambda_matrix, lambda_matrix, int);
void lambda_matrix_hermite (lambda_matrix, int, lambda_matrix, lambda_matrix);
void lambda_matrix_left_hermite (lambda_matrix, int, int, lambda_matrix, lambda_matrix);
void lambda_matrix_right_hermite (lambda_matrix, int, int, lambda_matrix, lambda_matrix);
int lambda_matrix_first_nz_vec (lambda_matrix, int, int, int);
void lambda_matrix_project_to_null (lambda_matrix, int, int, int, 
				    lambda_vector);
void print_lambda_matrix (FILE *, lambda_matrix, int, int);

lambda_trans_matrix lambda_trans_matrix_new (int, int);
bool lambda_trans_matrix_nonsingular_p (lambda_trans_matrix);
bool lambda_trans_matrix_fullrank_p (lambda_trans_matrix);
int lambda_trans_matrix_rank (lambda_trans_matrix);
lambda_trans_matrix lambda_trans_matrix_basis (lambda_trans_matrix);
lambda_trans_matrix lambda_trans_matrix_padding (lambda_trans_matrix);
lambda_trans_matrix lambda_trans_matrix_inverse (lambda_trans_matrix);
void print_lambda_trans_matrix (FILE *, lambda_trans_matrix);
void lambda_matrix_vector_mult (lambda_matrix, int, int, lambda_vector, 
				lambda_vector);
bool lambda_trans_matrix_id_p (lambda_trans_matrix);

lambda_body_vector lambda_body_vector_new (int);
lambda_body_vector lambda_body_vector_compute_new (lambda_trans_matrix, 
						   lambda_body_vector);
void print_lambda_body_vector (FILE *, lambda_body_vector);
lambda_loopnest gcc_loopnest_to_lambda_loopnest (struct loops *,
						 struct loop *,
						 VEC(tree) **,
						 VEC(tree) **,
						 bool);
void lambda_loopnest_to_gcc_loopnest (struct loop *, VEC(tree) *,
				      VEC(tree) *,
				      lambda_loopnest, 
				      lambda_trans_matrix);


static inline void lambda_vector_negate (lambda_vector, lambda_vector, int);
static inline void lambda_vector_mult_const (lambda_vector, lambda_vector, int, int);
static inline void lambda_vector_add (lambda_vector, lambda_vector,
				      lambda_vector, int);
static inline void lambda_vector_add_mc (lambda_vector, int, lambda_vector, int,
					 lambda_vector, int);
static inline void lambda_vector_copy (lambda_vector, lambda_vector, int);
static inline bool lambda_vector_zerop (lambda_vector, int);
static inline void lambda_vector_clear (lambda_vector, int);
static inline bool lambda_vector_equal (lambda_vector, lambda_vector, int);
static inline int lambda_vector_min_nz (lambda_vector, int, int);
static inline int lambda_vector_first_nz (lambda_vector, int, int);
static inline void print_lambda_vector (FILE *, lambda_vector, int);

/* Allocate a new vector of given SIZE.  */

static inline lambda_vector
lambda_vector_new (int size)
{
  return ggc_alloc_cleared (size * sizeof(int));
}



/* Multiply vector VEC1 of length SIZE by a constant CONST1,
   and store the result in VEC2.  */

static inline void
lambda_vector_mult_const (lambda_vector vec1, lambda_vector vec2,
			  int size, int const1)
{
  int i;

  if (const1 == 0)
    lambda_vector_clear (vec2, size);
  else
    for (i = 0; i < size; i++)
      vec2[i] = const1 * vec1[i];
}

/* Negate vector VEC1 with length SIZE and store it in VEC2.  */

static inline void 
lambda_vector_negate (lambda_vector vec1, lambda_vector vec2,
		      int size)
{
  lambda_vector_mult_const (vec1, vec2, size, -1);
}

/* VEC3 = VEC1+VEC2, where all three the vectors are of length SIZE.  */

static inline void
lambda_vector_add (lambda_vector vec1, lambda_vector vec2,
		   lambda_vector vec3, int size)
{
  int i;
  for (i = 0; i < size; i++)
    vec3[i] = vec1[i] + vec2[i];
}

/* VEC3 = CONSTANT1*VEC1 + CONSTANT2*VEC2.  All vectors have length SIZE.  */

static inline void
lambda_vector_add_mc (lambda_vector vec1, int const1,
		      lambda_vector vec2, int const2,
		      lambda_vector vec3, int size)
{
  int i;
  for (i = 0; i < size; i++)
    vec3[i] = const1 * vec1[i] + const2 * vec2[i];
}

/* Copy the elements of vector VEC1 with length SIZE to VEC2.  */

static inline void
lambda_vector_copy (lambda_vector vec1, lambda_vector vec2,
		    int size)
{
  memcpy (vec2, vec1, size * sizeof (*vec1));
}

/* Return true if vector VEC1 of length SIZE is the zero vector.  */

static inline bool 
lambda_vector_zerop (lambda_vector vec1, int size)
{
  int i;
  for (i = 0; i < size; i++)
    if (vec1[i] != 0)
      return false;
  return true;
}

/* Clear out vector VEC1 of length SIZE.  */

static inline void
lambda_vector_clear (lambda_vector vec1, int size)
{
  memset (vec1, 0, size * sizeof (*vec1));
}

/* Return true if two vectors are equal.  */
 
static inline bool
lambda_vector_equal (lambda_vector vec1, lambda_vector vec2, int size)
{
  int i;
  for (i = 0; i < size; i++)
    if (vec1[i] != vec2[i])
      return false;
  return true;
}

/* Return the minimum nonzero element in vector VEC1 between START and N.
   We must have START <= N.  */

static inline int
lambda_vector_min_nz (lambda_vector vec1, int n, int start)
{
  int j;
  int min = -1;
#ifdef ENABLE_CHECKING 
  if (start > n)
    abort ();
#endif
  for (j = start; j < n; j++)
    {
      if (vec1[j])
	if (min < 0 || vec1[j] < vec1[min])
	  min = j;
    }

  if (min < 0)
    abort ();

  return min;
}

/* Return the first nonzero element of vector VEC1 between START and N.
   We must have START <= N.   Returns N if VEC1 is the zero vector.  */

static inline int
lambda_vector_first_nz (lambda_vector vec1, int n, int start)
{
  int j = start;
  while (j < n && vec1[j] == 0)
    j++;
  return j;
}


/* Multiply a vector by a matrix.  */

static inline void
lambda_vector_matrix_mult (lambda_vector vect, int m, lambda_matrix mat, 
			   int n, lambda_vector dest)
{
  int i, j;
  lambda_vector_clear (dest, n);
  for (i = 0; i < n; i++)
    for (j = 0; j < m; j++)
      dest[i] += mat[j][i] * vect[j];
}


/* Print out a vector VEC of length N to OUTFILE.  */

static inline void
print_lambda_vector (FILE * outfile, lambda_vector vector, int n)
{
  int i;

  for (i = 0; i < n; i++)
    fprintf (outfile, "%3d ", vector[i]);
  fprintf (outfile, "\n");
}
#endif /* LAMBDA_H  */

