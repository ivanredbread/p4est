/*
  This file is part of p4est.
  p4est is a C library to manage a collection (a forest) of multiple
  connected adaptive quadtrees or octrees in parallel.

  Copyright (C) 2010 The University of Texas System
  Written by Carsten Burstedde, Lucas C. Wilcox, and Tobin Isaac

  p4est is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  p4est is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with p4est; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef P8EST_CONNECTIVITY_H
#define P8EST_CONNECTIVITY_H

#include <p4est_base.h>

SC_EXTERN_C_BEGIN;

/* the spatial dimension */
#define P8EST_DIM 3
#define P8EST_FACES (2 * P8EST_DIM)
#define P8EST_CHILDREN 8
#define P8EST_HALF (P8EST_CHILDREN / 2)
#define P8EST_EDGES 12
/* size of insulation layer */
#define P8EST_INSUL 27

/* size of face transformation encoding */
#define P8EST_FTRANSFORM 9

/* p8est identification string */
#define P8EST_STRING "p8est"

/* Increase this number whenever the on-disk format for
 * p8est_connectivity, p8est, or any other 3D data structure changes.
 * The format for reading and writing must be the same.
 */
#define P8EST_ONDISK_FORMAT 0x3000008

/* Several functions involve relationships between neighboring trees and/or
 * quadrants, and their behavior depends on how one defines adjacency:
 * 1) entities are adjacent if they share a face, or
 * 2) entities are adjacent if they share a face or corner, or
 * 3) entities are adjacent if they share a face, corner or edge.
 * p8est_connect_type_t is used to choose the desired behavior.
 */
typedef enum
{
  /* make sure to have different values 2D and 3D */
  P8EST_CONNECT_FACE = 31,
  P8EST_CONNECT_EDGE = 32,
  P8EST_CONNECT_CORNER = 33,
  P8EST_CONNECT_DEFAULT = P8EST_CONNECT_EDGE,
  P8EST_CONNECT_FULL = P8EST_CONNECT_CORNER
}
p8est_connect_type_t;

#ifndef P4EST_STRICT_API

/** The following definitions are deprecated, do NOT use in new code. */
#define P8EST_BALANCE_FACE    P8EST_CONNECT_FACE
#define P8EST_BALANCE_EDGE    P8EST_CONNECT_EDGE
#define P8EST_BALANCE_CORNER  P8EST_CONNECT_CORNER
#define P8EST_BALANCE_DEFAULT P8EST_CONNECT_DEFAULT
#define P8EST_BALANCE_FULL    P8EST_CONNECT_FULL
#define p8est_balance_type_t  p8est_connect_type_t

#endif /* P4EST_STRICT_API */

/** Convert the p8est_connect_type_t into a number.
 * \param [in] btype    The balance type to convert.
 * \return              Returns 1, 2 or 3.
 */
int                 p8est_connect_type_int (p8est_connect_type_t btype);

/** Convert the p8est_connect_type_t into a const string.
 * \param [in] btype    The balance type to convert.
 * \return              Returns a pointer to a constant string.
 */
const char         *p8est_connect_type_string (p8est_connect_type_t btype);

