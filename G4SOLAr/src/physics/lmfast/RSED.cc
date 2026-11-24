#include "RSED.hh"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>


namespace
{
    constexpr double FOUR_PI =
        12.5663706143591729538505735331180115367885;

    constexpr double PI =
        3.141592653589793238462643383279502884;

    /*
     * FFTW's plan interface uses int dimensions.
     */
    int checkedFFTWDimension(std::size_t n)
    {
        if (n == 0 ||
            n > static_cast<std::size_t>(
                    std::numeric_limits<int>::max()))
        {
            throw std::invalid_argument(
                "RSED: FFT dimension is invalid or too large.");
        }

        return static_cast<int>(n);
    }
}


// ============================================================================
// Constructor / destructor
// ============================================================================

RSED::RSED(double xb, double yb, double zb,
           double dx, double dy, double dz,
           double xo, double yo, double zo,
           double muRS, double muAbs)
    :
      xb_(xb),
      yb_(yb),
      zb_(zb),

      dx_(dx),
      dy_(dy),
      dz_(dz),

      xo_(xo),
      yo_(yo),
      zo_(zo),

      muRS_(muRS),
      muAbs_(muAbs),
      muTot_(muRS + muAbs),

      nx_(0),
      ny_(0),
      nz_(0),

      pnx_(0),
      pny_(0),
      pnz_(0),

      cnz_(0),

      physicalSize_(0),
      paddedSize_(0),
      complexSize_(0),

      density()
{
    if (xb_ <= 0.0 || yb_ <= 0.0 || zb_ <= 0.0)
        throw std::invalid_argument(
            "RSED: box dimensions must be positive.");

    if (dx_ <= 0.0 || dy_ <= 0.0 || dz_ <= 0.0)
        throw std::invalid_argument(
            "RSED: grid spacings must be positive.");

    if (muRS_ < 0.0 || muAbs_ < 0.0)
        throw std::invalid_argument(
            "RSED: attenuation coefficients must be non-negative.");

    if (muTot_ <= 0.0)
        throw std::invalid_argument(
            "RSED: muRS + muAbs must be positive.");

    if (xo_ < 0.0 || xo_ > xb_ ||
        yo_ < 0.0 || yo_ > yb_ ||
        zo_ < 0.0 || zo_ > zb_)
    {
        throw std::invalid_argument(
            "RSED: source position is outside the box.");
    }

    initializeStorage();
}


// ============================================================================

RSED::~RSED()
{
    releaseFFTResources();
}


// ============================================================================
// FFTW resource management
// ============================================================================

void RSED::releaseFFTResources()
{
    if (kernelForwardPlan_)
    {
        fftw_destroy_plan(kernelForwardPlan_);
        kernelForwardPlan_ = nullptr;
    }

    if (densityForwardPlan_)
    {
        fftw_destroy_plan(densityForwardPlan_);
        densityForwardPlan_ = nullptr;
    }

    if (densityBackwardPlan_)
    {
        fftw_destroy_plan(densityBackwardPlan_);
        densityBackwardPlan_ = nullptr;
    }

    if (kernelReal_)
    {
        fftw_free(kernelReal_);
        kernelReal_ = nullptr;
    }

    if (densityReal_)
    {
        fftw_free(densityReal_);
        densityReal_ = nullptr;
    }

    if (kernelSpectrum_)
    {
        fftw_free(kernelSpectrum_);
        kernelSpectrum_ = nullptr;
    }

    if (densitySpectrum_)
    {
        fftw_free(densitySpectrum_);
        densitySpectrum_ = nullptr;
    }
}


// ============================================================================

