#include <omp.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <stdlib.h>
#include <string>

using namespace std;

// Compute initial conditions for canonical Taylor-Green vortex problem
void setInitialConditions(float *p, float *u, float *v, float *w,
                        int ni, int nj, int nk, int kstart,
                        int iskip, int jskip, float L) {
    const int kskip = 1;
    const float l = 1.0;
    const float coef = 1.0;
    for (int i = 0; i < ni; ++i) {
        float dx = (1. / ni) * L;
        float x = 0.5 * dx + (i)*dx - 0.5 * L;
        for (int j = 0; j < nj; ++j) {
            float dy = (1. / nj) * L;
            float y = 0.5 * dy + j * dy - 0.5 * L;
            int offset = kstart + i * iskip + j * jskip;
            for (int k = 0; k < nk; ++k) {
                int indx = offset + k;
                float dz = (1. / nk) * L;
                float z = 0.5 * dz + k * dz - 0.5 * L;
                // 3-D taylor green vortex
                u[indx] = 1. * coef * sin(x / l) * cos(y / l) * cos(z / l);
                v[indx] = -1. * coef * cos(x / l) * sin(y / l) * cos(z / l);
                p[indx] = (1. / 16.) * coef * coef * (cos(2. * x / l) + cos(2. * y / l)) * (cos(2. * z / l) + 2.);
                w[indx] = 0;
            }
        }
    }
}

// Apply periodic boundary conditions at the boundary of the box
void copyPeriodic(float *p, float *u, float *v, float *w,
                  int ni, int nj, int nk, int kstart, int iskip, int jskip) {
    const int kskip = 1;
    // copy the i periodic faces
    for (int j = 0; j < nj; ++j) {
        for (int k = 0; k < nk; ++k) {
            int indx = kstart + j * jskip + k * kskip;
            p[indx - iskip] = p[indx + (ni - 1) * iskip];
            p[indx - 2 * iskip] = p[indx + (ni - 2) * iskip];
            p[indx + ni * iskip] = p[indx];
            p[indx + (ni + 1) * iskip] = p[indx + iskip];

            u[indx - iskip] = u[indx + (ni - 1) * iskip];
            u[indx - 2 * iskip] = u[indx + (ni - 2) * iskip];
            u[indx + ni * iskip] = u[indx];
            u[indx + (ni + 1) * iskip] = u[indx + iskip];

            v[indx - iskip] = v[indx + (ni - 1) * iskip];
            v[indx - 2 * iskip] = v[indx + (ni - 2) * iskip];
            v[indx + ni * iskip] = v[indx];
            v[indx + (ni + 1) * iskip] = v[indx + iskip];

            w[indx - iskip] = w[indx + (ni - 1) * iskip];
            w[indx - 2 * iskip] = w[indx + (ni - 2) * iskip];
            w[indx + ni * iskip] = w[indx];
            w[indx + (ni + 1) * iskip] = w[indx + iskip];
        }
    }
    // copy the j periodic faces
    for (int i = 0; i < ni; ++i)
    {
        int offset = kstart + i * iskip;
        for (int k = 0; k < nk; ++k)
        {
            const int indx = offset + k * kskip;
            p[indx - jskip] = p[indx + (nj - 1) * jskip];
            p[indx - 2 * jskip] = p[indx + (nj - 2) * jskip];
            p[indx + nj * jskip] = p[indx];
            p[indx + (nj + 1) * jskip] = p[indx + jskip];

            u[indx - jskip] = u[indx + (nj - 1) * jskip];
            u[indx - 2 * jskip] = u[indx + (nj - 2) * jskip];
            u[indx + nj * jskip] = u[indx];
            u[indx + (nj + 1) * jskip] = u[indx + jskip];

            v[indx - jskip] = v[indx + (nj - 1) * jskip];
            v[indx - 2 * jskip] = v[indx + (nj - 2) * jskip];
            v[indx + nj * jskip] = v[indx];
            v[indx + (nj + 1) * jskip] = v[indx + jskip];

            w[indx - jskip] = w[indx + (nj - 1) * jskip];
            w[indx - 2 * jskip] = w[indx + (nj - 2) * jskip];
            w[indx + nj * jskip] = w[indx];
            w[indx + (nj + 1) * jskip] = w[indx + jskip];
        }
    }
    // copy the k periodic faces
    for (int i = 0; i < ni; ++i)
    {
        int offset = kstart + i * iskip;
        for (int j = 0; j < nj; ++j)
        {
            const int indx = j * jskip + offset;
            p[indx - kskip] = p[indx + (nk - 1) * kskip];
            p[indx - 2 * kskip] = p[indx + (nk - 2) * kskip];
            p[indx + nk * kskip] = p[indx];
            p[indx + (nk + 1) * kskip] = p[indx + kskip];

            u[indx - kskip] = u[indx + (nk - 1) * kskip];
            u[indx - 2 * kskip] = u[indx + (nk - 2) * kskip];
            u[indx + nk * kskip] = u[indx];
            u[indx + (nk + 1) * kskip] = u[indx + kskip];

            v[indx - kskip] = v[indx + (nk - 1) * kskip];
            v[indx - 2 * kskip] = v[indx + (nk - 2) * kskip];
            v[indx + nk * kskip] = v[indx];
            v[indx + (nk + 1) * kskip] = v[indx + kskip];

            w[indx - kskip] = w[indx + (nk - 1) * kskip];
            w[indx - 2 * kskip] = w[indx + (nk - 2) * kskip];
            w[indx + nk * kskip] = w[indx];
            w[indx + (nk + 1) * kskip] = w[indx + kskip];
        }
    }
}

