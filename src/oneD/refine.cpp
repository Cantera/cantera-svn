#include <map>
#include <algorithm>
#include "cantera/oneD/Domain1D.h"

#include "cantera/oneD/refine.h"

using namespace std;

namespace Cantera
{

template<class M>
bool has_key(const M& m, size_t j)
{
    if (m.find(j) != m.end()) {
        return true;
    }
    return false;
}

static void r_drawline()
{
    string s(78,'#');
    s += '\n';
    writelog(s.c_str());
}

/**
 * Return the square root of machine precision.
 */
static doublereal eps()
{
    doublereal e = 1.0;
    while (1.0 + e != 1.0) {
        e *= 0.5;
    }
    return sqrt(e);
}


Refiner::Refiner(Domain1D& domain) :
    m_ratio(10.0), m_slope(0.8), m_curve(0.8), m_prune(-0.001),
    m_min_range(0.01), m_domain(&domain), m_npmax(3000)
{
    m_nv = m_domain->nComponents();
    m_active.resize(m_nv, true);
    m_thresh = eps();
}


int Refiner::analyze(size_t n, const doublereal* z,
                     const doublereal* x)
{

    if (n >= m_npmax) {
        writelog("max number of grid points reached ("+int2str(int(m_npmax))+".\n");
        return -2;
    }

    if (m_domain->nPoints() <= 1) {
        //writelog("can't refine a domain with 1 point: "+m_domain->id()+"\n");
        return 0;
    }

    m_loc.clear();
    m_c.clear();
    m_keep.clear();

    m_keep[0] = 1;
    m_keep[n-1] = 1;

    m_nv = m_domain->nComponents();

    // check consistency
    if (n != m_domain->nPoints()) {
        throw CanteraError("analyze","inconsistent");
    }


    /**
     * find locations where cell size ratio is too large.
     */
    size_t j;
    vector_fp dz(n-1, 0.0);
    string name;
    doublereal vmin, vmax, smin, smax, aa, ss;
    doublereal dmax, r;
    vector_fp v(n), s(n-1);

    for (size_t i = 0; i < m_nv; i++) {
        if (m_active[i]) {
            name = m_domain->componentName(i);
            //writelog("refine: examining "+name+"\n");
            // get component i at all points
            for (j = 0; j < n; j++) {
                v[j] = value(x, i, j);
            }

            // slope of component i
            for (j = 0; j < n-1; j++)
                s[j] = (value(x, i, j+1) - value(x, i, j))/
                       (z[j+1] - z[j]);

            // find the range of values and slopes

            vmin = *min_element(v.begin(), v.end());
            vmax = *max_element(v.begin(), v.end());
            smin = *min_element(s.begin(), s.end());
            smax = *max_element(s.begin(), s.end());

            // max absolute values of v and s
            aa = std::max(fabs(vmax), fabs(vmin));
            ss = std::max(fabs(smax), fabs(smin));

            // refine based on component i only if the range of v is
            // greater than a fraction 'min_range' of max |v|. This
            // eliminates components that consist of small fluctuations
            // on a constant background.

            if ((vmax - vmin) > m_min_range*aa) {

                // maximum allowable difference in value between
                // adjacent points.

                dmax = m_slope*(vmax - vmin) + m_thresh;
                for (j = 0; j < n-1; j++) {
                    r = fabs(v[j+1] - v[j])/dmax;
                    if (r > 1.0) {
                        m_loc[j] = 1;
                        m_c[name] = 1;
                        //if (int(m_loc.size()) + n > m_npmax) goto done;
                    }
                    if (r >= m_prune) {
                        m_keep[j] = 1;
                        m_keep[j+1] = 1;
                    } else {
                        //writelog(string("r = ")+fp2str(r)+"\n");
                        if (m_keep[j] == 0) {
                            //if (m_keep[j-1] > -1 && m_keep[j+1] > -1)
                            m_keep[j] = -1;
                        }
                        //if (m_keep[j+1] == 0) m_keep[j+1] = -1;
                    }
                }
            }


            // refine based on the slope of component i only if the
            // range of s is greater than a fraction 'min_range' of max
            // |s|. This eliminates components that consist of small
            // fluctuations on a constant slope background.

            if ((smax - smin) > m_min_range*ss) {

                // maximum allowable difference in slope between
                // adjacent points.
                dmax = m_curve*(smax - smin); // + 0.5*m_curve*(smax + smin);
                for (j = 0; j < n-2; j++) {
                    r = fabs(s[j+1] - s[j]) / (dmax + m_thresh/dz[j]);
                    if (r > 1.0) {
                        m_c[name] = 1;
                        m_loc[j] = 1;
                        m_loc[j+1] = 1;
                        //if (int(m_loc.size()) + n > m_npmax) goto done;
                    }
                    if (r >= m_prune) {
                        m_keep[j+1] = 1;
                    } else {
                        //writelog(string("r slope = ")+fp2str(r)+"\n");
                        if (m_keep[j+1] == 0) {
                            //if (m_keep[j] > -1 && m_keep[j+2] > -1)
                            m_keep[j+1] = -1;
                        }
                    }
                }
            }

        }
    }

    dz[0] = z[1] - z[0];
    for (j = 1; j < n-1; j++) {
        dz[j] = z[j+1] - z[j];
        if (dz[j] > m_ratio*dz[j-1]) {
            m_loc[j] = 1;
            m_c["point "+int2str(int(j))] = 1;
        }
        if (dz[j] < dz[j-1]/m_ratio) {
            m_loc[j-1] = 1;
            m_c["point "+int2str(int(j)-1)] = 1;
        }
        //if (m_loc.size() + n > m_npmax) goto done;
    }

    //done:
    //m_did_analysis = true;
    return int(m_loc.size());
}

double Refiner::value(const double* x, size_t i, size_t j)
{
    return x[m_domain->index(i,j)];
}

void Refiner::show()
{
    int nnew = static_cast<int>(m_loc.size());
    if (nnew > 0) {
        r_drawline();
        writelog(string("Refining grid in ") +
                 m_domain->id()+".\n"
                 +"    New points inserted after grid points ");
        map<size_t, int>::const_iterator b = m_loc.begin();
        for (; b != m_loc.end(); ++b) {
            writelog(int2str(int(b->first))+" ");
        }
        writelog("\n");
        writelog("    to resolve ");
        map<string, int>::const_iterator bb = m_c.begin();
        for (; bb != m_c.end(); ++bb) {
            writelog(string(bb->first)+" ");
        }
        writelog("\n");
    } else if (m_domain->nPoints() > 1) {
        writelog("no new points needed in "+m_domain->id()+"\n");
        //writelog("curve = "+fp2str(m_curve)+"\n");
        //writelog("slope = "+fp2str(m_slope)+"\n");
        //writelog("prune = "+fp2str(m_prune)+"\n");
    }
}


int Refiner::getNewGrid(int n, const doublereal* z,
                        int nn, doublereal* zn)
{
    int j;
    int nnew = static_cast<int>(m_loc.size());
    if (nnew + n > nn) {
        throw CanteraError("Refine::getNewGrid",
                           "array size too small.");
        return -1;
    }

    int jn = 0;
    if (m_loc.empty()) {
        copy(z, z + n,  zn);
        return 0;
    }

    for (j = 0; j < n - 1; j++) {
        zn[jn] = z[j];
        jn++;
        if (has_key(m_loc, j)) {
            zn[jn] = 0.5*(z[j] + z[j+1]);
            jn++;
        }
    }
    zn[jn] = z[n-1];
    return 0;
}
}