void RSED::initializeStorage()
{
    /*
     * The physical grid is:
     *
     *     nx_ x ny_ x nz_
     *
     * and the FFT grid is twice as large in every direction:
     *
     *     pnx_ x pny_ x pnz_
     *
     * The voxel centers are:
     *
     *     x_i = (i + 1/2) dx
     *
     * etc.
     */

    const double nxReal = xb_ / dx_;
    const double nyReal = yb_ / dy_;
    const double nzReal = zb_ / dz_;

    const double tol = 1.0e-10;

    if (std::abs(nxReal - std::round(nxReal)) > tol ||
        std::abs(nyReal - std::round(nyReal)) > tol ||
        std::abs(nzReal - std::round(nzReal)) > tol)
    {
        throw std::invalid_argument(
            "RSED: box dimensions must be integer multiples "
            "of the grid spacing.");
    }

    nx_ = static_cast<std::size_t>(std::llround(nxReal));
    ny_ = static_cast<std::size_t>(std::llround(nyReal));
    nz_ = static_cast<std::size_t>(std::llround(nzReal));

    if (nx_ == 0 || ny_ == 0 || nz_ == 0)
        throw std::invalid_argument(
            "RSED: physical grid contains zero cells.");

    pnx_ = 2 * nx_;
    pny_ = 2 * ny_;
    pnz_ = 2 * nz_;

    cnz_ = pnz_ / 2 + 1;

    physicalSize_ =
        nx_ * ny_ * nz_;

    paddedSize_ =
        pnx_ * pny_ * pnz_;

    complexSize_ =
        pnx_ * pny_ * cnz_;

    /*
     * The final density lives only on the physical grid.
     */
    density =
        RealTensor(
            static_cast<Eigen::Index>(nx_),
            static_cast<Eigen::Index>(ny_),
            static_cast<Eigen::Index>(nz_)
        );

    density.setZero();

    /*
     * Allocate FFTW buffers using fftw_malloc.
     *
     * This gives FFTW the alignment it wants for SIMD-optimized
     * transforms.
     */
    kernelReal_ =
        fftw_alloc_real(paddedSize_);

    densityReal_ =
        fftw_alloc_real(paddedSize_);

    kernelSpectrum_ =
        fftw_alloc_complex(complexSize_);

    densitySpectrum_ =
        fftw_alloc_complex(complexSize_);

    if (!kernelReal_ ||
        !densityReal_ ||
        !kernelSpectrum_ ||
        !densitySpectrum_)
    {
        releaseFFTResources();

        throw std::bad_alloc();
    }

    buildPlans();
}


// ============================================================================

void RSED::buildPlans()
{
    const int nx = checkedFFTWDimension(pnx_);
    const int ny = checkedFFTWDimension(pny_);
    const int nz = checkedFFTWDimension(pnz_);

    /*
     * Out-of-place R2C transform:
     *
     *     real:    pnx x pny x pnz
     *     complex: pnx x pny x (pnz/2+1)
     *
     * FFTW uses the last dimension as the fastest-changing
     * dimension in its C row-major layout.
     */
    kernelForwardPlan_ =
        fftw_plan_dft_r2c_3d(
            nx,
            ny,
            nz,
            kernelReal_,
            kernelSpectrum_,
            FFTW_MEASURE
        );

    densityForwardPlan_ =
        fftw_plan_dft_r2c_3d(
            nx,
            ny,
            nz,
            densityReal_,
            densitySpectrum_,
            FFTW_MEASURE
        );

    densityBackwardPlan_ =
        fftw_plan_dft_c2r_3d(
            nx,
            ny,
            nz,
            densitySpectrum_,
            densityReal_,
            FFTW_MEASURE
        );

    if (!kernelForwardPlan_ ||
        !densityForwardPlan_ ||
        !densityBackwardPlan_)
    {
        releaseFFTResources();

        throw std::runtime_error(
            "RSED: FFTW failed to create an FFT plan.");
    }
}


// ============================================================================
// Kernel
// ============================================================================

double RSED::kernelValue(double r) const noexcept
{
    if (r <= 0.0)
        return 0.0;

    return
        (muRS_ / (FOUR_PI * r * r)) *
        std::exp(-muTot_ * r);
}


// ============================================================================

double RSED::centralVoxelKernel() const noexcept
{
    /*
     * The kernel diverges as 1/r^2 at r=0, but the singularity
     * is integrable.
     *
     * We approximate the central rectangular voxel by a sphere
     * having the same volume.
     *
     *     V = dx * dy * dz
     *     V = 4*pi*R^3/3
     *
     * The integrated kernel is then
     *
     *     integral K dV
     *
     *       = muRS * integral exp(-muTot*r) dr
     *
     *       = muRS/muTot *
     *         (1 - exp(-muTot*R)).
     *
     * The result already includes the voxel-volume integration,
     * so unlike ordinary cells this value is NOT multiplied
     * by dx*dy*dz again.
     */

    const double volume =
        dx_ * dy_ * dz_;

    const double radius =
        std::cbrt(
            3.0 * volume / FOUR_PI
        );

    return
        (muRS_ / muTot_) *
        (1.0 - std::exp(-muTot_ * radius));
}