// Before summing up fluxes, zero out the residual term
void zeroResidual(float *presid, float *uresid, float *vresid, float *wresid,
                  int ni, int nj, int nk, int kstart, int iskip, int jskip)
{
    const int kskip = 1;
    for (int i = -1; i < ni + 1; ++i)
    {
        for (int j = -1; j < nj + 1; ++j)
        {
            int offset = kstart + i * iskip + j * jskip;
            for (int k = -1; k < nk + 1; ++k)
            {
                const int indx = k + offset;
                presid[indx] = 0;
                uresid[indx] = 0;
                vresid[indx] = 0;
                wresid[indx] = 0;
            }
        }
    }
}

// Compute the residue which is represent the computed rate of change for the
// pressure and the three components of the velocity vector denoted (u,v,w)
void computeResidual(float *presid, float *uresid, float *vresid, float *wresid,
                     const float *p,
                     const float *u, const float *v, const float *w,
                     float eta, float nu, float dx, float dy, float dz,
                     int ni, int nj, int nk, int kstart,
                     int iskip, int jskip)
{
    // iskip is 1
    // i dimension goes in the +x coordinate direction
    // j dimension goes in the +y coordinate direction
    // k dimension goes in the +z coordinate direction
    const int kskip = 1;
    // Loop through i faces of the mesh and compute fluxes in x direction
    // Add fluxes to cells that neighbor face
    for (int i = 0; i < ni + 1; ++i)
    {
        const float vcoef = nu / dx;
        const float area = dy * dz;
        for (int j = 0; j < nj; ++j)
        {
            int offset = kstart + i * iskip + j * jskip;
            for (int k = 0; k < nk; ++k)
            {
                const int indx = k + offset;
                // Compute the x direction inviscid flux
                // extract pressures from the stencil
                float ull = u[indx - 2 * iskip];
                float ul = u[indx - iskip];
                float ur = u[indx];
                float urr = u[indx + iskip];

                float vll = v[indx - 2 * iskip];
                float vl = v[indx - iskip];
                float vr = v[indx];
                float vrr = v[indx + iskip];

                float wll = w[indx - 2 * iskip];
                float wl = w[indx - iskip];
                float wr = w[indx];
                float wrr = w[indx + iskip];

                float pll = p[indx - 2 * iskip];
                float pl = p[indx - iskip];
                float pr = p[indx];
                float prr = p[indx + iskip];
                float pterm = (2. / 3.) * (pl + pr) - (1. / 12.) * (pl + pr + pll + prr);
                // x direction so the flux will be a function of u
                float udotn1 = ul + ur;
                float udotn2 = ul + urr;
                float udotn3 = ull + ur;
                float pflux = eta * ((2. / 3.) * udotn1 - (1. / 12.) * (udotn2 + udotn3));
                float uflux = ((1. / 3.) * (ul + ur) * udotn1 -
                               (1. / 24.) * ((ul + urr) * udotn2 + (ull + ur) * udotn3) +
                               pterm);
                float vflux = ((1. / 3.) * (vl + vr) * udotn1 -
                               (1. / 24.) * ((vl + vrr) * udotn2 + (vll + vr) * udotn3));

                float wflux = ((1. / 3.) * (wl + wr) * udotn1 -
                               (1. / 24.) * ((wl + wrr) * udotn2 + (wll + wr) * udotn3));

                // Add in viscous fluxes integrate over face area
                pflux *= area;
                uflux = area * (uflux - vcoef * ((5. / 4.) * (ur - ul) - (1. / 12.) * (urr - ull)));
                vflux = area * (vflux - vcoef * ((5. / 4.) * (vr - vl) - (1. / 12.) * (vrr - vll)));
                wflux = area * (wflux - vcoef * ((5. / 4.) * (wr - wl) - (1. / 12.) * (wrr - wll)));
                presid[indx - iskip] -= pflux;
                presid[indx] += pflux;
                uresid[indx - iskip] -= uflux;
                uresid[indx] += uflux;
                vresid[indx - iskip] -= vflux;
                vresid[indx] += vflux;
                wresid[indx - iskip] -= wflux;
                wresid[indx] += wflux;
            }
        }
    }
    // Loop through j faces of the mesh and compute fluxes in y direction
    // Add fluxes to cells that neighbor face
    for (int i = 0; i < ni; ++i)
    {
        const float vcoef = nu / dy;
        const float area = dx * dz;
        for (int j = 0; j < nj + 1; ++j)
        {
            int offset = kstart + i * iskip + j * jskip;
            for (int k = 0; k < nk; ++k)
            {
                const int indx = k + offset;
                // Compute the y direction inviscid flux
                // extract pressures and velocity from the stencil
                float ull = u[indx - 2 * jskip];
                float ul = u[indx - jskip];
                float ur = u[indx];
                float urr = u[indx + jskip];

                float vll = v[indx - 2 * jskip];
                float vl = v[indx - jskip];
                float vr = v[indx];
                float vrr = v[indx + jskip];

                float wll = w[indx - 2 * jskip];
                float wl = w[indx - jskip];
                float wr = w[indx];
                float wrr = w[indx + jskip];

                float pll = p[indx - 2 * jskip];
                float pl = p[indx - jskip];
                float pr = p[indx];
                float prr = p[indx + jskip];
                float pterm = (2. / 3.) * (pl + pr) - (1. / 12.) * (pl + pr + pll + prr);
                // y direction so the flux will be a function of v
                float udotn1 = vl + vr;
                float udotn2 = vl + vrr;
                float udotn3 = vll + vr;
                float pflux = eta * ((2. / 3.) * udotn1 - (1. / 12.) * (udotn2 + udotn3));
                float uflux = ((1. / 3.) * (ul + ur) * udotn1 -
                               (1. / 24.) * ((ul + urr) * udotn2 + (ull + ur) * udotn3));

                float vflux = ((1. / 3.) * (vl + vr) * udotn1 -
                               (1. / 24.) * ((vl + vrr) * udotn2 + (vll + vr) * udotn3) + pterm);

                float wflux = ((1. / 3.) * (wl + wr) * udotn1 -
                               (1. / 24.) * ((wl + wrr) * udotn2 + (wll + wr) * udotn3));

                // Add in viscous fluxes integrate over face area
                pflux *= area;
                uflux = area * (uflux - vcoef * ((5. / 4.) * (ur - ul) - (1. / 12.) * (urr - ull)));
                vflux = area * (vflux - vcoef * ((5. / 4.) * (vr - vl) - (1. / 12.) * (vrr - vll)));
                wflux = area * (wflux - vcoef * ((5. / 4.) * (wr - wl) - (1. / 12.) * (wrr - wll)));
                presid[indx - jskip] -= pflux;
                presid[indx] += pflux;
                uresid[indx - jskip] -= uflux;
                uresid[indx] += uflux;
                vresid[indx - jskip] -= vflux;
                vresid[indx] += vflux;
                wresid[indx - jskip] -= wflux;
                wresid[indx] += wflux;
            }
        }
    }
    // Loop through k faces of the mesh and compute fluxes in z direction
    // Add fluxes to cells that neighbor face
    for (int i = 0; i < ni; ++i)
    {
        const float vcoef = nu / dz;
        const float area = dx * dy;
        for (int j = 0; j < nj; ++j)
        {
            int offset = kstart + i * iskip + j * jskip;
            for (int k = 0; k < nk + 1; ++k)
            {
                const int indx = k + offset;
                // Compute the y direction inviscid flux
                // extract pressures and velocity from the stencil
                float ull = u[indx - 2 * kskip];
                float ul = u[indx - kskip];
                float ur = u[indx];
                float urr = u[indx + kskip];

                float vll = v[indx - 2 * kskip];
                float vl = v[indx - kskip];
                float vr = v[indx];
                float vrr = v[indx + kskip];

                float wll = w[indx - 2 * kskip];
                float wl = w[indx - kskip];
                float wr = w[indx];
                float wrr = w[indx + kskip];

                float pll = p[indx - 2 * kskip];
                float pl = p[indx - kskip];
                float pr = p[indx];
                float prr = p[indx + kskip];
                float pterm = (2. / 3.) * (pl + pr) - (1. / 12.) * (pl + pr + pll + prr);
                // y direction so the flux will be a function of v
                float udotn1 = wl + wr;
                float udotn2 = wl + wrr;
                float udotn3 = wll + wr;
                float pflux = eta * ((2. / 3.) * udotn1 - (1. / 12.) * (udotn2 + udotn3));
                float uflux = ((1. / 3.) * (ul + ur) * udotn1 -
                               (1. / 24.) * ((ul + urr) * udotn2 + (ull + ur) * udotn3));

                float vflux = ((1. / 3.) * (vl + vr) * udotn1 -
                               (1. / 24.) * ((vl + vrr) * udotn2 + (vll + vr) * udotn3));

                float wflux = ((1. / 3.) * (wl + wr) * udotn1 -
                               (1. / 24.) * ((wl + wrr) * udotn2 + (wll + wr) * udotn3) + pterm);

                // Add in viscous fluxes integrate over face area
                pflux *= area;
                uflux = area * (uflux - vcoef * ((5. / 4.) * (ur - ul) - (1. / 12.) * (urr - ull)));
                vflux = area * (vflux - vcoef * ((5. / 4.) * (vr - vl) - (1. / 12.) * (vrr - vll)));
                wflux = area * (wflux - vcoef * ((5. / 4.) * (wr - wl) - (1. / 12.) * (wrr - wll)));
                presid[indx - kskip] -= pflux;
                presid[indx] += pflux;
                uresid[indx - kskip] -= uflux;
                uresid[indx] += uflux;
                vresid[indx - kskip] -= vflux;
                vresid[indx] += vflux;
                wresid[indx - kskip] -= wflux;
                wresid[indx] += wflux;
            }
        }
    }
}

