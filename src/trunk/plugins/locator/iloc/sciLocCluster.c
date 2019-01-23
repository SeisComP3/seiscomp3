/*
 * Copyright (c) 2018-2019, Istvan Bondar,
 * Written by Istvan Bondar, ibondar2014@gmail.com
 *
 * BSD Open Source License.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "sciLocInterface.h"

/*
 *
 * Routines for single-linkage clustering algorithm
 * based on the functions from
 *
 * The C clustering library.
 * Copyright (C) 2002 Michiel Jan Laurens de Hoon.
 *
 * de Hoon, M.J.L., S. Imoto, J. Nolan and S. Miyano, 2004,
 * Open source clustering software,
 * Bioinformatics, 20, 1453-1454.
 *
 * This library was written at the Laboratory of DNA Information Analysis,
 * Human Genome Center, Institute of Medical Science, University of Tokyo,
 * 4-6-1 Shirokanedai, Minato-ku, Tokyo 108-8639, Japan.
 * Contact: mdehoon 'AT' gsc.riken.jp
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation with or without modifications and for any purpose and
 * without fee is hereby granted, provided that any copyright notices
 * appear in all copies and that both those copyright notices and this
 * permission notice appear in supporting documentation, and that the
 * names of the contributors or copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific prior permission.
 *
 * THE CONTRIBUTORS AND COPYRIGHT HOLDERS OF THIS SOFTWARE DISCLAIM ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * A ILOC_NODE struct describes a single node in a tree created by hierarchical
 * clustering. The tree can be represented by an array of n ILOC_NODE structs,
 * where n is the number of elements minus one. The integers left and right
 * in each node struct refer to the two elements or subnodes that are joined
 * in this node. The original elements are numbered 0..nsta-1, and the
 * NODEs -1..-(nsta-1). For each node, distance contains the distance
 * between the two subnodes that were joined.
 */

/*
 * Local functions
 */
static ILOC_NODE *pslcluster(int nsta, double **distmatrix);
static void TreeSort(int n, int *order, int *nodecounts,
        ILOC_NODE *node, ILOC_STAORDER *staorder, int *clusterids);
static int NodeCompare(const void *a, const void *b);
static int StaOrderCompare(const void *a, const void *b);


/*
 *  Title:
 *     iLoc_HierarchicalCluster
 *  Synopsis:
 *     determines the nearest-neighbour station order by performing
 *     single-linkage hierarchical clustering on the station separations.
 *  Input Arguments:
 *     nsta       - number of stations
 *     distmatrix - distance matrix of station separations
 *  Output Arguments:
 *     staorder - ILOC_STAORDER structure that maps the station list to
 *                a nearest-neighbour station order
 *  Return:
 *     Success/error
 *  Calls:
 *     pslcluster, TreeSort
 */