// ============================================================================

void RSED::buildKernel()
{
    /*
     * Construct the displacement kernel directly in the FFT
     * array layout.
     *
     * For index i:
     *
     *     i <= pnx/2 : positive displacement
     *     i >  pnx/2 : negative displacement
     *
     * Thus FFT index 0 is r=0, while the negative part of
     * the kernel is stored at the end of the array.
     *
     * This is the correct periodic embedding for a linear
     * convolution of the physical box.
     */

    auto kernel =
        RealMap(
            kernelReal_,
            static_cast<Eigen::Index>(pnx_),
            static_cast<Eigen::Index>(pny_),
            static_cast<Eigen::Index>(pnz_)
        );

    kernel.setZero();

    const double dV =
        dx_ * dy_ * dz_;

    for (std::size_t i = 0; i < pnx_; ++i)
    {
        const long di =
            (i <= pnx_ / 2)
                ? static_cast<long>(i)
                : static_cast<long>(i) -
                  static_cast<long>(pnx_);

        const double x = static_cast<double>(di) * dx_;

        for (std::size_t j = 0; j < pny_; ++j)
        {
            const long dj =
                (j <= pny_ / 2)
                    ? static_cast<long>(j)
                    : static_cast<long>(j) -
                      static_cast<long>(pny_);

            const double y = static_cast<double>(dj) * dy_;

            for (std::size_t k = 0; k < pnz_; ++k)
            {
                const long dk =
                    (k <= pnz_ / 2)
                        ? static_cast<long>(k)
                        : static_cast<long>(k) -
                          static_cast<long>(pnz_);

                const double z =
                    static_cast<double>(dk) * dz_;

                const double r =
                    std::sqrt(
                        x * x +
                        y * y +
                        z * z
                    );

                if (r == 0.0)
                {
                    /*
                     * Integrated central voxel.
                     */
                    kernel(
                        static_cast<Eigen::Index>(i),
                        static_cast<Eigen::Index>(j),
                        static_cast<Eigen::Index>(k)
                    ) = centralVoxelKernel();
                }
                else
                {
                    /*
                     * Point quadrature for all other cells.
                     *
                     * The multiplication by dV converts the
                     * continuous kernel density into the discrete
                     * convolution coefficient.
                     */
                    kernel(
                        static_cast<Eigen::Index>(i),
                        static_cast<Eigen::Index>(j),
                        static_cast<Eigen::Index>(k)
                    ) =
                        kernelValue(r) * dV;
                }
            }
        }
    }

    /*
     * FFT the kernel once. Its spectrum remains fixed throughout
     * the entire multiple-scattering calculation.
     */
    fftw_execute(kernelForwardPlan_);
}


// ============================================================================
// Initial density
// ============================================================================

void RSED::initializeDensity()
{
    /*
     * The first scattering density is the kernel centered at
     * the source position:
     *
     *     rho_1(r) = K(r - r_source)
     *
     * restricted to the physical box.
     *
     * Everything outside the physical box is left at zero.
     */

    auto rho =
        RealMap(
            densityReal_,
            static_cast<Eigen::Index>(pnx_),
            static_cast<Eigen::Index>(pny_),
            static_cast<Eigen::Index>(pnz_)
        );

    rho.setZero();

    for (std::size_t i = 0; i < nx_; ++i)
    {
        /*
         * Cell-centered grid.
         */
        const double x =
            (static_cast<double>(i) + 0.5) * dx_;

        const double rx = x - xo_;

        for (std::size_t j = 0; j < ny_; ++j)
        {
            const double y =
                (static_cast<double>(j) + 0.5) * dy_;

            const double ry = y - yo_;

            for (std::size_t k = 0; k < nz_; ++k)
            {
                const double z =
                    (static_cast<double>(k) + 0.5) * dz_;

                const double rz = z - zo_;

                const double r =
                    std::sqrt(
                        rx * rx +
                        ry * ry +
                        rz * rz
                    );

                double value;

                if (r == 0.0)
                {
                    /*
                     * The source lies exactly at a voxel center.
                     * Use the same integrated central-cell
                     * approximation as for the convolution kernel.
                     */
                    value = centralVoxelKernel();
                }
                else
                {
                    /*
                     * rho is a density, so here we retain the
                     * continuous kernel density rather than the
                     * integrated kernel coefficient.
                     */
                    value = kernelValue(r);
                }

                rho(
                    static_cast<Eigen::Index>(i),
                    static_cast<Eigen::Index>(j),
                    static_cast<Eigen::Index>(k)
                ) = value;
            }
        }
    }
}