// Calculate the stable timestep considering inviscid and viscous terms
float computeStableTimestep(const float *u, const float *v, const float *w,
                            float cfl, float eta, float nu,
                            float dx, float dy, float dz,
                            int ni, int nj, int nk, int kstart,
                            int iskip, int jskip)
{
    const int kskip = 1;
    float minDt = 1e30;
    for (int i = 0; i < ni; ++i)
    {
        for (int j = 0; j < nj; ++j)
        {
            int offset = kstart + i * iskip + j * jskip;
            for (int k = 0; k < nk; ++k)
            {
                const int indx = k + offset;
                // inviscid timestep
                const float maxu2 = max(u[indx] * u[indx], max(v[indx] * v[indx], w[indx] * w[indx]));
                const float af = sqrt(maxu2 + eta);
                const float maxev = sqrt(maxu2) + af;
                const float sum = maxev * (1. / dx + 1. / dy + 1. / dz);
                minDt = min(minDt, cfl / sum);
                // viscous stable timestep
                const float dist = min(dx, min(dy, dz));
                minDt = min<float>(minDt, 0.2 * cfl * dist * dist / nu);
            }
        }
    }
    return minDt;
}

// Compute the fluid kinetic energy contained within the simulation domain
float integrateKineticEnergy(const float *u, const float *v, const float *w,
                             float dx, float dy, float dz,
                             int ni, int nj, int nk, int kstart,
                             int iskip, int jskip)
{
    const int kskip = 1;
    double vol = dx * dy * dz;
    double sum = 0;
    for (int i = 0; i < ni; ++i)
    {
        for (int j = 0; j < nj; ++j)
        {
            int offset = kstart + i * iskip + j * jskip;
            for (int k = 0; k < nk; ++k)
            {
                const int indx = k + offset;
                const float udotu = u[indx] * u[indx] + v[indx] * v[indx] + w[indx] * w[indx];
                sum += 0.5 * vol * udotu;
            }
        }
    }
    return sum;
}