/** This structure holds the 3D inter-tree connectivity information.
 * Identification of arbitrary faces, edges and corners is possible.
 *
 * The arrays tree_to_* are stored in z ordering.
 * For corners the order wrt. zyx is 000 001 010 011 100 101 110 111.
 * For faces the order is -x +x -y +y -z +z.
 * They are allocated [0][0]..[0][N-1]..[num_trees-1][0]..[num_trees-1][N-1].
 * where N is 6 for tree and face, 8 for corner, 12 for edge.
 *
 * The values for tree_to_face are in 0..23
 * where ttf % 6 gives the face number and ttf / 6 the face orientation code.
 * The orientation is determined as follows.  Let my_face and other_face
 * be the two face numbers of the connecting trees in 0..5.  Then the first
 * face corner of the lower of my_face and other_face connects to a face
 * corner numbered 0..3 in the higher of my_face and other_face.  The face
 * orientation is defined as this number.  If my_face == other_face, treating
 * either of both faces as the lower one leads to the same result.
 *
 * It is valid to specify num_vertices as 0.
 * In this case vertices and tree_to_vertex are set to NULL.
 * Otherwise the vertex coordinates are stored in the array vertices as
 * [0][0]..[0][2]..[num_vertices-1][0]..[num_vertices-1][2].
 *
 * The edges are only stored when they connect trees.
 * Otherwise the tree_to_edge entry must be -1 and this edge is ignored.
 * If num_edges == 0, tree_to_edge and edge_to_* arrays are set to NULL.
 *
 * The arrays edge_to_* store a variable number of entries per edge.
 * For edge e these are at position [ett_offset[e]]..[ett_offset[e+1]-1].
 * Their number for edge e is ett_offset[e+1] - ett_offset[e].
 * The size of the edge_to_* arrays is num_ett = ett_offset[num_edges].
 * The edge_to_edge array holds values in 0..23, where the lower 12 indicate
 * one edge orientation and the higher 12 the opposite edge orientation.
 *
 * The corners are only stored when they connect trees.
 * Otherwise the tree_to_corner entry must be -1 and this corner is ignored.
 * If num_corners == 0, tree_to_corner and corner_to_* arrays are set to NULL.
 *
 * The arrays corner_to_* store a variable number of entries per corner.
 * For corner c these are at position [ctt_offset[c]]..[ctt_offset[c+1]-1].
 * Their number for corner c is ctt_offset[c+1] - ctt_offset[c].
 * The size of the corner_to_* arrays is num_ctt = ctt_offset[num_corners].
 *
 * The *_to_attr arrays may have arbitrary contents defined by the user.
 */
typedef struct p8est_connectivity
{
  p4est_topidx_t      num_vertices;
  p4est_topidx_t      num_trees;
  p4est_topidx_t      num_edges;
  p4est_topidx_t      num_corners;

  double             *vertices;
  p4est_topidx_t     *tree_to_vertex;
  int8_t             *tree_to_attr;

  p4est_topidx_t     *tree_to_tree;
  int8_t             *tree_to_face;

  p4est_topidx_t     *tree_to_edge;
  p4est_topidx_t     *ett_offset;
  p4est_topidx_t     *edge_to_tree;
  int8_t             *edge_to_edge;

  p4est_topidx_t     *tree_to_corner;
  p4est_topidx_t     *ctt_offset;
  p4est_topidx_t     *corner_to_tree;
  int8_t             *corner_to_corner;
}
p8est_connectivity_t;

/** Calculate memory usage of a connectivity structure.
 * \param [in] conn   Connectivity structure.
 * \return            Memory used in bytes.
 */
size_t              p8est_connectivity_memory_used (p8est_connectivity_t *
                                                    conn);

typedef struct
{
  p4est_topidx_t      ntree;
  int8_t              nedge, naxis[3], nflip, corners;
}
p8est_edge_transform_t;

typedef struct
{
  int8_t              iedge;
  sc_array_t          edge_transforms;
}
p8est_edge_info_t;

typedef struct
{
  p4est_topidx_t      ntree;
  int8_t              ncorner;
}
p8est_corner_transform_t;

typedef struct
{
  p4est_topidx_t      icorner;
  sc_array_t          corner_transforms;
}
p8est_corner_info_t;

/** Store the corner numbers 0..7 for each tree face. */
extern const int    p8est_face_corners[6][4];

/** Store the face numbers 0..12 for each tree face. */
extern const int    p8est_face_edges[6][4];

/** Store the face numbers in the face neighbor's system. */
extern const int    p8est_face_dual[6];

/** Store only the 8 out of 24 possible permutations that occur. */
extern const int    p8est_face_permutations[8][4];