// ============================================================================
// FFT operations
// ============================================================================

void RSED::fftForward()
{
    fftw_execute(densityForwardPlan_);
}


// ============================================================================

void RSED::convolve()
{
    /*
     * FFT convolution:
     *
     *     rho_hat <- rho_hat * kernel_hat
     *
     * The TensorMaps are zero-copy views over the FFTW arrays.
     */

    auto rhoHat =
        ComplexMap(
            reinterpret_cast<Complex*>(densitySpectrum_),
            static_cast<Eigen::Index>(pnx_),
            static_cast<Eigen::Index>(pny_),
            static_cast<Eigen::Index>(cnz_)
        );

    auto kernelHat =
        ComplexMap(
            reinterpret_cast<Complex*>(kernelSpectrum_),
            static_cast<Eigen::Index>(pnx_),
            static_cast<Eigen::Index>(pny_),
            static_cast<Eigen::Index>(cnz_)
        );

    /*
     * Element-wise tensor multiplication.
     *
     * FFTW stores the complex values as pairs of doubles.
     * FFTW documents std::complex<double> interoperability
     * through reinterpret_cast for C++ implementations with
     * the corresponding binary representation.
     */
    rhoHat =
        rhoHat * kernelHat;
}


// ============================================================================

void RSED::fftReverse()
{
    fftw_execute(densityBackwardPlan_);

    /*
     * FFTW computes an unnormalized inverse transform.
     *
     * For an FFT grid
     *
     *     pnx x pny x pnz
     *
     * the result must therefore be divided by
     *
     *     pnx * pny * pnz.
     */
    const double normalization =
        1.0 /
        static_cast<double>(paddedSize_);

    auto rho =
        RealMap(
            densityReal_,
            static_cast<Eigen::Index>(pnx_),
            static_cast<Eigen::Index>(pny_),
            static_cast<Eigen::Index>(pnz_)
        );

    rho = rho * normalization;
}


// ============================================================================
// Padding
// ============================================================================

void RSED::zeroPad()
{
    /*
     * The physical box occupies:
     *
     *     0 <= i < nx_
     *     0 <= j < ny_
     *     0 <= k < nz_
     *
     * The remainder of the doubled FFT grid represents the
     * absorbing exterior and must be zero before the next FFT.
     */

    auto rho =
        RealMap(
            densityReal_,
            static_cast<Eigen::Index>(pnx_),
            static_cast<Eigen::Index>(pny_),
            static_cast<Eigen::Index>(pnz_)
        );

    for (std::size_t i = 0; i < pnx_; ++i)
    {
        for (std::size_t j = 0; j < pny_; ++j)
        {
            for (std::size_t k = 0; k < pnz_; ++k)
            {
                if (i >= nx_ ||
                    j >= ny_ ||
                    k >= nz_)
                {
                    rho(
                        static_cast<Eigen::Index>(i),
                        static_cast<Eigen::Index>(j),
                        static_cast<Eigen::Index>(k)
                    ) = 0.0;
                }
            }
        }
    }
}


// ============================================================================
// Accumulation
// ============================================================================