// Perform a weighted sum of three arrays
// Note, the last weight is used for the input array (no aliasing)
void weightedSum3(float *uout, float w1, const float *u1, float w2,
                  const float *u2, float w3,
                  int ni, int nj, int nk, int kstart,
                  int iskip, int jskip)
{
    const int kskip = 1;
    for (int i = 0; i < ni; ++i)
    {
        for (int j = 0; j < nj; ++j)
        {
            int offset = kstart + i * iskip + j * jskip;
            for (int k = 0; k < nk; ++k)
            {
                const int indx = k + offset;
                uout[indx] = w1 * u1[indx] + w2 * u2[indx] + w3 * uout[indx];
            }
        }
    }
}

int main(int ac, char *av[])
{
    // Default Simulation Parameters
    // Dimensions of the simulation mesh
    int ni = 16;
    int nj = 16;
    int nk = 16;
    // Length of the cube
    float L = 6.28318530718;
    // fluid viscosity
    float nu = 0.000625;
    // Reference velocity used for artificial compressibility apprach
    float refVel = 10;
    // Simulation stopping time
    float stopTime = 20;
    // Coefficient used to compute stable timestep
    float cflmax = 1.9;

    string outfile = "fke.dat";
    // parse command line arguments
    while (ac >= 2 && av[1][0] == '-')
    {
        if (ac >= 3 && !strcmp(av[1], "-n"))
        {
            ni = atoi(av[2]);
            nj = ni;
            nk = ni;
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-ni"))
        {
            ni = atoi(av[2]);
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-nj"))
        {
            nj = atoi(av[2]);
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-nk"))
        {
            nk = atoi(av[2]);
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-L"))
        {
            L = atof(av[2]);
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-nu"))
        {
            nu = atof(av[2]);
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-refVel"))
        {
            refVel = atof(av[2]);
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-stopTime"))
        {
            stopTime = atof(av[2]);
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-cflmax"))
        {
            cflmax = atof(av[2]);
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-outfile"))
        {
            outfile = av[2];
            av += 2;
            ac -= 2;
        }
        else if (ac >= 3 && !strcmp(av[1], "-o"))
        {
            outfile = av[2];
            av += 2;
            ac -= 2;
        }
        else
        {
            cerr << "unknown command line argument '" << av[1] << "'" << endl;
            av += 1;
            ac -= 1;
            exit(-1);
        }
    }
    // File to save the fluid kinetic energy history
    ofstream ke_file(outfile.c_str(), ios::trunc);

    int num_threads = 1;
#ifdef _OPENMP
#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        if (thread_id == 0)
            num_threads = omp_get_num_threads();
    }
    cout << "Running OpenMP with num threads = " << num_threads << endl;
    double start = omp_get_wtime();
#endif

    // Eta is a artificial compressibility parameter to the numerical scheme
    float eta = refVel * refVel;

    // The mesh cell sizes
    float dx = L / ni;
    float dy = L / nj;
    float dz = L / nk;

    //  Allocate a 3-D mesh with enough space ghost cells two layers thick on each
    //  side of the mesh.
    int allocsize = (ni + 4) * (nj + 4) * (nk + 4);
    cout << "allocating " << ((allocsize * 4 * 3 * sizeof(float)) >> 10) << " k bytes for fluid computation" << endl;
    // Fluid pressure and velocity
    vector<float> p(allocsize);
    vector<float> u(allocsize);
    vector<float> v(allocsize);
    vector<float> w(allocsize);
    // scratch space used to estimate the next timestep values in
    // time integration
    vector<float> pnext(allocsize);
    vector<float> unext(allocsize);
    vector<float> vnext(allocsize);
    vector<float> wnext(allocsize);
    // scratch space to store residual
    vector<float> presid(allocsize);
    vector<float> uresid(allocsize);
    vector<float> vresid(allocsize);
    vector<float> wresid(allocsize);
    int iskip = (nk + 4) * (nj + 4);
    int jskip = (nk + 4);
    int kstart = 2 * iskip + 2 * jskip + 2;

    // Setup initial conditions
    setInitialConditions(&p[0], &u[0], &v[0], &w[0],
                         ni, nj, nk, kstart, iskip, jskip, L);

    // Find initial integrated fluid kinetic energy to monitor solution
    float kprev = integrateKineticEnergy(&u[0], &v[0], &w[0], dx, dy, dz,
                                         ni, nj, nk, kstart, iskip, jskip);

    // We use this scaling parameter so we can plot normalized kinetic energy
    float kscale = 1. / kprev;

    // Starting simulation time
    float simTime = 0;
    int iter = 0;

    // begin Runge-Kutta 3rd Order Time Integration
    while (simTime < stopTime)
    {
        // Find the largest timestep that we can take for the numerical scheme
        float dt = computeStableTimestep(&u[0], &v[0], &w[0],
                                         cflmax, eta, nu, dx, dy, dz,
                                         ni, nj, nk, kstart, iskip, jskip);

        // copy data to the ghost cells to implement periodic boundary conditions
        copyPeriodic(&p[0], &u[0], &v[0], &w[0],
                     ni, nj, nk, kstart, iskip, jskip);

        // Zero out the residual function
        zeroResidual(&presid[0], &uresid[0], &vresid[0], &wresid[0],
                     ni, nj, nk, kstart, iskip, jskip);
        // Compute the residual, these will be used to compute the rates of change
        // of pressure and velocity components
        computeResidual(&presid[0], &uresid[0], &vresid[0], &wresid[0],
                        &p[0], &u[0], &v[0], &w[0],
                        eta, nu, dx, dy, dz,
                        ni, nj, nk, kstart, iskip, jskip);

        // First Step of the Runge-Kutta time integration
        // unext = u^n + dt/vol*L(u^n)
        weightedSum3(&pnext[0], 1.0, &p[0], dt / (dx * dy * dz), &presid[0], 0.0,
                     ni, nj, nk, kstart, iskip, jskip);
        weightedSum3(&unext[0], 1.0, &u[0], dt / (dx * dy * dz), &uresid[0], 0.0,
                     ni, nj, nk, kstart, iskip, jskip);
        weightedSum3(&vnext[0], 1.0, &v[0], dt / (dx * dy * dz), &vresid[0], 0.0,
                     ni, nj, nk, kstart, iskip, jskip);
        weightedSum3(&wnext[0], 1.0, &w[0], dt / (dx * dy * dz), &wresid[0], 0.0,
                     ni, nj, nk, kstart, iskip, jskip);

        // Now we are evaluating a residual a second time as part of the
        // third order time integration.  The residual is evaluated using
        // the first estimate of the time integrated solution that is found
        // in next version of the variables computed by the previous
        // wieghtedSum3 calls.

        // Now we are on the second step of the Runge-Kutta time integration
        copyPeriodic(&pnext[0], &unext[0], &vnext[0], &wnext[0],
                     ni, nj, nk, kstart, iskip, jskip);
        zeroResidual(&presid[0], &uresid[0], &vresid[0], &wresid[0],
                     ni, nj, nk, kstart, iskip, jskip);
        computeResidual(&presid[0], &uresid[0], &vresid[0], &wresid[0],
                        &pnext[0], &unext[0], &vnext[0], &wnext[0],
                        eta, nu, dx, dy, dz,
                        ni, nj, nk, kstart, iskip, jskip);

        // Second Step of the Runge-Kutta time integration
        // unext = 3/4 u^n + 1/4 u_next + (1/4)*(dt/vol)*L(unext)
        weightedSum3(&pnext[0], 3. / 4., &p[0], dt / (4. * dx * dy * dz), &presid[0], 1. / 4.,
                     ni, nj, nk, kstart, iskip, jskip);
        weightedSum3(&unext[0], 3. / 4., &u[0], dt / (4. * dx * dy * dz), &uresid[0], 1. / 4.,
                     ni, nj, nk, kstart, iskip, jskip);
        weightedSum3(&vnext[0], 3. / 4., &v[0], dt / (4. * dx * dy * dz), &vresid[0], 1. / 4.,
                     ni, nj, nk, kstart, iskip, jskip);
        weightedSum3(&wnext[0], 3. / 4., &w[0], dt / (4. * dx * dy * dz), &wresid[0], 1. / 4.,
                     ni, nj, nk, kstart, iskip, jskip);

        // Now we are evaluating the final step of the Runge-Kutta time integration
        // so we need to revaluate the residual on the pnext values
        copyPeriodic(&pnext[0], &unext[0], &vnext[0], &wnext[0],
                     ni, nj, nk, kstart, iskip, jskip);

        zeroResidual(&presid[0], &uresid[0], &vresid[0], &wresid[0],
                     ni, nj, nk, kstart, iskip, jskip);
        computeResidual(&presid[0], &uresid[0], &vresid[0], &wresid[0],
                        &pnext[0], &unext[0], &vnext[0], &wnext[0],
                        eta, nu, dx, dy, dz,
                        ni, nj, nk, kstart, iskip, jskip);

        // Third Step of the Runge-Kutta time integration
        // u^{n+1} = 1/3 u^n + 2/3 unext + (2/3)*(dt/vol)*L(unext)
        // Note, here we are writing the result into the previous timestep
        // so that we will be ready to proceed to the next iteration when
        // this step is finished.
        weightedSum3(&p[0], 2. / 3., &pnext[0], 2. * dt / (3. * dx * dy * dz), &presid[0], 1. / 3.,
                     ni, nj, nk, kstart, iskip, jskip);
        weightedSum3(&u[0], 2. / 3., &unext[0], 2. * dt / (3. * dx * dy * dz), &uresid[0], 1. / 3.,
                     ni, nj, nk, kstart, iskip, jskip);
        weightedSum3(&v[0], 2. / 3., &vnext[0], 2. * dt / (3. * dx * dy * dz), &vresid[0], 1. / 3.,
                     ni, nj, nk, kstart, iskip, jskip);
        weightedSum3(&w[0], 2. / 3., &wnext[0], 2. * dt / (3. * dx * dy * dz), &wresid[0], 1. / 3.,
                     ni, nj, nk, kstart, iskip, jskip);

        // Update the simulation time
        simTime += dt;
        iter++;

        // Collect information on the state of kinetic energy in the system
        float knext = integrateKineticEnergy(&u[0], &v[0], &w[0], dx, dy, dz,
                                            ni, nj, nk, kstart, iskip, jskip);
        // write out the data for post processing analysis
        ke_file << simTime << " " << kscale * knext << " " << -kscale * (knext - kprev) / dt << endl;
        // Every 128 iterations report the state so we can observe progress of
        // the simulation
        if ((iter & 0x7f) == 0)
            cout << "ke: " << simTime << ' ' << kscale * knext << endl;
        // keep track of the change in kinetic energy over timesteps so we can plot
        // the derivative of the kinetic energy with time.
        kprev = knext;
    }

    cout << "finished with iter = " << iter << endl;
#ifdef _OPENMP
    double end = omp_get_wtime();
    cerr << "time to solve: " << (end - start) << endl;
#endif
    return 0;
}
