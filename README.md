![Developed by atmc Badge](https://img.shields.io/badge/atmc-We%20made%20this!-%23303030?labelColor=%23c80000)
[![Build Status](https://img.shields.io/github/actions/workflow/status/YellowChemistryPublishing/Firework.Runtime/cmake.yml?branch=main&logo=github)](https://github.com/YellowChemistryPublishing/Firework.Runtime/actions/workflows/cmake.yml)

# Firework.Runtime

A a blazing fast<sup>\[citation needed\]</sup>, lightweight game and application development library that is intuitive and easy to use, whilst also being performant and efficient.

This library is _very_ in-development right now, and definitely should not be used beyond experimentation. (Some commits might not even build!)

## Acknowledgements

-   Truong, N., Yuksel, C. and Seiler, L. (2020). Quadratic Approximation of Cubic Curves. Proceedings of the ACM on Computer Graphics and Interactive Techniques, [online] 3(2),
    pp.1–17. doi:https://doi.org/10.1145/3406178.

    For the algorithm used to approximate cubic bezier curves as pairs of quadratic ones.

-   MDN Web Docs. (n.d.). SVG: Scalable Vector Graphics. [online] Available at: https://developer.mozilla.org/en-US/docs/Web/SVG.

    For providing a comprehensive overview of the SVG specifications, used in parsing `.svg` XML files.

-   Archive.org. (2025). Chapter 14 - OpenGL Programming Guide. [online] Available at:
    https://web.archive.org/web/20240118160026/http://www.glprogramming.com/red/chapter14.html#name13 [Accessed 20 Jul. 2025].

    For providing the algorithm to render arbitrary, concave polygons defined by `n` points in `O(n)` time. This algorithm has been extended in this library to support curved edges
    defined by quadratic beziers.