void RSED::accumulate()
{
    auto rho =
        RealMap(
            densityReal_,
            static_cast<Eigen::Index>(pnx_),
            static_cast<Eigen::Index>(pny_),
            static_cast<Eigen::Index>(pnz_)
        );

    for (std::size_t i = 0; i < nx_; ++i)
    {
        for (std::size_t j = 0; j < ny_; ++j)
        {
            for (std::size_t k = 0; k < nz_; ++k)
            {
                density(
                    static_cast<Eigen::Index>(i),
                    static_cast<Eigen::Index>(j),
                    static_cast<Eigen::Index>(k)
                ) +=
                    rho(
                        static_cast<Eigen::Index>(i),
                        static_cast<Eigen::Index>(j),
                        static_cast<Eigen::Index>(k)
                    );
            }
        }
    }
}


// ============================================================================
// Main iteration
// ============================================================================

void RSED::computeRSEDgrid(
    std::size_t maxIterations,
    double tolerance)
{
    if (maxIterations == 0)
        throw std::invalid_argument(
            "RSED: maxIterations must be greater than zero.");

    if (tolerance <= 0.0)
        throw std::invalid_argument(
            "RSED: tolerance must be positive.");

    iterations_ = 0;

    // print configuration
    // std::cout << "\nComputing RSED with: \n";

    //     std::cout << "\tbox size     (x,y,z) = ( " << xb_ << ", " << yb_ << ", " << zb_ << " )\n";
    //     std::cout << "\tnode spacing (x,y,z) = ( " << dx_ << ", " << dy_ << ", " << dz_ << " )\n";    
    //     std::cout << "\tlight origin (x,y,z) = ( " << xo_ << ", " << yo_ << ", " << zo_ << " )\n";

    //     std::cout << "\tmuRS = " << muRS_ << ", muAbs = " << muAbs_ << ", muTot = " << muTot_ << " )\n";

    /*
     * Build the fixed scattering kernel.
     */
    buildKernel();

    /*
     * Create the first-scattering density.
     */
    initializeDensity();

    /*
     * The final density starts with rho_1.
     */
    density.setZero();
    accumulate();

    /*
     * Keep track of the total amount of the current iteration.
     * This is used for convergence.
     */
    double previousContribution = 0.0;

    {
        auto rho =
            RealMap(
                densityReal_,
                static_cast<Eigen::Index>(pnx_),
                static_cast<Eigen::Index>(pny_),
                static_cast<Eigen::Index>(pnz_)
            );

        previousContribution = 0.0;

        for (std::size_t i = 0; i < nx_; ++i)
            for (std::size_t j = 0; j < ny_; ++j)
                for (std::size_t k = 0; k < nz_; ++k)
                    previousContribution +=
                        rho(
                            static_cast<Eigen::Index>(i),
                            static_cast<Eigen::Index>(j),
                            static_cast<Eigen::Index>(k)
                        );
    }

    /*
     * rho currently contains rho_1.
     *
     * Each loop generates:
     *
     *     rho_(n+1) = K * rho_n
     */
    for (std::size_t iteration = 2;
         iteration <= maxIterations;
         ++iteration)
    {
        fftForward();

        convolve();

        fftReverse();

        /*
         * The C2R result now contains the new scattering order
         * throughout the doubled FFT domain.
         *
         * Keep only the physical box. This is the absorbing-wall
         * operation.
         */
        zeroPad();

        /*
         * Measure the contribution before accumulating it.
         */
        double contribution = 0.0;

        {
            auto rho =
                RealMap(
                    densityReal_,
                    static_cast<Eigen::Index>(pnx_),
                    static_cast<Eigen::Index>(pny_),
                    static_cast<Eigen::Index>(pnz_)
                );

            for (std::size_t i = 0; i < nx_; ++i)
                for (std::size_t j = 0; j < ny_; ++j)
                    for (std::size_t k = 0; k < nz_; ++k)
                    {
                        contribution +=
                            rho(
                                static_cast<Eigen::Index>(i),
                                static_cast<Eigen::Index>(j),
                                static_cast<Eigen::Index>(k)
                            );
                    }
        }

        /*
         * Add rho_(n) to the accumulated result.
         */
        accumulate();

        iterations_ = iteration;

        /*
         * Convergence is based on the relative contribution of
         * the newly generated scattering order.
         */
        const double denominator =
            std::max(
                std::abs(previousContribution),
                std::numeric_limits<double>::min()
            );

        const double relativeContribution =
            std::abs(contribution) / denominator;

        previousContribution = contribution;

        if (relativeContribution < tolerance)
            break;
    }
}