int iLoc_HierarchicalCluster(int nsta, double **distmatrix, ILOC_STAORDER *staorder)
{
    int i1 = 0, i2 = 0, counts1 = 0, counts2 = 0;
    int nnodes = nsta - 1;
    int i, j, k;
    int *clusterids = (int *)NULL;
    int *nodecounts = (int *)NULL;
    int *order = (int *)NULL;
    ILOC_NODE *node = (ILOC_NODE *)NULL;
/*
 *  Perform single-linkage clustering
 */
    if ((node = pslcluster(nsta, distmatrix)) == NULL)
        return ILOC_MEMORY_ALLOCATION_ERROR;
/*
 *  memory allocations
 */
    clusterids = (int *)calloc(nsta, sizeof(int));
    order = (int *)calloc(nsta, sizeof(int));
    if ((nodecounts = (int *)calloc(nnodes, sizeof(int))) == NULL) {
        fprintf(stderr, "iLoc_HierarchicalCluster: cannot allocate memory\n");
        iLoc_Free(order);
        iLoc_Free(clusterids);
        iLoc_Free(node);
        return ILOC_MEMORY_ALLOCATION_ERROR;
    }
    for (i = 0; i < nsta; i++) {
        order[i] = i;
        clusterids[i] = i;
    }
/*
 *  join nodes
 */
    for (i = 0; i < nnodes; i++) {
/*
 *      i1 and i2 are the elements that are to be joined
 */
        i1 = node[i].left;
        i2 = node[i].right;
        if (i1 < 0) {
            j = -i1 - 1;
            counts1 = nodecounts[j];
            node[i].linkdist = ILOC_MAX(node[i].linkdist, node[j].linkdist);
        }
        else {
            counts1 = 1;
        }
        if (i2 < 0) {
            j = -i2 - 1;
            counts2 = nodecounts[j];
            node[i].linkdist = ILOC_MAX(node[i].linkdist, node[j].linkdist);
        }
        else {
            counts2 = 1;
        }
        nodecounts[i] = counts1 + counts2;
    }
/*
 *  get nearest-neighbour station order
 */
    TreeSort(nsta, order, nodecounts, node, staorder, clusterids);
    iLoc_Free(order);
    iLoc_Free(clusterids);
    for (k = nnodes - 1; k > -1; k--) {
        i = node[k].left;
        j = node[k].right;
/*
 *      leafs
 */
        if (i >= 0) staorder[i].index = i;
        if (j >= 0) staorder[j].index = j;
    }
/*
 *  sort by nearest-neighbour order
 */
    qsort(staorder, nsta, sizeof(ILOC_STAORDER), StaOrderCompare);
    iLoc_Free(nodecounts);
    iLoc_Free(node);
    return ILOC_SUCCESS;
}

/*
 *  Title:
 *     pslcluster
 *  Synopsis:
 *     The pslcluster routine performs single-linkage hierarchical clustering,
 *     using the distance matrix. This implementation is based on the SLINK
 *     algorithm, described in:
 *     Sibson, R. (1973). SLINK: An optimally efficient algorithm for the
 *         single-link cluster method. The Computer Journal, 16(1): 30-34.
 *     The output of this algorithm is identical to conventional single-linkage
 *     hierarchical clustering, but is much more memory-efficient and faster.
 *     Hence, it can be applied to large data sets, for which the conventional
 *     single-linkage algorithm fails due to lack of memory.
 *  Input Arguments:
 *     nsta       - number of stations
 *     distmatrix - distance matrix of station separations
 *  Return:
 *     tree - A pointer to a newly allocated array of ILOC_NODE structs,
 *            describing the hierarchical clustering solution consisting
 *            of nsta-1 nodes.
 *            If a memory error occurs, pslcluster returns NULL.
 */
static ILOC_NODE *pslcluster(int nsta, double **distmatrix)
{
    int i, j, k;
    int nnodes = nsta - 1;
    int *vector = (int *)NULL;
    int *index = (int *)NULL;
    double *temp = (double *)NULL;
    double *linkdist = (double *)NULL;
    ILOC_NODE *node = (ILOC_NODE *)NULL;
/*
 *  memory allocations
 */
    temp = (double *)calloc(nnodes, sizeof(double));
    linkdist = (double *)calloc(nsta, sizeof(double));
    vector = (int *)calloc(nnodes, sizeof(int));
    index = (int *)calloc(nsta, sizeof(int));
    if ((node = (ILOC_NODE *)calloc(nnodes, sizeof(ILOC_NODE))) == NULL) {
        fprintf(stderr, "pslcluster: cannot allocate memory\n");
        iLoc_Free(index);
        iLoc_Free(vector);
        iLoc_Free(temp);
        iLoc_Free(linkdist);
        return (ILOC_NODE *)NULL;
    }

    for (i = 0; i < nnodes; i++) vector[i] = i;
/*
 *  calculate linkage distances between nodes
 */
    for (i = 0; i < nsta; i++) {
        linkdist[i] = ILOC_NULLVAL;
        for (j = 0; j < i; j++) temp[j] = distmatrix[i][j];
        for (j = 0; j < i; j++) {
            k = vector[j];
            if (linkdist[j] >= temp[j]) {
                if (linkdist[j] < temp[k]) temp[k] = linkdist[j];
                linkdist[j] = temp[j];
                vector[j] = i;
            }
            else if (temp[j] < temp[k]) temp[k] = temp[j];
        }
        for (j = 0; j < i; j++)
            if (linkdist[j] >= linkdist[vector[j]]) vector[j] = i;
    }
    iLoc_Free(temp);
/*
 *  build the tree
 */
    for (i = 0; i < nnodes; i++) {
        node[i].left = i;
        node[i].linkdist = linkdist[i];
    }
    qsort(node, nnodes, sizeof(ILOC_NODE), NodeCompare);
    for (i = 0; i < nsta; i++) index[i] = i;
    for (i = 0; i < nnodes; i++) {
        j = node[i].left;
        k = vector[j];
        node[i].left = index[j];
        node[i].right = index[k];
        index[k] = -i-1;
    }
    iLoc_Free(vector);
    iLoc_Free(linkdist);
    iLoc_Free(index);
    return node;
}

