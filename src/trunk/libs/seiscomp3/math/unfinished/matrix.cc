# include <math.h>

# include "matrix.h"

float mean(int n, float *f);

# define SWAP(x,y) {float tmp=x;x=y;y=tmp;}
# define SIGN(x)   (((x)>0.0)?1.0:-1.0)

# define FOREVER    for(;;)

# define MAX_ITERATION 300
# define TINY 1.0e-20

# define MXE_NODIAG 2201

int
comp_cov_mat (int nsamp,                          // number of samples
              float *fz, float *fn, float *fe,    // 3 components
              float cov_mat[6])                   // covariance matrix
// compute 3-D covariance matrix
{
    float           //
    norm,         //
    mz, mn, me,   //
    sz, sn, se,   //
    var_z  = 0.0, // variances
    var_n  = 0.0, //
    var_e  = 0.0, //
    cov_zn = 0.0, // covariances
    cov_ze = 0.0, //
    cov_ne = 0.0; //

    norm = 1.0/(float)nsamp;

    mz = mean(nsamp, fz);
    mn = mean(nsamp, fn);
    me = mean(nsamp, fe);

    for(int i=0; i<nsamp; i++)
    {
        sz = fz[i] - mz;
        sn = fn[i] - mn;
        se = fe[i] - me;

        var_z  += sz*sz;
        var_n  += sn*sn;
        var_e  += se*se;
        cov_zn += sz*sn;
        cov_ze += sz*se;
        cov_ne += sn*se;
    }

    cov_mat[ZZ]  =  norm * var_z;
    cov_mat[NN]  =  norm * var_n;
    cov_mat[EE]  =  norm * var_e;
    cov_mat[ZN]  =  norm * cov_zn;
    cov_mat[ZE]  =  norm * cov_ze;
    cov_mat[NE]  =  norm * cov_ne;

    return AH_SUCCESS;
}

int
comp_cov_mat_eig_val (float cov_mat[], // 3-D covariance matrix
                      float eig_val[]) // 3 eigenvalues
// compute the eigenvalues of a 3-D covariance matrix
{
    double
    r,s,t,          // coefficients of characteristic polynomial
    p, q,           // coefficients of reduced equation
    D,              // discriminant
    rho, phi,       // scratch
    ev1,ev2,ev3;    // eigenvalues
    float *cm = cov_mat;

    // characteristic polynomial: p(x) = x^3 + r*x^2 + s*x + t
    r =  -cm[ZZ] - cm[NN] - cm[EE];
    s =   cm[ZZ]*cm[NN] + cm[ZZ]*cm[EE] + cm[NN]*cm[EE]
          - cm[ZN]*cm[ZN] - cm[ZE]*cm[ZE] - cm[NE]*cm[NE];
    t =   cm[ZZ]*cm[NE]*cm[NE] + cm[NN]*cm[ZE]*cm[ZE]
          + cm[EE]*cm[ZN]*cm[ZN] - cm[ZZ]*cm[NN]*cm[EE]
          - 2.0*cm[ZN]*cm[NE]*cm[ZE];
    r /= 3.0;

    // reduced equation: y^3 + p*y  + q = 0
    p = s - 3.0*r*r;
    q = 2.0*r*r*r - r*s + t;
    if (p > 0.0)
    {
        p = -p;
        ah_message ("*** this cannot happen ***");
    }

    rho = sqrt (-p);
    rho = rho*rho*rho/5.1961524; // sqrt(27.)
    D = (rho > 1.0E-20) ? -q/(2.0*rho) : 0.0;
    if  (D >  1.0)  D =  1.0;
    if  (D < -1.0)  D = -1.0;
    phi = acos (D);

    // compute eigenvalues
    D = (rho > 0.0) ? 2.0*pow (rho, 1.0/3.0) : 0.0;
    phi /= 3.0;
    ev1 = D * cos (phi);
    ev2 = D * cos (phi + 2.0*M_PI/3.0);
    ev3 = D * cos (phi + 4.0*M_PI/3.0);

    // undo reduction
    ev1 -= r;
    ev2 -= r;
    ev3 -= r;

    // sort eval's by size
    if (ev1 < ev2)  SWAP (ev1, ev2);
    if (ev2 < ev3)  SWAP (ev2, ev3);
    if (ev1 < ev2)  SWAP (ev1, ev2);

    eig_val[0] = ev1;
    eig_val[1] = ev2;
    eig_val[2] = ev3;

    return AH_SUCCESS;
}


void
unit_matrix (float m[3][3])
// makes m a unit matrix
{
    // set diagonal elements to 1
    m[0][0] = m[1][1] = m[2][2] = 1.0;

    // set off-diagonal elements to 0
    m[0][1] = m[1][2] = m[0][2] = 0.0;
    m[1][0] = m[2][1] = m[2][0] = 0.0;
} // end unit_matrix


void copy_matrix (float   m[3][3],
                  float m_c[3][3])
// copies the matrix "m" to "m_c".
{
    int i, k;

    for (i=0; i<3; i++)
        for (k=0; k<3; k++)
            m_c[i][k] = m[i][k];

} // end copy_matrix


void
mul_matrix (float  m1[3][3],
            float  m2[3][3],
            float res[3][3])