// ============================================================================
// Geometry helpers
// ============================================================================

bool RSED::insidePhysicalBox(
    std::size_t i,
    std::size_t j,
    std::size_t k
) const noexcept
{
    return
        i < nx_ &&
        j < ny_ &&
        k < nz_;
}


// ============================================================================

std::size_t RSED::physicalIndex(
    std::size_t i,
    std::size_t j,
    std::size_t k
) const noexcept
{
    /*
     * Row-major index:
     *
     *     ((i * Ny) + j) * Nz + k
     */
    return
        (i * ny_ + j) * nz_ + k;
}


// ============================================================================
// Interpolation
// ============================================================================

// // trilinear
// double RSED::GetRSEDat(
//     double x,
//     double y,
//     double z
// ) const
// {
//     if (x < 0.0 || x > xb_ ||
//         y < 0.0 || y > yb_ ||
//         z < 0.0 || z > zb_)
//     {
//         throw std::out_of_range(
//             "RSED::GetRSEDat: point is outside the physical box.");
//     }

//     /*
//      * Grid nodes are voxel centers:
//      *
//      *     x_i = (i + 1/2) dx
//      *
//      * We perform trilinear interpolation between neighboring
//      * cell centers.
//      *
//      * At the physical boundary there is no neighboring cell
//      * outside the box, so the coordinate is clamped to the
//      * nearest center.
//      */

//     auto coordinate =
//         [](double x,
//            double d,
//            std::size_t n,
//            std::size_t& i0,
//            std::size_t& i1,
//            double& t)
//         {
//             const double u =
//                 x / d - 0.5;

//             if (u <= 0.0)
//             {
//                 i0 = 0;
//                 i1 = 0;
//                 t = 0.0;
//                 return;
//             }

//             const double maxIndex =
//                 static_cast<double>(n - 1);

//             if (u >= maxIndex)
//             {
//                 i0 = n - 1;
//                 i1 = n - 1;
//                 t = 0.0;
//                 return;
//             }

//             i0 =
//                 static_cast<std::size_t>(
//                     std::floor(u)
//                 );

//             i1 = i0 + 1;

//             t =
//                 u - static_cast<double>(i0);
//         };

//     std::size_t i0, i1;
//     std::size_t j0, j1;
//     std::size_t k0, k1;

//     double tx, ty, tz;

//     coordinate(x, dx_, nx_, i0, i1, tx);
//     coordinate(y, dy_, ny_, j0, j1, ty);
//     coordinate(z, dz_, nz_, k0, k1, tz);

//     const double c000 =
//         density(
//             static_cast<Eigen::Index>(i0),
//             static_cast<Eigen::Index>(j0),
//             static_cast<Eigen::Index>(k0)
//         );

//     const double c100 =
//         density(
//             static_cast<Eigen::Index>(i1),
//             static_cast<Eigen::Index>(j0),
//             static_cast<Eigen::Index>(k0)
//         );

//     const double c010 =
//         density(
//             static_cast<Eigen::Index>(i0),
//             static_cast<Eigen::Index>(j1),
//             static_cast<Eigen::Index>(k0)
//         );

//     const double c110 =
//         density(
//             static_cast<Eigen::Index>(i1),
//             static_cast<Eigen::Index>(j1),
//             static_cast<Eigen::Index>(k0)
//         );

//     const double c001 =
//         density(
//             static_cast<Eigen::Index>(i0),
//             static_cast<Eigen::Index>(j0),
//             static_cast<Eigen::Index>(k1)
//         );

//     const double c101 =
//         density(
//             static_cast<Eigen::Index>(i1),
//             static_cast<Eigen::Index>(j0),
//             static_cast<Eigen::Index>(k1)
//         );

//     const double c011 =
//         density(
//             static_cast<Eigen::Index>(i0),
//             static_cast<Eigen::Index>(j1),
//             static_cast<Eigen::Index>(k1)
//         );