/*
 *
 *  TreeSort: sorts the tree according to linkage distances between nodes
 *
 */
static void TreeSort(int nsta, int *order, int *nodecounts,
        ILOC_NODE *node, ILOC_STAORDER *staorder, int *clusterids)
{
    int nnodes = nsta - 1;
    int i, i1 = 0, i2 = 0, j = 0, count1 = 0, count2 = 0, clusterid = 0;
    int order1 = 0, order2 = 0, inc = 0;
    for (i = 0; i < nnodes; i++) {
        i1 = node[i].left;
        i2 = node[i].right;
        if (i1 < 0) {
            j = -i1 - 1;
            order1 = staorder[j].x;
            count1 = nodecounts[j];
        }
        else {
            order1 = order[i1];
            count1 = 1;
        }
        if (i2 < 0) {
            j = -i2 - 1;
            order2 = staorder[j].x;
            count2 = nodecounts[j];
        }
        else {
            order2 = order[i2];
            count2 = 1;
        }
/*
 *     If order1 and order2 are equal, their order is determined by
 *     the order in which they were clustered
 */
        if (i1 < i2) {
            inc = (order1 < order2) ? count1 : count2;
            for (j = 0; j < nsta; j++) {
                clusterid = clusterids[j];
                if (clusterid == i1 && order1 >= order2) staorder[j].x += inc;
                if (clusterid == i2 && order1 <  order2) staorder[j].x += inc;
                if (clusterid == i1 || clusterid == i2) clusterids[j] = -i-1;
           }
        }
        else {
            inc = (order1 <= order2) ? count1 : count2;
            for (j = 0; j < nsta; j++) {
                clusterid = clusterids[j];
                if (clusterid == i1 && order1 >  order2) staorder[j].x += inc;
                if (clusterid == i2 && order1 <= order2) staorder[j].x += inc;
                if (clusterid == i1 || clusterid == i2) clusterids[j] = -i-1;
            }
        }
    }
}

/*
 *
 * NodeCompare: compares two ILOC_NODE records based on link distance
 *
 */
static int NodeCompare(const void *a, const void *b)
{
    if (((ILOC_NODE *)a)->linkdist < ((ILOC_NODE *)b)->linkdist)
        return -1;
    if (((ILOC_NODE *)a)->linkdist > ((ILOC_NODE *)b)->linkdist)
        return 1;
    return 0;
}

/*
 *
 * StaOrderCompare: compares two ILOC_STAORDER records based on x
 *
 */
static int StaOrderCompare(const void *a, const void *b)
{
    if (((ILOC_STAORDER *)a)->x < ((ILOC_STAORDER *)b)->x)
        return -1;
    if (((ILOC_STAORDER *)a)->x > ((ILOC_STAORDER *)b)->x)
        return 1;
    return 0;
}