/** Store the 3 occurring sets of 4 permutations per face. */
extern const int    p8est_face_permutation_sets[3][4];

/** For each face combination store the permutation set.
 * The order is [my_face][neighbor_face]
 */
extern const int    p8est_face_permutation_refs[6][6];

/** Store the face numbers 0..5 for each tree edge. */
extern const int    p8est_edge_faces[12][2];

/** Store the corner numbers 0..8 for each tree edge. */
extern const int    p8est_edge_corners[12][2];

/** Store the face corner numbers for the faces touching a tree edge. */
extern const int    p8est_edge_face_corners[12][6][2];

/** Store the face numbers 0..5 for each tree corner. */
extern const int    p8est_corner_faces[8][3];

/** Store the edge numbers 0..11 for each tree corner. */
extern const int    p8est_corner_edges[8][3];

/** Store the face corner numbers for the faces touching a tree corner. */
extern const int    p8est_corner_face_corners[8][6];

/** Store the faces for each child and edge, can be -1. */
extern const int    p8est_child_edge_faces[8][12];

/** Store the faces for each child and corner, can be -1. */
extern const int    p8est_child_corner_faces[8][8];

/** Store the edges for each child and corner, can be -1. */
extern const int    p8est_child_corner_edges[8][8];

/** Allocate a connectivity structure.
 * The attribute fields are initialized to NULL.
 * \param [in] num_vertices   Number of total vertices (i.e. geometric points).
 * \param [in] num_trees      Number of trees in the forest.
 * \param [in] num_edges      Number of tree-connecting edges.
 * \param [in] num_ett        Number of total trees in edge_to_tree array.
 * \param [in] num_corners    Number of tree-connecting corners.
 * \param [in] num_ctt        Number of total trees in corner_to_tree array.
 * \return                    A connectivity structure with allocated arrays.
 */
p8est_connectivity_t *p8est_connectivity_new (p4est_topidx_t num_vertices,
                                              p4est_topidx_t num_trees,
                                              p4est_topidx_t num_edges,
                                              p4est_topidx_t num_ett,
                                              p4est_topidx_t num_corners,
                                              p4est_topidx_t num_ctt);

/** Allocate a connectivity structure and populate from constants.
 * The attribute fields are initialized to NULL.
 * \param [in] num_vertices   Number of total vertices (i.e. geometric points).
 * \param [in] num_trees      Number of trees in the forest.
 * \param [in] num_edges      Number of tree-connecting edges.
 * \param [in] num_corners    Number of tree-connecting corners.
 * \param [in] eoff           Edge-to-tree offsets (num_edges + 1 values).
 * \param [in] coff           Corner-to-tree offsets (num_corners + 1 values).
 * \return                    The connectivity is checked for validity.
 */
p8est_connectivity_t *p8est_connectivity_new_copy (p4est_topidx_t
                                                   num_vertices,
                                                   p4est_topidx_t num_trees,
                                                   p4est_topidx_t num_edges,
                                                   p4est_topidx_t num_corners,
                                                   const double *vertices,
                                                   const p4est_topidx_t * ttv,
                                                   const p4est_topidx_t * ttt,
                                                   const int8_t * ttf,
                                                   const p4est_topidx_t * tte,
                                                   const p4est_topidx_t *
                                                   eoff,
                                                   const p4est_topidx_t * ett,
                                                   const int8_t * ete,
                                                   const p4est_topidx_t * ttc,
                                                   const p4est_topidx_t *
                                                   coff,
                                                   const p4est_topidx_t * ctt,
                                                   const int8_t * ctc);

/** Destroy a connectivity structure.  Also destroy all attributes.
 */
void                p8est_connectivity_destroy (p8est_connectivity_t *
                                                connectivity);

/** Allocate or free the attribute fields in a connectivity.
 * \param [in,out] conn         The conn->*_to_attr fields must either be NULL
 *                              or previously be allocated by this function.
 * \param [in] enable_tree_attr If false, tree_to_attr is freed (NULL is ok).
 *                              If true, it must be NULL and is allocated.
 */