//     const double c111 =
//         density(
//             static_cast<Eigen::Index>(i1),
//             static_cast<Eigen::Index>(j1),
//             static_cast<Eigen::Index>(k1)
//         );

//     const double c00 =
//         c000 * (1.0 - tx) +
//         c100 * tx;

//     const double c10 =
//         c010 * (1.0 - tx) +
//         c110 * tx;

//     const double c01 =
//         c001 * (1.0 - tx) +
//         c101 * tx;

//     const double c11 =
//         c011 * (1.0 - tx) +
//         c111 * tx;

//     const double c0 =
//         c00 * (1.0 - ty) +
//         c10 * ty;

//     const double c1 =
//         c01 * (1.0 - ty) +
//         c11 * ty;

//     return
//         c0 * (1.0 - tz) +
//         c1 * tz;
// }

// tricubic
double RSED::GetRSEDat(
    double x,
    double y,
    double z
) const
{
    if (x < 0.0 || x > xb_ ||
        y < 0.0 || y > yb_ ||
        z < 0.0 || z > zb_)
    {
        throw std::out_of_range(
            "RSED::GetRSEDat: point is outside the physical box.");
    }

    /*
     * Grid centers are located at
     *
     *     x_i = (i + 1/2) dx
     *
     *     y_j = (j + 1/2) dy
     *
     *     z_k = (k + 1/2) dz
     *
     * Convert the physical coordinate into grid-index coordinates.
     *
     *     u = x/dx - 1/2
     *
     * so that u = 0 corresponds to the first grid center.
     */

    const double ux = x / dx_ - 0.5;
    const double uy = y / dy_ - 0.5;
    const double uz = z / dz_ - 0.5;


    /*
     * Catmull-Rom cubic interpolation.
     *
     * Given four consecutive values:
     *
     *     p0 p1 p2 p3
     *
     * and t in [0,1], interpolate between p1 and p2.
     */
    auto cubic = [](
        double p0,
        double p1,
        double p2,
        double p3,
        double t
    )
    {
        return 0.5 *
        (
            2.0 * p1
            + (-p0 + p2) * t
            + (2.0*p0 - 5.0*p1 + 4.0*p2 - p3) * t * t
            + (-p0 + 3.0*p1 - 3.0*p2 + p3) * t * t * t
        );
    };


    /*
     * Return the four grid indices surrounding a coordinate.
     *
     * For an interior point:
     *
     *       i-1   i   i+1   i+2
     *        |    |    |     |
     *             <--->
     *              t
     *
     * The interpolation itself is between i and i+1.
     *
     * At boundaries we clamp the stencil indices. This avoids
     * accessing cells outside the physical grid.
     */
    auto stencil = [](
        double u,
        std::size_t n,
        std::size_t& i0,
        std::size_t& i1,
        std::size_t& i2,
        std::size_t& i3,
        double& t
    )
    {
        if (n < 2)
        {
            i0 = i1 = i2 = i3 = 0;
            t = 0.0;
            return;
        }

        /*
         * The requested point lies between floor(u) and
         * floor(u)+1.
         */
        const long i =
            static_cast<long>(std::floor(u));

        t =
            u - static_cast<double>(i);

        /*
         * Four-point stencil:
         *
         *     i-1, i, i+1, i+2
         */
        long l0 = i - 1;
        long l1 = i;
        long l2 = i + 1;
        long l3 = i + 2;

        /*
         * Clamp to the available grid.
         */
        const long maxIndex =
            static_cast<long>(n) - 1;

        l0 = std::max(0L, std::min(l0, maxIndex));
        l1 = std::max(0L, std::min(l1, maxIndex));
        l2 = std::max(0L, std::min(l2, maxIndex));
        l3 = std::max(0L, std::min(l3, maxIndex));

        i0 = static_cast<std::size_t>(l0);
        i1 = static_cast<std::size_t>(l1);
        i2 = static_cast<std::size_t>(l2);
        i3 = static_cast<std::size_t>(l3);
    };


    std::size_t ix0, ix1, ix2, ix3;
    std::size_t iy0, iy1, iy2, iy3;
    std::size_t iz0, iz1, iz2, iz3;

    double tx, ty, tz;

    stencil(
        ux, nx_,
        ix0, ix1, ix2, ix3,
        tx
    );

    stencil(
        uy, ny_,
        iy0, iy1, iy2, iy3,
        ty
    );

    stencil(
        uz, nz_,
        iz0, iz1, iz2, iz3,
        tz
    );


    /*
     * First interpolate in x.
     *
     * For each y/z location we obtain one interpolated
     * value in x.
     */
    double xy[4][4];

    const std::size_t xs[4] =
    {
        ix0, ix1, ix2, ix3
    };

    const std::size_t ys[4] =
    {
        iy0, iy1, iy2, iy3
    };

    const std::size_t zs[4] =
    {
        iz0, iz1, iz2, iz3
    };


    for (int j = 0; j < 4; ++j)
    {
        for (int k = 0; k < 4; ++k)
        {
            const double p0 =
                density(
                    static_cast<Eigen::Index>(xs[0]),
                    static_cast<Eigen::Index>(ys[j]),
                    static_cast<Eigen::Index>(zs[k])
                );

            const double p1 =
                density(
                    static_cast<Eigen::Index>(xs[1]),
                    static_cast<Eigen::Index>(ys[j]),
                    static_cast<Eigen::Index>(zs[k])
                );

            const double p2 =
                density(
                    static_cast<Eigen::Index>(xs[2]),
                    static_cast<Eigen::Index>(ys[j]),
                    static_cast<Eigen::Index>(zs[k])
                );

            const double p3 =
                density(
                    static_cast<Eigen::Index>(xs[3]),
                    static_cast<Eigen::Index>(ys[j]),
                    static_cast<Eigen::Index>(zs[k])
                );

            xy[j][k] =
                cubic(
                    p0, p1, p2, p3,
                    tx
                );
        }
    }


    /*
     * Then interpolate the four x-interpolated values
     * in y, for each z.
     */
    double xyz[4];

    for (int k = 0; k < 4; ++k)
    {
        xyz[k] =
            cubic(
                xy[0][k],
                xy[1][k],
                xy[2][k],
                xy[3][k],
                ty
            );
    }


    /*
     * Finally interpolate in z.
     */
    return cubic(
        xyz[0],
        xyz[1],
        xyz[2],
        xyz[3],
        tz
    );
}



