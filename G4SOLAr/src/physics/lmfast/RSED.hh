#ifndef RSED_HH
#define RSED_HH

#include <fftw3.h>
#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

#include <cstddef>
#include <complex>
// #include <vector>

// // Iteration control.
// std::size_t maxIterations = 1000;
// double tolerance = 1.0e-10;

class RSED{

  public:

    using Real    = double;
    using Complex = std::complex<double>;

    using RealTensor    = Eigen::Tensor<Real, 3, Eigen::RowMajor>;
    using ComplexTensor = Eigen::Tensor<Complex, 3, Eigen::RowMajor>;

    using RealMap    = Eigen::TensorMap<RealTensor>;
    using ComplexMap = Eigen::TensorMap<ComplexTensor>;


  public:

    RSED( double xb, double yb, double zb,
          double dx, double dy, double dz,
          double xo, double yo, double zo,
          double muRS, double muAbs
        );  // initialize all standard 

    ~RSED();

    RSED(const RSED&) = delete;
    RSED& operator=(const RSED&) = delete;


    const RealTensor& GetRSEDgrid() const noexcept {return density;}
    double            GetRSEDat(double x, double y, double z) const;

    std::size_t GetNx() const noexcept { return nx_; }
    std::size_t GetNy() const noexcept { return ny_; }
    std::size_t GetNz() const noexcept { return nz_; }

    std::size_t GetIterations() const noexcept {return iterations_;}

    void SetBoundaries(   double x, double y, double z);
    void SetOrigin(       double x, double y, double z);
    void SetGridSpacing(  double x, double y, double z);
    void SetCoefficients( double muRS, double muAbs   );

    void computeRSEDgrid( std::size_t maxIterations = 50,
                          double tolerance = 1.0e-10);


  private:

    void initializeStorage();
    void releaseFFTResources();
    void buildPlans();

    void buildKernel(); // compute values and fft - just once
    void initializeDensity(); // place kernel in the box

    // cycle iteration
    void zeroPad(); // might be done better depending on use
    void fftForward();
    void convolve();
    void fftReverse();  // check where best to normalize
    void accumulate();

    bool insidePhysicalBox(
        std::size_t i,
        std::size_t j,
        std::size_t k
    ) const noexcept;

    double kernelValue(double r) const noexcept;
    double centralVoxelKernel() const noexcept;

    std::size_t physicalIndex(
        std::size_t i,
        std::size_t j,
        std::size_t k
    ) const noexcept;


    double xb_;
    double yb_;
    double zb_;
      double dx_;
      double dy_;
      double dz_;

    double xo_;
    double yo_;
    double zo_;

    double muRS_, muAbs_, muTot_;

    std::size_t nx_;
    std::size_t ny_;
    std::size_t nz_;

    std::size_t pnx_;
    std::size_t pny_;
    std::size_t pnz_;

    // Number of stored complex values after r2c.
    int cnz_;

    std::size_t physicalSize_;
    std::size_t paddedSize_;
    std::size_t complexSize_;

    // FFTW real buffers.
    double* kernelReal_{nullptr};
    double* densityReal_{nullptr};

    // FFTW complex buffers.
    fftw_complex* kernelSpectrum_{nullptr};
    fftw_complex* densitySpectrum_{nullptr};

    // FFTW plans.
    fftw_plan kernelForwardPlan_{nullptr};
    fftw_plan densityForwardPlan_{nullptr};
    fftw_plan densityBackwardPlan_{nullptr};

    // Eigen views of FFTW memory.
    // Eigen::TensorMap<Eigen::Tensor<double, 3, Eigen::RowMajor>> kernelRealMap_;
    // Eigen::TensorMap<Eigen::Tensor<double, 3, Eigen::RowMajor>> densityRealMap_;

    // Eigen::TensorMap<Eigen::Tensor<std::complex<double>, 3, Eigen::RowMajor>> kernelSpectrumMap_;
    // Eigen::TensorMap<Eigen::Tensor<std::complex<double>, 3, Eigen::RowMajor>> densitySpectrumMap_;

    // Physical density from the previous iteration.
    // Eigen::ArrayXd previousPhysical_;
    // Eigen::ArrayXd density;

    //   std::size_t iterations = 0;

    RealTensor density;

    std::size_t iterations_{0};

};

#endif