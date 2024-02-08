FSR is a collection of algorithms relating to generating a higher resolution image.
This specific header focuses on single-image non-temporal image scaling, and related tools.

The core functions are EASU and RCAS:
  [EASU] Edge Adaptive Spatial Upsampling ....... 1x to 4x area range spatial scaling, clamped adaptive elliptical filter.
  [RCAS] Robust Contrast Adaptive Sharpening .... A non-scaling variation on CAS.
RCAS needs to be applied after EASU as a separate pass.

Optional utility functions are:
  [LFGA] Linear Film Grain Applicator ........... Tool to apply film grain after scaling.
  [SRTM] Simple Reversible Tone-Mapper .......... Linear HDR {0 to FP16_MAX} to {0 to 1} and back.
  [TEPD] Temporal Energy Preserving Dither ...... Temporally energy preserving dithered {0 to 1} linear to gamma 2.0 conversion.