// ============================================================================
// Setters
// ============================================================================

void RSED::SetBoundaries(
    double x,
    double y,
    double z)
{
    if (x <= 0.0 || y <= 0.0 || z <= 0.0)
        throw std::invalid_argument(
            "RSED::SetBoundaries: dimensions must be positive.");

    xb_ = x;
    yb_ = y;
    zb_ = z;

    /*
     * Changing the physical dimensions changes the FFT grid,
     * so all FFTW storage and plans must be rebuilt.
     */
    releaseFFTResources();
    initializeStorage();
}


// ============================================================================

void RSED::SetOrigin(
    double x,
    double y,
    double z)
{
    if (x < 0.0 || x > xb_ ||
        y < 0.0 || y > yb_ ||
        z < 0.0 || z > zb_)
    {
        throw std::invalid_argument(
            "RSED::SetOrigin: source position is outside the box.");
    }

    xo_ = x;
    yo_ = y;
    zo_ = z;
}


// ============================================================================

void RSED::SetGridSpacing(
    double x,
    double y,
    double z)
{
    if (x <= 0.0 || y <= 0.0 || z <= 0.0)
        throw std::invalid_argument(
            "RSED::SetGridSpacing: spacings must be positive.");

    dx_ = x;
    dy_ = y;
    dz_ = z;

    /*
     * Grid dimensions and therefore FFT dimensions have changed.
     */
    releaseFFTResources();
    initializeStorage();
}


// ============================================================================

void RSED::SetCoefficients(
    double muRS,
    double muAbs)
{
    if (muRS < 0.0 || muAbs < 0.0)
        throw std::invalid_argument(
            "RSED::SetCoefficients: coefficients must be non-negative.");

    if (muRS + muAbs <= 0.0)
        throw std::invalid_argument(
            "RSED::SetCoefficients: muRS + muAbs must be positive.");

    muRS_ = muRS;
    muAbs_ = muAbs;
    muTot_ = muRS_ + muAbs_;
}