// multiplies matrices "m1" and "m2"
// and stores result in "res"
{
    int i, k, ii;

    for (i=0; i<3; i++)
    {
        for (k=0; k<3; k++)
        {
            res[i][k] = 0.0;

            for (ii=0; ii<3; ii++)
                res[i][k] += m1[i][ii]*m2[ii][k];
        } // end for
    } // end for

} // end mul_matrix


void
transpose_matrix (float m[3][3], float m_t[3][3])
// transpones the matrix "m".  Result is matrix "m_t"
//
// parameters of routine
//   float   m[3][3];   input; input matrix
//   float m_t[3][3];  output; transponed matrix
{
    int i, k;

    for (i=0; i<3; i++)
        for (k=0; k<3; k++)
            m_t[i][k] = m[k][i];

} // end transpose_matrix


void
sort_eig (float eig_val[3], float eig_vec[3][3])
// sort eigenvalues (and corresponding eigenvectors)
// in descending order
{
    int i;

    if (eig_val[0] < eig_val[1])
    {
        SWAP (eig_val[0], eig_val[1]);
        for (i=0; i<3; i++)
            SWAP (eig_vec[i][0], eig_vec[i][1]);
    }

    if (eig_val[1] < eig_val[2])
    {
        SWAP (eig_val[1], eig_val[2]);
        for (i=0; i<3; i++)
            SWAP (eig_vec[i][1], eig_vec[i][2]);
    }

    if (eig_val[0] < eig_val[1])
    {
        SWAP (eig_val[0], eig_val[1]);
        for (i=0; i<3; i++)
            SWAP (eig_vec[i][0], eig_vec[i][1]);
    }
} // end sort_eig


void
diag_matrix (float sym_m[3][3],
             float eig_val[3],
             float eig_vec[3][3],
             int *status)
/* diagonalises symmetrical matrix "sym_m".  Result is "eig_val" containing
 * the eigenvalues of "sym_m", and "eig_veg" containing the transformation
 * matrix, i.e. the eigenvectors.  To the eigenvalue "eig_val[i]"
 * corresponds the "i"-th column of the transformation matrix "eig_vec"
 * as an eigenvector (evec[j] = trafo[j][i], j = 1,..,MXC_DIM)
 * For diagonalising the Jacobi method is used.
 *
 * parameters of routine
 * REAL       sym_m[MXC_DIM][MXC_DIM];  input; input matrix
 * REAL       eval[MXC_DIM];            output; eigenvalues of "sym_m"
 * REAL       trafo[MXC_DIM][MXC_DIM];  output; transformation matrix
 * int        *status;                  output; return status
 */
{
    int
    i, k,       // counters
    loop_cnt;   // iteration counter
    float         //
    sum_limit,  // exit criterion
    diff,       // scratch
    sum,        // convergence criterion
    phi,        // angle of elementary rotation
    m[3][3],    // matrices
    rot[3][3], rot_t[3][3], tmp[3][3];

    // clear eigenvalues and vectors
    for (i=0; i<3; i++)
    {
        eig_val[i] = 0.0;
        //      for (k=0; k<3; k++)
        //	eig_vec[i][k] = 0.0;
    }

    unit_matrix (eig_vec);
    copy_matrix (sym_m, m);

    // find sum limit
    sum_limit = 0.0;
    for (i=0; i<3; i++)
        for (k=i; k<3; k++)
            if (fabs (sym_m[i][k]) > sum_limit)
                sum_limit = fabs (sym_m[i][k]);
    sum_limit *= 1.0E-6;
    if (sum_limit == 0.0) return;

    loop_cnt = 0;
    FOREVER {
        if (++loop_cnt > MAX_ITERATION)
    {
        *status = MXE_NODIAG;
        return;
    }

    // test convergence
    sum = 0.0;
    for (i=0; i<2; i++)
        for (k=i+1; k<3; k++)
            sum += m[i][k] * m[i][k];
            if (sum < sum_limit) break;

                // elementary Jacobi rotation
                for (i=0; i<2; i++)
                {
                    for (k=i+1; k<3; k++)
                        {
                            diff = -m[i][i] + m[k][k];

                            if (fabs(diff) > TINY)
                                phi = 0.5 * atan (2.0*m[i][k]/diff);
                            else
                            {
                                if (fabs (m[i][k]) < TINY)
                                    phi = 0.0;
                                else
                                    phi = SIGN(m[i][k]) * SIGN(diff) * M_PI * 0.25;
                            }

                            unit_matrix (rot);
                            rot[i][i] =  cos (phi);
                            rot[i][k] =  sin (phi);
                            rot[k][i] = -rot[i][k];
                            rot[k][k] =  rot[i][i];
                            transpose_matrix (rot, rot_t);
                            mul_matrix (rot_t, m, tmp);
                            mul_matrix (tmp, rot, m);
                            mul_matrix (eig_vec, rot, tmp);
                            copy_matrix (tmp, eig_vec);
                        } // end for
                    } // end for

    } // end FOREVER

    for  (i=0; i<3; i++)
        eig_val[i] = m[i][i];

    // sort eigenvalues (and corresponding eigenvectors)
    // in descending order

    sort_eig (eig_val, eig_vec);

} // end diag_matrix ()