void                p8est_connectivity_set_attr (p8est_connectivity_t * conn,
                                                 int enable_tree_attr);

/** Examine a connectivity structure.
 * \return  Returns true if structure is valid, false otherwise.
 */
int                 p8est_connectivity_is_valid (p8est_connectivity_t *
                                                 connectivity);

/** Check two connectivity structures for equality.
 * \return          Returns true if structures are equal, false otherwise.
 */
int                 p8est_connectivity_is_equal (p8est_connectivity_t * conn1,
                                                 p8est_connectivity_t *
                                                 conn2);

/** Save a connectivity structure to disk.
 * \param [in] filename         Name of the file to write.
 * \param [in] connectivity     Valid connectivity structure.
 * \note            Aborts on file errors.
 */
void                p8est_connectivity_save (const char *filename,
                                             p8est_connectivity_t *
                                             connectivity);

/** Load a connectivity structure from disk.
 * \param [in] filename Name of the file to read.
 * \param [out] length  Size in bytes of connectivity on disk or NULL.
 * \return              Returns a valid connectivity structure.
 * \note                Aborts on file errors or invalid data.
 */
p8est_connectivity_t *p8est_connectivity_load (const char *filename,
                                               long *length);

/** Create a connectivity structure for the unit cube.
 */
p8est_connectivity_t *p8est_connectivity_new_unitcube (void);

/** Create a connectivity structure for an all-periodic unit cube.
 */
p8est_connectivity_t *p8est_connectivity_new_periodic (void);

/** Create a connectivity structure for a mostly periodic unit cube.
 * The left and right faces are identified, and bottom and top rotated.
 * Front and back are not identified.
 */
p8est_connectivity_t *p8est_connectivity_new_rotwrap (void);

/** Create a connectivity structure that contains two cubes.
 */
p8est_connectivity_t *p8est_connectivity_new_twocubes (void);

/** Create a connectivity structure that contains two cubes
 * where the two far ends are identified periodically.
 */
p8est_connectivity_t *p8est_connectivity_new_twowrap (void);

/** Create a connectivity structure that contains a few cubes.
 * These are rotated against each other to stress the topology routines.
 */
p8est_connectivity_t *p8est_connectivity_new_rotcubes (void);

/** An m by n by p array with periodicity in x, y, and z if
 * periodic_a, periodic_b, and periodic_c are true, respectively.
 */
p8est_connectivity_t *p8est_connectivity_new_brick (int m, int n, int p,
                                                    int periodic_a,
                                                    int periodic_b,
                                                    int periodic_c);

/** Create a connectivity structure that builds a spherical shell.
 * It is made up of six connected parts [-1,1]x[-1,1]x[1,2].
 * This connectivity reuses vertices and relies on a geometry transformation.
 * It is thus not suitable for p8est_connectivity_complete.
 */
p8est_connectivity_t *p8est_connectivity_new_shell (void);

/** Create a connectivity structure that builds a solid sphere.
 * It is made up of two layers and a cube in the center.
 * This connectivity reuses vertices and relies on a geometry transformation.
 * It is thus not suitable for p8est_connectivity_complete.
 */
p8est_connectivity_t *p8est_connectivity_new_sphere (void);

/** Fills arrays encoding the axis combinations for a face transform.
 * \param [in]  itree       The number of the originating tree.
 * \param [in]  iface       The number of the originating face.
 * \param [out] ftransform  This array holds 9 integers.
 *              [0]..[2]    The coordinate axis sequence of the origin face.
 *              [3]..[5]    The coordinate axis sequence of the target face.
 *              [6]..[8]    Edge reverse flag for axes 0, 1; face code for 2.
 * \return                  The face neighbor tree if it exists, -1 otherwise.
 */
p4est_topidx_t      p8est_find_face_transform (p8est_connectivity_t *
                                               connectivity,
                                               p4est_topidx_t itree,
                                               int iface, int ftransform[]);

