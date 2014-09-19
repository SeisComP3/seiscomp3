
/*
 *	FORTRAN to C interface to call function, indexx().
 */

#ifdef SCCSID
static	char	SccsId[] = "@(#)index_array_.c	44.1 9/20/91";
#endif

void index_array__ (n, arrin, indx)

int	*indx, *n;
double	*arrin;

{
	void	indexx();
	indexx (n, arrin-1, indx-1);
}


/*
 *	Indexes an array, arrin[1..n], i.e., outputs the array, indx[1..n],
 *	such that arrin[indx[j]] is in ascending order for j = 1, 2, .., N.
 *	The input quantities, n and arrin, are not changed.
 */

void indexx (n, arrin, indx)

int	indx[], *n;
double	arrin[];

{
	int	i, indxt, ir, j, l;
	double	q;

	for (j = 1; j <= *n; j++)	/* Initialize the index array */
		indx[j] = j;		/* with consecutive integers  */


	/* Let's do a Heapsort with indirect indexing thru indx[] */

	l  = (*n >> 1) + 1;
	ir = *n;

	for (;;)
	{
		if (l > 1)
			q = arrin[(indxt = indx[--l])];
		else
		{
			q        = arrin[(indxt = indx[ir])];
			indx[ir] = indx[1];
			if (--ir == 1)
			{
				indx[1] = indxt;
				return;
			}
		}
		i = l;
		j = l << 1;
		while (j <= ir)
		{
			if (j < ir && arrin[indx[j]] < arrin[indx[j+1]])
				j++;
			if (q < arrin[indx[j]])
			{
				indx[i] = indx[j];
				j += (i = j);
			}
			else
				j = ir + 1;
		}
		indx[i] = indxt;
	}
}