/** Fills an array with information about edge neighbors.
 * \param [in] itree    The number of the originating tree.
 * \param [in] iedge    The number of the originating edge.
 * \param [in,out] ei   A p8est_edge_info_t structure with initialized array.
 */
void                p8est_find_edge_transform (p8est_connectivity_t *
                                               connectivity,
                                               p4est_topidx_t itree,
                                               int iedge,
                                               p8est_edge_info_t * ei);

/** Fills an array with information about corner neighbors.
 * \param [in] itree    The number of the originating tree.
 * \param [in] icorner  The number of the originating corner.
 * \param [in,out] ci   A p8est_corner_info_t structure with initialized array.
 */
void                p8est_find_corner_transform (p8est_connectivity_t *
                                                 connectivity,
                                                 p4est_topidx_t itree,
                                                 int icorner,
                                                 p8est_corner_info_t * ci);

/** Internally connect a connectivity based on tree_to_vertex information.
 * Periodicity that is not inherent in the list of vertices will be lost.
 * \param [in,out] conn     The connectivity needs to have proper vertices
 *                          and tree_to_vertex fields.  The tree_to_tree
 *                          and tree_to_face fields must be allocated
 *                          and satisfy p8est_connectivity_is_valid (conn)
 *                          but will be overwritten.  The edge and corner
 *                          fields will be freed and allocated anew.
 */
void                p8est_connectivity_complete (p8est_connectivity_t * conn);

#ifdef P4EST_METIS

/** p8est_connectivity_reorder
 * This function takes a connectivity \a conn and a parameter \a k,
 * which will typically be the number of processes, and reorders the trees
 * such that if every processes is assigned (num_trees / k) trees, the
 * communication volume will be minimized.  This is intended for use with
 * connectivities that contain a large number of trees.  This should be done
 * BEFORE a p8est is created using the connectivity.  This is done in place:
 * any data structures that use indices to refer to trees before this
 * procedure will be invalid.  Note that this routine calls metis and not
 * parmetis because the connectivity is copied on every process.
 * A communicator is required because I'm not positive that metis is
 * deterministic. \a ctype determines when an edge exist between two trees in
 * the dual graph used by metis in the reordering.
 * \param [in]     comm       MPI communicator.
 * \param [in]     k          if k > 0, the number of pieces metis will use to
 *                            guide the reordering; if k = 0, the number of
 *                            pieces will be determined from the MPI
 *                            communicator.
 * \param [in,out] conn       connectivity that will be reordered.
 * \param [in]     ctype      determines when an edge exists in the dual graph
 *                            of the connectivity structure.
 */
void                p8est_connectivity_reorder (MPI_Comm comm, int k,
                                                p8est_connectivity_t * conn,
                                                p8est_connect_type_t ctype);

#endif /* P4EST_METIS */

/** Return a pointer to a p8est_edge_transform_t array element. */
/*@unused@*/
static inline p8est_edge_transform_t *
p8est_edge_array_index (sc_array_t * array, size_t it)
{
  P4EST_ASSERT (array->elem_size == sizeof (p8est_edge_transform_t));
  P4EST_ASSERT (it < array->elem_count);

  return (p8est_edge_transform_t *) (array->array +
                                     sizeof (p8est_edge_transform_t) * it);
}

/** Return a pointer to a p8est_corner_transform_t array element. */
/*@unused@*/
static inline p8est_corner_transform_t *
p8est_corner_array_index (sc_array_t * array, size_t it)
{
  P4EST_ASSERT (array->elem_size == sizeof (p8est_corner_transform_t));
  P4EST_ASSERT (it < array->elem_count);

  return
    (p8est_corner_transform_t *) (array->array +
                                  sizeof (p8est_corner_transform_t) * it);
}

SC_EXTERN_C_END;

#endif /* !P8EST_CONNECTIVITY_H */
